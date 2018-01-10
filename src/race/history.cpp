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
#include "karts/controller/controller.hpp"
#include "network/network_config.hpp"
#include "network/rewind_manager.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

History* history = 0;

//-----------------------------------------------------------------------------
/** Initialises the history object and sets the mode to none.
 */
History::History()
{
    m_replay_mode  = HISTORY_NONE;
    m_history_time = 0.0f;
}   // History

//-----------------------------------------------------------------------------
/** Initialise the history for a new recording. It especially allocates memory
 *  to store the history.
 */
void History::initRecording()
{
    unsigned int max_frames = (unsigned int)(  stk_config->m_replay_max_time
                                             / stk_config->m_replay_dt      );
    allocateMemory(max_frames);
    m_current     = -1;
    m_wrapped     = false;
    m_size        = 0;
    m_event_index = 0;
    m_all_input_events.clear();
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
/** Stores an input event (e.g. acceleration or steering event) into the
 *  history data for physics replay.
 *  \param kart_id The kart index which triggered the event.
 *  \param pa      The action.
 *  \param value   Value of the action (0=release, 32768 = pressed), in
 *                 between in case of analog devices.
 */
void History::addEvent(int kart_id, PlayerAction pa, int value)
{
    InputEvent ie;
    // The event is added before m_current is increased. So in order to
    // save the right index for this event, we need to use m_current+1.
    ie.m_index      = m_current+1;
    ie.m_action     = pa;
    ie.m_value      = value;
    ie.m_kart_index = kart_id;
    m_all_input_events.emplace_back(ie);
}   // addEvent

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
 *  \return dt Time step size.
 */
float History::updateReplayAndGetDT(float world_time, float dt)
{
    if (m_history_time > world_time && NetworkConfig::get()->isNetworking())
        return dt;

    World *world = World::getWorld();

    // In non-networked history races, we need to do at least one timestep
    // to get the right DT. The while loop is exited at the bottom in this
    // case/
    while (m_history_time < world_time + dt     ||
           !NetworkConfig::get()->isNetworking()   )
    {
        m_current++;
        if (m_current >= (int)m_all_deltas.size())
        {
            Log::info("History", "Replay finished");
            m_current      = 0;
            m_history_time = 0;
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
        if (m_replay_mode == HISTORY_POSITION)
        {
            unsigned int num_karts = world->getNumKarts();
            for (unsigned k = 0; k < num_karts; k++)
            {
                AbstractKart *kart = world->getKart(k);
                unsigned int index = m_current*num_karts + k;
                kart->setXYZ(m_all_xyz[index]);
                kart->setRotation(m_all_rotations[index]);
            }
        }
        else   // HISTORY_PHYSICS
        {
            while (m_event_index < m_all_input_events.size() &&
                m_all_input_events[m_event_index].m_index == m_current)
            {
                const InputEvent &ie = m_all_input_events[m_event_index];
                AbstractKart *kart = world->getKart(ie.m_kart_index);
                kart->getController()->action(ie.m_action, ie.m_value);
                m_event_index++;
            }

            //kart->getControls().set(m_all_controls[index]);
        }   // if HISTORY_PHYSICS

        if (World::getWorld()->isRacePhase())
            m_history_time += m_all_deltas[m_current];

        // If this is not networked, exit the loop after one iteration
        // and return the new dt
        if(!NetworkConfig::get()->isNetworking())
            return m_all_deltas[m_current];

    }

    // In network mode, don't adjust dt, just return the input value
    return dt;
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
    fprintf(fd, "Version-1:  %s\n",   STK_VERSION);
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
        fprintf(fd, "delta: %12.9f\n",m_all_deltas[index]);
        index=(index+1)%m_size;
    }

    index = m_wrapped ? m_current : 0;
    int event_index = 0;
    for(int i=0; i<m_size; i++)
    {
        int base_index = index * num_karts;
        for(int k=0; k<num_karts; k++)
        {
            fprintf(fd, "%f %f %d  %f %f %f  %f %f %f %f\n",
                    m_all_controls[base_index+k].getSteer(),
                    m_all_controls[base_index+k].getAccel(),
                    m_all_controls[base_index+k].getButtonsCompressed(),
                    m_all_xyz[base_index+k].getX(), m_all_xyz[base_index+k].getY(),
                    m_all_xyz[base_index+k].getZ(),
                    m_all_rotations[base_index+k].getX(),
                    m_all_rotations[base_index+k].getY(),
                    m_all_rotations[base_index+k].getZ(),
                    m_all_rotations[base_index+k].getW()  );
        }   // for k
        // Find number of events for this frame
        int count = 0;
        while ( event_index+count < (int)m_all_input_events.size() &&
                m_all_input_events[event_index+count].m_index == index )
        {
            count++;
        }
        fprintf(fd, "%d\n", count);
        for (int k = 0; k < count; k++)
        {
            fprintf(fd, "%d %d %d\n",
                    m_all_input_events[event_index].m_kart_index,
                    m_all_input_events[event_index].m_action,
                    m_all_input_events[event_index].m_value);
            event_index++;
        }
        index=(index+1)%m_size;
    }   // for i
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

    int version = 0;
    if (sscanf(s, "Version-1: %1023s", s1) == 1)
        version = 1;
    else if (sscanf(s,"Version: %1023s",s1)!=1)
        Log::fatal("History", "No Version information found in history "
                              "file (bogus history file).");
    if (strcmp(s1,STK_VERSION))
        Log::warn("History", "History is version '%s', STK version is '%s'.",
                  s1, STK_VERSION);
    if (version != 1)
        Log::fatal("History",
                   "Old-style history files are not supported anymore.");

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
        if(i<race_manager->getNumPlayers() && !MainMenuScreen::m_enable_online)
        {
            race_manager->setPlayerKart(i, s1);
        }
    }   // for i<nKarts
    // FIXME: The model information is currently ignored
    fgets(s, 1023, fd);
    if(sscanf(s,"size: %d",&m_size)!=1)
        Log::fatal("History", "Number of records not found in history file.");

    allocateMemory(m_size);
    m_current     = -1;
    m_event_index = 0;

    for(int i=0; i<m_size; i++)
    {
        fgets(s, 1023, fd);
        sscanf(s, "delta: %f\n",&m_all_deltas[i]);
    }

    // We need to disable the rewind manager here (otherwise setting the
    // KartControl data would access the rewind manager).
    bool rewind_manager_was_enabled = RewindManager::isEnabled();
    RewindManager::setEnable(false);
    m_all_input_events.clear();
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
        }   // for k
        fgets(s, 1023, fd);
        int count;
        if (sscanf(s, "%d\n", &count) != 1)
            Log::warn("History", "Problems reading event count: '%s'.", s);
        for (int k = 0; k < count; k++)
        {
            fgets(s, 1023, fd);
            InputEvent ie;
            ie.m_index = i;
            if (sscanf(s, "%d %d %d\n", &ie.m_kart_index, &ie.m_action,
                &ie.m_value) != 3)
            {
                Log::warn("History", "Problems reading event: '%s'", s);
            }
            m_all_input_events.emplace_back(ie);
        }   // for k < count
    }   // for i
    RewindManager::setEnable(rewind_manager_was_enabled);

    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Load

