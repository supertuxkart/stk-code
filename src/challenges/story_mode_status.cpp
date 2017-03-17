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
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"

//-----------------------------------------------------------------------------
StoryModeStatus::StoryModeStatus(const XMLNode *node)
{
    m_points            = 0;
    m_first_time        = true;
    m_easy_challenges   = 0;
    m_medium_challenges = 0;
    m_hard_challenges   = 0;
    m_current_challenge = NULL;

    // If there is saved data, load it
    if(node)
    {
        node->get("first-time", &m_first_time);
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
    m_challenges_state[cs->getData()->getId()] = cs;
}   // addStatus

//-----------------------------------------------------------------------------
bool StoryModeStatus::isLocked(const std::string& feature)
{
    if (UserConfigParams::m_everything_unlocked)
        return false;

    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked

//-----------------------------------------------------------------------------
void StoryModeStatus::computeActive()
{
    m_points = 0;
    m_easy_challenges = 0;
    m_medium_challenges = 0;
    m_hard_challenges = 0;

    m_locked_features.clear(); // start afresh

    std::map<std::string, ChallengeStatus*>::const_iterator i;
    for(i = m_challenges_state.begin();
        i != m_challenges_state.end();  i++)
    {
        // Changed challenge
        // -----------------
        if((i->second)->isSolvedAtAnyDifficulty())
        {
            // The constructor calls computeActive, which actually locks
            // all features, so unlock the solved ones (and don't try to
            // save the state, since we are currently reading it)

            if (i->second->isSolved(RaceManager::DIFFICULTY_EASY))
            {
                unlockFeature(i->second, RaceManager::DIFFICULTY_EASY,
                              /*save*/ false);
            }
            if (i->second->isSolved(RaceManager::DIFFICULTY_MEDIUM))
            {
                unlockFeature(i->second, RaceManager::DIFFICULTY_MEDIUM,
                              /*save*/ false);
            }
            if (i->second->isSolved(RaceManager::DIFFICULTY_HARD))
            {
                unlockFeature(i->second, RaceManager::DIFFICULTY_HARD,
                              /*save*/ false);
            }

            if (i->second->isSolved(RaceManager::DIFFICULTY_HARD))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_HARD];
                m_hard_challenges++;
            }
            else if (i->second->isSolved(RaceManager::DIFFICULTY_MEDIUM))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_MEDIUM];
                m_medium_challenges++;
            }
            else if (i->second->isSolved(RaceManager::DIFFICULTY_EASY))
            {
                m_points += CHALLENGE_POINTS[RaceManager::DIFFICULTY_EASY];
                m_easy_challenges++;
            }
        }
        else
        {
            // Otherwise lock the feature
            // --------------------------
            lockFeature(i->second);
        }

        if (i->second->isSolved(RaceManager::DIFFICULTY_HARD))
        {
            // challenge beaten at hardest, nothing more to do here
            continue;
        }
        else if (i->second->isSolved(RaceManager::DIFFICULTY_MEDIUM))
        {
            i->second->setActive(RaceManager::DIFFICULTY_HARD);
        }
        else if (i->second->isSolved(RaceManager::DIFFICULTY_EASY))
        {
            i->second->setActive(RaceManager::DIFFICULTY_HARD);
            i->second->setActive(RaceManager::DIFFICULTY_MEDIUM);
        }
        else
        {
            i->second->setActive(RaceManager::DIFFICULTY_HARD);
            i->second->setActive(RaceManager::DIFFICULTY_MEDIUM);
            i->second->setActive(RaceManager::DIFFICULTY_EASY);
        }
    }   // for i

    // now we have the number of points. Actually lock the tracks
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

    clearUnlocked();


}   // computeActive

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
 *  \param c  The challenge that was fulfilled.
 *  \param d Difficulty at which the challenge was solved.
 *  \param do_save If true update the challenge file on disk.
 */
void StoryModeStatus::unlockFeature(ChallengeStatus* c, RaceManager::Difficulty d,
                             bool do_save)
{
    const unsigned int amount=(unsigned int)c->getData()->getFeatures().size();
    for (unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getData()->getFeatures()[n].m_name;
        std::map<std::string,bool>::iterator p=m_locked_features.find(feature);
        if (p == m_locked_features.end())
        {
            c->setSolved(d);
            if(do_save) PlayerManager::get()->save();
            return;
        }
        m_locked_features.erase(p);
    }

    // Add to list of recently unlocked features
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
    if(m_current_challenge                                           &&
        m_current_challenge->isActive(race_manager->getDifficulty()) &&
        m_current_challenge->getData()->isChallengeFulfilled()           )
    {
        // cast const away so that the challenge can be set to fulfilled.
        // The 'clean' implementation would involve searching the challenge
        // in m_challenges_state, which is a bit of an overkill
        unlockFeature(const_cast<ChallengeStatus*>(m_current_challenge),
                      race_manager->getDifficulty());
    }   // if isActive && challenge solved
}   // raceFinished

//-----------------------------------------------------------------------------
/** This is called when a GP is finished. See if there is an active
 *  challenge that was fulfilled.
 */
void StoryModeStatus::grandPrixFinished()
{
    if(m_current_challenge                                           &&
        m_current_challenge->isActive(race_manager->getDifficulty()) &&
        m_current_challenge->getData()->isGPFulfilled()                 )
    {
        unlockFeature(const_cast<ChallengeStatus*>(m_current_challenge),
                      race_manager->getDifficulty());
    }   // if isActive && challenge solved

    race_manager->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------
/** Writes the data of this StoryModeStatus to the specified stream.
 *  \param out UTF stream to write to.
 */
void StoryModeStatus::save(UTFWriter &out)
{
    out << L"      <story-mode first-time=\"" << m_first_time  << L"\">\n";
    std::map<std::string, ChallengeStatus*>::const_iterator i;
    for(i = m_challenges_state.begin();
        i != m_challenges_state.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << L"      </story-mode>\n";
}  // save
