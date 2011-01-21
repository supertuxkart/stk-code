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

#include "challenges/unlock_manager.hpp"

#include <set>
#include <string>
#include <vector>
#include <stdio.h>

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "challenges/challenge_data.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"

UnlockManager* unlock_manager=0;
//-----------------------------------------------------------------------------

UnlockManager::UnlockManager()
{
    // The global variable 'unlock_manager' is needed in the challenges,
    // but it's not set yet - so we define it here (and it gets re-assign
    // in main).
    unlock_manager=this;

    m_locked_sound = sfx_manager->createSoundSource("locked");
    

    // Read challenges from .../data/challenges
    // ----------------------------------------
    std::set<std::string> result;
    file_manager->listFiles(result, "data/challenges");
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end()  ; i++)
    {
        if (StringUtils::hasSuffix(*i, ".challenge")) 
            addChallenge(file_manager->getDataFile("challenges/"+*i));
    }   // for i

    // Read challenges from .../data/tracks/*
    // --------------------------------------
    const std::vector<std::string> *all_track_dirs = track_manager->getAllTrackDirs();
    readAllChallengesInDirs(all_track_dirs);

    // Read challenges from .../data/karts/*
    // --------------------------------------
    const std::vector<std::string> *all_kart_dirs  = kart_properties_manager->getAllKartDirs();
    readAllChallengesInDirs(all_kart_dirs);

    // Hard coded challenges can be added here.

    load();

}   // UnlockManager

//-----------------------------------------------------------------------------
/** Saves the challenge status.
 */
UnlockManager::~UnlockManager()
{
    save();
    
    // sfx_manager is destroyed before UnlockManager is, so SFX will be already deleted
    // sfx_manager->deleteSFX(m_locked_sound);
}   // ~UnlockManager

//-----------------------------------------------------------------------------

void UnlockManager::readAllChallengesInDirs(const std::vector<std::string>* all_dirs)
{
    for(std::vector<std::string>::const_iterator dir = all_dirs->begin();
        dir != all_dirs->end(); dir++)
    {
        std::set<std::string> all_files;
        file_manager->listFiles(all_files, *dir, /*is_full_path*/ true);
        
        for(std::set<std::string>::iterator file = all_files.begin(); 
            file != all_files.end(); file++)
        {
            if (!StringUtils::hasSuffix(*file,".challenge")) continue;
            
            std::string filename = *dir + "/" + *file;
            
            FILE* f = fopen(filename.c_str(), "r");
            if (f)
            {
                fclose(f);
                ChallengeData* newChallenge = NULL;
                try
                {
                    newChallenge = new ChallengeData(filename);
                }
                catch (std::runtime_error& ex)
                {
                    std::cerr << "\n/!\\ An error occurred while loading challenge file '" << filename << "' : "
                              << ex.what() << " : challenge will be ignored.\n\n"; 
                    continue;
                }
                addChallenge(newChallenge);
            }   // if file
            
        }   // for file in files
    }   // for dir in all_track_dirs
}

//-----------------------------------------------------------------------------
void UnlockManager::addChallenge(Challenge *c)
{
    m_all_challenges[c->getId()]=c;
}   // addChallenge

//-----------------------------------------------------------------------------

void UnlockManager::addChallenge(const std::string& filename)
{
    ChallengeData* newChallenge = NULL;
    try
    {
        newChallenge = new ChallengeData(filename);
        newChallenge->check();
    }
    catch (std::runtime_error& ex)
    {
        std::cerr << "\n/!\\ An error occurred while loading challenge file '" << filename << "' : "
                  << ex.what() << " : challenge will be ignored.\n\n"; 
        if (newChallenge != NULL) delete newChallenge;
        return;
    }
    addChallenge(newChallenge);
    
}   // addChallenge

//-----------------------------------------------------------------------------
std::vector<const Challenge*> UnlockManager::getActiveChallenges()
{
    computeActive();
    std::vector<const Challenge*> all_active;
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive()) all_active.push_back(i->second);
    }
    return all_active;
}   // getActiveChallenges

//-----------------------------------------------------------------------------
const Challenge* UnlockManager::getChallenge(const std::string& id)
{
    if(m_all_challenges.find(id)==m_all_challenges.end()) return NULL;
    return m_all_challenges[id];
}   // getChallenge

//-----------------------------------------------------------------------------
/** This is called from user_config when reading the config file
*/
void UnlockManager::load()
{
    const std::string filename=file_manager->getChallengeFile("challenges.xml");
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root || root->getName() != "challenges")
    {
        std::cerr << "Challenge file '" << filename << "' will be created." 
                  << std::endl;
        save();
        
        if (root) delete root;
        return;
    }
    
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        i->second->load(root);
    }
    computeActive();
    
    delete root;
}   // load

//-----------------------------------------------------------------------------
void UnlockManager::save()
{
    std::ofstream challenge_file;
    std::string filename = file_manager->getChallengeFile("challenges.xml");
    challenge_file.open(filename.c_str());

    if(!challenge_file.is_open())
    {
        std::cerr << "Failed to open " << filename << " for writing, challenges won't be saved\n";
        return;
    }

    challenge_file << "<?xml version=\"1.0\"?>\n";
    challenge_file << "<challenges>\n";
    
    for(AllChallengesType::iterator i = m_all_challenges.begin(); 
                                    i!= m_all_challenges.end();  i++)
    {
        i->second->save(challenge_file);
    }
    
    challenge_file << "</challenges>\n\n";
    challenge_file.close();
}   // save

void UnlockManager::playLockSound() const
{
    m_locked_sound->play();
}

//-----------------------------------------------------------------------------
void UnlockManager::computeActive()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        // Changed challenge
        // -----------------
        if((i->second)->isSolved()) 
        {
            // The constructor calls computeActive, which actually locks 
            // all features, so unlock the solved ones (and don't try to
            // save the state, since we are currently reading it)
            
            unlockFeature(i->second, /*save*/ false);
            continue;
        }

        // Otherwise lock the feature, and check if the challenge is active
        // ----------------------------------------------------------------
        lockFeature(i->second);
        std::vector<std::string> pre_req=(i->second)->getPrerequisites();
        bool allSolved=true;
        for(std::vector<std::string>::iterator pre =pre_req.begin();
                                               pre!=pre_req.end(); pre++)
        {
            const Challenge*p = getChallenge(*pre);
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
/** This is called when a race is finished. Call all active challenges
*/
void UnlockManager::raceFinished()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive() && i->second->raceFinished())
        {
            unlockFeature(i->second);
        }   // if isActive && challenge solved
    }
    //race_manager->setCoinTarget(0);  //reset
}   // raceFinished

//-----------------------------------------------------------------------------
void UnlockManager::grandPrixFinished()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
                                    i!=m_all_challenges.end();  i++)
    {
        if(i->second->isActive() && i->second->grandPrixFinished())
        {
            std::cout << "===== A FEATURE WAS UNLOCKED BECAUSE YOU WON THE GP!! ==\n";
            unlockFeature(i->second);
        }
    }
    race_manager->setCoinTarget(0);
}   // grandPrixFinished

//-----------------------------------------------------------------------------
void UnlockManager::lockFeature(Challenge* challenge)
{
    const unsigned int amount = (unsigned int)challenge->getFeatures().size();
    for(unsigned int n=0; n<amount; n++)
        m_locked_features[challenge->getFeatures()[n].name]=true;
}   // lockFeature

//-----------------------------------------------------------------------------

void UnlockManager::unlockFeature(Challenge* c, bool do_save)
{
    const unsigned int amount = (unsigned int)c->getFeatures().size();
    for(unsigned int n=0; n<amount; n++)
    {
        std::string feature = c->getFeatures()[n].name;
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
    m_unlocked_features.push_back(c);
    c->setSolved();  // reset isActive flag
    
    // Save the new unlock information
    if(do_save) save();
}   // unlockFeature

//-----------------------------------------------------------------------------
bool UnlockManager::isLocked(const std::string& feature)
{
    return m_locked_features.find(feature)!=m_locked_features.end();
}  // featureIsLocked
//-----------------------------------------------------------------------------
const std::vector<const Challenge*>   UnlockManager::getUnlockedFeatures()
{    
    std::vector<const Challenge*>  out;
    
    for(AllChallengesType::const_iterator i =m_all_challenges.begin(); 
        i!=m_all_challenges.end();  i++)
    {
        if (i->second->isSolved()) out.push_back(i->second);
    }
    
    return out;
}
//-----------------------------------------------------------------------------
const std::vector<const Challenge*>   UnlockManager::getLockedChallenges()
{    
    std::vector<const Challenge*>  out;
    
    for(AllChallengesType::const_iterator i =m_all_challenges.begin(); 
        i!=m_all_challenges.end();  i++)
    {
        if (!i->second->isSolved() && !i->second->isActive()) out.push_back(i->second);
    }
    
    return out;
}


