//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Eduardo Hernandez Munoz
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


//The AI debugging works best with just 1 AI kart, so set the number of karts
//to 2 in main.cpp with quickstart and run supertuxkart with the arg -N.
#undef AI_DEBUG

#include "karts/controller/end_controller.hpp"

#ifdef AI_DEBUG
#  include "irrlicht.h"
   using namespace irr;
#endif

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>

#ifdef AI_DEBUG
#include "graphics/irr_driver.hpp"
#endif

#include "karts/abstract_kart.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/linear_world.hpp"
#include "race/race_manager.hpp"
#include "states_screens/race_result_gui.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

EndController::EndController(AbstractKart *kart,
                             Controller *prev_controller)
             : AIBaseLapController(kart)
{
    m_previous_controller = prev_controller;
    if(!RaceManager::get()->isBattleMode() &&
       RaceManager::get()->getMinorMode()!=RaceManager::MINOR_MODE_SOCCER)
    {
        // Overwrite the random selected default path from AIBaseLapController
        // with a path that always picks the first branch (i.e. it follows
        // the main driveline).
        std::vector<unsigned int> next;
        for(unsigned int i=0; i<DriveGraph::get()->getNumNodes(); i++)
        {
            // 0 is always a valid successor - so even if the kart should end
            // up by accident on a non-selected path, it will keep on working.
            m_successor_index[i] = 0;

            next.clear();
            DriveGraph::get()->getSuccessors(i, next);
            m_next_node_index[i] = next[0];
        }

        const unsigned int look_ahead=10;
        // Now compute for each node in the graph the list of the next 'look_ahead'
        // graph nodes. This is the list of node that is tested in checkCrashes.
        // If the look_ahead is too big, the AI can skip loops (see
        // DriveGraph::findRoadSector for details), if it's too short the AI won't
        // find too good a driveline. Note that in general this list should
        // be computed recursively, but since the AI for now is using only
        // (randomly picked) path this is fine
        for(unsigned int i=0; i<DriveGraph::get()->getNumNodes(); i++)
        {
            std::vector<int> l;
            int current = i;
            for(unsigned int j=0; j<look_ahead; j++)
            {
                l.push_back(m_next_node_index[current]);
                current = m_next_node_index[current];
            }   // for j<look_ahead
            m_all_look_aheads[i] = l;
        }
    }   // if not battle mode

    // Reset must be called after DriveGraph::get() etc. is set up
    reset();

    m_max_handicap_accel = 1.0f;

#ifdef AI_DEBUG
    m_debug_sphere = irr_driver->getSceneManager()->addSphereSceneNode(1);
#endif

    // Set the name of the previous controller as this controller name, otherwise
    // we get the incorrect name when printing statistics in profile mode.
    setControllerName(prev_controller->getControllerName());
}   // EndController

//-----------------------------------------------------------------------------
/** The destructor deletes the shared TrackInfo objects if no more EndController
 *  instances are around.
 */
EndController::~EndController()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
#endif
}   // ~EndController

//-----------------------------------------------------------------------------
void EndController::reset()
{
    AIBaseLapController::reset();

    m_crash_time       = 0.0f;
    m_time_since_stuck = 0.0f;

    m_track_node       = Graph::UNKNOWN_SECTOR;
    // In battle mode there is no quad graph, so nothing to do in this case
    if(!RaceManager::get()->isBattleMode() &&
       RaceManager::get()->getMinorMode()!=RaceManager::MINOR_MODE_SOCCER)
    {
        DriveGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node);

        // Node that this can happen quite easily, e.g. an AI kart is
        // taken over by the end controller while it is off track.
        if(m_track_node==Graph::UNKNOWN_SECTOR)
        {
            m_track_node = DriveGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
        }
    }
    m_min_steps = 2;
}   // reset

//-----------------------------------------------------------------------------
/** Callback when a new lap is triggered. It is used to switch to the first
 *  end camera (which is esp. useful in fixing up end cameras in reverse mode,
 *  since otherwise the switch to the first end camera usually happens way too
 *  late)
 */
void  EndController::newLap(int lap)
{
    // Forward the call to the original controller. This will implicitely
    // trigger setting the first end camera to be active if the controller
    // is a player controller.
    m_previous_controller->newLap(lap);
}   // newLap

//-----------------------------------------------------------------------------
/** The end controller must forward 'fire' presses to the race gui.
 */
bool EndController::action(PlayerAction action, int value, bool dry_run)
{
    if(action!=PA_FIRE) return true;
    RaceResultGUI *race_result_gui =
        dynamic_cast<RaceResultGUI*>(World::getWorld()->getRaceGUI());
    if(!race_result_gui) return true;
    race_result_gui->nextPhase();
    return true;
}   // action

//-----------------------------------------------------------------------------
void EndController::update(int ticks)
{
    // This is used to enable firing an item backwards.
    m_controls->setLookBack(false);
    m_controls->setNitro(false);
    m_controls->setBrake(false);
    m_controls->setAccel(1.0f);

    AIBaseLapController::update(ticks);

    // In case of battle mode: don't do anything
    if(RaceManager::get()->isBattleMode() ||
       RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_SOCCER  ||
       RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_EASTER_EGG)
    {
        m_controls->setAccel(0.0f);
        // Brake while we are still driving forwards (if we keep
        // on braking, the kart will reverse otherwise)
        m_controls->setBrake(m_kart->getSpeed()>0);
        return;
    }
    // Keep update slow down in case for network rewind overwrite it
    m_kart->setSlowdown(MaxSpeed::MS_DECREASE_AI, 0.3f, 2);

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    calcSteps();

    /*Response handling functions*/
    float dt = stk_config->ticks2Time(ticks);
    handleSteering(dt);
    handleRescue(dt);
}   // update

//-----------------------------------------------------------------------------
void EndController::handleSteering(float dt)
{
    Vec3  target_point;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if( fabsf(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ))  >
       0.5f* DriveGraph::get()->getNode(m_track_node)->getPathWidth()+0.5f )
    {
        const int next = m_next_node_index[m_track_node];
        target_point = DriveGraph::get()->getNode(next)->getCenter();
#ifdef AI_DEBUG
        Log::debug("end_controller.cpp", "- Outside of road: steer to center point.");
#endif
    }
    else
    {
        findNonCrashingPoint(&target_point);
    }
#ifdef AI_DEBUG
    m_debug_sphere->setPosition(target_point.toIrrVector());
#endif

    setSteering(steerToPoint(target_point), dt);
}   // handleSteering

//-----------------------------------------------------------------------------
void EndController::handleRescue(const float DELTA)
{
    // check if kart is stuck
    if(m_kart->getSpeed()<2.0f && !m_kart->getKartAnimation() &&
        !m_world->isStartPhase())
    {
        m_time_since_stuck += DELTA;
        if(m_time_since_stuck > 2.0f)
        {
            RescueAnimation::create(m_kart);
            m_time_since_stuck=0.0f;
        }   // m_time_since_stuck > 2.0f
    }
    else
    {
        m_time_since_stuck = 0.0f;
    }
}   // handleRescue

//-----------------------------------------------------------------------------
/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve after wards,
 *  and return the position of that edge.
 */
void EndController::findNonCrashingPoint(Vec3 *result)
{
    unsigned int sector = m_next_node_index[m_track_node];

    Vec3 direction;
    Vec3 step_track_coord;
    float distance;

    //We exit from the function when we have found a solution
    while( 1 )
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        int target_sector = m_next_node_index[sector];

        //direction is a vector from our kart to the sectors we are testing
        direction = DriveGraph::get()->getNode(target_sector)->getCenter()
                  - m_kart->getXYZ();

        float len=direction.length();
        int steps = int( len / m_kart_length );
        if( steps < 3 ) steps = 3;

        //Protection against having vel_normal with nan values
        if(len>0.0f) {
            direction*= 1.0f/len;
        }

        Vec3 step_coord;
        //Test if we crash if we drive towards the target sector
        for( int i = 2; i < steps; ++i )
        {
            step_coord = m_kart->getXYZ()+direction*m_kart_length * float(i);

            DriveGraph::get()->spatialToTrack(&step_track_coord, step_coord,
                                                   sector );

            distance = fabsf(step_track_coord[0]);

            //If we are outside, the previous sector is what we are looking for
            if ( distance + m_kart_width * 0.5f
                 > DriveGraph::get()->getNode(sector)->getPathWidth()*0.5f )
            {
                *result = DriveGraph::get()->getNode(sector)->getCenter();
                return;
            }
        }
        sector = target_sector;
    }
}   // findNonCrashingPoint

//-----------------------------------------------------------------------------
/** calc_steps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart.
 *  The calling sequence guarantees that m_future_sector is not UNKNOWN.
 */
int EndController::calcSteps()
{
    int steps = int( m_kart->getVelocityLC().getZ() / m_kart_length );
    if( steps < m_min_steps ) steps = m_min_steps;

    return steps;
}   // calcSteps
