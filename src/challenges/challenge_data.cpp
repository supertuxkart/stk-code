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
#include "challenges/challenge_data.hpp"

#include <stdexcept>
#include <sstream>

#include "challenges/unlock_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

ChallengeData::ChallengeData(const std::string& filename)
{
    m_filename     = filename;
    m_mode         = CM_SINGLE_RACE;
    m_minor        = RaceManager::MINOR_MODE_NORMAL_RACE;
    m_num_laps     = -1;
    m_reverse      = false;
    m_track_id     = "";
    m_gp_id        = "";
    m_version      = 0;
    m_num_trophies = 0;
    m_num_completed_challenges = 0;
    m_is_unlock_list  = false;
    m_is_ghost_replay = false;
    m_unlock_special_type = SPECIAL_NONE;
    m_unlock_special_value = -1;

    for (int d=0; d<RaceManager::DIFFICULTY_COUNT; d++)
    {
        m_default_num_karts[d] = -1;
        m_position[d]  = -1;
        m_time[d]      = -1.0f;
        m_energy[d]    = -1;
        m_ai_superpower[d] = RaceManager::SUPERPOWER_NONE;
    }

    // we are using auto_ptr to make sure the XML node is released when leaving
    // the scope
    std::unique_ptr<XMLNode> root(new XMLNode( filename ));

    if(root.get() == NULL || root->getName()!="challenge")
    {
        std::ostringstream msg;
        msg << "Couldn't load challenge '" << filename
            << "': no challenge node.";
        throw std::runtime_error(msg.str());
    }

    setChallengeId(StringUtils::removeExtension(StringUtils::getBasename(filename)));

    root->get("version", &m_version);
    // No need to get the rest of the data if this challenge
    // is not supported anyway (id is needed for warning message)
    if(!unlock_manager->isSupportedVersion(*this))
    {
        Log::warn("ChallengeData", "Challenge <%s> is older "
                  "or newer than this version of STK, will be ignored.\n",
                  filename.c_str());
        return;
    }

    m_is_unlock_list = false;
    const XMLNode* unlock_list_node = root->getNode("unlock_list");
    if (unlock_list_node != NULL)
    {
        std::string list;
        unlock_list_node->get("list", &list);
        m_is_unlock_list = (list=="true");
    }

    std::vector<XMLNode*> unlocks;
    root->getNodes("unlock", unlocks);
    for(unsigned int i=0; i<unlocks.size(); i++)
    {
        std::string s;
        if(unlocks[i]->get("kart", &s))
            setUnlocks(s, ChallengeData::UNLOCK_KART);
        else if(unlocks[i]->get("track", &s))
            addUnlockTrackReward(s);
        else if(unlocks[i]->get("gp", &s))
            setUnlocks(s, ChallengeData::UNLOCK_GP);
        else if(unlocks[i]->get("mode", &s))
            setUnlocks(s, ChallengeData::UNLOCK_MODE);
        else if(unlocks[i]->get("difficulty", &s))
            setUnlocks(s, ChallengeData::UNLOCK_DIFFICULTY);
        else
        {
            Log::warn("ChallengeData", "Unknown unlock entry. Must be one of kart, track, gp, mode, difficulty.");
            throw std::runtime_error("Unknown unlock entry");
        }
    }

    const XMLNode* requirements_node = root->getNode("requirements");
    if (requirements_node == NULL)
    {
        throw std::runtime_error("Challenge file " + filename +
                                 " has no <requirements> node!");
    }
    requirements_node->get("trophies", &m_num_trophies);

    requirements_node->get("challenges", &m_num_completed_challenges);

    if (m_is_unlock_list)
    {
        requirements_node = root->getNode("alt_requirements");
        if (requirements_node != NULL)
        {
            if(requirements_node->get("max-req-in-lower-diff", &m_unlock_special_value))
                m_unlock_special_type = SPECIAL_MAX_REQ_IN_LOWER_DIFF;
        }
    }

    //Don't check further if this is an unlock list
    if(m_is_unlock_list)
        return;

    const XMLNode* mode_node = root->getNode("mode");
    if (mode_node == NULL)
    {
        throw std::runtime_error("Challenge file " + filename +
                                 " has no <mode> node!");
    }

    std::string mode;
    mode_node->get("major", &mode);

    if(mode=="grandprix")
        m_mode = CM_GRAND_PRIX;
    else if(mode=="single")
        m_mode = CM_SINGLE_RACE;
    else if(mode=="any")
        m_mode = CM_ANY;
    else
        error("major");

    mode_node->get("minor", &mode);
    if(mode=="timetrial")
        m_minor = RaceManager::MINOR_MODE_TIME_TRIAL;
    else if(mode=="quickrace")
        m_minor = RaceManager::MINOR_MODE_NORMAL_RACE;
    else if(mode=="followtheleader")
        m_minor = RaceManager::MINOR_MODE_FOLLOW_LEADER;
    else
        error("minor");

    const XMLNode* track_node = root->getNode("track");
    const XMLNode* gp_node = root->getNode("grandprix");

    if (m_mode == CM_SINGLE_RACE && track_node == NULL)
    {
        throw std::runtime_error("Challenge file " + filename +
                                 " has no <track> node!");
    }
    if (m_mode == CM_GRAND_PRIX && gp_node == NULL)
    {
        throw std::runtime_error("Challenge file " + filename +
                                 " has no <grandprix> node!");
    }

    if (track_node != NULL)
    {
        if (!track_node->get("id",  &m_track_id ))
        {
            error("track");
        }
        if (track_manager->getTrack(m_track_id) == NULL)
        {
            error("track");
        }

        if (!track_node->get("laps", &m_num_laps) &&
            m_minor != RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            error("laps");
        }
        if (!track_node->get("reverse", &m_reverse))
        {
            Log::warn("Challenge Data",
                      "No reverse mode specified for challenge %s, defaulting to normal",
                      filename.c_str());
        }
    }
    else if (gp_node != NULL)
    {
        if (!gp_node->get("id",  &m_gp_id ))
        {
            error("grandprix");
        }
    }

    const XMLNode* difficulties[RaceManager::DIFFICULTY_COUNT];
    difficulties[0] = root->getNode("easy");
    difficulties[1] = root->getNode("medium");
    difficulties[2] = root->getNode("hard");
    difficulties[3] = root->getNode("best");

    if (difficulties[0] == NULL || difficulties[1] == NULL ||
        difficulties[2] == NULL || difficulties[3] == NULL)
    {
        error("<easy> or <medium> or <hard> or <best>");
    }

    for (int d=0; d<=RaceManager::DIFFICULTY_BEST; d++)
    {
        const XMLNode* karts_node = difficulties[d]->getNode("karts");
        if (karts_node == NULL) error("<karts .../>");

        int num_karts = -1;
        if (!karts_node->get("number", &num_karts)) error("karts");
        m_default_num_karts[d] = num_karts;

        std::string replay_file;
        if (karts_node->get("replay_file", &replay_file))
        {
            m_is_ghost_replay = true;
            m_replay_files[d] = replay_file;
        }

        std::string ai_kart_ident;
        if (karts_node->get("aiIdent", &ai_kart_ident))
            m_ai_kart_ident[d] = ai_kart_ident;

        std::string superPower;
        if (karts_node->get("superPower", &superPower))
        {
            if (superPower == "nolokBoss")
            {
                m_ai_superpower[d] = RaceManager::SUPERPOWER_NOLOK_BOSS;
            }
            else
            {
                Log::warn("ChallengeData", "Unknown superpower '%s'",
                          superPower.c_str());
            }
        }

        const XMLNode* requirements_node =
                                   difficulties[d]->getNode("requirements");
        if (requirements_node == NULL) error("<requirements .../>");

        int position = -1;
        if (!requirements_node->get("position", &position) &&
            (m_minor == RaceManager::MINOR_MODE_FOLLOW_LEADER ||
             m_mode  == CM_GRAND_PRIX))
        {
            error("position");
        }
        else
        {
            m_position[d] = position;
        }

        int time = -1;
        if (requirements_node->get("time", &time)) m_time[d] = (float)time;

        if (m_time[d] < 0 && m_position[d] < 0) error("position/time");

        // This is optional
        int energy = -1;
        if (requirements_node->get("energy", &energy)) m_energy[d] = energy;
    }
}   // ChallengeData

// ----------------------------------------------------------------------------

const irr::core::stringw ChallengeData::getChallengeDescription() const
{
    core::stringw description;

    if (m_is_unlock_list)
        return description;

    if (m_mode == CM_GRAND_PRIX)
    {
        if (m_minor == RaceManager::MINOR_MODE_NORMAL_RACE)
            description = _("Normal Race (Grand Prix)");
        else if (m_minor == RaceManager::MINOR_MODE_TIME_TRIAL)
            description = _("Time-Trial (Grand Prix)");
    }
    else if (!m_track_id.empty())
    {
        if (m_is_ghost_replay)
            description = _("Time-Trial - beat the replay");
        else if (m_energy[0] > 0)
            description = _("Time-Trial - nitro challenge");
        else if (m_minor == RaceManager::MINOR_MODE_NORMAL_RACE)
            description = _("Normal Race (single race)");
        else if (m_minor == RaceManager::MINOR_MODE_TIME_TRIAL)
            description = _("Time-Trial (single race)");
        else if (m_minor == RaceManager::MINOR_MODE_FOLLOW_LEADER)
            description = _("Follow the Leader (single race)");
    }

    if (m_reverse == true)
    {
        description += core::stringw(L"\n");
        description += _("Mode: Reverse");
    }
    return description;
}   // getChallengeDescription

// ----------------------------------------------------------------------------
void ChallengeData::error(const char *id) const
{
    std::ostringstream msg;
    msg << "Undefined or incorrect value for '" << id
        << "' in challenge file '" << m_filename << "'.";

    Log::error("ChallengeData", "%s", msg.str().c_str());

    throw std::runtime_error(msg.str());
}   // error

// ----------------------------------------------------------------------------
/** Checks if this challenge is valid, i.e. contains a valid track or a valid
 *  GP. If incorrect data are found, STK is aborted with an error message.
 *  (otherwise STK aborts when trying to do this challenge, which is worse).
 */
void ChallengeData::check() const
{
    if(m_mode==CM_SINGLE_RACE)
    {
        try
        {
            track_manager->getTrack(m_track_id);
        }
        catch(std::exception&)
        {
            error("track");
        }
    }
    else if(m_mode==CM_GRAND_PRIX)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(m_gp_id);

        if (gp == NULL)
        {
            error("gp");
        }
        const bool gp_ok = gp->checkConsistency(false);
        if (!gp_ok)
        {
            error("gp");
        }
    }
}   // check

// ----------------------------------------------------------------------------
/** Adds all rewards for fulfilling this challenge.
 *  \param id Name of track or gp or kart or mode or difficulty reward.
 *  \param reward Type of reward (track, gp, mode, difficulty, kart).
 */
void ChallengeData::setUnlocks(const std::string &id, RewardType reward)
{
    if (id.empty()) return;

    switch(reward)
    {
    case UNLOCK_TRACK:      assert (false);
        break;

    case UNLOCK_GP:         addUnlockGPReward(id);
                            break;

    case UNLOCK_MODE:       {
                            const RaceManager::MinorRaceModeType mode =
                                RaceManager::getModeIDFromInternalName(id);
                            addUnlockModeReward(id,
                                                RaceManager::getNameOf(mode));
                            break;
                            }
    case UNLOCK_DIFFICULTY:
                            {
                            addUnlockDifficultyReward(id, core::stringw(id.c_str()));
                            break;
                            }
    case UNLOCK_KART:       {
                            const KartProperties* prop =
                                kart_properties_manager->getKart(id);
                            if (prop == NULL)
                            {
                                Log::warn("ChallengeData", "Challenge refers to kart %s, "
                                          "which is unknown. Ignoring reward.",
                                          id.c_str());
                                break;
                            }
                            irr::core::stringw user_name = prop->getName();
                            addUnlockKartReward(id, user_name);
                            break;
                            }
    }   // switch
}   // setUnlocks

// ----------------------------------------------------------------------------
void ChallengeData::setRace(RaceManager::Difficulty d) const
{
    if(m_mode==CM_GRAND_PRIX)
        RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
    else if(m_mode==CM_SINGLE_RACE)
        RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);
    else
    {
        Log::error("challenge_data", "Invalid mode %d in setRace.", m_mode);
        assert(false);
    }

    if(m_mode==CM_SINGLE_RACE)
    {
        RaceManager::get()->setMinorMode(m_minor);
        RaceManager::get()->setTrack(m_track_id);
        RaceManager::get()->setNumLaps(m_num_laps);
        RaceManager::get()->setReverseTrack(m_reverse);
        RaceManager::get()->setNumKarts(m_default_num_karts[d]);
        RaceManager::get()->setNumPlayers(1);
        RaceManager::get()->setCoinTarget(m_energy[d]);
        RaceManager::get()->setDifficulty(d);

        if (m_time[d] >= 0.0f)
        {
            RaceManager::get()->setTimeTarget(m_time[d]);
        }
        else
        {
            RaceManager::get()->setTimeTarget(0.0f);
        }
    }
    else if(m_mode==CM_GRAND_PRIX)
    {
        RaceManager::get()->setMinorMode(m_minor);
        RaceManager::get()->setGrandPrix(*grand_prix_manager->getGrandPrix(m_gp_id));
        RaceManager::get()->setDifficulty(d);
        RaceManager::get()->setNumKarts(m_default_num_karts[d]);
        RaceManager::get()->setNumPlayers(1);
    }

    if (m_is_ghost_replay)
    {
        const bool result = ReplayPlay::get()->addReplayFile(file_manager
            ->getAsset(FileManager::REPLAY, m_replay_files[d]),
            true/*custom_replay*/);
        if (!result)
            Log::fatal("ChallengeData", "Can't open replay for challenge!");
        RaceManager::get()->setRaceGhostKarts(true);
    }

    if (m_ai_kart_ident[d] != "")
    {
        RaceManager::get()->setAIKartOverride(m_ai_kart_ident[d]);
    }
    if (m_ai_superpower[d] != RaceManager::SUPERPOWER_NONE)
    {
        RaceManager::get()->setAISuperPower(m_ai_superpower[d]);
    }
}   // setRace

// ----------------------------------------------------------------------------
/** Returns true if this (non-GP) challenge is fulfilled.
 *  \param check_best : if true, check if the requirement
 *         for the best difficulty are met at a lower one.
 *         (requires SuperTux challenges to have a time
 ù          requirement to make sense)
 */
bool ChallengeData::isChallengeFulfilled(bool check_best) const
{
    // GP's use the grandPrixFinished() function,
    // so they can't be fulfilled here.
    if(m_mode==CM_GRAND_PRIX) return false;

    // Single races
    // ------------
    World *world = World::getWorld();
    std::string track_name = Track::getCurrentTrack()->getIdent();

    int d = (check_best) ? RaceManager::DIFFICULTY_BEST :
                           RaceManager::get()->getDifficulty();

    AbstractKart* kart = world->getPlayerKart(0);

    if (kart->isEliminated()                                               ) return false;
    if (track_name != m_track_id                                           ) return false;
    if (((int)world->getNumKarts() < m_default_num_karts[d]) && !check_best) return false;
    if (m_energy[d] > 0   && kart->getEnergy() < m_energy[d]               ) return false;
    if (m_position[d] > 0 && kart->getPosition() > m_position[d]           ) return false;

    // Follow the leader
    // -----------------
    if(m_minor==RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {
        // All possible conditions were already checked, so:
        // must have been successful
        return true;
    }
    // Quickrace / Timetrial
    // ---------------------
    // FIXME - encapsulate this better, each race mode needs to be able
    // to specify its own challenges and deal with them
    LinearWorld* lworld = dynamic_cast<LinearWorld*>(world);
    if(lworld != NULL)
    {
        // wrong number of laps
        if(lworld->getLapForKart( kart->getWorldKartId() ) != m_num_laps)
            return false;
    }
    // too slow
    if (m_time[d] > 0.0f && kart->getFinishTime() > m_time[d]) return false;

    // too slow
    if (m_is_ghost_replay)
    {
        ReplayPlay::get()->addReplayFile(file_manager
            ->getAsset(FileManager::REPLAY, m_replay_files[d]),
            true/*custom_replay*/);
        const ReplayPlay::ReplayData& rd = ReplayPlay::get()->getCurrentReplayData();
        if (kart->getFinishTime() > rd.m_min_time)
            return false;
    }

    if (m_ai_superpower[d] != RaceManager::SUPERPOWER_NONE &&
        RaceManager::get()->getAISuperPower() != m_ai_superpower[d])
    {
        return false;
    }

    return true;
}   // isChallengeFulfilled

// ----------------------------------------------------------------------------
/** Returns true if this GP challenge is fulfilled.
 */
ChallengeData::GPLevel ChallengeData::isGPFulfilled() const
{
    int d = RaceManager::get()->getDifficulty();

    // Note that we have to call RaceManager::get()->getNumKarts, since there
    // is no world objects to query at this stage.
    if (RaceManager::get()->getMajorMode()  != RaceManager::MAJOR_MODE_GRAND_PRIX  ||
        RaceManager::get()->getMinorMode()  != m_minor                             ||
        RaceManager::get()->getGrandPrix().getId() != m_gp_id                      ||
        RaceManager::get()->getNumberOfKarts() < (unsigned int)m_default_num_karts[d]      ||
        RaceManager::get()->getNumPlayers() > 1) return GP_NONE;

    // check if the player came first.
    // rank == 0 if first, 1 if second, etc.
    const int rank = RaceManager::get()->getLocalPlayerGPRank(0);

    // In superior difficulty levels, losing a place means
    // getting a cup of the inferior level rather than
    // nothing at all
    int unlock_level = d - rank;
    if (unlock_level == 3)
        return GP_BEST;
    if (unlock_level == 2)
        return GP_HARD;
    if (unlock_level == 1)
        return GP_MEDIUM;
    if (unlock_level == 0)
        return GP_EASY;
    return GP_NONE;
}   // isGPFulfilled

// ----------------------------------------------------------------------------
const irr::core::stringw
                   ChallengeData::UnlockableFeature::getUnlockedMessage() const
{
    switch (m_type)
    {
        case UNLOCK_TRACK:
        {    // {} avoids compiler warning
            const Track* track = track_manager->getTrack(m_name);

            // shouldn't happen but let's avoid crashes as much as possible...
            if (track == NULL) return irr::core::stringw( L"????" );

            return _("New track '%s' now available", track->getName());
        }
        case UNLOCK_MODE:
        {
            return _("New game mode '%s' now available", m_user_name);
        }
        case UNLOCK_GP:
        {
            const GrandPrixData* gp = grand_prix_manager->getGrandPrix(m_name);

            // shouldn't happen but let's avoid crashes as much as possible...
            if (gp == NULL) return irr::core::stringw( L"????" );

            const irr::core::stringw& gp_user_name = gp->getName();
            return _("New Grand Prix '%s' now available", gp_user_name);
        }
        case UNLOCK_DIFFICULTY:
        {
            return _("New difficulty '%s' now available", m_user_name);
        }
        case UNLOCK_KART:
        {
            const KartProperties* kp =
            kart_properties_manager->getKart(m_name);

            // shouldn't happen but let's avoid crashes as much as possible...
            if (kp == NULL) return irr::core::stringw( L"????" );

            return _("New kart '%s' now available", kp->getName());
        }
        default:
            assert(false);
            return L"";
    }   // switch
}   // UnlockableFeature::getUnlockedMessage

//-----------------------------------------------------------------------------
/** Sets that the given track will be unlocked if this challenge
 *  is unlocked.
 *  \param track_name Name of the track to unlock.
 */
void ChallengeData::addUnlockTrackReward(const std::string &track_name)
{

    if (track_manager->getTrack(track_name) == NULL)
    {
        throw std::runtime_error(
            StringUtils::insertValues("Challenge refers to unknown track <%s>",
                                      track_name.c_str()));
    }

    UnlockableFeature feature;
    feature.m_name = track_name;
    feature.m_type = UNLOCK_TRACK;
    m_feature.push_back(feature);
}   // addUnlockTrackReward

//-----------------------------------------------------------------------------

void ChallengeData::addUnlockModeReward(const std::string &internal_mode_name,
                                      const irr::core::stringw &user_mode_name)
{
    UnlockableFeature feature;
    feature.m_name = internal_mode_name;
    feature.m_type = UNLOCK_MODE;
    feature.m_user_name = user_mode_name;
    m_feature.push_back(feature);
}   // addUnlockModeReward

//-----------------------------------------------------------------------------
void ChallengeData::addUnlockGPReward(const std::string &gp_name)
{
    if (grand_prix_manager->getGrandPrix(gp_name) == NULL)
    {
        throw std::runtime_error(
            StringUtils::insertValues(
                           "Challenge refers to unknown Grand Prix <%s>",
                           gp_name.c_str()));
    }

    UnlockableFeature feature;

    feature.m_name = gp_name.c_str();

    feature.m_type = UNLOCK_GP;
    m_feature.push_back(feature);
}   // addUnlockGPReward

//-----------------------------------------------------------------------------

void ChallengeData::addUnlockDifficultyReward(const std::string &internal_name,
                                           const irr::core::stringw &user_name)
{
    UnlockableFeature feature;
    feature.m_name = internal_name;
    feature.m_type = UNLOCK_DIFFICULTY;
    feature.m_user_name = user_name;
    m_feature.push_back(feature);
}   // addUnlockDifficultyReward

//-----------------------------------------------------------------------------
void ChallengeData::addUnlockKartReward(const std::string &internal_name,
                                        const irr::core::stringw &user_name)
{
    try
    {
        kart_properties_manager->getKartId(internal_name);
    }
    catch (std::exception& e)
    {
        (void)e;   // avoid compiler warning
        throw std::runtime_error(
            StringUtils::insertValues("Challenge refers to unknown kart <%s>",
                                       internal_name.c_str()));
    }

    UnlockableFeature feature;
    feature.m_name = internal_name;
    feature.m_type = UNLOCK_KART;
    feature.m_user_name = user_name;
    m_feature.push_back(feature);
}   // addUnlockKartReward
