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

#ifndef HEADER_CHALLENGE_DATA_HPP
#define HEADER_CHALLENGE_DATA_HPP

#include <string>
#include <vector>
#include <stdio.h>
#include <stdexcept>

#include "challenges/challenge.hpp"
#include "race/race_manager.hpp"

/**
  * \brief the description of one challenge
  * \ingroup challenges
  */
class ChallengeData
{
public:
    enum REWARD_TYPE    {UNLOCK_TRACK,
        UNLOCK_GP,
        UNLOCK_MODE,
        UNLOCK_KART,
        UNLOCK_DIFFICULTY};
    // ------------------------------------------------------------------------
    class UnlockableFeature
    {
    public:
        
        std::string        m_name; // internal name
        irr::core::stringw m_user_name; // not all types of feature have one
        REWARD_TYPE        m_type;
        
        const irr::core::stringw getUnlockedMessage() const;
    };   // UnlockableFeature
    // ------------------------------------------------------------------------
    
private:
    RaceManager::MajorRaceModeType m_major;
    RaceManager::MinorRaceModeType m_minor;
    int                            m_num_laps;
    int                            m_position[RaceManager::DIFFICULTY_COUNT];
    int                            m_num_karts[RaceManager::DIFFICULTY_COUNT];
    float                          m_time[RaceManager::DIFFICULTY_COUNT];
    int                            m_energy[RaceManager::DIFFICULTY_COUNT];
    std::string                    m_gp_id;
    std::string                    m_track_id;
    std::string                    m_filename;
    /** Version number of the challenge. */
    int                            m_version;

    void getUnlocks(const XMLNode *root, const std::string &type, 
                    ChallengeData::REWARD_TYPE reward);
    void error(const char *id) const;

    /** Short, internal name for this challenge. */
    std::string              m_id;

    /** Features to unlock. */
    std::vector<UnlockableFeature> m_feature;

    irr::core::stringw m_challenge_description;
    
public:
#ifdef WIN32
                 ChallengeData(const std::string& filename);
#else
                 ChallengeData(const std::string& filename) throw(std::runtime_error);
#endif
    
    virtual      ~ChallengeData() {}
    
    /** sets the right parameters in RaceManager to try this challenge */
    void         setRace(RaceManager::Difficulty d) const;
    
    virtual void check() const;
    virtual bool raceFinished();
    virtual bool grandPrixFinished();
    /** Returns the version number of this challenge. */
    int          getVersion() const { return m_version; }
    

    const std::vector<UnlockableFeature>&
        getFeatures() const                    { return m_feature;         }
    
    /** Returns the id of the challenge. */
    const std::string &getId() const           { return m_id;              }

    /** Sets the id of this challenge. */
    void  setId(const std::string& s)          { m_id = s;                 }
    
    const std::string& getTrackId() const      { return m_track_id;        }

    int   getNumLaps() const                   { return m_num_laps;        }

    void  addUnlockTrackReward(const std::string &track_name);
    void  addUnlockModeReward(const std::string &internal_mode_name, 
                              const irr::core::stringw &user_mode_name);
    void  addUnlockGPReward(const std::string &gp_name);
    void  addUnlockDifficultyReward(const std::string &internal_name, 
                                    const irr::core::stringw &user_name);
    void  addUnlockKartReward(const std::string &internal_name,
                              const irr::core::stringw &user_name);
    
    
    RaceManager::MajorRaceModeType getMajorMode()  const { return m_major;      }
    RaceManager::MinorRaceModeType getMinorMode()  const { return m_minor;      }

    const irr::core::stringw& getChallengeDescription() const { return m_challenge_description; }
    
    int   getPosition(RaceManager::Difficulty difficulty) const { return m_position[difficulty];   }
    int   getNumKarts(RaceManager::Difficulty difficulty) const { return m_num_karts[difficulty];  }
    float getTime    (RaceManager::Difficulty difficulty) const { return m_time[difficulty];       }
    int   getEnergy  (RaceManager::Difficulty difficulty) const { return m_energy[difficulty];     }
    
};   // Ch

#endif   // HEADER_CHALLENGE_DATA_HPP
