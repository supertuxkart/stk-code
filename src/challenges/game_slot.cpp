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

#include "challenges/challenge_data.hpp"
#include "challenges/unlock_manager.hpp"
#include "io/xml_writer.hpp"

//-----------------------------------------------------------------------------

bool GameSlot::isLocked(const std::string& feature)
{
    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked
   //-----------------------------------------------------------------------------

const std::vector<const ChallengeData*> GameSlot::getUnlockedFeatures()
{
    std::vector<const ChallengeData*> out;
    
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if (i->second->isSolved()) out.push_back(i->second->getData());
    }
    
    return out;
}
//-----------------------------------------------------------------------------
const std::vector<const ChallengeData*> GameSlot::getLockedChallenges()
{    
    std::vector<const ChallengeData*> out;
    
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if (!i->second->isSolved()) out.push_back(i->second->getData());
    }
    
    return out;
}

//-----------------------------------------------------------------------------
void GameSlot::computeActive()
{
    m_points = 0;
    
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        // Changed challenge
        // -----------------
        if((i->second)->isSolved()) 
        {
            // The constructor calls computeActive, which actually locks 
            // all features, so unlock the solved ones (and don't try to
            // save the state, since we are currently reading it)
            
            unlockFeature(i->second, /*save*/ false);
            m_points++;
            continue;
        }
        
        // Otherwise lock the feature, and check if the challenge is active
        // ----------------------------------------------------------------
        lockFeature(i->second);
        std::vector<std::string> pre_req=(i->second)->getData()->getPrerequisites();
        bool allSolved=true;
        for(std::vector<std::string>::iterator pre =pre_req.begin();
            pre!=pre_req.end(); pre++)
        {
            const Challenge* p = m_challenges_state[*pre];
            if(!p)
            {
                fprintf(stderr,"Challenge prerequisite '%s' of '%s' not found - ignored\n",
                        pre->c_str(), i->first.c_str());
                //continue;
                allSolved=false;
                break;
            }
            else if(!p->isSolved())
            {
                allSolved=false;
                break;
            }
        }   // for all pre in pre_req
        if(allSolved)
        {
            i->second->setActive();
        }   // if solved
    }   // for i
    clearUnlocked();
}   // computeActive

//-----------------------------------------------------------------------------

void GameSlot::lockFeature(Challenge *challenge)
{
    const std::vector<ChallengeData::UnlockableFeature>& features = challenge->getData()->getFeatures();
    
    const unsigned int amount = (unsigned int)features.size();
    for(unsigned int n=0; n<amount; n++)
        m_locked_features[features[n].m_name]=true;
}   // lockFeature

//-----------------------------------------------------------------------------

void GameSlot::unlockFeature(Challenge* c, bool do_save)
{
    const unsigned int amount = (unsigned int)c->getData()->getFeatures().size();
    for(unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getData()->getFeatures()[n].m_name;
        std::map<std::string,bool>::iterator p=m_locked_features.find(feature);
        if(p==m_locked_features.end())
        {
            //fprintf(stderr,"Unlocking feature '%s' failed: feature is not locked.\n",
            //        (feature).c_str());
            return;
        }
        m_locked_features.erase(p);
    }
    
    // Add to list of recently unlocked features
    m_unlocked_features.push_back(c->getData());
    c->setSolved();  // reset isActive flag
    
    // Save the new unlock information
    if(do_save) unlock_manager->save();
}   // unlockFeature

//-----------------------------------------------------------------------------

std::vector<const ChallengeData*> GameSlot::getActiveChallenges()
{
    computeActive();

    std::vector<const ChallengeData*> out;
    
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if (i->second->isActive()) out.push_back(i->second->getData());
    }
    
    return out;
}   // getActiveChallenges

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
        if(i->second->isActive() && i->second->getData()->raceFinished())
        {
            unlockFeature(i->second);
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
        if(i->second->isActive() && i->second->getData()->grandPrixFinished())
        {
            printf("===== A FEATURE WAS UNLOCKED BECAUSE YOU WON THE GP!! ==\n");
            unlockFeature(i->second);
        }
    }
    race_manager->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------

void GameSlot::save(XMLWriter& out)
{
    out << L"    <gameslot player=\"" << m_player_name.c_str() << L"\" kart=\""
        << m_kart_ident.c_str() << L"\">\n";
    std::map<std::string, Challenge*>::const_iterator i;
    for(i = m_challenges_state.begin(); 
        i != m_challenges_state.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "    </gameslot>\n";
}
