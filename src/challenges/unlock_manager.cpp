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
#include <iostream>

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "challenges/challenge_data.hpp"
#include "io/file_manager.hpp"
#include "io/xml_writer.hpp"
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
    unlock_manager = this;

    m_current_game_slot = "";
    
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
    
    for(AllChallengesType::iterator i =m_all_challenges.begin(); 
        i!=m_all_challenges.end();  i++)
    {
        delete i->second;
    }
    
    
    std::map<std::string, GameSlot*>::iterator it;
    for (it = m_game_slots.begin(); it != m_game_slots.end(); it++)
    {
        delete it->second;
    }
    
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
                ChallengeData* new_challenge = NULL;
                try
                {
                    new_challenge = new ChallengeData(filename);
                }
                catch (std::runtime_error& ex)
                {
                    std::cerr << "\n/!\\ An error occurred while loading challenge file '" << filename << "' : "
                              << ex.what() << " : challenge will be ignored.\n\n"; 
                    continue;
                }
                addOrFreeChallenge(new_challenge);
            }   // if file
            
        }   // for file in files
    }   // for dir in all_track_dirs
}

//-----------------------------------------------------------------------------
/** If a challenge is supported by this binary (i.e. has an appropriate 
 *  challenge version number), add this challenge to the set of all challenges,
 *  otherwise free the memory for this challenge.
 *  \param c The challenge that is either stored or freed.
 */
void UnlockManager::addOrFreeChallenge(ChallengeData *c)
{
    if(isSupportedVersion(*c))
        m_all_challenges[c->getId()]=c;
    else
    {
        printf("[challenge] Challenge '%s' is not supported - ignored.\n",
               c->getId().c_str());
        delete c;
    }
}   // addOrFreeChallenge

//-----------------------------------------------------------------------------
/** Reads a challenge from the given filename. The challenge will then either
 *  be stored, or (if the challenge version is not supported anymore
 *  \param filename Name of the challenge file to read.
 */
void UnlockManager::addChallenge(const std::string& filename)
{
    ChallengeData* new_challenge = NULL;
    try
    {
        new_challenge = new ChallengeData(filename);
        new_challenge->check();
    }
    catch (std::runtime_error& ex)
    {
        std::cerr << "\n/!\\ An error occurred while loading challenge file '" << filename << "' : "
                  << ex.what() << " : challenge will be ignored.\n\n"; 
        if (new_challenge != NULL) delete new_challenge;
        return;
    }
    addOrFreeChallenge(new_challenge);
    
}   // addChallenge

//-----------------------------------------------------------------------------
const ChallengeData* UnlockManager::getChallenge(const std::string& id)
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
        
        createSlotsIfNeeded();
        save();
        
        if (root) delete root;
        return;
    }
    
    std::vector<XMLNode*> xml_game_slots;
    root->getNodes("gameslot", xml_game_slots);
    for (unsigned int n=0; n<xml_game_slots.size(); n++)
    {
        std::string player_id;
        if (!xml_game_slots[n]->get("playerID", &player_id))
        {
            fprintf(stderr, "[UnlockManager] WARNING: Found game slot without a player ID attached. Discarding it\n");
            continue;
        }
        
        GameSlot* slot = new GameSlot(player_id);
        
        std::string kart_id;
        xml_game_slots[n]->get("kart", &kart_id);
        slot->setKartIdent(kart_id);
        
        m_game_slots[player_id] = slot;
        
        bool first_time = true;
        xml_game_slots[n]->get("firstTime", &first_time);
        slot->setFirstTime(first_time);
        
        for(AllChallengesType::iterator i = m_all_challenges.begin(); 
            i!=m_all_challenges.end();  i++)
        {
            ChallengeData* curr = i->second;
            Challenge* state = new Challenge(curr);
            
            slot->m_challenges_state[curr->getId()] = state;
            state->load(xml_game_slots[n]);
        }
        slot->computeActive();
    }
    
    bool something_changed = createSlotsIfNeeded();
    if (something_changed) save();
    
    delete root;
}   // load

//-----------------------------------------------------------------------------

void UnlockManager::save()
{
    std::string filename = file_manager->getChallengeFile("challenges.xml");
    
    std::ofstream challenge_file(filename.c_str(), std::ios::out);

    if (!challenge_file.is_open())
    {
        std::cerr << "Failed to open " << filename << " for writing, challenges won't be saved\n";
        return;
    }

    challenge_file << "<?xml version=\"1.0\"?>\n";
    challenge_file << "<challenges>\n";
    
    std::map<std::string, GameSlot*>::iterator it;
    for (it = m_game_slots.begin(); it != m_game_slots.end(); it++)
    {
        it->second->save(challenge_file);
    }
    
    challenge_file << "</challenges>\n\n";
    challenge_file.close();
}   // save

//-----------------------------------------------------------------------------

bool UnlockManager::createSlotsIfNeeded()
{
    bool something_changed = false;
    
    // make sure all players have at least one game slot associated
    PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
    for (int n=0; n<players.size(); n++)
    {
        bool exists = false;
        
        std::map<std::string, GameSlot*>::iterator it;
        for (it = m_game_slots.begin(); it != m_game_slots.end(); it++)
        {
            GameSlot* curr_slot = it->second;
            if (curr_slot->getPlayerID() == players[n].getUniqueID())
            {
                exists = true;
                break;
            }
        }
        
        if (!exists)
        {
            GameSlot* slot = new GameSlot(players[n].getUniqueID());
            for(AllChallengesType::iterator i = m_all_challenges.begin(); 
                i!=m_all_challenges.end();  i++)
            {
                ChallengeData* cd = i->second;
                slot->m_challenges_state[cd->getId()] = new Challenge(cd);
            }
            slot->computeActive();
            
            m_game_slots[players[n].getUniqueID()] = slot;
            
            something_changed = true;
        }
    }
    
    return something_changed;
}

//-----------------------------------------------------------------------------
void UnlockManager::playLockSound() const
{
    m_locked_sound->play();
}   // playLockSound

//-----------------------------------------------------------------------------
/** Test if the given challenge is supported by this binary.
 *  \param challenge The challenge to test.
 */
bool UnlockManager::isSupportedVersion(const ChallengeData &challenge)
{
    // Test if challenge version number is in between minimum
    // and maximum supported version.
    return (challenge.getVersion()>=2 && challenge.getVersion()<=2);
}   // isSupportedVersion


//-----------------------------------------------------------------------------

PlayerProfile* UnlockManager::getCurrentPlayer()
{
    PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
    for (int n=0; n<players.size(); n++)
    {
        if (players[n].getUniqueID() == m_current_game_slot) return players.get(n);
    }
    return NULL;
}

//-----------------------------------------------------------------------------

void UnlockManager::updateActiveChallengeList()
{
    getCurrentSlot()->computeActive();
}

