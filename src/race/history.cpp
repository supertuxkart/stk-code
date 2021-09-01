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
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"

History* history = 0;
bool History::m_online_history_replay = false;
//-----------------------------------------------------------------------------
/** Initialises the history object and sets the mode to none.
 */
History::History()
{
    m_replay_history = false;
}   // History

//-----------------------------------------------------------------------------
/** Initialise the history for a new recording. It especially allocates memory
 *  to store the history.
 */
void History::initRecording()
{
    allocateMemory();
    m_event_index = 0;
    m_all_input_events.clear();
}   // initRecording

//-----------------------------------------------------------------------------
/** Allocates memory for the history. This is used when recording as well
 *  as when replaying (since in replay the data is read into memory first).
 *  \param number_of_frames Maximum number of frames to store.
 */
void History::allocateMemory(int size)
{
    m_all_input_events.clear();
    if(size<0)
        m_all_input_events.reserve(1024);
    else
        m_all_input_events.resize(size);
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
    ie.m_world_ticks = World::getWorld()->getTicksSinceStart();
    ie.m_action      = pa;
    ie.m_value       = value;
    ie.m_kart_index  = kart_id;
    m_all_input_events.emplace_back(ie);
}   // addEvent

//-----------------------------------------------------------------------------
/** Sets the kart position and controls to the recorded history value.
 *  \param world_ticks WOrld time in ticks.
 *  \param ticks Number of time steps.
 */
void History::updateReplay(int world_ticks)
{
    World *world = World::getWorld();

    while (m_event_index < m_all_input_events.size() &&
        m_all_input_events[m_event_index].m_world_ticks <= world_ticks)
    {
        const InputEvent &ie = m_all_input_events[m_event_index];
        AbstractKart *kart = world->getKart(ie.m_kart_index);
        Log::verbose("history", "time %d event-time %d action %d %d",
            world->getTicksSinceStart(), ie.m_world_ticks, ie.m_action,
            ie.m_value);
        kart->getController()->action(ie.m_action, ie.m_value);
        m_event_index++;
    }   // while we have events for current time step.

    // Check if we have reached the end of the buffer
    if(m_event_index >= m_all_input_events.size())
    {
        Log::info("History", "Replay finished");
        m_event_index= 0;
        // This is useful to use a reproducable rewind problem:
        // replay it with history, for debugging only
#undef DO_REWIND_AT_END_OF_HISTORY
#ifdef DO_REWIND_AT_END_OF_HISTORY
        RewindManager::get()->rewindTo(5.0f);
        exit(-1);
#else
        world->reset();
#endif
    }   // if m_event_index >= m_all_input_events.size()

}   // updateReplay

//-----------------------------------------------------------------------------
/** Saves the history stored in the internal data structures into a file called
 *  history.dat.
 */
void History::Save()
{
    World *world   = World::getWorld();
    if (!world)
        return;
    FILE *fd = fopen("history.dat","w");
    if(fd)
        Log::info("History", "Saved in ./history.dat.");
    else
    {
        std::string fn = file_manager->getUserConfigFile("history.dat");
        fd = FileUtils::fopenU8Path(fn, "w");
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

    const int num_karts = world->getNumKarts();
    fprintf(fd, "STK-version:      %s\n",   STK_VERSION);
    fprintf(fd, "History-version:  %d\n",   1);
    fprintf(fd, "numkarts:         %d\n",   num_karts);
    fprintf(fd, "numplayers:       %d\n", RaceManager::get()->getNumPlayers());
    fprintf(fd, "difficulty:       %d\n", RaceManager::get()->getDifficulty());
    fprintf(fd, "reverse: %c\n", RaceManager::get()->getReverseTrack() ? 'y' : 'n');

    fprintf(fd, "track: %s\n", Track::getCurrentTrack()->getIdent().c_str());

    assert(num_karts > 0);

    int k;
    for(k=0; k<num_karts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getIdent().c_str());
    }
    fprintf(fd, "count:     %zu\n", m_all_input_events.size());

    for (unsigned int i = 0; i < m_all_input_events.size(); i++)
    {
        fprintf(fd, "%d %d %d %d\n",
                m_all_input_events[i].m_world_ticks,
                m_all_input_events[i].m_kart_index,
                m_all_input_events[i].m_action,
                m_all_input_events[i].m_value      );
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
        fd = FileUtils::fopenU8Path(fn, "r");
        if(fd)
            Log::info("History", "Reading '%s'.", fn.c_str());
    }
    if(!fd)
        Log::fatal("History", "Could not open history.dat");

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("History", "Could not read history.dat.");

    // Check for unsupported hsitory file formats:
    if (sscanf(s, "Version-2: %1023s", s1) == 1 ||
        sscanf(s, "Version-1: %1023s", s1) == 1)
    {
        Log::fatal("History",
                   "Old history file format is not supported anymore.");
    }

    if (sscanf(s,"STK-version: %1023s",s1)!=1)
        Log::fatal("History", "No Version information found in history "
                              "file (bogus history file).");
    if (strcmp(s1,STK_VERSION))
        Log::warn("History", "History is version '%s', STK version is '%s'.",
                  s1, STK_VERSION);

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("History", "Could not read history.dat.");

    int version;
    if (sscanf(s, "History-version: %1023d", &version) != 1)
        Log::fatal("Invalid version number found: '%s'", s);

    if (version != 1)
        Log::fatal("History",
                   "Old-style history files are not supported anymore.");

    if (fgets(s, 1023, fd) == NULL)
        Log::fatal("History", "Could not read history.dat.");

    unsigned int num_karts;
    if(sscanf(s, "numkarts: %u", &num_karts)!=1)
        Log::fatal("History", "No number of karts found in history file.");
    RaceManager::get()->setNumKarts(num_karts);

    fgets(s, 1023, fd);
    if(sscanf(s, "numplayers: %d",&n)!=1)
        Log::fatal("History", "No number of players found in history file.");
    RaceManager::get()->setNumPlayers(n);

    fgets(s, 1023, fd);
    if(sscanf(s, "difficulty: %d",&n)!=1)
        Log::fatal("History", "No difficulty found in history file.");
    RaceManager::get()->setDifficulty((RaceManager::Difficulty)n);


    fgets(s, 1023, fd);
    char r;
    if (sscanf(s, "reverse: %c", &r) != 1)
        Log::fatal("History", "Could not read reverse information: '%s'", s);
    RaceManager::get()->setReverseTrack(r == 'y');

    fgets(s, 1023, fd);
    if(sscanf(s, "track: %1023s",s1)!=1)
        Log::warn("History", "Track not found in history file.");
    RaceManager::get()->setTrack(s1);
    // This value doesn't really matter, but should be defined, otherwise
    // the racing phase can switch to 'ending'
    RaceManager::get()->setNumLaps(100);

    for(unsigned int i=0; i<num_karts; i++)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "model %d: %1023s",&n, s1) != 2)
            Log::fatal("History", "No model information for kart %d found.", i);
        m_kart_ident.push_back(s1);
        if(i<RaceManager::get()->getNumPlayers() && !m_online_history_replay)
        {
            RaceManager::get()->setPlayerKart(i, s1);
        }
    }   // for i<nKarts
    // FIXME: The model information is currently ignored

    fgets(s, 1023, fd);
    int count;
    if(sscanf(s,"count: %d",&count)!=1)
        Log::fatal("History", "Number of records not found in history file.");

    allocateMemory(count);
    m_event_index = 0;

    // We need to disable the rewind manager here (otherwise setting the
    // KartControl data would access the rewind manager).
    bool rewind_manager_was_enabled = RewindManager::isEnabled();
    RewindManager::setEnable(false);

    for (int i=0; i<count; i++)
    {
        fgets(s, 1023, fd);
        InputEvent &ie = m_all_input_events[i];
        int action = 0;
        if (sscanf(s, "%d %d %d %d\n", &ie.m_world_ticks, &ie.m_kart_index,
            &action, &ie.m_value) != 4)
        {
            Log::warn("History", "Problems reading event: '%s'", s);
        }
        ie.m_action = (PlayerAction)action;
    }   // for i
    RewindManager::setEnable(rewind_manager_was_enabled);

    fclose(fd);
}   // Load

