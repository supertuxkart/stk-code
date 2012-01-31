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
#define AI_DEBUG

#include "karts/controller/new_ai_controller.hpp"

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
#include "items/attachment.hpp"
#include "modes/linear_world.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

NewAIController::NewAIController(Kart *kart) : AIBaseController(kart)
{
    // Reset must be called after QuadGraph::get() etc. is set up        
    reset();
    
    switch( race_manager->getDifficulty())
    {
    case RaceManager::RD_EASY:
        m_wait_for_players   = true;
        m_max_handicap_accel = 0.9f;
        m_item_tactic        = IT_TEN_SECONDS;
        m_max_start_delay    = 0.5f;
        m_min_steps          = 0;
        m_nitro_level        = NITRO_NONE;
        m_handle_bomb        = false;
        setSkiddingFraction(4.0f);
        break;
    case RaceManager::RD_MEDIUM:
        m_wait_for_players   = true;
        m_max_handicap_accel = 0.95f;
        m_item_tactic        = IT_CALCULATE;
        m_max_start_delay    = 0.4f;
        m_min_steps          = 1;
        m_nitro_level        = NITRO_SOME;
        m_handle_bomb        = true;
        setSkiddingFraction(3.0f);
        break;
    case RaceManager::RD_HARD:
        m_wait_for_players   = false;
        m_max_handicap_accel = 1.0f;
        m_item_tactic        = IT_CALCULATE;
        m_max_start_delay    = 0.1f;
        m_min_steps          = 2;
        m_nitro_level        = NITRO_ALL;
        m_handle_bomb        = true;
        setSkiddingFraction(2.0f);
        break;
    }

#ifdef AI_DEBUG
    m_debug_sphere = irr_driver->getSceneManager()->addSphereSceneNode(1);
    m_debug_left   = irr_driver->getSceneManager()->addSphereSceneNode(1);
    m_debug_right  = irr_driver->getSceneManager()->addSphereSceneNode(1);
#endif
}   // NewAIController

//-----------------------------------------------------------------------------
/** The destructor deletes the shared TrackInfo objects if no more NewAIController
 *  instances are around.
 */
NewAIController::~NewAIController()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
    irr_driver->removeNode(m_debug_left  );
    irr_driver->removeNode(m_debug_right );
#endif
}   // ~NewAIController

//-----------------------------------------------------------------------------
/** Returns the pre-computed successor of a graph node.
 *  \parameter index The index of the graph node for which the successor
 *              is searched.
 */
unsigned int NewAIController::getNextSector(unsigned int index)
{
    return m_successor_index[index];
}   // getNextSector

//-----------------------------------------------------------------------------
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.
void NewAIController::update(float dt)
{
    AIBaseController::update(dt);
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;

    // The client does not do any AI computations.
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) 
        return;

    if( m_world->isStartPhase() )
    {
        handleRaceStart();
        return;
    }

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    int steps = 0;

    steps = calcSteps();

    computeNearestKarts();
    checkCrashes( steps, m_kart->getXYZ() );
    findCurve();

    // Special behaviour if we have a bomb attach: try to hit the kart ahead 
    // of us.
    bool commands_set = false;
    if(m_handle_bomb && 
        m_kart->getAttachment()->getType()==Attachment::ATTACH_BOMB && 
        m_kart_ahead )
    {
        // Use nitro if the kart is far ahead, or faster than this kart
        m_controls->m_nitro = m_distance_ahead>10.0f || 
                              m_kart_ahead->getSpeed() > m_kart->getSpeed();
        // If we are close enough, try to hit this kart
        if(m_distance_ahead<=10)
        {
            Vec3 target = m_kart_ahead->getXYZ();

            // If we are faster, try to predict the point where we will hit
            // the other kart
            if(m_kart_ahead->getSpeed() < m_kart->getSpeed())
            {
                float time_till_hit = m_distance_ahead
                                    / (m_kart->getSpeed()-m_kart_ahead->getSpeed());
                target += m_kart_ahead->getVelocity()*time_till_hit;
            }
            float steer_angle = steerToPoint(m_kart_ahead->getXYZ());
            setSteering(steer_angle, dt);
            commands_set = true;
        }
        handleRescue(dt);
    }
    if(!commands_set)
    {
        /*Response handling functions*/
        handleAcceleration(dt);
        handleSteering(dt);
        handleItems(dt, steps);
        handleRescue(dt);
        handleBraking();
        // If a bomb is attached, nitro might already be set.
        if(!m_controls->m_nitro)
            handleNitroAndZipper();
    }
    // If we are supposed to use nitro, but have a zipper, 
    // use the zipper instead
    if(m_controls->m_nitro && 
        m_kart->getPowerup()->getType()==PowerupManager::POWERUP_ZIPPER && 
        m_kart->getSpeed()>1.0f && 
        m_kart->getSpeedIncreaseTimeLeft(MaxSpeed::MS_INCREASE_ZIPPER)<=0)
    {
        // Make sure that not all AI karts use the zipper at the same
        // time in time trial at start up, so during the first 5 seconds
        // this is done at random only.
        if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_TIME_TRIAL ||
            (m_world->getTime()<3.0f && rand()%50==1) )
        {
            m_controls->m_nitro = false;
            m_controls->m_fire  = true;
        }
    }
    m_collided = false;
}   // update

//-----------------------------------------------------------------------------
void NewAIController::handleBraking()
{
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER &&
        m_kart->getPosition() < m_world->getKart(0)->getPosition()           &&
        m_kart->getInitialPosition()>1                                          )
    {
        m_controls->m_brake = true;
        return;
    }
        
    const float MIN_SPEED = 5.0f;
    //We may brake if we are about to get out of the road, but only if the
    //kart is on top of the road, and if we won't slow down below a certain
    //limit.
    if (m_crashes.m_road && m_kart->getVelocityLC().getZ() > MIN_SPEED && 
        m_world->isOnRoad(m_kart->getWorldKartId()) )
    {
        float kart_ang_diff = 
            QuadGraph::get()->getAngleToNext(m_track_node,
                                         m_successor_index[m_track_node])
          - m_kart->getHeading();
        kart_ang_diff = normalizeAngle(kart_ang_diff);
        kart_ang_diff = fabsf(kart_ang_diff);

        const float MIN_TRACK_ANGLE = DEGREE_TO_RAD*190.0f;
        const float CURVE_INSIDE_PERC = 0.25f;

        //Brake only if the road does not goes somewhat straight.
        if(m_curve_angle > MIN_TRACK_ANGLE) //Next curve is left
        {
            //Avoid braking if the kart is in the inside of the curve, but
            //if the curve angle is bigger than what the kart can steer, brake
            //even if we are in the inside, because the kart would be 'thrown'
            //out of the curve.
            if(!(m_world->getDistanceToCenterForKart(m_kart->getWorldKartId()) 
                 > QuadGraph::get()->getNode(m_track_node).getPathWidth() *
                 -CURVE_INSIDE_PERC || m_curve_angle > RAD_TO_DEGREE*m_kart->getMaxSteerAngle()))
            {
                m_controls->m_brake = false;
                return;
            }
        }
        else if( m_curve_angle < -MIN_TRACK_ANGLE ) //Next curve is right
        {
            if(!(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ) 
                < QuadGraph::get()->getNode(m_track_node).getPathWidth() *
                 CURVE_INSIDE_PERC || m_curve_angle < -RAD_TO_DEGREE*m_kart->getMaxSteerAngle()))
            {
                m_controls->m_brake = false;
                return;
            }
        }

        //Brake if the kart's speed is bigger than the speed we need
        //to go through the curve at the widest angle, or if the kart
        //is not going straight in relation to the road.
        if(m_kart->getVelocityLC().getZ() > m_curve_target_speed ||
           kart_ang_diff                  > MIN_TRACK_ANGLE         )
        {
#ifdef AI_DEBUG
        std::cout << "BRAKING" << std::endl;
#endif
            m_controls->m_brake = true;
            return;
        }

    }

    m_controls->m_brake = false;
}   // handleBraking

//-----------------------------------------------------------------------------
void NewAIController::handleSteering(float dt)
{
    const int next = m_next_node_index[m_track_node];
    
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if( fabsf(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ))  >
       0.5f* QuadGraph::get()->getNode(m_track_node).getPathWidth()+1.0f )
    {
        steer_angle = steerToPoint(m_last_target_point);

#ifdef AI_DEBUG
        m_debug_sphere->setPosition(m_last_target_point.toIrrVector());
        std::cout << "- Outside of road: steer to last aimed at point." <<
            std::endl;
#endif
    }
    //If we are going to crash against a kart, avoid it if it doesn't
    //drives the kart out of the road
    else if( m_crashes.m_kart != -1 && !m_crashes.m_road )
    {
        //-1 = left, 1 = right, 0 = no crash.
        if( m_start_kart_crash_direction == 1 )
        {
            steer_angle = steerToAngle(next, -M_PI*0.5f );
            m_start_kart_crash_direction = 0;
        }
        else if(m_start_kart_crash_direction == -1)
        {
            steer_angle = steerToAngle(next, M_PI*0.5f);
            m_start_kart_crash_direction = 0;
        }
        else
        {
            if(m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() ) >
               m_world->getDistanceToCenterForKart( m_crashes.m_kart ))
            {
                steer_angle = steerToAngle(next, -M_PI*0.5f );
                m_start_kart_crash_direction = 1;
            }
            else
            {
                steer_angle = steerToAngle(next, M_PI*0.5f );
                m_start_kart_crash_direction = -1;
            }
        }

#ifdef AI_DEBUG
        std::cout << "- Velocity vector crashes with kart and doesn't " <<
            "crashes with road : steer 90 degrees away from kart." <<
            std::endl;
#endif

    }
    else
    {
        m_start_kart_crash_direction = 0;
        Vec3 straight_point;
        steer_angle = findNonCrashingAngle();
    }

    setSteering(steer_angle, dt);
}   // handleSteering

//-----------------------------------------------------------------------------
void NewAIController::handleItems( const float DELTA, const int STEPS )
{
    m_controls->m_fire = false;
    if(m_kart->playingEmergencyAnimation() || 
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING ) 
        return;

    m_time_since_last_shot += DELTA;

    // Tactic 1: wait ten seconds, then use item
    // -----------------------------------------
    if(m_item_tactic==IT_TEN_SECONDS)
    {
        if( m_time_since_last_shot > 10.0f )
        {
            m_controls->m_fire = true;
            m_time_since_last_shot = 0.0f;
        }
        return;
    }

    // Tactic 2: calculate
    // -------------------
    switch(m_kart->getPowerup()->getType() )
    {
    case PowerupManager::POWERUP_ZIPPER:
        // Do nothing. Further up a zipper is used if nitro should be selected,
        // saving the (potential more valuable nitro) for later
        break;

    case PowerupManager::POWERUP_BUBBLEGUM:
        // Either use the bubble gum after 10 seconds, or if the next kart 
        // behind is 'close' but not too close (too close likely means that the
        // kart is not behind but more to the side of this kart and so won't 
        // be hit by the bubble gum anyway). Should we check the speed of the
        // kart as well? I.e. only drop if the kart behind is faster? Otoh 
        // this approach helps preventing an overtaken kart to overtake us 
        // again.
        m_controls->m_fire = (m_distance_behind < 15.0f &&
                               m_distance_behind > 3.0f   ) || 
                            m_time_since_last_shot>10.0f;
        if(m_distance_behind < 10.0f && m_distance_behind > 2.0f   )
            m_distance_behind *= 1.0f;
        break;
    // All the thrown/fired items might be improved by considering the angle
    // towards m_kart_ahead. And some of them can fire backwards, too - which
    // isn't yet supported for AI karts.
    case PowerupManager::POWERUP_CAKE:
        m_controls->m_fire = (m_kart_ahead && m_distance_ahead < 20.0f) ||
                             m_time_since_last_shot > 10.0f;
        break;
    case PowerupManager::POWERUP_BOWLING:
        {
            // Bowling balls slower, so only fire on closer karts - but when
            // firing backwards, the kart can be further away, since the ball
            // acts a bit like a mine (and the kart is racing towards it, too)
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind 
                                            : m_distance_ahead;
            m_controls->m_fire = (fire_backwards && distance < 30.0f)  || 
                                 (!fire_backwards && distance <10.0f)  ||
                                 m_time_since_last_shot > 10.0f;
            if(m_controls->m_fire)
                m_controls->m_look_back = fire_backwards;
            break;
        }
    case PowerupManager::POWERUP_PLUNGER:
        {
            // Plungers can be fired backwards and are faster,
            // so allow more distance for shooting.
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind 
                                            : m_distance_ahead;
            m_controls->m_fire = distance               < 30.0f || 
                                 m_time_since_last_shot > 10.0f;
            if(m_controls->m_fire)
                m_controls->m_look_back = fire_backwards;
            break;
        }
    case PowerupManager::POWERUP_ANVIL:
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_controls->m_fire = m_world->getTime()<1.0f && m_kart->getPosition()>2;
        }
        else
        {
            m_controls->m_fire = m_time_since_last_shot > 3.0f && 
                                 m_kart->getPosition()>1;
        }
    case PowerupManager::POWERUP_RUBBERBALL:
        // Perhaps some more sophisticated algorithm might be useful.
        // For now: fire if there is a kart ahead (which means that
        // this kart is certainly not the first kart)
        m_controls->m_fire = m_kart_ahead != NULL;
        break;

    case PowerupManager::POWERUP_SWATTER:  // fallthrough
    default:
        m_controls->m_fire = true;
    }
    if(m_controls->m_fire)  m_time_since_last_shot = 0.0f;
    return;
}   // handleItems

//-----------------------------------------------------------------------------
/** Determines the closest karts just behind and in front of this kart. The
 *  'closeness' is for now simply based on the position, i.e. if a kart is
 *  more than one lap behind or ahead, it is not considered to be closest.
 */
void NewAIController::computeNearestKarts()
{
    bool need_to_check = false;
    int my_position    = m_kart->getPosition();
    // See if the kart ahead has changed:
    if( ( m_kart_ahead && m_kart_ahead->getPosition()+1!=my_position ) ||
        (!m_kart_ahead && my_position>1                              )    )
       need_to_check = true;
    // See if the kart behind has changed:
    if( ( m_kart_behind && m_kart_behind->getPosition()-1!=my_position   ) ||
        (!m_kart_behind && my_position<(int)m_world->getCurrentNumKarts())    )
        need_to_check = true;
    if(!need_to_check) return;

    m_kart_behind    = m_kart_ahead      = NULL;
    m_distance_ahead = m_distance_behind = 9999999.9f;
    float my_dist = m_world->getDistanceDownTrackForKart(m_kart->getWorldKartId());
    for(unsigned int i=0; i<m_world->getNumKarts(); i++)
    {
        Kart *k = m_world->getKart(i);
        if(k->isEliminated() || k==m_kart) continue;
        if(k->getPosition()==my_position+1) 
        {
            m_kart_behind = k;
            m_distance_behind = my_dist - m_world->getDistanceDownTrackForKart(i);
            if(m_distance_behind<0.0f)
                m_distance_behind += m_track->getTrackLength();
        }
        else 
            if(k->getPosition()==my_position-1)
            {
                m_kart_ahead = k;
                m_distance_ahead = m_world->getDistanceDownTrackForKart(i) - my_dist;
                if(m_distance_ahead<0.0f)
                    m_distance_ahead += m_track->getTrackLength();
            }
    }   // for i<world->getNumKarts()
}   // computeNearestKarts

//-----------------------------------------------------------------------------
void NewAIController::handleAcceleration( const float DELTA )
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
        if(!(m_kart->getSpeed() > m_kart->getCurrentMaxSpeed() / 2))
            m_controls->m_accel = 0.05f;
        else 
            m_controls->m_accel = 0.0f;
        return;
    }
    
    if( m_wait_for_players )
    {
        //Find if any player is ahead of this kart
        bool player_winning = false;
        for(unsigned int i = 0; i < race_manager->getNumPlayers(); ++i )
            if( m_kart->getPosition() > m_world->getPlayerKart(i)->getPosition() )
            {
                player_winning = true;
                break;
            }

        if( player_winning )
        {
            m_controls->m_accel = m_max_handicap_accel;
            return;
        }
    }

    m_controls->m_accel = stk_config->m_ai_acceleration;
}   // handleAcceleration

//-----------------------------------------------------------------------------
void NewAIController::handleRaceStart()
{
    //FIXME: make karts able to get a penalty for accelerating too soon
    //like players, should happen to about 20% of the karts in easy,
    //5% in medium and less than 1% of the karts in hard.
    if( m_time_till_start <  0.0f )
    {
        //Each kart starts at a different, random time, and the time is
        //smaller depending on the difficulty.
        m_time_till_start = ( float ) rand() / RAND_MAX * m_max_start_delay;
    }
}   // handleRaceStart

//-----------------------------------------------------------------------------
void NewAIController::handleRescue(const float DELTA)
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
/** Decides wether to use nitro or not.
 */
void NewAIController::handleNitroAndZipper()
{
    m_controls->m_nitro = false;
    // If we are already very fast, save nitro.
    if(m_kart->getSpeed() > 0.95f*m_kart->getCurrentMaxSpeed())
        return;
    // Don't use nitro when the AI has a plunger in the face!
    if(m_kart->hasViewBlockedByPlunger()) return;
    
    // Don't use nitro if the kart doesn't have any or is not on ground.
    if(!m_kart->isOnGround() || m_kart->hasFinishedRace()) return;
    
    // Don't compute nitro usage if we don't have nitro or are not supposed
    // to use it, and we don't have a zipper or are not supposed to use
    // it (calculated).
    if( (m_kart->getEnergy()==0 || m_nitro_level==NITRO_NONE)  &&
        (m_kart->getPowerup()->getType()!=PowerupManager::POWERUP_ZIPPER  ||
         m_item_tactic==IT_TEN_SECONDS                                      ) )
        return;

    // If a parachute or anvil is attached, the nitro doesn't give much
    // benefit. Better wait till later.
    const bool has_slowdown_attachment = 
        m_kart->getAttachment()->getType()==Attachment::ATTACH_PARACHUTE ||
        m_kart->getAttachment()->getType()==Attachment::ATTACH_ANVIL;
    if(has_slowdown_attachment) return;

    // If the kart is very slow (e.g. after rescue), use nitro
    if(m_kart->getSpeed()<5)
    {
        m_controls->m_nitro = true;
        return;
    }

    // If this kart is the last kart, and we have enough 
    // (i.e. more than 2) nitro, use it.
    // -------------------------------------------------
    const unsigned int num_karts = m_world->getCurrentNumKarts();
    if(m_kart->getPosition()== (int)num_karts && m_kart->getEnergy()>2.0f)
    {
        m_controls->m_nitro = true;
        return;
    }

    // On the last track shortly before the finishing line, use nitro 
    // anyway. Since the kart is faster with nitro, estimate a 50% time
    // decrease (additionally some nitro will be saved when top speed
    // is reached).
    if(m_world->getLapForKart(m_kart->getWorldKartId())==race_manager->getNumLaps()-1 &&
        m_nitro_level == NITRO_ALL)
    {
        float finish = m_world->getEstimatedFinishTime(m_kart->getWorldKartId());
        if( 1.5f*m_kart->getEnergy() >= finish - m_world->getTime() )
        {
            m_controls->m_nitro = true;
            return;
        }
    }

    // A kart within this distance is considered to be overtaking (or to be
    // overtaken).
    const float overtake_distance = 10.0f;

    // Try to overtake a kart that is close ahead, except 
    // when we are already much faster than that kart
    // --------------------------------------------------
    if(m_kart_ahead                               && 
        m_distance_ahead < overtake_distance      &&
        m_kart_ahead->getSpeed()+5.0f > m_kart->getSpeed()   )
    {
            m_controls->m_nitro = true;
            return;
    }

    if(m_kart_behind                          &&
        m_distance_behind < overtake_distance &&
        m_kart_behind->getSpeed() > m_kart->getSpeed()    )
    {
        // Only prevent overtaking on highest level
        m_controls->m_nitro = m_nitro_level==NITRO_ALL;
        return;
    }
    
}   // handleNitroAndZipper

//-----------------------------------------------------------------------------
void NewAIController::checkCrashes( const int STEPS, const Vec3& pos )
{
    //Right now there are 2 kind of 'crashes': with other karts and another
    //with the track. The sight line is used to find if the karts crash with
    //each other, but the first step is twice as big as other steps to avoid
    //having karts too close in any direction. The crash with the track can
    //tell when a kart is going to get out of the track so it steers.
    m_crashes.clear();

    const size_t NUM_KARTS = m_world->getNumKarts();

    //Protection against having vel_normal with nan values
    const Vec3 &VEL = m_kart->getVelocity();
    Vec3 vel_normal(VEL.getX(), 0.0, VEL.getZ());
    float speed=vel_normal.length();
    // If the velocity is zero, no sense in checking for crashes in time
    if(speed==0) return;

    // Time it takes to drive for m_kart_length units.
    float dt = m_kart_length / speed; 
    vel_normal/=speed;

    int current_node = m_track_node;
    for(int i = 1; STEPS > i; ++i)
    {
        Vec3 step_coord = pos + vel_normal* m_kart_length * float(i);

        /* Find if we crash with any kart, as long as we haven't found one
         * yet
         */
        if( m_crashes.m_kart == -1 )
        {
            for( unsigned int j = 0; j < NUM_KARTS; ++j )
            {
                const Kart* kart = m_world->getKart(j);
                if(kart==m_kart||kart->isEliminated()) continue;   // ignore eliminated karts
                const Kart *other_kart = m_world->getKart(j);
                // Ignore karts ahead that are faster than this kart.
                if(m_kart->getVelocityLC().getZ() < other_kart->getVelocityLC().getZ())
                    continue;
                Vec3 other_kart_xyz = other_kart->getXYZ() + other_kart->getVelocity()*(i*dt);
                float kart_distance = (step_coord - other_kart_xyz).length_2d();

                if( kart_distance < m_kart_length)
                    m_crashes.m_kart = j;
            }
        }

        /*Find if we crash with the drivelines*/
        if(current_node!=QuadGraph::UNKNOWN_SECTOR &&
            m_next_node_index[current_node]!=-1)
            QuadGraph::get()->findRoadSector(step_coord, &current_node,
                        /* sectors to test*/ &m_all_look_aheads[current_node]);

        if( current_node == QuadGraph::UNKNOWN_SECTOR)
        {
            m_crashes.m_road = true;
            return;
        }
    }
}   // checkCrashes

//-----------------------------------------------------------------------------
float NewAIController::findNonCrashingAngle()
{    
    unsigned int current_sector = m_next_node_index[m_track_node];
    const Vec3 &xyz   = m_kart->getXYZ();
    const Quad &q     = QuadGraph::get()->getQuadOfNode(current_sector);
    const Vec3 &right = q[2];
    const Vec3 &left  = q[3];
    Vec3 final_right  = q[2];
    Vec3 final_left   = q[3];

    float sign        = 1;
    float very_right  = -atan2(right.getX()-xyz.getX(),
                               right.getZ()-xyz.getZ());
    float very_left   = -atan2(left.getX()-xyz.getX(),
                               left.getZ()-xyz.getZ());
    if(very_left < very_right || very_right>M_PI*0.5f ||
        very_left < -M_PI*0.5f)
    {
        sign = -1;
        very_right  = -atan2(-right.getX()+xyz.getX(),
                             -right.getZ()+xyz.getZ());
        very_left   = -atan2(-left.getX()+xyz.getX(),
                             -left.getZ()+xyz.getZ());
    }
    float dist        = 0;

    while(dist<40.0f)
    {
        const Quad &q = QuadGraph::get()->getQuadOfNode(current_sector);
        const Vec3 &right = q[2];
        const Vec3 &left  = q[3];

        float angle_right = -atan2(sign*(right.getX()-xyz.getX()),
                                   sign*(right.getZ()-xyz.getZ()));
        float angle_left  = -atan2(sign*(left.getX()-xyz.getX()),
                                   sign*(left.getZ()-xyz.getZ()));
#ifdef DO_PRINTS
        printf("angle %f %f %f %f\n",
            very_left, angle_left, angle_right, very_right);
#endif
        // Stop if the left and the right beam overlap.
        if(angle_left<very_right ||
            angle_right>very_left) 
        {
            if(dist<0.1)
                break;
            break;
        }

        if(angle_left <very_left )
        {
            very_left  = angle_left;
            final_left = left;
        }
        if(angle_right>very_right)
        {
            very_right  = angle_right;
            final_right = right;
        }
        dist += QuadGraph::get()->getDistanceToNext(current_sector, 
                                                m_successor_index[current_sector]);
        current_sector = m_next_node_index[current_sector];
    }
    m_last_target_point = (final_left+final_right)*0.5f;
    float steer_angle   = steerToPoint(m_last_target_point);
#ifdef AI_DEBUG
    m_debug_left->setPosition(final_left.toIrrVector());
    m_debug_right->setPosition(final_right.toIrrVector());
    m_debug_sphere->setPosition(m_last_target_point.toIrrVector());
#endif
    return steer_angle;
}   // findNonCrashingAngle

//-----------------------------------------------------------------------------
void NewAIController::reset()
{
    m_time_since_last_shot       = 0.0f;
    m_start_kart_crash_direction = 0;
    m_curve_target_speed         = m_kart->getCurrentMaxSpeed();
    m_curve_angle                = 0.0;
    m_time_till_start            = -1.0f;
    m_crash_time                 = 0.0f;
    m_collided                   = false;
    m_time_since_stuck           = 0.0f;
    m_kart_ahead                 = NULL;
    m_distance_ahead             = 0.0f;
    m_kart_behind                = NULL;
    m_distance_behind            = 0.0f;
    m_track_node               = QuadGraph::UNKNOWN_SECTOR;
    AIBaseController::reset();
    QuadGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node);
    if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
    {
        fprintf(stderr, "Invalid starting position for '%s' - not on track - can be ignored.\n",
                m_kart->getIdent().c_str());
        m_track_node = QuadGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
    }

}   // reset

//-----------------------------------------------------------------------------
/** calc_steps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart.
 *  The calling sequence guarantees that m_future_sector is not UNKNOWN.
 */
int NewAIController::calcSteps()
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
            (int)( QuadGraph::get()->getNode(m_future_sector).getPathWidth()
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
void NewAIController::findCurve()
{
    float total_dist = 0.0f;
    int i;
    for(i = m_track_node; total_dist < m_kart->getVelocityLC().getZ(); 
        i = m_next_node_index[i])
    {
        total_dist += QuadGraph::get()->getDistanceToNext(i, m_successor_index[i]);
    }


    m_curve_angle = 
        normalizeAngle(QuadGraph::get()->getAngleToNext(i, m_successor_index[i])
                      -QuadGraph::get()->getAngleToNext(m_track_node, 
                                                    m_successor_index[m_track_node]) );
    
    m_curve_target_speed = m_kart->getCurrentMaxSpeed();
}   // findCurve
