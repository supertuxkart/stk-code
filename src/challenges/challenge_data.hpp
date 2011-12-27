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
    RaceManager::Difficulty        m_difficulty;
    int                            m_num_laps;
    int                            m_position;
    int                            m_num_karts;
    float                          m_time;
    std::string                    m_gp_id;
    std::string                    m_track_name;
    int                            m_energy;
    std::string                    m_filename;
    /** Version number of the challenge. */
    int                            m_version;

    void getUnlocks(const XMLNode *root, const std::string &type, 
                    ChallengeData::REWARD_TYPE reward);
    void error(const char *id) const;

    /** Short, internal name for this challenge. */
    std::string              m_id;
    /** Name used in menu for this challenge. */
    irr::core::stringw       m_name; 
    /** Message the user gets when the feature is not yet unlocked. */
    irr::core::stringw       m_challenge_description;
    /** Features to unlock. */
    std::vector<UnlockableFeature> m_feature;
    /** What needs to be done before accessing this challenge. */
    std::vector<std::string> m_prerequisites;

    
public:
#ifdef WIN32
                 ChallengeData(const std::string& filename);
#else
                 ChallengeData(const std::string& filename) throw(std::runtime_error);
#endif
    
    virtual      ~ChallengeData() {}
    
    /** sets the right parameters in RaceManager to try this challenge */
    void         setRace() const;
    
    virtual void check() const;
    virtual bool raceFinished();
    virtual bool grandPrixFinished();
    /** Returns the version number of this challenge. */
    int          getVersion() const { return m_version; }
    

    const std::vector<UnlockableFeature>&
        getFeatures() const                    { return m_feature;          }

    void  setChallengeDescription(const irr::core::stringw& d) 
        {m_challenge_description=d;  }

    const irr::core::stringw getChallengeDescription() const 
        {return _(m_challenge_description.c_str()); }

    void  addDependency(const std::string id)  {m_prerequisites.push_back(id);}
    
    const std::vector<std::string>& 
        getPrerequisites() const                  { return m_prerequisites;   }
    
    /** Returns the id of the challenge. */
    const std::string &getId() const              { return m_id;              }

    /** Returns the name of the challenge. */
    const irr::core::stringw getName() const
    { return irr::core::stringw(_(m_name.c_str())); }

    /** Sets the name of the challenge. */
    void  setName(const irr::core::stringw & s)   { m_name = s;               }

    /** Sets the id of this challenge. */
    void  setId(const std::string& s)             { m_id = s;                 }
    
    const std::string& getTrackName() const        { return m_track_name;     }

    
    void  addUnlockTrackReward(const std::string &track_name);
    void  addUnlockModeReward(const std::string &internal_mode_name, 
                              const irr::core::stringw &user_mode_name);
    void  addUnlockGPReward(const std::string &gp_name);
    void  addUnlockDifficultyReward(const std::string &internal_name, 
                                    const irr::core::stringw &user_name);
    void  addUnlockKartReward(const std::string &internal_name,
                              const irr::core::stringw &user_name);
    
};   // ChallengeData

#endif   // HEADER_CHALLENGE_DATA_HPP
