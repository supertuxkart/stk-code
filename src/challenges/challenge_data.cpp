//  $Id: challenge_data.cpp 2173 2008-07-21 01:55:41Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include "challenges/challenge_data.hpp"

#include <stdexcept>
#include <sstream>

#include "karts/kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

ChallengeData::ChallengeData(const std::string& filename)
#ifndef WIN32
                                                      throw(std::runtime_error)
#endif
{
    m_filename    = filename;
    m_major       = RaceManager::MAJOR_MODE_SINGLE;
    m_minor       = RaceManager::MINOR_MODE_NORMAL_RACE;
    m_difficulty  = RaceManager::RD_EASY;
    m_num_laps    = -1;
    m_num_karts   = -1;

    m_time        = -1.0f;
    m_track_name  = "";
    m_gp_id       = "";
    m_energy      = -1;

    XMLNode *root = new XMLNode( filename );
//    if(!root || root->getName()!="challenges")
    if(!root || root->getName()!="challenge")
    {
        delete root;
        std::ostringstream msg;
        msg << "Couldn't load challenge '" << filename << "': no challenge node.";
        throw std::runtime_error(msg.str());
    }

    std::string mode;
    root->get("major", &mode);

    if(mode=="grandprix")
        m_major = RaceManager::MAJOR_MODE_GRAND_PRIX;
    else if(mode=="single")
        m_major = RaceManager::MAJOR_MODE_SINGLE;
    else
        error("major");

    root->get("minor", &mode);
    if(mode=="timetrial")
        m_minor = RaceManager::MINOR_MODE_TIME_TRIAL;
    else if(mode=="quickrace")
        m_minor = RaceManager::MINOR_MODE_NORMAL_RACE;
    else if(mode=="followtheleader")
        m_minor = RaceManager::MINOR_MODE_FOLLOW_LEADER;
    else
        error("minor");

    std::string s;
    if(!root->get("name", &s) ) error("name");
    //std::cout << "    // Challenge name = <" << s.c_str() << ">\n";
    setName( _(s.c_str()) );

    if(!root->get("id", &s) ) error("id");
    setId(s);

    if(!root->get("description", &s) ) error("description");
    setChallengeDescription( _(s.c_str()) );
    //std::cout << "    // Challenge description = <" << s.c_str() << ">\n";

    if(!root->get("karts", &m_num_karts)  ) error("karts");

    // Position is optional except in GP and FTL
    if(!root->get("position", &m_position) &&
       //RaceManager::getWorld()->areKartsOrdered() ) // FIXME - order and optional are not the same thing
        (m_minor==RaceManager::MINOR_MODE_FOLLOW_LEADER ||
         m_major==RaceManager::MAJOR_MODE_GRAND_PRIX))
                                           error("position");
    root->get("difficulty", &s);
    if(s=="easy")
        m_difficulty = RaceManager::RD_EASY;
    else if(s=="medium")
        m_difficulty = RaceManager::RD_MEDIUM;
    else if(s=="hard")
        m_difficulty = RaceManager::RD_HARD;
    else
        error("difficulty");

    root->get("time", &m_time );  // one of time/position

    root->get("position", &m_position );  // must be set
    if(m_time<0 && m_position<0) error("position/time");

    root->get("energy", &m_energy ); // This is optional
    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        if (!root->get("track",  &m_track_name ))
        {
            error("track");
        }
        if (track_manager->getTrack(m_track_name) == NULL)
        {
            error("track");
        }
        
        if (!root->get("laps",   &m_num_laps   ) && m_minor!=RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
           error("laps");
        }
    }
    else   // GP
    {
        if (!root->get("gp",   &m_gp_id ))                     error("gp");
        if (grand_prix_manager->getGrandPrix(m_gp_id) == NULL) error("gp");
    }

    getUnlocks(root, "unlock-track",      UNLOCK_TRACK);
    getUnlocks(root, "unlock-gp",         UNLOCK_GP   );
    getUnlocks(root, "unlock-mode",       UNLOCK_MODE );
    getUnlocks(root, "unlock-difficulty", UNLOCK_DIFFICULTY);
    getUnlocks(root, "unlock-kart",       UNLOCK_KART);

    if (getFeatures().size() == 0)
    {
        error("missing unlocked features");
    }
    
    std::vector< std::string > deps;
    root->get("depend-on", &deps);
    for (unsigned int i=0; i<deps.size(); i++)
    {
        if (deps[i].size() > 0) addDependency(deps[i]);
    }
    delete root;

}   // ChallengeData

// ----------------------------------------------------------------------------
void ChallengeData::error(const char *id) const
{
    std::ostringstream msg;
    msg << "Undefined or incorrect value for '" << id 
        << "' in challenge file '" << m_filename << "'.";
    
    std::cerr << "ChallengeData : " << msg.str() << std::endl;
    
    throw std::runtime_error(msg.str());
}   // error

// ----------------------------------------------------------------------------
/** Checks if this challenge is valid, i.e. contains a valid track or a valid
 *  GP. If incorrect data are found, STK is aborted with an error message. 
 *  (otherwise STK aborts when trying to do this challenge, which is worse).
 */
void ChallengeData::check() const
{
    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        try
        {
            track_manager->getTrack(m_track_name);
        }
        catch(std::exception&)
        {
            error("track");
        }
    }
    else if(m_major==RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(m_gp_id);
        
        if (gp == NULL)
        {
            error("gp");
        }
        const bool gp_ok = gp->checkConsistency(false);
        if (!gp_ok)
        {
            error("gp");
        }
    }
}   // check

// ----------------------------------------------------------------------------

void ChallengeData::getUnlocks(const XMLNode *root, const std:: string type,
                               REWARD_TYPE reward)
{
    std:: string attrib;
    root->get(type, &attrib);
    
    if (attrib . empty()) return;

    //std:: vector< std:: string > data;
    //std:: size_t space = attrib.find_first_of(' ');
    //data.push_back( attrib.substr(0, space) );
    //if( space != std:: string:: npos )
    //{
    //    data.push_back( attrib.substr(space, std:: string:: npos) );
    //}

    switch(reward)
    {
    case UNLOCK_TRACK:      addUnlockTrackReward     (attrib        );
                            break;
            
    case UNLOCK_GP:         addUnlockGPReward        (attrib        );
                            break;
            
    case UNLOCK_MODE:       {
                            const RaceManager::MinorRaceModeType mode =
                                RaceManager::getModeIDFromInternalName(attrib.c_str());
                            addUnlockModeReward      (attrib, RaceManager::getNameOf(mode));
                            break;
                            }
    case UNLOCK_DIFFICULTY:
                            {
                            irr::core::stringw user_name = "?"; //TODO: difficulty names when unlocking
                            addUnlockDifficultyReward(attrib, user_name);
                            break;
                            }
    case UNLOCK_KART:       {
                            const KartProperties* prop = kart_properties_manager->getKart(attrib);
                            if (prop == NULL)
                            {
                                std::cerr << "Challenge refers to kart " << attrib <<
                                             ", which is unknown. Ignoring reward.\n";
                                break;
                            }
                            irr::core::stringw user_name = prop->getName();
                            addUnlockKartReward(attrib, user_name);
                            break;
                            }
    }   // switch
}   // getUnlocks
// ----------------------------------------------------------------------------
void ChallengeData::setRace() const
{
    race_manager->setMajorMode(m_major);
    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        race_manager->setMinorMode(m_minor);
        race_manager->setTrack(m_track_name);
        race_manager->setDifficulty(m_difficulty);
        race_manager->setNumLaps(m_num_laps);
        race_manager->setNumKarts(m_num_karts);
        race_manager->setNumPlayers(1);
        race_manager->setNumLocalPlayers(1);
        race_manager->setCoinTarget(m_energy);
    }
    else   // GP
    {
        race_manager->setMinorMode(m_minor);
        const GrandPrixData *gp = grand_prix_manager->getGrandPrix(m_gp_id);
        race_manager->setGrandPrix(*gp);
        race_manager->setDifficulty(m_difficulty);
        race_manager->setNumKarts(m_num_karts);
        race_manager->setNumPlayers(1);
        race_manager->setNumLocalPlayers(1);
    }
}   // setRace

// ----------------------------------------------------------------------------
bool ChallengeData::raceFinished()
{
    // GP's use the grandPrixFinished() function, so they can't be fulfilled here.
    if(m_major==RaceManager::MAJOR_MODE_GRAND_PRIX) return false;

    // Single races
    // ------------
    World *world = World::getWorld();
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!=m_track_name                         ) return false;
    if((int)world->getNumKarts()<m_num_karts            ) return false;
    Kart* kart = world->getPlayerKart(0);
    if(m_energy>0   && kart->getEnergy()  < m_energy    ) return false;
    if(m_position>0 && kart->getPosition()> m_position  ) return false;
    if(race_manager->getDifficulty()      < m_difficulty) return false;

    // Follow the leader
    // -----------------
    if(m_minor==RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {
        // All possible conditions were already checked, so: must have been successful
        return true;
    }
    // Quickrace / Timetrial
    // ---------------------
    // FIXME - encapsulate this better, each race mode needs to be able to specify
    // its own challenges and deal with them
    LinearWorld* lworld = dynamic_cast<LinearWorld*>(world);
    if(lworld != NULL)
    {
        if(lworld->getLapForKart( kart->getWorldKartId() ) != m_num_laps) return false;         // wrong number of laps
    }
    if(m_time>0.0f && kart->getFinishTime()>m_time) return false;    // too slow
    return true;
}   // raceFinished

// ----------------------------------------------------------------------------
bool ChallengeData::grandPrixFinished()
{
    // Note that we have to call race_manager->getNumKarts, since there
    // is no world objects to query at this stage.
    if (race_manager->getMajorMode()  != RaceManager::MAJOR_MODE_GRAND_PRIX  ||
        race_manager->getMinorMode()  != m_minor                             ||
        race_manager->getGrandPrix()->getId() != m_gp_id                     ||
        race_manager->getDifficulty() < m_difficulty                         ||
        race_manager->getNumberOfKarts() < (unsigned int)m_num_karts         ||
        race_manager->getNumPlayers() > 1) return false;

    // check if the player came first.
    //assert(World::getWorld() != NULL);
    // Kart* kart = World::getWorld()->getPlayerKart(0);
    //const int rank = race_manager->getKartGPRank(kart->getWorldKartId());
    const int rank = race_manager->getLocalPlayerGPRank(0);
    
    //printf("getting rank for %s : %i \n", kart->getName().c_str(), rank );
    if (rank != 0) return false;

    return true;
}   // grandPrixFinished
