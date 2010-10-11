// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2010 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2010 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2010 Joerg Henrichs
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
#include "modes/linear_world.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/race_result_gui.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

EndController::EndController(Kart *kart, StateManager::ActivePlayer *player) 
             : AIBaseController(kart, player)
{
    m_next_node_index.reserve(m_quad_graph->getNumNodes());
    m_successor_index.reserve(m_quad_graph->getNumNodes());
    std::vector<unsigned int> next;
    for(unsigned int i=0; i<m_quad_graph->getNumNodes(); i++)
    {
        // 0 is always a valid successor - so even if the kart should end
        // up by accident on a non-selected path, it will keep on working.
        m_successor_index.push_back(0);

        next.clear();
        m_quad_graph->getSuccessors(i, next);
        m_next_node_index.push_back(next[0]);
    }

    const unsigned int look_ahead=10;
    // Now compute for each node in the graph the list of the next 'look_ahead'
    // graph nodes. This is the list of node that is tested in checkCrashes.
    // If the look_ahead is too big, the AI can skip loops (see 
    // QuadGraph::findRoadSector for details), if it's too short the AI won't
    // find too good a driveline. Note that in general this list should
    // be computed recursively, but since the AI for now is using only 
    // (randomly picked) path this is fine
    m_all_look_aheads.reserve(m_quad_graph->getNumNodes());
    for(unsigned int i=0; i<m_quad_graph->getNumNodes(); i++)
    {
        std::vector<int> l;
        int current = i;
        for(unsigned int j=0; j<look_ahead; j++)
        {
            l.push_back(m_next_node_index[current]);
            current = m_next_node_index[current];
        }   // for j<look_ahead
        m_all_look_aheads.push_back(l);
    }
    // Reset must be called after m_quad_graph etc. is set up        
    reset();

    m_max_handicap_accel = 1.0f;
    m_min_steps          = 2;
    setSkiddingFraction(1.3f);

#ifdef AI_DEBUG
    m_debug_sphere = irr_driver->getSceneManager()->addSphereSceneNode(1);
#endif
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
    m_crash_time                 = 0.0f;
    m_time_since_stuck           = 0.0f;

    Controller::reset();
    m_track_node               = QuadGraph::UNKNOWN_SECTOR;
    m_quad_graph->findRoadSector(m_kart->getXYZ(), &m_track_node);

    // Node that this can happen quite easily, e.g. an AI kart is
    // taken over by the end controller while it is off track.
    if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
    {
        m_track_node = m_quad_graph->findOutOfRoadSector(m_kart->getXYZ());
    }
}   // reset

//-----------------------------------------------------------------------------
/** The end controller must forward 'fire' presses to the race gui.
 */
void EndController::action(PlayerAction action, int value)
{
    if(action!=PA_FIRE) return;
    RaceResultGUI *race_result_gui = dynamic_cast<RaceResultGUI*>(World::getWorld()->getRaceGUI());
    if(!race_result_gui) return;
    race_result_gui->nextPhase();
}   // action

//-----------------------------------------------------------------------------
void EndController::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;
    m_controls->m_brake     = false;
    m_controls->m_accel     = 0.3f;

    // Update the current node:
    if(m_track_node!=QuadGraph::UNKNOWN_SECTOR)
    {
        int old_node = m_track_node;
        m_quad_graph->findRoadSector(m_kart->getXYZ(), &m_track_node, 
                                     &m_all_look_aheads[m_track_node]);
        // IF the AI is off track (or on a branch of the track it did not
        // select to be on), keep the old position.
        if(m_track_node==QuadGraph::UNKNOWN_SECTOR ||
            m_next_node_index[m_track_node]==-1)
            m_track_node = old_node;
    }
    if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
    {
        m_track_node = m_quad_graph->findOutOfRoadSector(m_kart->getXYZ());
    }

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    int steps = 0;

    steps = calcSteps();

    /*Response handling functions*/
    handleSteering(dt);
    handleRescue(dt);

    /*And obviously general kart stuff*/
    Controller::update(dt);
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
       0.5f* m_quad_graph->getNode(m_track_node).getPathWidth()+0.5f )
    {
        const int next = m_next_node_index[m_track_node];
        target_point = m_quad_graph->getQuad(next).getCenter();
#ifdef AI_DEBUG
        std::cout << "- Outside of road: steer to center point." <<
            std::endl;
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
    if(m_kart->getSpeed()<2.0f && !m_kart->playingEmergencyAnimation() &&
        !m_world->isStartPhase())
    {
        m_time_since_stuck += DELTA;
        if(m_time_since_stuck > 2.0f)
        {
            m_kart->forceRescue();
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
    int target_sector;

    Vec3 direction;
    Vec3 step_track_coord;
    float distance;
    int steps;

    //We exit from the function when we have found a solution
    while( 1 )
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        target_sector = m_next_node_index[sector];

        //direction is a vector from our kart to the sectors we are testing
        direction = m_quad_graph->getQuad(target_sector).getCenter() - m_kart->getXYZ();

        float len=direction.length_2d();
        steps = int( len / m_kart_length );
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

            m_quad_graph->spatialToTrack(&step_track_coord, step_coord,
                                                   sector );
 
            distance = fabsf(step_track_coord[0]);

            //If we are outside, the previous sector is what we are looking for
            if ( distance + m_kart_width * 0.5f 
                 > m_quad_graph->getNode(sector).getPathWidth() )
            {
                *result = m_quad_graph->getQuad(sector).getCenter();
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

