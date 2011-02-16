//  $Id$
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

#ifndef HEADER_CHALLENGE_HPP
#define HEADER_CHALLENGE_HPP

/**
  * \defgroup challenges
  */

#include <string>
#include <vector>
#include <fstream>
#include <irrlicht.h>

#include "utils/no_copy.hpp"

class XMLNode;


/**
  * \brief A class for all challenges
  * \ingroup challenges
  */
class Challenge : public NoCopy
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
    enum {CH_INACTIVE,                 // challenge not yet possible
          CH_ACTIVE,                   // challenge possible, but not yet solved
          CH_SOLVED}         m_state;  // challenge was solved
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
             Challenge(const std::string &id, const std::string &name);
             Challenge() : m_id(""), m_name(""), m_state(CH_INACTIVE)
             { }
    virtual ~Challenge() {};
    void  addUnlockTrackReward(const std::string &track_name);
    void  addUnlockModeReward(const std::string &internal_mode_name, 
                              const irr::core::stringw &user_mode_name);
    void  addUnlockGPReward(const std::string &gp_name);
    void  addUnlockDifficultyReward(const std::string &internal_name, 
                                    const irr::core::stringw &user_name);
    void  addUnlockKartReward(const std::string &internal_name,
                              const irr::core::stringw &user_name);
    void  load(const XMLNode* config);
    void  save(std::ofstream& writer);
    // These functions are meant for customisation, e.g. load/save
    // additional state information specific to the challenge
    virtual void loadAdditionalInfo(const XMLNode* config) {};
    virtual void saveAdditionalInfo(std::ofstream& writer)     {};
    // ------------------------------------------------------------------------
    /** Returns the id of the challenge. */
    const std::string &getId() const              { return m_id;              }
    // ------------------------------------------------------------------------
    /** Returns the name of the challenge. */
    const irr::core::stringw &getName() const     { return m_name;            }
    // ------------------------------------------------------------------------
    /** Sets the name of the challenge. */
    void  setName(const irr::core::stringw & s)   { m_name = s;               }
    // ------------------------------------------------------------------------
    /** Sets the id of this challenge. */
    void  setId(const std::string& s)             { m_id = s;                 }
    // ------------------------------------------------------------------------
    const std::vector<UnlockableFeature>&
          getFeatures() const                    { return m_feature;          }
    // ------------------------------------------------------------------------    
    void  setChallengeDescription(const irr::core::stringw& d) 
                                                 {m_challenge_description=d;  }
    // ------------------------------------------------------------------------
    const irr::core::stringw& 
          getChallengeDescription() const    {return m_challenge_description; }
    // ------------------------------------------------------------------------
    void  addDependency(const std::string id)  {m_prerequisites.push_back(id);}
    // ------------------------------------------------------------------------
    bool  isSolved() const                       {return m_state==CH_SOLVED;  }
    // ------------------------------------------------------------------------
    bool  isActive() const                       {return m_state==CH_ACTIVE;  }
    // ------------------------------------------------------------------------
    void  setSolved()                            {m_state = CH_SOLVED;        }
    // ------------------------------------------------------------------------
    void  setActive()                            {m_state = CH_ACTIVE;        }
    // ------------------------------------------------------------------------
    const std::vector<std::string>& 
          getPrerequisites() const               {return m_prerequisites;     }
    // ------------------------------------------------------------------------
    /** These functions are called when a race is finished. It allows
     *  the challenge to unlock features (when returning true), otherwise
     *  the feature remains locked. */
    virtual bool raceFinished()                  {return false;               }
    // ------------------------------------------------------------------------
    /** These functions are called when a grand prix is finished. It allows
     *  the challenge to unlock features (when returning true), otherwise
     *  the feature remains locked. */
    virtual bool grandPrixFinished()             {return false;               }
    // ------------------------------------------------------------------------    
    /** Sets the right parameters in RaceManager to try this challenge */
    virtual void setRace() const = 0;
    // ------------------------------------------------------------------------
    /** Checks if a challenge is valid. */
    virtual void check() const = 0;
};
#endif
