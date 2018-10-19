//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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
#include "challenges/story_mode_status.hpp"

#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"

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

    /* The challenges who don't have a race, only unlockables */
    AllChallengesType             m_list_challenges;

    void readAllChallengesInDirs(const std::vector<std::string>* all_dirs);

public:
               UnlockManager     ();
              ~UnlockManager     ();
    void       addOrFreeChallenge(ChallengeData *c);
    void       addListChallenge(ChallengeData *c);
    void       addChallenge      (const std::string& filename);

    const ChallengeData *getChallengeData(const std::string& id);

    bool       isSupportedVersion(const ChallengeData &challenge);

    /** Eye- (or rather ear-) candy. Play a sound when user tries to access a locked area */
    void       playLockSound() const;

    void       findWhatWasUnlocked(int pointsBefore, int pointsNow,
                                   std::vector<std::string>& tracks,
                                   std::vector<std::string>& gps,
                                   std::vector<std::string>& karts,
                                   std::vector<const ChallengeData*>& unlocked);
    bool       unlockByPoints(int points, ChallengeStatus* unlock_list);
    bool       unlockSpecial(ChallengeStatus* unlock_list, int max_req_in_lower_diff);

    StoryModeStatus *createStoryModeStatus(const XMLNode *node=NULL);

};   // UnlockManager

extern UnlockManager* unlock_manager;
#endif
