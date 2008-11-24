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

#include "track.hpp"
#include "translation.hpp"
#include "grand_prix_data.hpp"
#include "grand_prix_manager.hpp"
#include "karts/kart.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "modes/linear_world.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif


ChallengeData::ChallengeData(const std::string& filename)
{
    m_filename    = filename;
    m_major       = RaceManager::MAJOR_MODE_SINGLE;
    m_minor       = RaceManager::MINOR_MODE_QUICK_RACE;
    m_difficulty  = RaceManager::RD_EASY;
    m_num_laps    = -1;
    m_num_karts   = -1;

    m_time        = -1.0f;
    m_track_name  = "";
    m_gp_id       = "";
    m_energy      = -1;

    lisp::Parser parser;
    const lisp::Lisp* const ROOT = parser.parse(filename);

    const lisp::Lisp* const lisp = ROOT->getLisp("challenge");
    if(!lisp)
    {
        delete ROOT;
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 "Couldn't load challenge '%s': no challenge node.",
                 filename.c_str());
        throw std::runtime_error(msg);
    }

    std::string mode;
    lisp->get("major", mode);

    if(mode=="grandprix")
        m_major = RaceManager::MAJOR_MODE_GRAND_PRIX;
    else if(mode=="single")
        m_major = RaceManager::MAJOR_MODE_SINGLE;
    else
        error("major");
        
    lisp->get("minor", mode);
    if(mode=="timetrial")
        m_minor = RaceManager::MINOR_MODE_TIME_TRIAL;
    else if(mode=="quickrace")
        m_minor = RaceManager::MINOR_MODE_QUICK_RACE;
    else if(mode=="followtheleader")
        m_minor = RaceManager::MINOR_MODE_FOLLOW_LEADER;
    else
        error("minor");
    std::string s;
    if(!lisp->get("name", s)             ) error("name");
    setName(s);
    if(!lisp->get("id", s)               ) error("id");
    setId(s);
    if(!lisp->get("description", s)      ) error("description");
    setChallengeDescription(s); 
    if(!lisp->get("karts", m_num_karts)  ) error("karts");
    // Position is optional except in GP and FTL
    if(!lisp->get("position", m_position) && 
       //RaceManager::getWorld()->areKartsOrdered() ) // FIXME - order and optional are not the same thing
        (m_minor==RaceManager::MINOR_MODE_FOLLOW_LEADER || 
         m_major==RaceManager::MAJOR_MODE_GRAND_PRIX))
                                           error("position");
    lisp->get("difficulty", s);
    if(s=="easy")
        m_difficulty = RaceManager::RD_EASY;
    else if(s=="medium")
        m_difficulty = RaceManager::RD_MEDIUM;
    else if(s=="hard")
        m_difficulty = RaceManager::RD_HARD;
    else
        error("difficulty");

    lisp->get("time",       m_time       );  // one of time/position
    lisp->get("position",   m_position   );  // must be set
    if(m_time<0 && m_position<0) error("position/time");
    lisp->get("energy", m_energy     ); // This is optional
    if(m_major==RaceManager::MAJOR_MODE_SINGLE)
    {
        if(!lisp->get("track",  m_track_name )) error("track");
        if(!lisp->get("laps",   m_num_laps   ) && 
           m_minor!=RaceManager::MINOR_MODE_FOLLOW_LEADER)
           error("laps");        
    }
    else   // GP
    {
        if(!lisp->get("gp",   m_gp_id )) error("gp");
    }
    
    getUnlocks(lisp, "unlock-track",      UNLOCK_TRACK);
    getUnlocks(lisp, "unlock-gp",         UNLOCK_GP   );
    getUnlocks(lisp, "unlock-mode",       UNLOCK_MODE );
    getUnlocks(lisp, "unlock-difficulty", UNLOCK_DIFFICULTY);
    getUnlocks(lisp, "unlock-kart",       UNLOCK_KART);

    std::vector<std::string> vec;
    lisp->getVector("depend-on", vec);
    for(unsigned int i=0; i<vec.size(); i++) addDependency(vec[i]);
    delete ROOT;

}   // ChallengeData

// ----------------------------------------------------------------------------
void ChallengeData::error(const char *id)
{
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Undefined or incorrect value for '%s' in challenge file '%s'.",
             id, m_filename.c_str());
    throw std::runtime_error(msg);
}   // error
// ----------------------------------------------------------------------------


void ChallengeData::getUnlocks(const lisp::Lisp *lisp, const char* type, 
                               REWARD_TYPE reward)
{
    std::vector<std::string> v;
    v.clear();

    lisp->getVector(type, v);
    for(unsigned int i=0; i<v.size(); i++)
    {
        switch(reward)
        {
        case UNLOCK_TRACK:      addUnlockTrackReward     (v[i]        );      break;
        case UNLOCK_GP:         addUnlockGPReward        (v[i]        );      break;
        case UNLOCK_MODE:       if(i+1<v.size())
                                {
                                    addUnlockModeReward  (v[i], v[i+1]); 
                                    i++; break;
                                }
                                else
                                    fprintf(stderr, "Unlock mode name missing.\n");
                                break;
        case UNLOCK_DIFFICULTY: if(i+1<v.size())
                                {
                                    addUnlockDifficultyReward(v[i], v[i+1]); 
                                    i++;
                                }
                                else
                                    fprintf(stderr, "Difficult name missing.\n");
                                break;
        case UNLOCK_KART:       if(i+1<v.size())
                                {
                                    addUnlockKartReward(v[i], v[i+1]); 
                                    i++;
                                }
                                else
                                    fprintf(stderr, "Kart name missing.\n");
                                break;
        }   // switch
    }
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
    }
    else   // GP
    {
        race_manager->setMinorMode(m_minor);
        const GrandPrixData *gp = grand_prix_manager->getGrandPrix(m_gp_id);
        race_manager->setGrandPrix(*gp);
        race_manager->setDifficulty(m_difficulty);
        race_manager->setNumKarts(m_num_karts);
        race_manager->setNumPlayers(1);
        //race_manager->setGrandPrix();
    }
}   // setRace

// ----------------------------------------------------------------------------
bool ChallengeData::raceFinished()
{
    // GP's use the grandPrixFinished() function, so they can't be fulfilled here.
    if(m_major==RaceManager::MAJOR_MODE_GRAND_PRIX) return false;

    // Single races
    // ------------
    std::string track_name = RaceManager::getTrack()->getIdent();
    if(track_name!=m_track_name                    ) return false;    // wrong track
    if((int)race_manager->getNumKarts()<m_num_karts) return false;    // not enough AI karts

    Kart* kart = RaceManager::getPlayerKart(0);
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
    LinearWorld* lworld = dynamic_cast<LinearWorld*>(RaceManager::getWorld());
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
    if (race_manager->getMajorMode()  != RaceManager::MAJOR_MODE_GRAND_PRIX  ||
        race_manager->getMinorMode()  != m_minor                             ||
        race_manager->getGrandPrix()->getId() != m_gp_id                     ||
        race_manager->getDifficulty()!= m_difficulty                         ||
        race_manager->getNumKarts()   < (unsigned int)m_num_karts            ||
        race_manager->getNumPlayers() > 1) return false;

    Kart* kart = RaceManager::getPlayerKart(0);
    if(kart->getPosition()>m_position) return false;
    return true;
}   // grandPrixFinished
