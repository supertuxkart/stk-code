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
            
            if (i->second->isSolved(RaceManager::RD_EASY))
            {
                unlockFeature(i->second, RaceManager::RD_EASY, /*save*/ false);
            }
            if (i->second->isSolved(RaceManager::RD_MEDIUM))
            {
                unlockFeature(i->second, RaceManager::RD_MEDIUM, /*save*/ false);
            }
            if (i->second->isSolved(RaceManager::RD_HARD))
            {
                unlockFeature(i->second, RaceManager::RD_HARD, /*save*/ false);
            }
            
            if (i->second->isSolved(RaceManager::RD_HARD))
            {
                m_points += CHALLENGE_POINTS[RaceManager::RD_HARD];
                m_hard_challenges++;
            }
            else if (i->second->isSolved(RaceManager::RD_MEDIUM))
            {
                m_points += CHALLENGE_POINTS[RaceManager::RD_MEDIUM];
                m_medium_challenges++;
            }
            else if (i->second->isSolved(RaceManager::RD_EASY))
            {
                m_points += CHALLENGE_POINTS[RaceManager::RD_EASY];
                m_easy_challenges++;
            }
        }
        else
        {
            // Otherwise lock the feature
            // --------------------------
            lockFeature(i->second);
        }
        
        if (i->second->isSolved(RaceManager::RD_HARD))
        {
            // challenge beaten at hardest, nothing more to do here
            continue;
        }
        else if (i->second->isSolved(RaceManager::RD_MEDIUM))
        {
            i->second->setActive(RaceManager::RD_HARD);
        }
        else if (i->second->isSolved(RaceManager::RD_EASY))
        {
            i->second->setActive(RaceManager::RD_HARD);
            i->second->setActive(RaceManager::RD_MEDIUM);
        }
        else
        {
            i->second->setActive(RaceManager::RD_HARD);
            i->second->setActive(RaceManager::RD_MEDIUM);
            i->second->setActive(RaceManager::RD_EASY);
        }
    }   // for i
    
    // now we have the number of points. Actually lock the tracks
    for (i = m_challenges_state.begin(); i != m_challenges_state.end();  i++)
    {
        if (m_points < i->second->getData()->getNumTrophies())
        {
            m_locked_features[i->second->getData()->getTrackId()] = true;
        }
    }
    
    clearUnlocked();
    
    
}   // computeActive

//-----------------------------------------------------------------------------

void GameSlot::lockFeature(Challenge *challenge)
{
    const std::vector<ChallengeData::UnlockableFeature>& features = challenge->getData()->getFeatures();
    
    const unsigned int amount = (unsigned int)features.size();
    for (unsigned int n=0; n<amount; n++)
    {
        m_locked_features[features[n].m_name] = true;
    }
}   // lockFeature

//-----------------------------------------------------------------------------

void GameSlot::unlockFeature(Challenge* c, RaceManager::Difficulty d, bool do_save)
{
    const unsigned int amount = (unsigned int)c->getData()->getFeatures().size();
    for (unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getData()->getFeatures()[n].m_name;
        std::map<std::string,bool>::iterator p = m_locked_features.find(feature);
        if (p == m_locked_features.end())
        {
            //fprintf(stderr,"Unlocking feature '%s' failed: feature is not locked.\n",
            //        (feature).c_str());
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

/** This is called when a race is finished. Call all active challenges
 */
void GameSlot::raceFinished()
{
    printf("=== Race finished ===\n");
    
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if(i->second->isActive(race_manager->getDifficulty()) && i->second->getData()->raceFinished())
        {
            unlockFeature(i->second, race_manager->getDifficulty());
        }   // if isActive && challenge solved
    }
    
    //race_manager->setCoinTarget(0);  //reset
}   // raceFinished

//-----------------------------------------------------------------------------

void GameSlot::grandPrixFinished()
{
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if(i->second->isActive(race_manager->getDifficulty()) &&
           i->second->getData()->grandPrixFinished())
        {
            printf("===== A FEATURE WAS UNLOCKED BECAUSE YOU WON THE GP!! ==\n");
            unlockFeature(i->second, race_manager->getDifficulty());
        }
    }
    
    race_manager->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------

void GameSlot::save(std::ofstream& out)
{
    out << "    <gameslot playerID=\"" << m_player_unique_id.c_str() << "\" kart=\""
        << m_kart_ident.c_str() << "\" firstTime=\"" << (m_first_time ? "true" : "false")
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
