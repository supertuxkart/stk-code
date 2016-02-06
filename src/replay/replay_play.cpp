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
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"

#include <stdio.h>
#include <string>

ReplayPlay *ReplayPlay::m_replay_play = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
ReplayPlay::ReplayPlay()
{
    m_next = 0;
    m_ghost_karts_list.clear();
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
    m_next = 0;
    m_ghost_karts_list.clear();
    for(unsigned int i=0; i<(unsigned int)m_ghost_karts.size(); i++)
    {
        m_ghost_karts[i].reset();
    }
}   // reset

//-----------------------------------------------------------------------------
/** Updates all ghost karts.
 *  \param dt Time step size.
 */
void ReplayPlay::update(float dt)
{
    // First update all ghost karts
    for(unsigned int i=0; i<(unsigned int)m_ghost_karts.size(); i++)
        m_ghost_karts[i].update(dt);

}   // update

//-----------------------------------------------------------------------------
/** Loads the ghost karts info in the replay file, required by race manager.
 */
void ReplayPlay::loadKartInfo()
{
    char s[1024];

    FILE *fd = openReplayFile(/*writeable*/false);
    if(!fd)
    {
        Log::error("Replay", "Can't read '%s', ghost replay disabled.",
               getReplayFilename().c_str());
        destroy();
        return;
    }

    Log::info("Replay", "Reading ghost karts info");
    while(true)
    {
        if (fgets(s, 1023, fd) == NULL)
            Log::fatal("Replay", "Could not read '%s'.", getReplayFilename().c_str());
        std::string is_end = std::string(s);
        if (is_end == "kart_list_end\n" || is_end == "kart_list_end\r\n") break;
        char s1[1024];

        if (sscanf(s,"kart: %s", s1) != 1)
            Log::fatal("Replay", "Could not read ghost karts info!");

        m_ghost_karts_list.push_back(std::string(s1));
    }
    fclose(fd);
}   // loadKartInfo

//-----------------------------------------------------------------------------
/** Loads a replay data from  file called 'trackname'.replay.
 */
void ReplayPlay::load()
{
    m_ghost_karts.clearAndDeleteAll();
    char s[1024], s1[1024];

    FILE *fd = openReplayFile(/*writeable*/false);
    if(!fd)
    {
        Log::error("Replay", "Can't read '%s', ghost replay disabled.",
               getReplayFilename().c_str());
        destroy();
        return;
    }

    Log::info("Replay", "Reading replay file '%s'.", getReplayFilename().c_str());
    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("Replay", "Could not read '%s'.", getReplayFilename().c_str());

    for (unsigned int i = 0; i < m_ghost_karts_list.size(); i++)
    {
        if (fgets(s, 1023, fd) == NULL)
            Log::fatal("Replay", "Could not read '%s'.", getReplayFilename().c_str());
        // Skip kart info which is already read.
    }
    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("Replay", "Could not read '%s'.", getReplayFilename().c_str());
    // Skip kart_list_end

    unsigned int version;
    if (sscanf(s,"version: %u", &version) != 1)
        Log::fatal("Replay", "No Version information found in replay file (bogus replay file).");

    if (version != getReplayVersion())
    {
        Log::warn("Replay", "Replay is version '%d'",version);
        Log::warn("Replay", "STK version is '%d'",getReplayVersion());
        Log::warn("Replay", "We try to proceed, but it may fail.");
    }

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("Replay", "Could not read '%s'.", getReplayFilename().c_str());

    int n;
    if(sscanf(s, "difficulty: %d", &n) != 1)
        Log::fatal("Replay", " No difficulty found in replay file.");

    if(race_manager->getDifficulty()!=(RaceManager::Difficulty)n)
        Log::warn("Replay", "Difficulty of replay is '%d', "
                  "while '%d' is selected.",
                  race_manager->getDifficulty(), n);

    fgets(s, 1023, fd);
    if(sscanf(s, "track: %s", s1) != 1)
        Log::warn("Replay", "Track not found in replay file.");
    assert(std::string(s1)==race_manager->getTrackName());
    race_manager->setTrack(s1);

    unsigned int num_laps;
    fgets(s, 1023, fd);
    if(sscanf(s, "laps: %u", &num_laps) != 1)
        Log::fatal("Replay", "No number of laps found in replay file.");

    race_manager->setNumLaps(num_laps);

    // eof actually doesn't trigger here, since it requires first to try
    // reading behind eof, but still it's clearer this way.
    while(!feof(fd))
    {
        if(fgets(s, 1023, fd)==NULL)  // eof reached
            break;
        readKartData(fd, s);
    }

    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // load

//-----------------------------------------------------------------------------
/** Reads all data from a replay file for a specific kart.
 *  \param fd The file descriptor from which to read.
 */
void ReplayPlay::readKartData(FILE *fd, char *next_line)
{
    char s[1024];
    const unsigned int kart_num = m_ghost_karts.size();
    m_ghost_karts.push_back(new GhostKart(m_ghost_karts_list.at(kart_num),
        kart_num, kart_num + 1));
    m_ghost_karts[kart_num].init(RaceManager::KT_GHOST);

    unsigned int size;
    if(sscanf(next_line,"size: %u",&size)!=1)
        Log::fatal("Replay", "Number of records not found in replay file "
            "for kart %d.",
            m_ghost_karts.size()-1);

    for(unsigned int i=0; i<size; i++)
    {
        fgets(s, 1023, fd);
        float x, y, z, rx, ry, rz, rw, time, speed, steer, w1, w2, w3, w4;
        int nitro, zipper;

        // Check for EV_TRANSFORM event:
        // -----------------------------
        if(sscanf(s, "%f  %f %f %f  %f %f %f %f  %f  %f  %f %f %f %f  %d  %d\n",
            &time,
            &x, &y, &z,
            &rx, &ry, &rz, &rw,
            &speed, &steer, &w1, &w2, &w3, &w4,
            &nitro, &zipper
            )==16)
        {
            btQuaternion q(rx, ry, rz, rw);
            btVector3 xyz(x, y, z);
            PhysicInfo pi = {0};
            KartReplayEvent kre = {0};
            pi.m_speed = speed;
            pi.m_steer = steer;
            pi.m_suspension_length[0] = w1;
            pi.m_suspension_length[1] = w2;
            pi.m_suspension_length[2] = w3;
            pi.m_suspension_length[3] = w4;
            kre.m_on_nitro = (bool)nitro;
            kre.m_on_zipper = (bool)zipper;
            m_ghost_karts[m_ghost_karts.size()-1].addReplayEvent(time,
                btTransform(q, xyz), pi, kre);
        }
        else
        {
            // Invalid record found
            // ---------------------
            Log::warn("Replay", "Can't read replay data line %d:", i);
            Log::warn("Replay", "%s", s);
            Log::warn("Replay", "Ignored.");
        }
    }   // for i

}   // readKartData
