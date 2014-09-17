//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#include "achievements/achievements_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/challenge_data.hpp"
#include "challenges/challenge_status.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <set>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>

UnlockManager* unlock_manager=0;
//-----------------------------------------------------------------------------

UnlockManager::UnlockManager()
{
    // The global variable 'unlock_manager' is needed in the challenges,
    // but it's not set yet - so we define it here (and it gets re-assign
    // in main).
    unlock_manager = this;

    m_locked_sound = SFXManager::get()->createSoundSource("locked");


    // Read challenges from .../data/challenges
    // ----------------------------------------
    std::set<std::string> result;
    std::string challenge_dir = file_manager->getAsset(FileManager::CHALLENGE, "");
    file_manager->listFiles(result, challenge_dir);
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end()  ; i++)
    {
        if (StringUtils::hasSuffix(*i, ".challenge"))
            addChallenge(file_manager->getAsset("challenges/"+*i));
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

}   // UnlockManager

//-----------------------------------------------------------------------------
/** Saves the challenge status.
 */
UnlockManager::~UnlockManager()
{
    for(AllChallengesType::iterator i =m_all_challenges.begin();
        i!=m_all_challenges.end();  i++)
    {
        delete i->second;
    }

}   // ~UnlockManager

//-----------------------------------------------------------------------------

void UnlockManager::readAllChallengesInDirs(const std::vector<std::string>* all_dirs)
{
    for(std::vector<std::string>::const_iterator dir = all_dirs->begin();
        dir != all_dirs->end(); dir++)
    {
        std::set<std::string> all_files;
        file_manager->listFiles(all_files, *dir);

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
                    Log::warn("unlock_manager", "An error occurred while "
                              "loading challenge file '%s' : %s.\n"
                              "Challenge will be ignored.",
                              filename.c_str(), ex.what());
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
    {
        m_all_challenges[c->getId()]=c;
    }
    else
    {
        Log::warn("Challenge", "Challenge '%s' is not supported - ignored.",
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
        Log::warn("unlock_manager", "An error occurred while loading "
                   "challenge file '%s' : %s challenge will be ignored.",
                   filename.c_str(), ex.what());
        if (new_challenge) delete new_challenge;
        return;
    }
    addOrFreeChallenge(new_challenge);

}   // addChallenge

//-----------------------------------------------------------------------------
/** Returns the challenge data for a challenge id, or NULL if no such
 *  challenge exist.
 *  \param id Id of the challenge.
 */
const ChallengeData* UnlockManager::getChallengeData(const std::string& id)
{
    AllChallengesType::const_iterator it = m_all_challenges.find(id);
    if(it==m_all_challenges.end()) return NULL;
    return it->second;
}   // getChallengeData

//-----------------------------------------------------------------------------
/** Creates a game slot. It initialises the game slot's status with the
 *  information in the xml node (if given), basically restoring the saved
 *  states for a player.
 *  \param node The XML game-slots node with all data for a player.
 */
StoryModeStatus* UnlockManager::createStoryModeStatus(const XMLNode *node)
{

    StoryModeStatus *status = new StoryModeStatus(node);

    for(AllChallengesType::iterator i = m_all_challenges.begin();
                                    i!=m_all_challenges.end();  i++)
    {
        ChallengeData* cd = i->second;
        ChallengeStatus *challenge_status = new ChallengeStatus(cd);
        if(node)
            challenge_status->load(node);
        status->addStatus(challenge_status);
    }

    status->computeActive();
    return status;
}   // createStoryModeStatus

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

void UnlockManager::findWhatWasUnlocked(int points_before, int points_now,
                                        std::vector<std::string>& tracks,
                                        std::vector<std::string>& gps)
{
    for (AllChallengesType::iterator it = m_all_challenges.begin();
         it != m_all_challenges.end(); it++)
    {
        ChallengeData* c = it->second;
        if (c->getNumTrophies() > points_before &&
            c->getNumTrophies() <= points_now      )
        {
            if (c->getMode() == ChallengeData::CM_SINGLE_RACE && c->getTrackId() != "")
            {
                if (!PlayerManager::getCurrentPlayer()->isLocked(c->getTrackId()))
                    tracks.push_back(c->getTrackId());
            }
            else if (c->getMode() == ChallengeData::CM_GRAND_PRIX && c->getGPId() != "")
            {
                if (!PlayerManager::getCurrentPlayer()->isLocked(c->getGPId()))
                    gps.push_back(c->getGPId());
            }
        }
    }
}
