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
#include "tracks/quad_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

EndController::EndController(Kart *kart, StateManager::ActivePlayer *player) 
             : AIBaseController(kart, player)
{
    m_next_node_index.reserve(m_quad_graph->getNumNodes());
    m_successor_index.reserve(m_quad_graph->getNumNodes());

    // Initialise the fields
    for(unsigned int i=0; i<m_quad_graph->getNumNodes(); i++)
    {
        m_next_node_index.push_back(-1);
        // 0 is always a valid successor - so even if the kart should end
        // up by accident on a non-selected path, it will keep on working.
        m_successor_index.push_back(0);
    }
    // For now pick one part on random, which is not adjusted during the run
    std::vector<unsigned int> next;
    int count=0;
    int current_node=0;
    while (1)
    {
        next.clear();
        m_quad_graph->getSuccessors(current_node, next);
        int indx = rand() % next.size();
        m_successor_index[current_node] = indx;
        m_next_node_index[current_node] = next[indx];
        current_node = next[indx];
        if(current_node==0) break;
        count++;
        if(count>(int)m_quad_graph->getNumNodes())
        {
            fprintf(stderr, "AI can't find a loop going back to node 0, aborting.\n");
            exit(-1);
        }
    };

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
            int next = m_next_node_index[current];
            if(next==-1) break;
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
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.
void EndController::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;

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
    // The client does not do any AI computations.
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) 
    {
        Controller::update(dt);
        return;
    }


    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    int steps = 0;

    steps = calcSteps();

    findCurve();

    /*Response handling functions*/
    handleAcceleration(dt);
    handleSteering(dt);
    handleRescue(dt);
    handleBraking();

    /*And obviously general kart stuff*/
    Controller::update(dt);
}   // update

//-----------------------------------------------------------------------------
void EndController::handleBraking()
{
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER &&
        m_kart->getPosition() < m_world->getKart(0)->getPosition()         &&
        m_kart->getInitialPosition()>1                                          )
    {
        m_controls->m_brake = true;
        return;
    }
        
    //We may brake if we are about to get out of the road, but only if the
    //kart is on top of the road, and if we won't slow down below a certain
    //limit.
    m_controls->m_brake = false;
}   // handleBraking

//-----------------------------------------------------------------------------
void EndController::handleSteering(float dt)
{
    const int next = m_next_node_index[m_track_node];
    
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if( fabsf(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ))  >
       0.5f* m_quad_graph->getNode(m_track_node).getPathWidth()+0.5f )
    {
        steer_angle = steerToPoint(m_quad_graph->getQuad(next).getCenter());

#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_quad_graph->getQuad(next).getCenter().toIrrVector());
        std::cout << "- Outside of road: steer to center point." <<
            std::endl;
#endif
    }
    //If we are going to crash against a kart, avoid it if it doesn't
    //drives the kart out of the road

    else
    {
        m_start_kart_crash_direction = 0;
        Vec3 straight_point;
        findNonCrashingPoint(&straight_point);
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(straight_point.toIrrVector());
#endif
        steer_angle = steerToPoint(straight_point);
    }

#ifdef AI_DEBUG
    std::cout << "- Fallback."  << std::endl;
#endif


    setSteering(steer_angle, dt);
}   // handleSteering

//-----------------------------------------------------------------------------
void EndController::handleAcceleration( const float DELTA )
{
    //Do not accelerate until we have delayed the start enough
    if( m_time_till_start > 0.0f )
    {
        m_time_till_start -= DELTA;
        m_controls->m_accel = 0.0f;
        return;
    }

    if( m_controls->m_brake == true )
    {
        m_controls->m_accel = 0.0f;
        return;
    }

    if(m_kart->hasViewBlockedByPlunger())
    {
        if(!(m_kart->getSpeed() > m_kart->getMaxSpeedOnTerrain() / 2))
            m_controls->m_accel = 0.05f;
        else 
            m_controls->m_accel = 0.0f;
        return;
    }
    
    m_controls->m_accel = 1.0f;
}   // handleAcceleration

//-----------------------------------------------------------------------------
void EndController::handleRescue(const float DELTA)
{
    // check if kart is stuck
    if(m_kart->getSpeed()<2.0f && !m_kart->isRescue() && !m_world->isStartPhase())
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
void EndController::reset()
{
    m_start_kart_crash_direction = 0;
    m_curve_target_speed         = m_kart->getMaxSpeedOnTerrain();
    m_curve_angle                = 0.0;
    m_time_till_start            = -1.0f;
    m_crash_time                 = 0.0f;
    m_time_since_stuck           = 0.0f;

    Controller::reset();
    m_track_node               = QuadGraph::UNKNOWN_SECTOR;
    m_quad_graph->findRoadSector(m_kart->getXYZ(), &m_track_node);
    if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
    {
        fprintf(stderr, "Invalid starting position for '%s' - not on track - can be ignored.\n",
                m_kart->getIdent().c_str());
        m_track_node = m_quad_graph->findOutOfRoadSector(m_kart->getXYZ());
    }

}   // reset

//-----------------------------------------------------------------------------
/** calc_steps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart.
 *  The calling sequence guarantees that m_future_sector is not UNKNOWN.
 */
int EndController::calcSteps()
{
    int steps = int( m_kart->getVelocityLC().getZ() / m_kart_length );
    if( steps < m_min_steps ) steps = m_min_steps;

    //Increase the steps depending on the width, if we steering hard,
    //mostly for curves.
#if 0
    // FIXME: I don't understand this: if we are steering hard, we check
    //        for more steps if we hit another kart?? If we steer hard,
    //        the approximation used (pos + velocity*dt) will be even
    //        worse, since it doesn't take steering into account.
    if( fabsf(m_controls->m_steer) > 0.95 )
    {
        const int WIDTH_STEPS = 
            (int)( m_quad_graph->getNode(m_future_sector).getPathWidth()
                   /( m_kart_length * 2.0 ) );

        steps += WIDTH_STEPS;
    }
#endif
    return steps;
}   // calcSteps

//-----------------------------------------------------------------------------
/**FindCurve() gathers info about the closest sectors ahead: the curve
 * angle, the direction of the next turn, and the optimal speed at which the
 * curve can be travelled at it's widest angle.
 *
 * The number of sectors that form the curve is dependant on the kart's speed.
 */
void EndController::findCurve()
{
    float total_dist = 0.0f;
    int i;
    for(i = m_track_node; total_dist < m_kart->getVelocityLC().getZ(); 
        i = m_next_node_index[i])
    {
        total_dist += m_quad_graph->getDistanceToNext(i, m_successor_index[i]);
    }


    m_curve_angle = 
        normalizeAngle(m_quad_graph->getAngleToNext(i, m_successor_index[i])
                      -m_quad_graph->getAngleToNext(m_track_node, 
                                                    m_successor_index[m_track_node]) );
    
    m_curve_target_speed = m_kart->getMaxSpeedOnTerrain();
}   // findCurve
