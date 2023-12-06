//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#include "replay/replay_play.hpp"

#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/controller/ghost_controller.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/mem_utils.hpp"
#include "utils/string_utils.hpp"

#include <stdio.h>
#include <string>
#include <cinttypes>

ReplayPlay::SortOrder ReplayPlay::m_sort_order = ReplayPlay::SO_DEFAULT;
ReplayPlay *ReplayPlay::m_replay_play = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayPlay::ReplayPlay()
{
    m_current_replay_file   = 0;
    m_second_replay_file    = 0;
    m_second_replay_enabled = false;
}   // ReplayPlay

//-----------------------------------------------------------------------------
/** Frees all stored data. */
ReplayPlay::~ReplayPlay()
{
}   // ~Replay

//-----------------------------------------------------------------------------
/** Resets all ghost karts back to start position.
 */
void ReplayPlay::reset()
{
    for(unsigned int i=0; i<(unsigned int)m_ghost_karts.size(); i++)
    {
        m_ghost_karts[i]->reset();
    }
}   // reset

//-----------------------------------------------------------------------------
void ReplayPlay::loadAllReplayFile()
{
    m_replay_file_list.clear();

    // Load stock replay first
    std::set<std::string> pre_record;
    file_manager->listFiles(pre_record, file_manager
        ->getAssetDirectory(FileManager::REPLAY), /*is_full_path*/ true);
    for (std::set<std::string>::iterator i  = pre_record.begin();
                                         i != pre_record.end(); ++i)
    {
        if (!addReplayFile(*i, /*custom_replay*/ true))
        {
            // Skip invalid replay file
            continue;
        }
    }

    // Now user recorded replay
    std::set<std::string> files;
    file_manager->listFiles(files, file_manager->getReplayDir(),
        /*is_full_path*/ false);

    int j=0;

    for (std::set<std::string>::iterator i  = files.begin();
                                         i != files.end(); ++i)
    {
        if (!addReplayFile(*i, false, j))
        {
            // Skip invalid replay file
            continue;
        }
        j++;
    }

}   // loadAllReplayFile

//-----------------------------------------------------------------------------
bool ReplayPlay::addReplayFile(const std::string& fn, bool custom_replay, int call_index)
{

    char s[1024], s1[1024];
    if (StringUtils::getExtension(fn) != "replay") return false;
    FILE* fd = FileUtils::fopenU8Path(custom_replay ? fn :
        file_manager->getReplayDir() + fn, "r");
    if (fd == NULL) return false;
    auto scoped = [&]() { fclose(fd); };
    MemUtils::deref<decltype(scoped)> cls(scoped); 
    ReplayData rd;

    // custom_replay is true when full path of filename is given
    rd.m_custom_replay_file = custom_replay;
    rd.m_filename = fn;

    fgets(s, 1023, fd);
    unsigned int version;
    if (sscanf(s,"version: %u", &version) != 1)
    {
        Log::warn("Replay", "No Version information "
                  "found in replay file (bogus replay file).");
        return false;
    }
    if ( version < getMinSupportedReplayVersion() )
    {
        Log::warn("Replay", "Replay is version '%d', Minimum supported replay version is '%d', skipped '%s'",
                  version, getMinSupportedReplayVersion(), fn.c_str());
        return false;
    }
    else if (version > getCurrentReplayVersion())
    {
        Log::warn("Replay", "Replay is version '%d', STK replay version is '%d', skipped '%s'",
                  version, getCurrentReplayVersion(), fn.c_str());
        return false;
    }

    rd.m_replay_version = version;

    if (version >= 4)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "stk_version: %1023s", s1) != 1)
        {
            Log::warn("Replay", "No STK release version found in replay file, '%s'.", fn.c_str());
            return false;
        }
        rd.m_stk_version = s1;
    }
    else
        rd.m_stk_version = "";

    while(true)
    {
        fgets(s, 1023, fd);
        core::stringc is_end(s);
        is_end.trim();
        if (is_end == "kart_list_end") break;
        char s1[1024];
        char display_name_encoded[1024];

        int scanned = sscanf(s,"kart: %1023s %1023[^\n]", s1, display_name_encoded);
        if (scanned < 1)
        {
            Log::warn("Replay", "Could not read ghost karts info!");
            break;
        }

        rd.m_kart_list.push_back(std::string(s1));
        if (scanned == 2)
        {
            // If username of kart is present, use it
            rd.m_name_list.push_back(StringUtils::xmlDecode(std::string(display_name_encoded)));
            if (rd.m_name_list.size() == 1)
            {
                // First user is the game master and the "owner" of this replay file
                rd.m_user_name = rd.m_name_list[0];
            }
        } else
        { // scanned == 1
            // If username is not present, kart display name will default to kart name
            // (see GhostController::getName)
            rd.m_name_list.push_back("");
        }

        // Read kart color data
        if (version >= 4)
        {
            float f = 0;
            fgets(s, 1023, fd);
            if(sscanf(s, "kart_color: %f", &f) != 1)
            {
                Log::warn("Replay", "Kart color missing in replay file, '%s'.", fn.c_str());
                return false;
            }
            rd.m_kart_color.push_back(f);
        }
        else
            rd.m_kart_color.push_back(0.0f); // Use default kart color
    }

    int reverse = 0;
    fgets(s, 1023, fd);
    if(sscanf(s, "reverse: %d", &reverse) != 1)
    {
        Log::warn("Replay", "No reverse info found in replay file, '%s'.", fn.c_str());
        return false;
    }
    rd.m_reverse = reverse != 0;

    fgets(s, 1023, fd);
    if (sscanf(s, "difficulty: %u", &rd.m_difficulty) != 1)
    {
        Log::warn("Replay", " No difficulty found in replay file, '%s'.", fn.c_str());
        return false;
    }

    if (version >= 4)
    {
        fgets(s, 1023, fd);
        if (sscanf(s, "mode: %1023s", s1) != 1)
        {
            Log::warn("Replay", "Replay mode not found in replay file, '%s'.", fn.c_str());
            return false;
        }
        rd.m_minor_mode = s1;
    }
    // Assume time-trial mode for old replays
    else
        rd.m_minor_mode = "time-trial";

    // sscanf always stops at whitespaces, but a track name may contain a whitespace
    // Official tracks should avoid whitespaces in their name, but it
    // unavoidably occurs with some addons or WIP tracks.
    fgets(s, 1023, fd);
    if (std::strncmp(s, "track: ", 7) == 0)
    {
        int i = 0;
        for(i = 7; s[i] != '\0'; i++)
        {
            // Break when newline is reached
            if (s[i] == '\n' || s[i] == '\r')
                break;
            s1[i-7] = s[i];
        }
        s1[i-7] = '\0';

        if (i >= 8)
        {
            rd.m_track_name = std::string(s1);
        }
        else
        {
            Log::warn("Replay", "Track name is empty in replay file, '%s'.", fn.c_str());
            return false;
        }         
    }
    else
    {
        Log::warn("Replay", "Track info not found in replay file, '%s'.", fn.c_str());
        return false;
    }

    // If former official tracks are present as addons, show the matching replays.
    if (rd.m_track_name.compare("greenvalley") == 0)
        rd.m_track_name = std::string("addon_green-valley");
    if (rd.m_track_name.compare("mansion") == 0)
        rd.m_track_name = std::string("addon_blackhill-mansion");

    Track* t = track_manager->getTrack(rd.m_track_name);
    if (t == NULL)
    {
        Log::warn("Replay", "Track '%s' used in replay '%s' not found in STK!",
        rd.m_track_name.c_str(), fn.c_str());
        return false;
    }

    rd.m_track = t;

    fgets(s, 1023, fd);
    if (sscanf(s, "laps: %u", &rd.m_laps) != 1)
    {
        Log::warn("Replay", "No number of laps found in replay file, '%s'.", fn.c_str());
        return false;
    }

    fgets(s, 1023, fd);
    if (sscanf(s, "min_time: %f", &rd.m_min_time) != 1)
    {
        Log::warn("Replay", "Finish time not found in replay file, '%s'.", fn.c_str());
        return false;
    }

    if (version >= 4)
    {
        fgets(s, 1023, fd);
        if (sscanf(s, "replay_uid: %" PRIu64, &rd.m_replay_uid) != 1)
        {
            Log::warn("Replay", "Replay UID not found in replay file, '%s'.", fn.c_str());
            return false;
        }
    }
    // No UID in old replay format
    else
        rd.m_replay_uid = call_index;

    m_replay_file_list.push_back(rd);

    assert(m_replay_file_list.size() > 0);
    // Force to use custom replay file immediately
    if (custom_replay)
        m_current_replay_file = (unsigned int)m_replay_file_list.size() - 1;

    return true;

}   // addReplayFile

//-----------------------------------------------------------------------------
void ReplayPlay::load()
{
    m_ghost_karts.clear();

    if (m_second_replay_enabled)
        loadFile(/* second replay */ true);

    // Always load the first replay
    loadFile(/* second replay */ false);
    
} // load

//-----------------------------------------------------------------------------
void ReplayPlay::loadFile(bool second_replay)
{
    char s[1024];

    int replay_index = second_replay ? m_second_replay_file : m_current_replay_file;
    int replay_file_number = second_replay ? 2 : 1;

    FILE *fd = openReplayFile(/*writeable*/false,
            m_replay_file_list.at(replay_index).m_custom_replay_file, replay_file_number);

    if(!fd)
    {
        Log::error("Replay", "Can't read '%s', ghost replay disabled.",
                    getReplayFilename(replay_file_number).c_str());
        destroy();
        return;
    }

    Log::info("Replay", "Reading replay file '%s'.",
                    getReplayFilename(replay_file_number).c_str());

    ReplayData &rd = m_replay_file_list[replay_index];
    unsigned int num_kart = (unsigned int)m_replay_file_list.at(replay_index)
                                                            .m_kart_list.size();
    unsigned int lines_to_skip = (rd.m_replay_version == 3) ? 7 : 10;
    lines_to_skip += (rd.m_replay_version == 3) ? num_kart : 2*num_kart;

    for (unsigned int i = 0; i < lines_to_skip; i++)
        fgets(s, 1023, fd);

    // eof actually doesn't trigger here, since it requires first to try
    // reading behind eof, but still it's clearer this way.
    while(!feof(fd))
    {
        if(fgets(s, 1023, fd)==NULL)  // eof reached
            break;
        readKartData(fd, s, second_replay);
    }

    fclose(fd);
}   // loadFile

//-----------------------------------------------------------------------------
/** Reads all data from a replay file for a specific kart.
 *  \param fd The file descriptor from which to read.
 */
void ReplayPlay::readKartData(FILE *fd, char *next_line, bool second_replay)
{
    char s[1024];

    int replay_index = second_replay ? m_second_replay_file
                                     : m_current_replay_file;

    const unsigned int kart_num = (unsigned int)m_ghost_karts.size();
    unsigned int first_loaded_f_num = 0;

    if (!second_replay && m_second_replay_enabled)
        first_loaded_f_num = (unsigned int)m_replay_file_list.at(m_second_replay_file)
                                                             .m_kart_list.size();

    ReplayData &rd = m_replay_file_list[replay_index];
    m_ghost_karts.push_back(std::make_shared<GhostKart>
        (rd.m_kart_list.at(kart_num-first_loaded_f_num), kart_num, kart_num + 1,
        rd.m_kart_color.at(kart_num-first_loaded_f_num), rd));
    m_ghost_karts[kart_num]->init(RaceManager::KT_GHOST);
    Controller* controller = new GhostController(getGhostKart(kart_num).get(),
                                                 rd.m_name_list[kart_num-first_loaded_f_num]);
    getGhostKart(kart_num)->setController(controller);

    unsigned int size;
    if(sscanf(next_line,"size: %u",&size)!=1)
        Log::fatal("Replay", "Number of records not found in replay file "
            "for kart %d.", kart_num);

    for(unsigned int i=0; i<size; i++)
    {
        fgets(s, 1023, fd);
        float x, y, z, rx, ry, rz, rw, time, speed, steer, w1, w2, w3, w4, nitro_amount, distance;
        int skidding_state, attachment, item_amount, item_type, special_value,
            nitro, zipper, skidding, red_skidding, jumping;

        // Check for EV_TRANSFORM event:
        // -----------------------------

        // Up to STK 0.9.3 replays
        if (rd.m_replay_version == 3)
        {
            if(sscanf(s, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f  %d %d %d %d %d\n",
                &time,
                &x, &y, &z,
                &rx, &ry, &rz, &rw,
                &speed, &steer, &w1, &w2, &w3, &w4,
                &nitro, &zipper, &skidding, &red_skidding, &jumping
                )==19)
            {
                btQuaternion q(rx, ry, rz, rw);
                btVector3 xyz(x, y, z);
                PhysicInfo pi             = {0};
                BonusInfo bi              = {0};
                KartReplayEvent kre       = {0};

                pi.m_speed                = speed;
                pi.m_steer                = steer;
                pi.m_suspension_length[0] = w1;
                pi.m_suspension_length[1] = w2;
                pi.m_suspension_length[2] = w3;
                pi.m_suspension_length[3] = w4;
                pi.m_skidding_state       = 0;    //not saved in version 3 replays
                bi.m_attachment           = 0;    //not saved in version 3 replays
                bi.m_nitro_amount         = 0;    //not saved in version 3 replays
                bi.m_item_amount          = 0;    //not saved in version 3 replays
                bi.m_item_type            = 0;    //not saved in version 3 replays
                bi.m_special_value        = 0;    //not saved in version 3 replays
                kre.m_distance            = 0.0f; //not saved in version 3 replays
                kre.m_nitro_usage         = nitro;
                kre.m_zipper_usage        = zipper!=0;
                kre.m_skidding_effect     = skidding;
                kre.m_red_skidding        = red_skidding!=0;
                kre.m_jumping             = jumping != 0;
                m_ghost_karts[kart_num]->addReplayEvent(time,
                    btTransform(q, xyz), pi, bi, kre);
            }
            else
            {
                // Invalid record found
                // ---------------------
                Log::warn("Replay", "Can't read replay data line %d:", i);
                Log::warn("Replay", "%s", s);
                Log::warn("Replay", "Ignored.");
            }
        }

        //version 4 replays (STK 0.9.4 and higher)
        else
        {
            if(sscanf(s, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f %d  %d %f %d %d %d  %f %d %d %d %d %d\n",
                &time,
                &x, &y, &z,
                &rx, &ry, &rz, &rw,
                &speed, &steer, &w1, &w2, &w3, &w4, &skidding_state,
                &attachment, &nitro_amount, &item_amount, &item_type, &special_value,
                &distance, &nitro, &zipper, &skidding, &red_skidding, &jumping
                )==26)
            {
                btQuaternion q(rx, ry, rz, rw);
                btVector3 xyz(x, y, z);
                PhysicInfo pi             = {0};
                BonusInfo bi              = {0};
                KartReplayEvent kre       = {0};

                pi.m_speed                = speed;
                pi.m_steer                = steer;
                pi.m_suspension_length[0] = w1;
                pi.m_suspension_length[1] = w2;
                pi.m_suspension_length[2] = w3;
                pi.m_suspension_length[3] = w4;
                pi.m_skidding_state       = skidding_state;
                bi.m_attachment           = attachment;
                bi.m_nitro_amount         = nitro_amount;
                bi.m_item_amount          = item_amount;
                bi.m_item_type            = item_type;
                bi.m_special_value        = special_value;
                kre.m_distance            = distance;
                kre.m_nitro_usage         = nitro;
                kre.m_zipper_usage        = zipper!=0;
                kre.m_skidding_effect     = skidding;
                kre.m_red_skidding        = red_skidding!=0;
                kre.m_jumping             = jumping != 0;
                m_ghost_karts[kart_num]->addReplayEvent(time,
                    btTransform(q, xyz), pi, bi, kre);
            }
            else
            {
                // Invalid record found
                // ---------------------
                Log::warn("Replay", "Can't read replay data line %d:", i);
                Log::warn("Replay", "%s", s);
                Log::warn("Replay", "Ignored.");
            }
        }
    }   // for i

}   // readKartData

//-----------------------------------------------------------------------------
/** call getReplayIdByUID and set the current replay file to the first one
 *  with a matching UID.
 *  \param uid The UID to match
 */
void ReplayPlay::setReplayFileByUID(uint64_t uid)
{
    m_current_replay_file = getReplayIdByUID(uid);
} //setReplayFileByUID

//-----------------------------------------------------------------------------
/** Search among replay file and return the first index with a matching UID.
 *  \param uid The UID to match
 */
unsigned int ReplayPlay::getReplayIdByUID(uint64_t uid)
{
    for(unsigned int i = 0; i < m_replay_file_list.size(); i++)
    {
        if (m_replay_file_list[i].m_replay_uid == uid)
        {
            return i;
        }
    }

    Log::error("Replay", "Replay with UID of %" PRIu64 " not found.", uid);
    return 0;
} //getReplayIdByUID
