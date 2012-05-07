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

#include "modes/demo_world.hpp"

#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"

std::vector<std::string> DemoWorld::m_demo_tracks;
int                      DemoWorld::m_num_karts         = 2;
float                    DemoWorld::m_max_idle_time     = 99999.0f;
float                    DemoWorld::m_current_idle_time = 0;
bool                     DemoWorld::m_do_demo           = false;

//-----------------------------------------------------------------------------
/** The constructor sets the number of (local) players to 0, since only AI
 *  karts are used.
 */
DemoWorld::DemoWorld()
{
    m_abort = false;
    ProfileWorld::setProfileModeLaps(m_num_laps);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    race_manager->setDifficulty(RaceManager::RD_HARD);
    race_manager->setNumKarts(m_num_karts);
    race_manager->setNumLocalPlayers(1);
    race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

}   // DemoWorld

//-----------------------------------------------------------------------------
/** Destructor. Resets the flag that the next race is a demo mode. 
 */
DemoWorld::~DemoWorld()
{
    m_current_idle_time = 0;
    m_do_demo           = false;
}   // ~DemoWorld

//-----------------------------------------------------------------------------
/** Sets the list of tracks that is to be used.
 *  \param tracks List of track identifiers.
 */
void DemoWorld::setTracks(const std::vector<std::string> &tracks)
{
    m_demo_tracks = tracks;
}   // setTracks

//-----------------------------------------------------------------------------
/** The race is over if either the requested number of laps have been done
 *  or the requested time is over.
 */
bool DemoWorld::isRaceOver()
{
    if(m_abort) return true;

    // Now it must be laps based profiling:
    return race_manager->getFinishedKarts()==getNumKarts();
}   // isRaceOver

//-----------------------------------------------------------------------------
/** This function is called when the race is finished, but end-of-race
 *  animations have still to be played. In the case of profiling,
 *  we can just abort here without waiting for the animations.
 */
void DemoWorld::enterRaceOverState()
{
    // Do not call profile enterRaceOverState, since it will abort
    StandardRace::enterRaceOverState();
}    // enterRaceOverState

//-----------------------------------------------------------------------------
/** Updates the current idle time by dt. If the accumulated idle time is
 *  large enough, a demo race is started.
 *  \param dt Time step size, which is added to the idle time.
 *  \return true if a demo is started.
 */
bool DemoWorld::updateIdleTimeAndStartDemo(float dt) 
{
    m_current_idle_time += dt; 
    if(m_current_idle_time <= m_max_idle_time)
        return false;

    if(m_demo_tracks.size()==0)
        m_demo_tracks = track_manager->getAllTrackIdentifiers();
    Track *track = track_manager->getTrack(m_demo_tracks[0]);
    // Remove arena tracks (outside the if statement above in case that
    // a user requests a arena track ;)  )
    while((!track || track->isArena()) && m_demo_tracks.size()>1)
    {
        if(!track)
            printf("Invalid demo track identifier '%s'.\n", 
                   m_demo_tracks[0].c_str());
        m_demo_tracks.erase(m_demo_tracks.begin());
        track = track_manager->getTrack(m_demo_tracks[0]);
    }
    if(m_demo_tracks.size()==0)
    {
        printf("No valid tracks found, no demo started.\n");
        return false;
    }

    StateManager::get()->enterGameState();
    race_manager->setNumLocalPlayers(1);
    InputDevice *device;

    // Use keyboard 0 by default in --no-start-screen
    device = input_manager->getDeviceList()->getKeyboard(0);
    StateManager::get()->createActivePlayer( 
        UserConfigParams::m_all_players.get(0), device );
    // ASSIGN should make sure that only input from assigned devices
    // is read.
    input_manager->getDeviceList()->setAssignMode(ASSIGN);

    m_do_demo = true;
    race_manager->setNumKarts(m_num_karts);
    race_manager->setLocalKartInfo(0, "tux");
    network_manager->setupPlayerKartInfo();
    race_manager->startSingleRace(m_demo_tracks[0], m_num_laps);
    m_demo_tracks.push_back(m_demo_tracks[0]);
    m_demo_tracks.erase(m_demo_tracks.begin());
    
    return true;
}   // updateIdleTime