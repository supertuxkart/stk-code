//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "replay/replay.hpp"


#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/kart.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#include <stdio.h>
#include <string>

bool    Replay::m_do_replay = false;
Replay *Replay::m_replay    = NULL;

//-----------------------------------------------------------------------------
/** Initialises the Replay engine
 */
Replay::Replay()
{
    m_next            = 0;
    m_num_ghost_karts = 0;
}   // Replay

//-----------------------------------------------------------------------------
/** Frees all stored data. */
Replay::~Replay()
{
    m_events.clear();
}   // ~Replay

//-----------------------------------------------------------------------------
/** Starts replay from the replay file in the current directory.
 */
void Replay::initReplay()
{
    m_next = 0;
    Load();
}   // initReplayd

//-----------------------------------------------------------------------------
/** Initialise the replay for a new recording. It especially allocates memory
 *  to store the replay data.
 */
void Replay::initRecording()
{
    unsigned int size = stk_config->m_max_history
                      * race_manager->getNumberOfKarts();
    m_events.resize(size);
    m_next            = 0;
    m_ghost_karts.clear();
}   // initRecording

//-----------------------------------------------------------------------------
/** Resets all ghost karts back to start position.
 */
void Replay::reset()
{
    m_next = 0;
    for(unsigned int i=0; i<m_ghost_karts.size(); i++)
    {
        m_ghost_karts[i]->reset();
    }
}   // reset

//-----------------------------------------------------------------------------
/** Depending on mode either saves the data for the current time step, or 
 *  replays the data. 
 *  /param dt Time step.
 */
void Replay::update(float dt)
{
    if(m_do_replay)
        updateReplay(dt);
    else
        updateRecording(dt);
}   // update

//-----------------------------------------------------------------------------
/** Saves the current replay data.
 *  \param dt Time step size.
 */
void Replay::updateRecording(float dt)
{
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();

    if(m_next + num_karts>=m_events.size())
    {
        printf("Can't store more replay information.\n");
        return;
    }

    // Once we use interpolate results, we don't have to increase
    // m_next by num_karts, so count how often to increase
    unsigned int count = 0;
    for(unsigned int i=0; i<num_karts; i++)
    {
        ReplayEvent *p   = &(m_events[m_next+i]);
        const Kart *kart = world->getKart(i);
        p->m_time        = World::getWorld()->getTime();
        p->m_type        = ReplayEvent::EV_TRANSFORM;
        p->m_kart_id     = i;
        p->m_event.m_t   = kart->getTrans();
        count ++;
    }   // for i
    m_next += count;
}   // updateRecording

//-----------------------------------------------------------------------------
/** Updates all ghost karts.
 *  \param dt Time step size.
 */
void Replay::updateReplay(float dt)
{
    // First update all ghost karts
    for(unsigned int i=0; i<m_ghost_karts.size(); i++)
        m_ghost_karts[i]->update(dt);

}   // updateReplay

//-----------------------------------------------------------------------------
/** Saves the replay data stored in the internal data structures.
 */
void Replay::Save()
{
    std::string filename = file_manager->getConfigDir()+"/"
                         + race_manager->getTrackName()+".replay";
    FILE *fd = fopen(filename.c_str(),"w");
    if(!fd)
    {
        filename = race_manager->getTrackName()+".replay";
        fd = fopen(filename.c_str(), "w");
    }
    if(!fd)
    {
        printf("Can't open '%s' for writing - can't save replay data.\n",
               filename.c_str());
        return;
    }
    printf("Replay saved in '%s'.\n", filename.c_str());

    World *world   = World::getWorld();
    int  num_karts = world->getNumKarts();
    fprintf(fd, "Version:  %s\n",   STK_VERSION);
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());
    fprintf(fd, "Laps: %d\n",       race_manager->getNumLaps());
    fprintf(fd, "numkarts: %d\n",   num_karts);

    int k;
    for(k=0; k<num_karts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getIdent().c_str());
    }
    fprintf(fd, "size:     %d\n", m_next);

    for(unsigned int i=0; i<m_events.size(); i++)
    {
        const ReplayEvent *p=&(m_events[i]);
        if(p->m_type==ReplayEvent::EV_TRANSFORM)
            fprintf(fd, "%d %f  %d %f %f %f  %f %f %f %f\n",
                        p->m_type, p->m_time, p->m_kart_id, 
                        p->m_event.m_t.getOrigin().getX(),
                        p->m_event.m_t.getOrigin().getY(),
                        p->m_event.m_t.getOrigin().getZ(),
                        p->m_event.m_t.getRotation().getX(),
                        p->m_event.m_t.getRotation().getY(),
                        p->m_event.m_t.getRotation().getZ(),
                        p->m_event.m_t.getRotation().getW()
                        );
    }   // for k
    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // Save

//-----------------------------------------------------------------------------
/** Loads a replay data from  file called 'trackname'.replay.
 */
void Replay::Load()
{
    char s[1024], s1[1024];
    int  n;

    std::string filename = file_manager->getConfigDir()+"/"
                         + race_manager->getTrackName() + ".replay";

    FILE *fd = fopen(filename.c_str(),"r");
    if(!fd)
    {
        filename = race_manager->getTrackName()+".replay";
        fd = fopen(filename.c_str(), "r");
        if(!fd)
        {
            printf("Can't read '%s', ghost replay disabled.\n", 
                   filename.c_str());
            m_do_replay = false;
            initRecording();
            return;
        }
    }
    printf("Reading replay file '%s'.\n", filename.c_str());
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read '%s'.\n", filename.c_str());
        exit(-2);
    }
    
    if (sscanf(s,"Version: %s",s1)!=1)
    {
        fprintf(stderr, "ERROR: no Version information found in replay file"
                        " (bogus replay file)\n");
        exit(-2);
    }
    else
    {
        if (strcmp(s1,STK_VERSION))
        {
            fprintf(stderr, "WARNING: replay is version '%s'\n",s1);
            fprintf(stderr, "         STK version is '%s'\n",STK_VERSION);
        }
    }
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read '%s'.\n", filename.c_str());
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
    if(sscanf(s, "numkarts: %d",&m_num_ghost_karts)!=1)
    {
        fprintf(stderr,"WARNING: No number of karts found in replay file.\n");
        exit(-2);
    }

    for(unsigned int i=0; i<m_num_ghost_karts; i++)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "model %d: %s",&n, s1)!=2)
        {
            fprintf(stderr,
                    "WARNING: No model information for kart %d found.\n",
                    i);
            exit(-2);
        }
        m_ghost_karts.push_back(new GhostKart(std::string(s1)));
        m_kart_ident.push_back(s1);
        if(i<race_manager->getNumPlayers())
        {
            race_manager->setLocalKartInfo(i, s1);
        }
    }   // for i<nKarts
    // FIXME: The model information is currently ignored
    fgets(s, 1023, fd);
    unsigned int size;
    if(sscanf(s,"size: %d",&size)!=1)
    {
        fprintf(stderr,
                "WARNING: Number of records not found in replay file.\n");
        exit(-2);
    }

    m_events.resize(size);
    m_next = 0;

    for(unsigned int i=0; i<size; i++)
    {
        fgets(s, 1023, fd);
        unsigned int kart_id;
        float x, y, z, rx, ry, rz, rw, time;

        // Check for EV_TRANSFORM event:
        // -----------------------------
        if(sscanf(s, "0 %f  %d   %f %f %f  %f %f %f %f\n",
                    &time, &kart_id,
                    &x, &y, &z,
                    &rx, &ry, &rz, &rw
                    )==9)
        {
            btQuaternion q(rx, ry, rz, rw);
            btVector3 xyz(x, y, z);
            m_ghost_karts[kart_id]->addTransform(time, btTransform(q, xyz));
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
    fprintf(fd, "Replay file end.\n");
    fclose(fd);
}   // Load

