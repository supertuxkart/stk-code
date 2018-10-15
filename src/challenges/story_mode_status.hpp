//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#ifndef GAME_SLOT_HPP
#define GAME_SLOT_HPP

#include "challenges/challenge_data.hpp"
#include "race/race_manager.hpp"

#include <irrString.h>
using namespace irr;

#include <string>
#include <map>
#include <vector>

class ChallengeData;
class ChallengeStatus;
class UTFWriter;
class XMLNode;

const int CHALLENGE_POINTS[] = { 6, 7, 8, 10 };
const int GP_FACTOR = 3;

/** This class contains the progression through challenges for the story mode.
 *  It maintains a list of all challenges in a mapping of challenge id to
 *  an instance of ChallengeStatus. Each ChallengeStatus stores at which level
 *  a challenge was solved.
 *  This object also keeps track of the overall points a player has.
 * \ingroup challenges
 */

class StoryModeStatus
{
private:
    /** Contains whether each feature of the challenge is locked or unlocked */
    std::map<std::string, bool>   m_locked_features;

    /** Recently unlocked features (they are waiting here
      * until they are shown to the user) */
    std::vector<const ChallengeData*> m_unlocked_features;

    std::map<std::string, ChallengeStatus*> m_challenges_state;

    /** A pointer to the current challenge, or NULL
     *  if no challenge is active. */
    const ChallengeStatus *m_current_challenge;

    int m_points;
    int m_points_before; // used for unlocks
    int m_next_unlock_points;

    /** Set to false after the initial stuff (intro, select kart, etc.) */
    bool m_first_time;

    int m_easy_challenges;
    int m_medium_challenges;
    int m_hard_challenges;
    int m_best_challenges;

public:

     StoryModeStatus(const XMLNode *node=NULL);
    ~StoryModeStatus();

    void       computeActive     (bool first_call=false);
    bool       isLocked          (const std::string& feature);
    void       unlockFeatureByList();
    void       lockFeature       (ChallengeStatus *challenge);
    void       unlockFeature     (ChallengeStatus* c, RaceManager::Difficulty d,
                                  bool do_save=true);
    void       raceFinished      ();
    void       grandPrixFinished ();
    void       save              (UTFWriter &out);
    void       addStatus(ChallengeStatus *cs);
    void       setCurrentChallenge(const std::string &challenge_id);

    // ------------------------------------------------------------------------
    /** Returns the list of recently unlocked features (e.g. call at the end
     *  of a race to know if any features were unlocked) */
    const std::vector<const ChallengeData*>
        getRecentlyCompletedChallenges() {return m_unlocked_features;}
    // ------------------------------------------------------------------------
    /** Clear the list of recently unlocked challenges */
    void       clearUnlocked     () {m_unlocked_features.clear(); }
    // ------------------------------------------------------------------------
    /** Returns the number of completed challenges. */
    int        getNumCompletedChallenges  () const { return (m_easy_challenges + m_medium_challenges +
                                                             m_hard_challenges + m_best_challenges); }
    // ------------------------------------------------------------------------
    /** Returns the number of challenges with the superTux time beaten in a lower difficulty. */
    int        getNumReqMetInLowerDiff  () const;
    // ------------------------------------------------------------------------
    /** Returns the number of points accumulated. */
    int        getPoints          () const { return m_points; }
    // ------------------------------------------------------------------------
    /** Returns the number of points before the previous point increase */
    int        getPointsBefore     () const { return m_points_before; }
    // ------------------------------------------------------------------------
    /** Returns the number of points needed by the next unlockable. 0 if none. */
    int        getNextUnlockPoints () const { return m_next_unlock_points; }
    // ------------------------------------------------------------------------
    /** Returns the number of fulfilled challenges at easy level. */
    int        getNumEasyTrophies  () const { return m_easy_challenges;   }
    // ------------------------------------------------------------------------
    /* Returns the number of fulfilled challenges at medium level. */
    int        getNumMediumTrophies() const { return m_medium_challenges; }
    // ------------------------------------------------------------------------
    /** Returns the number of fulfilled challenges at hard level. */
    int        getNumHardTrophies  () const { return m_hard_challenges;   }
    // ------------------------------------------------------------------------
    /** Returns the number of fulfilled challenges at best level. */
    int        getNumBestTrophies  () const { return m_best_challenges;   }
    // ------------------------------------------------------------------------
    /** Sets if this is the first time the intro is shown. */
    void       setFirstTime(bool ft) { m_first_time = ft;   }
    // ------------------------------------------------------------------------
    /** Returns if this is the first time the intro is shown. */
    bool       isFirstTime() const   { return m_first_time; }
    // ------------------------------------------------------------------------
    const ChallengeStatus *getCurrentChallengeStatus() const
    {
        return m_current_challenge;
    }   // getCurrentChallengeStatus
    // ------------------------------------------------------------------------
    /** Returns a challenge given the challenge id.
     */
    const ChallengeStatus* getChallengeStatus(const std::string& id) const
    {
        std::map<std::string, ChallengeStatus*>::const_iterator it =
            m_challenges_state.find(id);
        assert(it!=m_challenges_state.end());
        return it->second;
    }   // getChallengeStatus
};   // StoryModeStatus

#endif
