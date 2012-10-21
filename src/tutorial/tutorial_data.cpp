//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#include "tutorial/tutorial_data.hpp"

#include <stdexcept>
#include <iostream>

#include "karts/abstract_kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

TutorialData::TutorialData(const std::string& filename)
#ifndef WIN32
    throw(std::runtime_error)
#endif
{
    m_filename    = filename;
    m_major       = RaceManager::MAJOR_MODE_SINGLE;
    m_minor       = RaceManager::MINOR_MODE_NORMAL_RACE;
    m_difficulty  = RaceManager::DIFFICULTY_EASY;
    m_num_laps    = -1;
    m_num_karts   = -1;
    m_time        = -1.0f;
    m_track_name  = "";
    m_gp_id       = "";
    m_energy      = -1;
    
    std::string s_property;
    int         i_property;

    XMLNode     *root = new XMLNode( filename );

    // Check if the file have been load correctly 
    if(!root || root->getName()!="tutorial")
    {
        delete root;
        std::ostringstream msg;
        msg << "Couldn't load tutorial '" << filename << "': no tutorial node.";
        throw std::runtime_error(msg.str());
    }
    // Start the loading process (ordered as it is on the file)

    // ID
    if(!root->get("id", &s_property) ) 
        error("id");
    setId(s_property);

    // Name
    if(!root->get("name", &s_property) ) 
        error("name");    
    setName( _(s_property.c_str()) );

    // Description
    if(!root->get("s_property", &s_property) ) 
        error("description");
    setTutorialDescription( _(s_property.c_str()) );

    // Major    
    root->get("major", &s_property);
    setMajor(s_property);

    // Minor
    root->get("minor", &s_property);
    setMinor(s_property);
    
    // Karts
    if(!root->get("karts", &i_property)  ) 
        error("karts");
    setNumKarts(i_property);
    

    // Difficulty
    root->get("difficulty", &s_property);
    setDifficulty(s_property);

    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        // Track
        if (!root->get("track",  &s_property ))        
            error("track");     
        setTrack(s_property);       
    }

    // Num Players
    root->get("num_players", &i_property);
    setNumPlayers(i_property);
    delete root;

}   // TutorialData

// ----------------------------------------------------------------------------
void TutorialData::error(const char *id) const
{
    std::ostringstream msg;
    msg << "Undefined or incorrect value for '" << id 
        << "' in tutorial file '" << m_filename << "'.";
    
    std::cerr << "TutorialData : " << msg.str() << std::endl;
    
    throw std::runtime_error(msg.str());
}   // error

// ----------------------------------------------------------------------------
/** Checks if this tutorial is valid, i.e. contains a valid track or a valid
 *  GP. If incorrect data are found, STK is aborted with an error message. 
 *  (otherwise STK aborts when trying to do this challenge, which is worse).
 */
void TutorialData::check() const
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


void TutorialData::setRace() const
{
    race_manager->setMajorMode(m_major);
    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        race_manager->setMinorMode(m_minor);
        race_manager->setTrack(m_track_name);
        race_manager->setDifficulty(m_difficulty);
        race_manager->setNumLaps(m_num_laps);
        race_manager->setNumKarts(m_num_karts);
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
        race_manager->setNumLocalPlayers(1);
    }
}   // setRace

// ----------------------------------------------------------------------------
bool TutorialData::raceFinished()
{
    // GP's use the grandPrixFinished() function, so they can't be fulfilled here.
    if(m_major==RaceManager::MAJOR_MODE_GRAND_PRIX) return false;

    // Single races
    // ------------
    World *world = World::getWorld();
    std::string track_name = world->getTrack()->getIdent();
    if(track_name!=m_track_name                      ) return false;    // wrong track
    if((int)world->getNumKarts()<m_num_karts         ) return false;    // not enough AI karts

    AbstractKart* kart = world->getPlayerKart(0);
    if(m_energy>0   && kart->getEnergy()  <m_energy  ) return false;  // not enough energy
    if(m_position>0 && kart->getPosition()>m_position) return false;  // too far behind

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
bool TutorialData::grandPrixFinished()
{
    // Note that we have to call race_manager->getNumKarts, since there
    // is no world objects to query at this stage.
    if (race_manager->getMajorMode()  != RaceManager::MAJOR_MODE_GRAND_PRIX  ||
        race_manager->getMinorMode()  != m_minor                             ||
        race_manager->getGrandPrix()->getId() != m_gp_id                     ||
        race_manager->getDifficulty()!= m_difficulty                         ||
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

void TutorialData::setNumKarts(int num_karts)
{
    this->m_num_karts= num_karts;
}
void TutorialData::setLaps(int num_laps)
{
    this->m_num_laps = num_laps;
}
void TutorialData::setTrack(std::string track_name)
{
    this->m_track_name == track_name;
    if (track_manager->getTrack(m_track_name) == NULL)        
        error("track");
}

void TutorialData::setDifficulty(std::string difficulty)
{
    if(difficulty == "easy")
        this->m_difficulty = RaceManager::DIFFICULTY_EASY;
    else if(difficulty =="medium")
        this->m_difficulty = RaceManager::DIFFICULTY_MEDIUM;
    else if(difficulty =="hard")
        this->m_difficulty = RaceManager::DIFFICULTY_HARD;
    else
        error("difficulty");
}

void TutorialData::setMinor(std::string minor)
{
    if(minor=="timetrial")
        this->m_minor = RaceManager::MINOR_MODE_TIME_TRIAL;
    else if(minor=="quickrace")
        this->m_minor = RaceManager::MINOR_MODE_NORMAL_RACE;
    else if(minor=="followtheleader")
        this->m_minor = RaceManager::MINOR_MODE_FOLLOW_LEADER;
    else
        error("minor");
}

void TutorialData::setMajor(std::string major)
{
    if(major=="grandprix")
        this->m_major = RaceManager::MAJOR_MODE_GRAND_PRIX;
    else if(major=="single")
        this->m_major = RaceManager::MAJOR_MODE_SINGLE;
    else
        error("major");
}

void TutorialData::setPosition(int position)
{
    this->m_position = position;
}

void TutorialData::setTime (float time)
{
    this->m_time = time;
}

void TutorialData::setEnergy(int energy)
{
    m_energy = energy;    
}

void TutorialData::setNumPlayers (int num_players)
{
    this->m_num_players = num_players;
}
