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


#include "challenges/story_mode_status.hpp"

#include "challenges/challenge_status.hpp"
#include "challenges/challenge_data.hpp"
#include "challenges/story_mode_timer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"

//-----------------------------------------------------------------------------
StoryModeStatus::StoryModeStatus(const XMLNode *node)
{
    m_points                  = 0;
    m_points_before           = 0;
    m_next_unlock_points      = 0;
    m_first_time              = true;
    m_easy_challenges         = 0;
    m_medium_challenges       = 0;
    m_hard_challenges         = 0;
    m_best_challenges         = 0;
    m_current_challenge       = NULL;
    m_story_mode_finished     = false;
    m_valid_speedrun_finished = false;
    m_story_mode_milliseconds = 0;
    m_speedrun_milliseconds   = 0;

    // If there is saved data, load it
    if(node)
    {
        node->get("first-time", &m_first_time);

        // If the timer sub-nodes are missing, don't load junk data
        // Disable the timers if story mode has already been started.
        if(!node->get("finished", &m_story_mode_finished))
            m_story_mode_finished     = !m_first_time;
        if(!node->get("speedrun-finished", &m_valid_speedrun_finished))
            m_valid_speedrun_finished = false;
        // Disable showing story mode timer if starting stk with old
        // players.xml
        if(!node->get("story-ms", &m_story_mode_milliseconds))
        {
            UserConfigParams::m_display_story_mode_timer = false;
            m_story_mode_milliseconds = -1;
        }
        if(!node->get("speedrun-ms", &m_speedrun_milliseconds))
        {
            UserConfigParams::m_display_story_mode_timer = false;
            m_speedrun_milliseconds   = -1;
        }
    }   // if node
}   // StoryModeStatus

//-----------------------------------------------------------------------------
StoryModeStatus::~StoryModeStatus()
{
    std::map<std::string, ChallengeStatus*>::iterator it;
    for (it = m_challenges_state.begin();it != m_challenges_state.end();it++)
    {
        delete it->second;
    }
} // ~StoryModeStatus

//-----------------------------------------------------------------------------
/** Adds a ChallengeStatus with the specified id to the set of all statuses
 *  of this object.
 *  \param cs The challenge status.
 */
void StoryModeStatus::addStatus(ChallengeStatus *cs)
{
    m_challenges_state[cs->getData()->getChallengeId()] = cs;
}   // addStatus

//-----------------------------------------------------------------------------
bool StoryModeStatus::isLocked(const std::string& feature)
{
    if (UserConfigParams::m_unlock_everything > 0)
        return false;

    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked

//-----------------------------------------------------------------------------
void StoryModeStatus::computeActive(bool first_call)
{
    int old_points = m_points;
    m_points = 0;
    m_next_unlock_points = 0;
    m_easy_challenges = 0;
    m_medium_challenges = 0;
    m_hard_challenges = 0;
    m_best_challenges = 0;

    m_locked_features.clear(); // start afresh

    std::map<std::string, ChallengeStatus*>::const_iterator i;
    for(i = m_challenges_state.begin();
        i != m_challenges_state.end();  i++)
    {
        // Lock features from unsolved challenges
        if(!(i->second)->isSolvedAtAnyDifficulty())
        {
            lockFeature(i->second);
        }
        // Count points from solved challenges
        else if(!i->second->isUnlockList())
        {
            int gp_factor = i->second->isGrandPrix() ? GP_FACTOR : 1;

            if (i->second->isSolved(RaceManager::DIFFICULTY_BEST))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_BEST]*gp_factor;
                m_best_challenges++;
            }
            else if (i->second->isSolved(RaceManager::DIFFICULTY_HARD))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD]*gp_factor;
                m_hard_challenges++;
            }
            else if (i->second->isSolved(RaceManager::DIFFICULTY_MEDIUM))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_MEDIUM]*gp_factor;
                m_medium_challenges++;
            }
            else if (i->second->isSolved(RaceManager::DIFFICULTY_EASY))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_EASY]*gp_factor;
                m_easy_challenges++;
            }
        }

        switch(i->second->highestSolved())
        {
            // Uses switch fallthrough
            case RaceManager::DIFFICULTY_NONE:
                i->second->setActive(RaceManager::DIFFICULTY_EASY);
            case RaceManager::DIFFICULTY_EASY:
                i->second->setActive(RaceManager::DIFFICULTY_MEDIUM);
            case RaceManager::DIFFICULTY_MEDIUM:
                i->second->setActive(RaceManager::DIFFICULTY_HARD);
            case RaceManager::DIFFICULTY_HARD:
                i->second->setActive(RaceManager::DIFFICULTY_BEST);
            default:
                break;
        }
    }   // for i

    // now we have the number of points.

    // Update the previous number of points
    // On game launch, set it to the number of points the player has
    if (old_points != m_points)
        m_points_before = (first_call) ? m_points : old_points;

    unlockFeatureByList();

    //Actually lock the tracks
    for (i = m_challenges_state.begin(); i != m_challenges_state.end();  i++)
    {
        if (m_points < i->second->getData()->getNumTrophies())
        {
            if (i->second->getData()->isSingleRace())
                m_locked_features[i->second->getData()->getTrackId()] = true;
            else if (i->second->getData()->isGrandPrix())
                m_locked_features[i->second->getData()->getGPId()] = true;
            else
            {
                // FIXME when more challenge types are implemented.
                assert(false);
            }
        }
    }
}   // computeActive

//-----------------------------------------------------------------------------

void StoryModeStatus::unlockFeatureByList()
{
    // test if we have unlocked a feature requiring a certain number of points
    std::map<std::string, ChallengeStatus*>::const_iterator i;
    for(i = m_challenges_state.begin();
        i != m_challenges_state.end();  i++)
    {
        if (i->second->isUnlockList())
        {
            if (i->second->isSolvedAtAnyDifficulty())
                continue;

            bool newly_solved = unlock_manager->unlockByPoints(m_points,i->second);
            newly_solved = newly_solved || unlock_manager->unlockSpecial(i->second, getNumReqMetInLowerDiff());

            // Add to list of recently unlocked features
            if(newly_solved)
                m_unlocked_features.push_back(i->second->getData());

            //Retrieve the smallest number of points for the next unlockable
            if (i->second->getData()->getNumTrophies() > m_points && (m_next_unlock_points == 0
                || i->second->getData()->getNumTrophies() < m_next_unlock_points) )
                m_next_unlock_points = i->second->getData()->getNumTrophies();
        }
    }
} //unlockFeatureByList


//-----------------------------------------------------------------------------

void StoryModeStatus::lockFeature(ChallengeStatus *challenge_status)
{
    const std::vector<ChallengeData::UnlockableFeature>& features =
        challenge_status->getData()->getFeatures();

    const unsigned int amount = (unsigned int)features.size();
    for (unsigned int n=0; n<amount; n++)
    {
        m_locked_features[features[n].m_name] = true;
    }
}   // lockFeature

//-----------------------------------------------------------------------------
/** Unlocks a feature.
 *  ComputeActive resets the locked feature list, so no special code
 *  is required in order to update m_locked_features.
 *  \param c  The challenge that was fulfilled.
 *  \param d Difficulty at which the challenge was solved.
 *  \param do_save If true update the challenge file on disk.
 */
void StoryModeStatus::unlockFeature(ChallengeStatus* c, RaceManager::Difficulty d,
                             bool do_save)
{
    // Add to list of recently unlocked features
    // if the challenge is newly completed at the current difficulty
    if (!c->isSolved(d))
        m_unlocked_features.push_back(c->getData());

    c->setSolved(d);  // reset isActive flag

    // Save the new unlock information
    if (do_save) PlayerManager::get()->save();
}   // unlockFeature

//-----------------------------------------------------------------------------
/** Set the current challenge (or NULL if no challenge is done).
 *  \param challenge Pointer to the challenge (or NULL)
 */
void StoryModeStatus::setCurrentChallenge(const std::string &challenge_id)
{
    m_current_challenge = challenge_id=="" ? NULL
                                           : getChallengeStatus(challenge_id);
}   // setCurrentChallenge

//-----------------------------------------------------------------------------
/** This is called when a race is finished. See if there is an active
 *  challenge that was fulfilled.
 */
void StoryModeStatus::raceFinished()
{
    if(m_current_challenge                                            &&
        RaceManager::get()->getDifficulty() != RaceManager::DIFFICULTY_BEST &&
        m_current_challenge->getData()->isChallengeFulfilled(true /*best*/))
    {
        ChallengeStatus* c = const_cast<ChallengeStatus*>(m_current_challenge);
        c->setMaxReqInLowerDiff();
    }

    if(m_current_challenge                                           &&
        m_current_challenge->isActive(RaceManager::get()->getDifficulty()) &&
        m_current_challenge->getData()->isChallengeFulfilled()           )
    {
        // cast const away so that the challenge can be set to fulfilled.
        // The 'clean' implementation would involve searching the challenge
        // in m_challenges_state, which is a bit of an overkill
        unlockFeature(const_cast<ChallengeStatus*>(m_current_challenge),
                      RaceManager::get()->getDifficulty());
    }   // if isActive && challenge solved
    
    //This updates the number of points.
    //It then calls unlockFeatureByList which checks the specially unlocked features (by points, etc)
    computeActive();
}   // raceFinished

//-----------------------------------------------------------------------------
/** This is called when a GP is finished. See if there is an active
 *  challenge that was fulfilled.
 */
void StoryModeStatus::grandPrixFinished()
{
    if(m_current_challenge                                           &&
        m_current_challenge->isActive(RaceManager::get()->getDifficulty()) )
    {
        ChallengeData::GPLevel unlock_level = m_current_challenge->getData()->isGPFulfilled();

        RaceManager::Difficulty difficulty = RaceManager::DIFFICULTY_EASY;

        switch (unlock_level)
        {
        case ChallengeData::GP_NONE:
            RaceManager::get()->setCoinTarget(0);
            return; //No cup unlocked
        case ChallengeData::GP_EASY:
            difficulty = RaceManager::DIFFICULTY_EASY;
            break;
        case ChallengeData::GP_MEDIUM:
            difficulty = RaceManager::DIFFICULTY_MEDIUM;
            break;
        case ChallengeData::GP_HARD:
            difficulty = RaceManager::DIFFICULTY_HARD;
            break;
        case ChallengeData::GP_BEST:
            difficulty = RaceManager::DIFFICULTY_BEST;
            break;
        }

        RaceManager::get()->setDifficulty(difficulty);
        unlockFeature(const_cast<ChallengeStatus*>(m_current_challenge), difficulty);
    }   // if isActive && challenge solved

    RaceManager::get()->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------
/** Writes the data of this StoryModeStatus to the specified stream.
 *  \param out UTF stream to write to.
 */
void StoryModeStatus::save(UTFWriter &out, bool current_player)
{
    if (story_mode_timer->playerLoaded() && current_player)
    {
        // Make sure the timer pauses time is correct
        if (story_mode_timer->isStoryModePaused())
        {
            story_mode_timer->unpauseTimer(/*loading*/ false);
            story_mode_timer->updateTimer();
            story_mode_timer->pauseTimer(/*loading*/ false);
        }

        if(m_first_time) 
        {
            m_speedrun_milliseconds = 0;
            m_story_mode_milliseconds = 0;    
        }
        else
        {
            m_speedrun_milliseconds = story_mode_timer->getSpeedrunTime();
            m_story_mode_milliseconds = story_mode_timer->getStoryModeTime();
        }
    }

    out << "      <story-mode first-time=\"" << m_first_time  << "\"";
    out << " finished=\"" << m_story_mode_finished  << "\"";
    out << " speedrun-finished=\"" << m_valid_speedrun_finished  << "\"\n";
    out << "                  story-ms=\"" << m_story_mode_milliseconds  << "\"";
    out << " speedrun-ms=\"" << m_speedrun_milliseconds  << "\">\n";

    std::map<std::string, ChallengeStatus*>::const_iterator i;
    for(i = m_challenges_state.begin();
        i != m_challenges_state.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "      </story-mode>\n";
}  // save

int StoryModeStatus::getNumReqMetInLowerDiff() const
{
    int counter = 0;
    for ( const auto &c : m_challenges_state)
    {
        counter += (c.second->areMaxReqMetInLowerDiff()) ? 1 : 0;
    }
    return counter;
}  // getNumReqMetInLowerDiff

/* EOF */
