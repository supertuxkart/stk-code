//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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


#include "challenges/game_slot.hpp"

#include "challenges/challenge.hpp"
#include "challenges/challenge_data.hpp"
#include "challenges/unlock_manager.hpp"
#include "io/xml_writer.hpp"

//-----------------------------------------------------------------------------
GameSlot::~GameSlot()
{
    std::map<std::string, Challenge*>::iterator it;
    for (it = m_challenges_state.begin();it != m_challenges_state.end();it++)
    {
        delete it->second;
    }
} // ~GameSlot
//-----------------------------------------------------------------------------
bool GameSlot::isLocked(const std::string& feature)
{
    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked

//-----------------------------------------------------------------------------
void GameSlot::computeActive()
{
    m_points = 0;
    m_easy_challenges = 0;
    m_medium_challenges = 0;
    m_hard_challenges = 0;
    
    m_locked_features.clear(); // start afresh
    
    std::map<std::string, Challenge*>::const_iterator i;
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
            if (i->second->getData()->getTrackId().size() > 0)
                m_locked_features[i->second->getData()->getTrackId()] = true;
            else if (i->second->getData()->getGPId().size() > 0)
                m_locked_features[i->second->getData()->getGPId()] = true;
        }
    }
    
    clearUnlocked();
    
    
}   // computeActive

//-----------------------------------------------------------------------------

void GameSlot::lockFeature(Challenge *challenge)
{
    const std::vector<ChallengeData::UnlockableFeature>& features = 
        challenge->getData()->getFeatures();
    
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
void GameSlot::unlockFeature(Challenge* c, RaceManager::Difficulty d, 
                             bool do_save)
{
    const unsigned int amount=(unsigned int)c->getData()->getFeatures().size();
    for (unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getData()->getFeatures()[n].m_name;
        std::map<std::string,bool>::iterator p=m_locked_features.find(feature);
        if (p == m_locked_features.end())
        {
            return;
        }
        m_locked_features.erase(p);
    }
    
    // Add to list of recently unlocked features
    m_unlocked_features.push_back(c->getData());
    c->setSolved(d);  // reset isActive flag
    
    // Save the new unlock information
    if (do_save) unlock_manager->save();
}   // unlockFeature

//-----------------------------------------------------------------------------
/** Set the current challenge (or NULL if no challenge is done).
 *  \param challenge Pointer to the challenge (or NULL)
 */
void GameSlot::setCurrentChallenge(const std::string &challenge_id)
{
    m_current_challenge = challenge_id=="" ? NULL 
                                           : getChallenge(challenge_id);
}   // setCurrentChallenge

//-----------------------------------------------------------------------------
/** This is called when a race is finished. See if there is an active
 *  challenge that was fulfilled.
 */
void GameSlot::raceFinished()
{
    if(m_current_challenge                                           &&
        m_current_challenge->isActive(race_manager->getDifficulty()) && 
        m_current_challenge->getData()->isChallengeFulfilled()           )
    {
        // cast const away so that the challenge can be set to fulfilled.
        // The 'clean' implementation would involve searching the challenge
        // in m_challenges_state, which is a bit of an overkill
        unlockFeature(const_cast<Challenge*>(m_current_challenge), 
                      race_manager->getDifficulty());
    }   // if isActive && challenge solved
}   // raceFinished

//-----------------------------------------------------------------------------
/** This is called when a GP is finished. See if there is an active
 *  challenge that was fulfilled.
 */
void GameSlot::grandPrixFinished()
{
    if(m_current_challenge                                           &&
        m_current_challenge->isActive(race_manager->getDifficulty()) && 
        m_current_challenge->getData()->isGPFulfilled()                 )
    {
        unlockFeature(const_cast<Challenge*>(m_current_challenge), 
                      race_manager->getDifficulty());
    }   // if isActive && challenge solved
    
    race_manager->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------

void GameSlot::save(std::ofstream& out)
{
    out << "    <gameslot playerID=\"" << m_player_unique_id.c_str() 
        << "\" kart=\""                << m_kart_ident.c_str() 
        << "\" firstTime=\""           << (m_first_time ? "true" : "false")
        << "\">\n";
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "    </gameslot>\n";
}
