//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "race/history.hpp"

#include <stdio.h>

#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "karts/abstract_kart.hpp"
#include "network/rewind_manager.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

History* history = 0;

//-----------------------------------------------------------------------------
/** Initialises the history object and sets the mode to none.
 */
History::History()
{
    m_replay_mode = HISTORY_NONE;
}   // History

//-----------------------------------------------------------------------------
/** Starts replay from the history file in the current directory.
 */
void History::startReplay()
{
    Load();
}   // startReplay

//-----------------------------------------------------------------------------
/** Initialise the history for a new recording. It especially allocates memory
 *  to store the history.
 */
void History::initRecording()
{
    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time
                                             / stk_config->m_replay_dt      );
    allocateMemory(max_frames);
    m_current = -1;
    m_wrapped = false;
    m_size    = 0;
}   // initRecording

//-----------------------------------------------------------------------------
/** Allocates memory for the history. This is used when recording as well
 *  as when replaying (since in replay the data is read into memory first).
 *  \param number_of_frames Maximum number of frames to store.
 */
void History::allocateMemory(int number_of_frames)
{
    m_all_deltas.resize   (number_of_frames);
    unsigned int num_karts = race_manager->getNumberOfKarts();
    m_all_controls.resize (number_of_frames*num_karts);
    m_all_xyz.resize      (number_of_frames*num_karts);
    m_all_rotations.resize(number_of_frames*num_karts);
}   // allocateMemory

//-----------------------------------------------------------------------------
/** Saves the current history.
 *  \param dt Time step size.
 */
void History::updateSaving(float dt)
{
    m_current++;
    if(m_current>=(int)m_all_deltas.size())
    {
        m_wrapped = true;
        m_current = 0;
    }
    else
    {
        // m_size must be m_all_deltas.size() or smaller
        if(m_size<(int)m_all_deltas.size())
            m_size ++;
    }
    m_all_deltas[m_current] = dt;

    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    unsigned int index     = m_current*num_karts;
    for(unsigned int i=0; i<num_karts; i++)
    {
        const AbstractKart *kart         = world->getKart(i);
        m_all_controls[index+i]  = kart->getControls();
        m_all_xyz[index+i]       = kart->getXYZ();
        m_all_rotations[index+i] = kart->getVisualRotation();
    }   // for i
}   // updateSaving

//-----------------------------------------------------------------------------
/** Sets the kart position and controls to the recorded history value.
 *  \param dt Time step size.
 */
float History::updateReplayAndGetDT()
{
    m_current++;
    World *world = World::getWorld();
    if(m_current>=(int)m_all_deltas.size())
    {
        Log::info("History", "Replay finished");
        m_current = 0;
        // This is useful to use a reproducable rewind problem:
        // replay it with history, for debugging only
#undef DO_REWIND_AT_END_OF_HISTORY
#ifdef DO_REWIND_AT_END_OF_HISTORY
        RewindManager::get()->rewindTo(5.0f);
        exit(-1);
#else
        // Note that for physics replay all physics parameters
        // need to be reset, e.g. velocity, ...
        world->reset();
#endif
    }
    unsigned int num_karts = world->getNumKarts();
    for(unsigned k=0; k<num_karts; k++)
    {
        AbstractKart *kart = world->getKart(k);
        unsigned int index=m_current*num_karts+k;
        if(m_replay_mode==HISTORY_POSITION)
        {
            kart->setXYZ(m_all_xyz[index]);
            kart->setRotation(m_all_rotations[index]);
        }
        else
        {
            kart->getControls().set(m_all_controls[index]);
        }
    }
    return m_all_deltas[m_current];
}   // updateReplayAndGetDT

//-----------------------------------------------------------------------------
/** Saves the history stored in the internal data structures into a file called
 *  history.dat.
 */
void History::Save()
{
    FILE *fd = fopen("history.dat","w");
    if(fd)
        Log::info("History", "Saved in ./history.dat.");
    else
    {
        std::string fn = file_manager->getUserConfigFile("history.dat");
        fd = fopen(fn.c_str(), "w");
        if(fd)
            Log::info("History", "Saved in '%s'.", fn.c_str());
    }
    if(!fd)
    {
        Log::info("History", "Can't open history.dat file for writing - can't save history.");
        Log::info("History", "Make sure history.dat in the current directory "
                             "or the config directory is writable.");
        return;
    }

    World *world   = World::getWorld();
    const int num_karts = world->getNumKarts();
    fprintf(fd, "Version:  %s\n",   STK_VERSION);
    fprintf(fd, "numkarts: %d\n",   num_karts);
    fprintf(fd, "numplayers: %d\n", race_manager->getNumPlayers());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "reverse: %c\n", race_manager->getReverseTrack() ? 'y' : 'n');

    fprintf(fd, "track: %s\n",      Track::getCurrentTrack()->getIdent().c_str());

    assert(num_karts > 0);

    int k;
    for(k=0; k<num_karts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getIdent().c_str());
    }
    fprintf(fd, "size:     %d\n", m_size);

    int index = m_wrapped ? m_current : 0;
    for(int i=0; i<m_size; i++)
    {
        fprintf(fd, "delta: %f\n",m_all_deltas[index]);
        index=(index+1)%m_size;
    }

    index = num_karts * (m_wrapped ? m_current : 0);
    for(int i=0; i<m_size; i++)
    {
        for(int k=0; k<num_karts; k++)
        {
            fprintf(fd, "%f %f %d  %f %f %f  %f %f %f %f\n",
                    m_all_controls[index+k].getSteer(),
                    m_all_controls[index+k].getAccel(),
                    m_all_controls[index+k].getButtonsCompressed(),
                    m_all_xyz[index+k].getX(), m_all_xyz[index+k].getY(),
                    m_all_xyz[index+k].getZ(),
                    m_all_rotations[index+k].getX(),
                    m_all_rotations[index+k].getY(),
                    m_all_rotations[index+k].getZ(),
                    m_all_rotations[index+k].getW()  );
        }   // for i
        index=(index+num_karts)%(num_karts*m_size);
    }   // for k
    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Save

//-----------------------------------------------------------------------------
/** Loads a history from history.dat in the current directory.
 */
void History::Load()
{
    char s[1024], s1[1024];
    int  n;

    FILE *fd = fopen("history.dat","r");
    if(fd)
        Log::info("History", "Reading ./history.dat");
    else
    {
        std::string fn = file_manager->getUserConfigFile("history.dat");
        fd = fopen(fn.c_str(), "r");
        if(fd)
            Log::info("History", "Reading '%s'.", fn.c_str());
    }
    if(!fd)
        Log::fatal("History", "Could not open history.dat");

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("History", "Could not read history.dat.");

    if (sscanf(s,"Version: %1023s",s1)!=1)
        Log::fatal("History", "No Version information found in history file (bogus history file).");
    else if (strcmp(s1,STK_VERSION))
        Log::warn("History", "History is version '%s', STK version is '%s'.", s1, STK_VERSION);

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("History", "Could not read history.dat.");

    unsigned int num_karts;
    if(sscanf(s, "numkarts: %u", &num_karts)!=1)
        Log::fatal("History", "No number of karts found in history file.");
    race_manager->setNumKarts(num_karts);

    fgets(s, 1023, fd);
    if(sscanf(s, "numplayers: %d",&n)!=1)
        Log::fatal("History", "No number of players found in history file.");
    race_manager->setNumPlayers(n);

    fgets(s, 1023, fd);
    if(sscanf(s, "difficulty: %d",&n)!=1)
        Log::fatal("History", "No difficulty found in history file.");
    race_manager->setDifficulty((RaceManager::Difficulty)n);


    // Optional (not supported in older history files): include reverse
    fgets(s, 1023, fd);
    char r;
    if (sscanf(s, "reverse: %c", &r) == 1)
    {
        fgets(s, 1023, fd);
        race_manager->setReverseTrack(r == 'y');
    }


    if(sscanf(s, "track: %1023s",s1)!=1)
        Log::warn("History", "Track not found in history file.");
    race_manager->setTrack(s1);
    // This value doesn't really matter, but should be defined, otherwise
    // the racing phase can switch to 'ending'
    race_manager->setNumLaps(10);

    for(unsigned int i=0; i<num_karts; i++)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "model %d: %1023s",&n, s1) != 2)
            Log::fatal("History", "No model information for kart %d found.", i);
        m_kart_ident.push_back(s1);
        if(i<race_manager->getNumPlayers())
        {
            race_manager->setPlayerKart(i, s1);
        }
    }   // for i<nKarts
    // FIXME: The model information is currently ignored
    fgets(s, 1023, fd);
    if(sscanf(s,"size: %d",&m_size)!=1)
        Log::fatal("History", "Number of records not found in history file.");

    allocateMemory(m_size);
    m_current = -1;

    for(int i=0; i<m_size; i++)
    {
        fgets(s, 1023, fd);
        sscanf(s, "delta: %f\n",&m_all_deltas[i]);
    }

    // We need to disable the rewind manager here (otherwise setting the
    // KartControl data would access the rewind manager).
    bool rewind_manager_was_enabled = RewindManager::isEnabled();
    RewindManager::setEnable(false);

    for(int i=0; i<m_size; i++)
    {
        for(unsigned int k=0; k<num_karts; k++)
        {
            unsigned int index = num_karts * i+k;
            fgets(s, 1023, fd);
            int buttonsCompressed;
            float x,y,z,rx,ry,rz,rw, steer, accel;
            sscanf(s, "%f %f %d  %f %f %f  %f %f %f %f\n",
                    &steer, &accel, &buttonsCompressed,
                    &x, &y, &z,
                    &rx, &ry, &rz, &rw
                    );
            m_all_controls[index].setSteer(steer);
            m_all_controls[index].setAccel(accel);
            m_all_xyz[index]       = Vec3(x,y,z);
            m_all_rotations[index] = btQuaternion(rx,ry,rz,rw);
            m_all_controls[index].setButtonsCompressed(char(buttonsCompressed));
        }   // for i
    }   // for k
    RewindManager::setEnable(rewind_manager_was_enabled);

    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Load

