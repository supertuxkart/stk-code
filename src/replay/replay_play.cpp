//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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
    m_next            = 0;
}   // ReplayPlay

//-----------------------------------------------------------------------------
/** Frees all stored data. */
ReplayPlay::~ReplayPlay()
{
}   // ~Replay

//-----------------------------------------------------------------------------
/** Starts replay from the replay file in the current directory.
 */
void ReplayPlay::init()
{
    m_next = 0;
    Load();
}   // init

//-----------------------------------------------------------------------------
/** Resets all ghost karts back to start position.
 */
void ReplayPlay::reset()
{
    m_next = 0;
    for(unsigned int i=0; i<m_ghost_karts.size(); i++)
    {
        m_ghost_karts[i]->reset();
    }
}   // reset

//-----------------------------------------------------------------------------
/** Updates all ghost karts.
 *  \param dt Time step size.
 */
void ReplayPlay::update(float dt)
{
    // First update all ghost karts
    for(unsigned int i=0; i<m_ghost_karts.size(); i++)
        m_ghost_karts[i]->update(dt);

}   // update

//-----------------------------------------------------------------------------
/** Loads a replay data from  file called 'trackname'.replay.
 */
void ReplayPlay::Load()
{
    for(unsigned int i=0; i<m_ghost_karts.size(); i++)
    {
        delete m_ghost_karts[i];
    }
    m_ghost_karts.clear();
    char s[1024], s1[1024];
    int  n;

    FILE *fd = openReplayFile(/*writeable*/false);
    if(!fd)
    {
        printf("Can't read '%s', ghost replay disabled.\n", 
               getReplayFilename().c_str());
        destroy();
        return;
    }

    printf("Reading replay file '%s'.\n", getReplayFilename().c_str());
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read '%s'.\n", 
                getReplayFilename().c_str());
        exit(-2);
    }
    
    int version;
    if (sscanf(s,"Version: %d", &version)!=1)
    {
        fprintf(stderr, "ERROR: no Version information found in replay file"
                        " (bogus replay file)\n");
        exit(-2);
    }

    if (version!=getReplayVersion())
    {
        fprintf(stderr, "WARNING: replay is version '%d'\n",version);
        fprintf(stderr, "         STK version is '%s'\n",getReplayVersion());
        fprintf(stderr, "         We try to proceed, but it may fail.\n");
    }
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read '%s'.\n", 
                getReplayFilename().c_str());
        exit(-2);
    }
    
    if(sscanf(s, "difficulty: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: No difficulty found in replay file.\n");
        exit(-2);
    }

    if(race_manager->getDifficulty()!=(RaceManager::Difficulty)n)
        printf("Warning, difficulty of replay is '%d', "
               "while '%d' is selected.\n", 
               race_manager->getDifficulty(), n);

    fgets(s, 1023, fd);
    if(sscanf(s, "track: %s",s1)!=1)
    {
        fprintf(stderr,"WARNING: Track not found in replay file.\n");
    }
    assert(std::string(s1)==race_manager->getTrackName());
    race_manager->setTrack(s1);

    unsigned int num_laps;
    fgets(s, 1023, fd);
    if(sscanf(s, "Laps: %d",&num_laps)!=1)
    {
        fprintf(stderr,"WARNING: No number of laps found in replay file.\n");
        exit(-2);
    }
    race_manager->setNumLaps(num_laps);

    fgets(s, 1023, fd);
    unsigned int num_ghost_karts;
    if(sscanf(s, "numkarts: %d",&num_ghost_karts)!=1)
    {
        fprintf(stderr,"WARNING: No number of karts found in replay file.\n");
        exit(-2);
    }

    for(unsigned int k=0; k<num_ghost_karts; k++)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "model %d: %s",&n, s1)!=2)
        {
            fprintf(stderr,
                    "WARNING: No model information for kart %d found.\n",
                    k);
            exit(-2);
        }
        if(n != k)
        {
            fprintf(stderr, 
                    "WARNING: Expected kart id %d, got %d - ignored.\n",
                    k, n);
        }
        m_ghost_karts.push_back(new GhostKart(std::string(s1)));

        fgets(s, 1023, fd);
        unsigned int size;
        if(sscanf(s,"size: %d",&size)!=1)
        {
            fprintf(stderr,
                    "WARNING: Number of records not found in replay file "
                    "for kart %d.\n",
                    k);
            exit(-2);
        }

        for(unsigned int i=0; i<size; i++)
        {
            fgets(s, 1023, fd);
            float x, y, z, rx, ry, rz, rw, time;

            // Check for EV_TRANSFORM event:
            // -----------------------------
            if(sscanf(s, "%f  %f %f %f  %f %f %f %f\n",
                &time, 
                &x, &y, &z,
                &rx, &ry, &rz, &rw
                )==8)
            {
                btQuaternion q(rx, ry, rz, rw);
                btVector3 xyz(x, y, z);
                m_ghost_karts[k]->addTransform(time, btTransform(q, xyz));
            }
            else
            {
                // Invalid record found
                // ---------------------
                fprintf(stderr, "Can't read replay data line %d:\n", i);
                fprintf(stderr, "%s", s);
                fprintf(stderr, "Ignored.\n");
            }
        }   // for k

    }   // for i<nKarts

    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // Load

