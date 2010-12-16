//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago Varela
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

#ifndef TUTORIAL_H
#define TUTORIAL_H

/**
  * \defgroup tutorial
  */

// External includes
#include <string>
#include <vector>
#include <fstream>
#include <irrlicht.h>
// SuperTuxKart includes
#include "utils/no_copy.hpp"


using namespace std; // To avoid using std:: all the time

class XMLNode;

enum REWARD_TYPE
   {UNLOCK_TRACK,
    UNLOCK_GP,
    UNLOCK_MODE,
    UNLOCK_KART,
    UNLOCK_DIFFICULTY};

struct UnlockableFeature
{
    string name; // internal name
    irr::core::stringw user_name; // not all types of feature have one
    REWARD_TYPE type;

    const irr::core::stringw getUnlockedMessage() const;
};


/**
  * \brief A class for all tutorials
  * \ingroup challenges
  */
class Tutorial : public NoCopy
{
private:
    enum {CH_INACTIVE,                 // challenge not yet possible
          CH_ACTIVE,                   // challenge possible, but not yet solved
          CH_SOLVED}         m_state;  // challenge was solved

    string              m_Id;                    // short, internal name for this tutorial
    irr::core::stringw       m_Name;                  // name used in menu for this tutorial
    irr::core::stringw       m_challenge_description; // Message the user gets when the feature is not yet unlocked
    vector<UnlockableFeature> m_feature;         // Features to unlock
    vector<string> m_prerequisites;         // what needs to be done before accessing this tutorial

public:
    // Constructors + Destructor
             Tutorial(const string &id, const string &name);
             Tutorial() {m_Id=""; m_Name="";m_state=CH_INACTIVE;}
    virtual ~Tutorial() {};

    const string &getId() const              { return m_Id;                  }
    const irr::core::stringw &getName() const     { return m_Name;                }
    void  setName(const irr::core::stringw & s)   { m_Name = s;                   }
    void  setId(const string& s)             { m_Id = s;                     }
    void  addUnlockTrackReward(const string &track_name);
    void  addUnlockModeReward(const string &internal_mode_name,
                              const irr::core::stringw &user_mode_name);
    void  addUnlockGPReward(const string &gp_name);
    void  addUnlockDifficultyReward(const string &internal_name,
                                    const irr::core::stringw &user_name);
    void  addUnlockKartReward(const string &internal_name,
                              const irr::core::stringw &user_name);

    const vector<UnlockableFeature>&
          getFeatures() const                    { return m_feature;             }
    void  setChallengeDescription(const irr::core::stringw& d)
                                                 {m_challenge_description=d;      }
    const irr::core::stringw&
          getChallengeDescription() const        {return m_challenge_description; }
    void  addDependency(const string id)    {m_prerequisites.push_back(id);  }
    bool  isSolved() const                       {return m_state==CH_SOLVED;      }
    bool  isActive() const                       {return m_state==CH_ACTIVE;      }
    void  setSolved()                            {m_state = CH_SOLVED;            }
    void  setActive()                            {m_state = CH_ACTIVE;            }
    const vector<string>&
          getPrerequisites() const               {return m_prerequisites;         }
    void  load(const XMLNode* config);
    void  save(ofstream& writer);

    // These functions are meant for customisation, e.g. load/save
    // additional state information specific to the challenge
    virtual void loadAdditionalInfo(const XMLNode* config) {};
    virtual void saveAdditionalInfo(ofstream& writer)     {};

    // These functions are called when a race/gp is finished. It allows
    // the challenge to unlock features (when returning true), otherwise
    // the feature remains locked.
    virtual bool raceFinished()                      {return false;}   // end of a race
    virtual bool grandPrixFinished()                 {return false;}   // end of a GP

    /** sets the right parameters in RaceManager to try this tutorial */
    virtual void setRace() const = 0;

    /** Checks if a tutorial is valid. */
    virtual void check() const = 0;
};
#endif // TUTORIAL_H
