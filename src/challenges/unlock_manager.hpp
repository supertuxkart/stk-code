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

#ifndef HEADER_UNLOCK_MANAGER_HPP
#define HEADER_UNLOCK_MANAGER_HPP

#include <map>

#include "challenges/challenge_data.hpp"
#include "utils/no_copy.hpp"

#include <fstream>

class XMLNode;
class SFXBase;

/**
  * \brief main class to handle locking/challenges
  * \ingroup challenges
  */
class UnlockManager : public NoCopy
{
private:
    SFXBase    *m_locked_sound;
    typedef std::map<std::string, ChallengeData*> AllChallengesType;
    AllChallengesType             m_all_challenges;
    std::map<std::string, bool>   m_locked_features;
    std::vector<const ChallengeData*> m_unlocked_features;
    void       computeActive     ();
    void       load              ();
    
    void       unlockFeature     (ChallengeData* c, bool do_save=true);
    void readAllChallengesInDirs(const std::vector<std::string>* all_dirs);
    
public:
               UnlockManager     ();
              ~UnlockManager     ();
    void       addOrFreeChallenge(ChallengeData *c);
    void       addChallenge      (const std::string& filename);
    void       save              ();
    std::vector<const ChallengeData*> 
               getActiveChallenges();
    
    /** Returns the list of recently unlocked features (e.g. call at the end of a
        race to know if any features were unlocked) */
    const std::vector<const ChallengeData*> 
               getRecentlyUnlockedFeatures() {return m_unlocked_features;}

    /** Clear the list of recently unlocked challenges */
    void       clearUnlocked     () {m_unlocked_features.clear(); }
    
    /** Returns a complete list of all solved challenges */
    const std::vector<const ChallengeData*>   getUnlockedFeatures();

    /** Returns the list of currently inaccessible (locked) challenges */
    const std::vector<const ChallengeData*>   getLockedChallenges();
    
    const ChallengeData *getChallenge      (const std::string& id);

    void       raceFinished      ();
    void       grandPrixFinished ();
    void       lockFeature       (const ChallengeData *challenge);
    bool       isLocked          (const std::string& feature);
    bool       isSupportedVersion(const ChallengeData &challenge);

    /** Eye- (or rather ear-) candy. Play a sound when user tries to access a locked area */
    void       playLockSound() const;
    
};   // UnlockManager

extern UnlockManager* unlock_manager;
#endif
