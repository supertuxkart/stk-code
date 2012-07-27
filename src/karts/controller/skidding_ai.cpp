//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2012 Joerg Henrichs
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
#ifdef DEBUG
   // Enable AeI graphical debugging
#  undef AI_DEBUG
   // Shows left and right lines when using new findNonCrashing function
#  undef AI_DEBUG_NEW_FIND_NON_CRASHING
   // Show the predicted turn circles
#  undef AI_DEBUG_CIRCLES
   // Show the heading of the kart
#  undef AI_DEBUG_KART_HEADING
#endif

#include "karts/controller/skidding_ai.hpp"

#ifdef AI_DEBUG
#  include "irrlicht.h"
   using namespace irr;
#endif

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>

#ifdef AI_DEBUG
#  include "graphics/irr_driver.hpp"
#endif
#include "graphics/show_curve.hpp"
#include "graphics/slip_stream.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/skidding.hpp"
#include "karts/skidding_properties.hpp"
#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "modes/linear_world.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

SkiddingAI::SkiddingAI(AbstractKart *kart) 
                   : AIBaseController(kart)
{
    reset();

    switch( race_manager->getDifficulty())
    {
    case RaceManager::RD_EASY:
        m_wait_for_players        = true;
        m_make_use_of_slipstream  = false;
        m_max_handicap_speed      = 0.9f;
        m_item_tactic             = IT_TEN_SECONDS;
        m_false_start_probability = 0.08f;
        m_min_start_delay         = 0.3f;
        m_max_start_delay         = 0.5f;
        m_min_steps               = 1;
        m_nitro_level             = NITRO_NONE;
        m_handle_bomb             = false;
        setSkiddingFraction(4.0f);
        break;
    case RaceManager::RD_MEDIUM:
        m_wait_for_players        = true;
        m_make_use_of_slipstream  = false;
        m_max_handicap_speed      = 0.95f;
        m_item_tactic             = IT_CALCULATE;
        m_false_start_probability = 0.04f;
        m_min_start_delay         = 0.25f;
        m_max_start_delay         = 0.4f;
        m_min_steps               = 1;
        m_nitro_level             = NITRO_SOME;
        m_handle_bomb             = true;
        setSkiddingFraction(3.0f);
        break;
    case RaceManager::RD_HARD:
        m_wait_for_players        = false;
        m_make_use_of_slipstream  = true;
        m_max_handicap_speed      = 1.0f;
        m_item_tactic             = IT_CALCULATE;
        m_false_start_probability = 0.01f;
        // See http://www.humanbenchmark.com/tests/reactiontime/stats.php
        // Average reaction time is around 0.215 s, so using .15 as minimum
        // gives an AI average slightly above the human average
        m_min_start_delay         = 0.15f;
        m_max_start_delay         = 0.28f;
        m_min_steps               = 2;
        m_nitro_level             = NITRO_ALL;
        m_handle_bomb             = true;
        setSkiddingFraction(2.0f);
        break;
    }

#ifdef AI_DEBUG
    m_debug_sphere = irr_driver->getSceneManager()->addSphereSceneNode(1);
#define CURVE_PREDICT1   0
#define CURVE_KART       1
#define CURVE_LEFT       2
#define CURVE_RIGHT      3
#define CURVE_QG         4
#define NUM_CURVES (CURVE_QG+1)

    m_curve   = new ShowCurve*[NUM_CURVES];
    for(unsigned int i=0; i<NUM_CURVES; i++)
        m_curve[i] = NULL;
#ifdef AI_DEBUG_CIRCLES
    m_curve[CURVE_PREDICT1]  = new ShowCurve(0.05f, 0.5f, 
                                   irr::video::SColor(128,   0,   0, 128));
#endif
#ifdef AI_DEBUG_KART_HEADING
    m_curve[CURVE_KART]      = new ShowCurve(0.5f, 0.5f, 
                                   irr::video::SColor(128,   0,   0, 128));
#endif
#ifdef AI_DEBUG_NEW_FIND_NON_CRASHING
    m_curve[CURVE_LEFT]      = new ShowCurve(0.5f, 0.5f, 
                                   irr::video::SColor(128, 128,   0,   0));
    m_curve[CURVE_RIGHT]     = new ShowCurve(0.5f, 0.5f, 
                                   irr::video::SColor(128,   0, 128,   0));
#endif
    m_curve[CURVE_QG]        = new ShowCurve(0.5f, 0.5f, 
                                   irr::video::SColor(128,   0, 128,   0));
#endif
    setControllerName("Skidding");
}   // SkiddingAI

//-----------------------------------------------------------------------------
/** The destructor deletes the shared TrackInfo objects if no more SkiddingAI
 *  instances are around.
 */
SkiddingAI::~SkiddingAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
    for(unsigned int i=0; i<NUM_CURVES; i++)
    {
        delete m_curve[i];
    }
    delete m_curve;
#endif
}   // ~SkiddingAI

//-----------------------------------------------------------------------------
void SkiddingAI::reset()
{
    m_time_since_last_shot       = 0.0f;
    m_start_kart_crash_direction = 0;
    m_start_delay                = -1.0f;
    m_time_since_stuck           = 0.0f;
    m_kart_ahead                 = NULL;
    m_distance_ahead             = 0.0f;
    m_kart_behind                = NULL;
    m_distance_behind            = 0.0f;
    m_current_curve_radius       = 0.0f;
    m_current_track_direction    = GraphNode::DIR_STRAIGHT;

    AIBaseController::reset();
    m_track_node               = QuadGraph::UNKNOWN_SECTOR;
    QuadGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node);
    if(m_track_node==QuadGraph::UNKNOWN_SECTOR)
    {
        fprintf(stderr, 
                "Invalid starting position for '%s' - not on track"
                " - can be ignored.\n",
                m_kart->getIdent().c_str());
        m_track_node = QuadGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
    }

	AIBaseController::reset();
}   // reset

//-----------------------------------------------------------------------------
const irr::core::stringw& SkiddingAI::getNamePostfix() const 
{
    // Static to avoid returning the address of a temporary stringq
    static irr::core::stringw name="(default)";
    return name;
}   // getNamePostfix

//-----------------------------------------------------------------------------
/** Returns the pre-computed successor of a graph node.
 *  \parameter index The index of the graph node for which the successor
 *              is searched.
 */
unsigned int SkiddingAI::getNextSector(unsigned int index)
{
    return m_successor_index[index];
}   // getNextSector

//-----------------------------------------------------------------------------
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.
void SkiddingAI::update(float dt)
{
    // This is used to enable firing an item backwards.
    m_controls->m_look_back = false;
    m_controls->m_nitro     = false;

    // Don't do anything if there is currently a kart animations shown.
    if(m_kart->getKartAnimation())
        return;

    // Having a non-moving AI can be useful for debugging, e.g. aiming
    // or slipstreaming.
#undef AI_DOES_NOT_MOVE_FOR_DEBUGGING
#ifdef AI_DOES_NOT_MOVE_FOR_DEBUGGING
    m_controls->m_accel     = 0;
    m_controls->m_steer     = 0;
    return;
#endif

    // The client does not do any AI computations.
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) 
    {
        AIBaseController::update(dt);
        return;
    }

	// If the kart needs to be rescued, do it now (and nothing else)
	if(isStuck() && !m_kart->getKartAnimation())
	{
		new RescueAnimation(m_kart);
		AIBaseController::update(dt);
		return;
	}

    if( m_world->isStartPhase() )
    {
        handleRaceStart();
        AIBaseController::update(dt);
        return;
    }

    // Get information that is needed by more than 1 of the handling funcs
    computeNearestKarts();

    //Detect if we are going to crash with the track and/or kart
    checkCrashes(m_kart->getXYZ());
    determineTrackDirection();
    
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
            float steer_angle = steerToPoint(target);
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
        handleItems(dt);
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

    /*And obviously general kart stuff*/
    AIBaseController::update(dt);
}   // update

//-----------------------------------------------------------------------------
void SkiddingAI::handleBraking()
{
    m_controls->m_brake = false;
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER &&
        m_kart->getPosition() < m_world->getKart(0)->getPosition()           &&
        m_kart->getInitialPosition()>1                                         )
    {
#ifdef DEBUG
    if(m_ai_debug)
        printf("[AI] braking: %s ahead of leader.\n", 
               m_kart->getIdent().c_str());
#endif

        m_controls->m_brake = true;
        return;
    }
    
    // A kart will not brake when the speed is already slower than this 
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    // If the kart is not facing roughly in the direction of the track, brake
    // so that it is easier for the kart to turn in the right direction.
    if(m_current_track_direction==GraphNode::DIR_UNDEFINED &&
        m_kart->getSpeed() > MIN_SPEED)
    {
#ifdef DEBUG
        if(m_ai_debug)
            printf("[AI] braking: %s not aligned with track.\n",
            m_kart->getIdent().c_str());
#endif
        //m_controls->m_brake = true;
        return;
    }
    if(m_current_track_direction==GraphNode::DIR_LEFT ||
       m_current_track_direction==GraphNode::DIR_RIGHT   )
    {
        float max_turn_speed = 
            m_kart->getKartProperties()
                   ->getSpeedForTurnRadius(m_current_curve_radius);

        if(m_kart->getSpeed() > 1.5f*max_turn_speed  && 
            m_kart->getSpeed()>MIN_SPEED             &&
            fabsf(m_controls->m_steer) > 0.95f          )
        {
            m_controls->m_brake = true;
#ifdef DEBUG
            if(m_ai_debug)
                printf("[AI] braking: %s too tight curve: radius %f "
                       "speed %f.\n",
                       m_kart->getIdent().c_str(),
                       m_current_curve_radius, m_kart->getSpeed() );
#endif
        }
        return;
    }

    return;

}   // handleBraking

//-----------------------------------------------------------------------------
void SkiddingAI::handleSteering(float dt)
{
    const int next = m_next_node_index[m_track_node];
    
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
	float side_dist = 
		m_world->getDistanceToCenterForKart( m_kart->getWorldKartId() );
    if( fabsf(side_dist)  >
       0.5f* QuadGraph::get()->getNode(m_track_node).getPathWidth()+0.5f )
    {
        steer_angle = steerToPoint(QuadGraph::get()->getQuadOfNode(next)
                                                    .getCenter());

#ifdef AI_DEBUG
        m_debug_sphere->setPosition(QuadGraph::get()->getQuadOfNode(next)
                       .getCenter().toIrrVector());
        std::cout << "- Outside of road: steer to center point." <<
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
#undef NEW_ALGORITHM
#ifdef NEW_ALGORITHM
        findNonCrashingPoint2(&straight_point);
#else
        findNonCrashingPoint(&straight_point);
#endif

#ifdef AI_DEBUG
        m_debug_sphere->setPosition(straight_point.toIrrVector());
#endif
        steer_angle = steerToPoint(straight_point);
    }

    setSteering(steer_angle, dt);
}   // handleSteering

//-----------------------------------------------------------------------------
/** Handle all items depending on the chosen strategy: Either (low level AI)
 *  just use an item after 10 seconds, or do a much better job on higher level
 *  AI - e.g. aiming at karts ahead/behind, wait an appropriate time before 
 *  using multiple items etc.
 */
void SkiddingAI::handleItems(const float dt)
{
    m_controls->m_fire = false;
    if(m_kart->getKartAnimation() || 
        m_kart->getPowerup()->getType() == PowerupManager::POWERUP_NOTHING ) 
        return;

    m_time_since_last_shot += dt;

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
    switch( m_kart->getPowerup()->getType() )
    {
    case PowerupManager::POWERUP_BUBBLEGUM:
        // Avoid dropping all bubble gums one after another
        if( m_time_since_last_shot <3.0f) break;

        // Either use the bubble gum after 10 seconds, or if the next kart 
        // behind is 'close' but not too close (too close likely means that the
        // kart is not behind but more to the side of this kart and so won't 
        // be hit by the bubble gum anyway). Should we check the speed of the
        // kart as well? I.e. only drop if the kart behind is faster? Otoh 
        // this approach helps preventing an overtaken kart to overtake us 
        // again.
        m_controls->m_fire = (m_distance_behind < 15.0f &&
                              m_distance_behind > 3.0f    );
        break;   // POWERUP_BUBBLEGUM

    // All the thrown/fired items might be improved by considering the angle
    // towards m_kart_ahead.
    case PowerupManager::POWERUP_CAKE:
        {
            // Leave some time between shots
            if(m_time_since_last_shot<3.0f) break;
            // Since cakes can be fired all around, just use a sane distance
            // with a bit of extra for backwards, as enemy will go towards cake
            bool fire_backwards = (m_kart_behind && m_kart_ahead &&
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind
                                            : m_distance_ahead;
            m_controls->m_fire = (fire_backwards && distance < 25.0f)  ||
                                 (!fire_backwards && distance < 20.0f);
            if(m_controls->m_fire)
                m_controls->m_look_back = fire_backwards;
            break;
        }   // POWERUP_CAKE

    case PowerupManager::POWERUP_BOWLING:
        {
            // Leave more time between bowling balls, since they are 
            // slower, so it should take longer to hit something which
            // can result in changing our target.
            if(m_time_since_last_shot < 5.0f) break;
            // Bowling balls are slower, so only fire on closer karts - but when
            // firing backwards, the kart can be further away, since the ball
            // acts a bit like a mine (and the kart is racing towards it, too)
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance = fire_backwards ? m_distance_behind 
                                            : m_distance_ahead;
            m_controls->m_fire = ( (fire_backwards && distance < 30.0f)  ||
                                   (!fire_backwards && distance <10.0f)    ) &&
                                m_time_since_last_shot > 3.0f;
            if(m_controls->m_fire)
                m_controls->m_look_back = fire_backwards;
            break;
        }   // POWERUP_BOWLING

    case PowerupManager::POWERUP_ZIPPER:
        // Do nothing. Further up a zipper is used if nitro should be selected,
        // saving the (potential more valuable nitro) for later
        break;   // POWERUP_ZIPPER

    case PowerupManager::POWERUP_PLUNGER:
        {
            // Leave more time after a plunger, since it will take some
            // time before a plunger effect becomes obvious.
            if(m_time_since_last_shot < 5.0f) break;

            // Plungers can be fired backwards and are faster,
            // so allow more distance for shooting.
            bool fire_backwards = (m_kart_behind && m_kart_ahead && 
                                   m_distance_behind < m_distance_ahead) ||
                                  !m_kart_ahead;
            float distance      = fire_backwards ? m_distance_behind 
                                                 : m_distance_ahead;
            m_controls->m_fire  = distance < 30.0f                 || 
                                  m_time_since_last_shot > 10.0f;
            if(m_controls->m_fire)
                m_controls->m_look_back = fire_backwards;
            break;
        }   // POWERUP_PLUNGER

    case PowerupManager::POWERUP_SWITCH:
        // For now don't use a switch if this kart is first (since it's more 
        // likely that this kart then gets a good iteam), otherwise use it 
        // after a waiting an appropriate time
        if(m_kart->getPosition()>1 && 
            m_time_since_last_shot > stk_config->m_item_switch_time+2.0f)
            m_controls->m_fire = true;
        break;   // POWERUP_SWITCH

    case PowerupManager::POWERUP_PARACHUTE:
        // Wait one second more than a previous parachute
        if(m_time_since_last_shot > stk_config->m_parachute_time_other+1.0f)
            m_controls->m_fire = true;
        break;   // POWERUP_PARACHUTE

    case PowerupManager::POWERUP_ANVIL:
        // Wait one second more than a previous anvil
        if(m_time_since_last_shot < stk_config->m_anvil_time+1.0f) break;

        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        {
            m_controls->m_fire = m_world->getTime()<1.0f && 
                                 m_kart->getPosition()>2;
        }
        else
        {
            m_controls->m_fire = m_time_since_last_shot > 3.0f && 
                                 m_kart->getPosition()>1;
        }
        break;   // POWERUP_ANVIL

    case PowerupManager::POWERUP_SWATTER:
        {
            // Squared distance for which the swatter works
            float d2 = m_kart->getKartProperties()->getSwatterDistance2();
            // Fire if the closest kart ahead or to the back is not already 
            // squashed and close enough.
            // FIXME: this can be improved on, since more than one kart might 
            //        be hit, and a kart ahead might not be at an angle at 
            //        which the glove can be used.
            if(  ( m_kart_ahead && !m_kart_ahead->isSquashed()             &&
                    (m_kart_ahead->getXYZ()-m_kart->getXYZ()).length2()<d2 &&
                    m_kart_ahead->getSpeed() < m_kart->getSpeed()            ) ||
                 ( m_kart_behind && !m_kart_behind->isSquashed() &&
                    (m_kart_behind->getXYZ()-m_kart->getXYZ()).length2()<d2) )
                    m_controls->m_fire = true;
            break;
        }
    case PowerupManager::POWERUP_RUBBERBALL:
        // Perhaps some more sophisticated algorithm might be useful.
        // For now: fire if there is a kart ahead (which means that
        // this kart is certainly not the first kart)
        m_controls->m_fire = m_kart_ahead != NULL;
        break;
    default:
        printf("Invalid or unhandled powerup '%d' in default AI.\n",
                m_kart->getPowerup()->getType());
        assert(false);
    }
    if(m_controls->m_fire)  m_time_since_last_shot = 0.0f;
}   // handleItems

//-----------------------------------------------------------------------------
/** Determines the closest karts just behind and in front of this kart. The
 *  'closeness' is for now simply based on the position, i.e. if a kart is
 *  more than one lap behind or ahead, it is not considered to be closest.
 */
void SkiddingAI::computeNearestKarts()
{
    int my_position    = m_kart->getPosition();

    // See if the kart ahead has changed:
    if( ( m_kart_ahead && m_kart_ahead->getPosition()+1!=my_position ) ||
        (!m_kart_ahead && my_position>1                              )    )
    {
        m_kart_ahead = m_world->getKartAtPosition(my_position-1);
        if(m_kart_ahead && 
              ( m_kart_ahead->isEliminated() || m_kart_ahead->hasFinishedRace()))
              m_kart_ahead = NULL;
    }

    // See if the kart behind has changed:
    if( ( m_kart_behind && m_kart_behind->getPosition()-1!=my_position   ) ||
        (!m_kart_behind && my_position<(int)m_world->getCurrentNumKarts())    )
    {
        m_kart_behind = m_world->getKartAtPosition(my_position+1);
        if(m_kart_behind && 
            (m_kart_behind->isEliminated() || m_kart_behind->hasFinishedRace()))
            m_kart_behind = NULL;
    }

    m_distance_ahead = m_distance_behind = 9999999.9f;
    float my_dist = m_world->getDistanceDownTrackForKart(m_kart->getWorldKartId());
    if(m_kart_ahead)
    {
        m_distance_ahead = 
            m_world->getDistanceDownTrackForKart(m_kart_ahead->getWorldKartId())
          - my_dist;
        if(m_distance_ahead<0.0f)
            m_distance_ahead += m_track->getTrackLength();
    }
    if(m_kart_behind)
    {
        m_distance_behind = my_dist
            -m_world->getDistanceDownTrackForKart(m_kart_behind->getWorldKartId());
        if(m_distance_behind<0.0f)
            m_distance_behind += m_track->getTrackLength();
    }
}   // computeNearestKarts

//-----------------------------------------------------------------------------
void SkiddingAI::handleAcceleration( const float dt)
{
    //Do not accelerate until we have delayed the start enough
    if( m_start_delay > 0.0f )
    {
        m_start_delay -= dt;
        m_controls->m_accel = 0.0f;
        return;
    }

    if( m_controls->m_brake )
    {
        m_controls->m_accel = 0.0f;
        return;
    }

    if(m_kart->hasViewBlockedByPlunger())
    {
        if(m_kart->getSpeed() < m_kart->getCurrentMaxSpeed() / 2)
            m_controls->m_accel = 0.05f;
        else 
            m_controls->m_accel = 0.0f;
        return;
    }
    
    m_controls->m_accel = stk_config->m_ai_acceleration;
    if(!m_wait_for_players)
        return;

    //Find if any player is ahead of this kart
    for(unsigned int i = 0; i < race_manager->getNumPlayers(); ++i )
    {
        if( m_kart->getPosition() > m_world->getPlayerKart(i)->getPosition() )
        {
            m_controls->m_accel = m_max_handicap_speed;
            return;
        }
    }

}   // handleAcceleration

//-----------------------------------------------------------------------------
void SkiddingAI::handleRaceStart()
{
    if( m_start_delay <  0.0f )
    {
        // Each kart starts at a different, random time, and the time is
        // smaller depending on the difficulty.
        m_start_delay = m_min_start_delay 
                      + (float) rand() / RAND_MAX * (m_max_start_delay-m_min_start_delay);

        // Now check for a false start. If so, add 1 second penalty time.
        if(rand() < RAND_MAX * m_false_start_probability)
        {
            m_start_delay+=stk_config->m_penalty_time;
            return;
        }
    }
}   // handleRaceStart

//-----------------------------------------------------------------------------
void SkiddingAI::handleRescue(const float dt)
{
    // check if kart is stuck
    if(m_kart->getSpeed()<2.0f && !m_kart->getKartAnimation() && 
        !m_world->isStartPhase())
    {
        m_time_since_stuck += dt;
        if(m_time_since_stuck > 2.0f)
        {
            new RescueAnimation(m_kart);
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
void SkiddingAI::handleNitroAndZipper()
{
    m_controls->m_nitro = false;
    // If we are already very fast, save nitro.
    if(m_kart->getSpeed() > 0.95f*m_kart->getCurrentMaxSpeed())
        return;
    // Don't use nitro when the AI has a plunger in the face!
    if(m_kart->hasViewBlockedByPlunger()) return;
    
    // Don't use nitro if we are braking
    if(m_controls->m_brake) return;

    // Don't use nitro if the kart doesn't have any or is not on ground.
    if(!m_kart->isOnGround() || m_kart->hasFinishedRace()) return;
    
    // Don't compute nitro usage if we don't have nitro or are not supposed
    // to use it, and we don't have a zipper or are not supposed to use
    // it (calculated).
    if( (m_kart->getEnergy()==0 || m_nitro_level==NITRO_NONE)  &&
        (m_kart->getPowerup()->getType()!=PowerupManager::POWERUP_ZIPPER ||
          m_item_tactic==IT_TEN_SECONDS                                    ) )
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
    if(m_kart_ahead                                       && 
        m_distance_ahead < overtake_distance              &&
        m_kart_ahead->getSpeed()+5.0f > m_kart->getSpeed()   )
    {
            m_controls->m_nitro = true;
            return;
    }

    if(m_kart_behind                                   &&
        m_distance_behind < overtake_distance          &&
        m_kart_behind->getSpeed() > m_kart->getSpeed()    )
    {
        // Only prevent overtaking on highest level
        m_controls->m_nitro = m_nitro_level==NITRO_ALL;
        return;
    }
    
}   // handleNitroAndZipper

//-----------------------------------------------------------------------------
void SkiddingAI::checkCrashes(const Vec3& pos )
{
    int steps = int( m_kart->getVelocityLC().getZ() / m_kart_length );
    if( steps < m_min_steps ) steps = m_min_steps;

    // The AI drives significantly better with more steps, so for now
    // add 5 additional steps.
    steps+=5;

    //Right now there are 2 kind of 'crashes': with other karts and another
    //with the track. The sight line is used to find if the karts crash with
    //each other, but the first step is twice as big as other steps to avoid
    //having karts too close in any direction. The crash with the track can
    //tell when a kart is going to get out of the track so it steers.
    m_crashes.clear();

    // If slipstream should be handled actively, trigger overtaking the
    // kart which gives us slipstream if slipstream is ready
    const SlipStream *slip=m_kart->getSlipstream();
    if(m_make_use_of_slipstream && slip->isSlipstreamReady() &&
        slip->getSlipstreamTarget())
    {
        //printf("%s overtaking %s\n", m_kart->getIdent().c_str(),
        //    m_kart->getSlipstreamKart()->getIdent().c_str());
        // FIXME: we might define a minimum distance, and if the target kart
        // is too close break first - otherwise the AI hits the kart when
        // trying to overtake it, actually speeding the other kart up.
        m_crashes.m_kart = slip->getSlipstreamTarget()->getWorldKartId();
    }

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
    if(steps<1 || steps>1000)
    {
        printf("Warning, incorrect STEPS=%d. kart_length %f velocity %f\n",
            steps, m_kart_length, m_kart->getVelocityLC().getZ());
        steps=1000;
    }
    for(int i = 1; steps > i; ++i)
    {
        Vec3 step_coord = pos + vel_normal* m_kart_length * float(i);

        /* Find if we crash with any kart, as long as we haven't found one
         * yet
         */
        if( m_crashes.m_kart == -1 )
        {
            for( unsigned int j = 0; j < NUM_KARTS; ++j )
            {
                const AbstractKart* kart = m_world->getKart(j);
                // Ignore eliminated karts
                if(kart==m_kart||kart->isEliminated()) continue;
                const AbstractKart *other_kart = m_world->getKart(j);
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
/** This is a new version of findNonCrashingPoint, which at this stage is
 *  slightly inferior (though faster and more correct) than the original 
 *  version - the original code cuts corner more aggressively than this
 *  version (and in most cases cuting the corner does not end in a 
 *  collision, so it's actually faster).
 *  This version find the point furthest ahead which can be reached by
 *  travelling in a straight direction from the current location of the
 *  kart. This is done by using two lines: one from the kart to the
 *  lower left side of the next quad, and one from the kart to the
 *  lower right side of the next quad. The area between those two lines
 *  can be reached by the kart in a straight line, and will not go off
 *  track (assuming that the kart is on track). Then the next quads are 
 *  tested: New left/right lines are computed. If the new left line is to 
 *  the right of the old left line, the new left line becomes the current 
 *  left line:
 *
 *      X       The new left line connecting kart to X will be to the right
 *              of the old left line, so the available space for the kart
 *    \      /  (
 *     \    /
 *      kart
 *  Similarly for the right side. This will narrow down the available area
 *  the kart can aim at, till finally the left and right line overlap.
 *  All points between the connection of the two end points of the left and 
 *  right line can be reached without getting off track. Which point the
 *  kart aims at then depends on the direction of the track: if there is
 *  a left turn, the kart will aim to the left point (and vice versa for
 *  right turn) - slightly offset by the width of the kart to avoid that 
 *  the kart is getting off track.
 *  \return result The new point the kart should aim at when steering.
*/
void SkiddingAI::findNonCrashingPoint2(Vec3 *result)
{    
    unsigned int sector = m_next_node_index[m_track_node];
    const core::vector2df xz = m_kart->getXYZ().toIrrVector2d();

    const Quad &q = QuadGraph::get()->getQuadOfNode(sector);

    // Index of the left and right end of a quad.
    const unsigned int LEFT_END_POINT  = 0;
    const unsigned int RIGHT_END_POINT = 1;
    core::line2df left (xz, q[LEFT_END_POINT ].toIrrVector2d());
    core::line2df right(xz, q[RIGHT_END_POINT].toIrrVector2d());

#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_LEFT]->clear();
    m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps);
    m_curve[CURVE_LEFT]->addPoint(q[LEFT_END_POINT]+eps);
    m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps);
    m_curve[CURVE_RIGHT]->clear();
    m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps);
    m_curve[CURVE_RIGHT]->addPoint(q[RIGHT_END_POINT]+eps);
    m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps);
#endif
#ifdef AI_DEBUG_KART_HEADING
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_KART]->clear();
    m_curve[CURVE_KART]->addPoint(m_kart->getXYZ()+eps);
    Vec3 forw(0, 0, 50);
    m_curve[CURVE_KART]->addPoint(m_kart->getTrans()(forw)+eps);
#endif
    while(1)
    {
        unsigned int next_sector = m_next_node_index[sector];
        const Quad &q_next = QuadGraph::get()->getQuadOfNode(next_sector);
        // Test if the next left point is to the right of the left
        // line. If so, a new left line is defined.
        if(left.getPointOrientation(q_next[LEFT_END_POINT].toIrrVector2d())
            < 0 )
        {
            core::vector2df p = q_next[LEFT_END_POINT].toIrrVector2d();
            // Stop if the new point is to the right of the right line
            if(right.getPointOrientation(p)<0)
                break;
            left.end = p;
#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)
            Vec3 ppp(p.X, m_kart->getXYZ().getY(), p.Y);
            m_curve[CURVE_LEFT]->addPoint(ppp+eps);
            m_curve[CURVE_LEFT]->addPoint(m_kart->getXYZ()+eps);
#endif
        }

        // Test if new right point is to the left of the right line. If
        // so, a new right line is defined.
        if(right.getPointOrientation(q_next[RIGHT_END_POINT].toIrrVector2d())
            > 0 )
        {
            core::vector2df p = q_next[RIGHT_END_POINT].toIrrVector2d();
            // Break if new point is to the left of left line
            if(left.getPointOrientation(p)>0)
                break;
#if defined(AI_DEBUG) && defined(AI_DEBUG_NEW_FIND_NON_CRASHING)

            Vec3 ppp(p.X, m_kart->getXYZ().getY(), p.Y);
            m_curve[CURVE_RIGHT]->addPoint(ppp+eps);
            m_curve[CURVE_RIGHT]->addPoint(m_kart->getXYZ()+eps);
#endif
            right.end = p;
        }
        sector = next_sector;
    }   // while
    
    // Now look for the next curve to find out to which side of the
    // track the AI should aim at

    sector = m_track_node;
    int count = 0;
    while(1)
    {
        GraphNode::DirectionType dir;
        unsigned int last;
        unsigned int succ = m_successor_index[sector];
        const GraphNode &gn = QuadGraph::get()->getNode(sector);
        gn.getDirectionData(succ, &dir, &last);
        if(dir==GraphNode::DIR_LEFT)
        {
            core::vector2df diff = left.end - right.end;
            diff.normalize();
            diff *= m_kart->getKartWidth()*0.5f;
            *result = Vec3(left.end.X - diff.X,
                           m_kart->getXYZ().getY(), 
                           left.end.Y - diff.Y);
            return;
        }
        else if(dir==GraphNode::DIR_RIGHT)
        {
            core::vector2df diff = right.end - left.end;
            diff.normalize();
            diff *= m_kart->getKartWidth()*0.5f;
            *result = Vec3(right.end.X-diff.X, 
                           m_kart->getXYZ().getY(),
                           right.end.Y-diff.Y);
            return;
        }

        // We are going straight. Determine point to aim for based on the 
        // direction of the track after the straight section

        sector = m_next_node_index[last];
        count++;
        if(count>1)
            printf("That shouldn't happen %d!!!\n", count);
    }
    
    Vec3 ppp(0.5f*(left.end.X+right.end.X),
             m_kart->getXYZ().getY(),
             0.5f*(left.end.Y+right.end.Y));
    *result = QuadGraph::get()->getQuadOfNode(sector).getCenter();
    *result = ppp;
}   // findNonCrashingPoint2

//-----------------------------------------------------------------------------
/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve after wards,
 *  and return the position of that edge.
 */
void SkiddingAI::findNonCrashingPoint(Vec3 *result)
{    
#ifdef AI_DEBUG_KART_HEADING
    const Vec3 eps(0,0.5f,0);
    m_curve[CURVE_KART]->clear();
    m_curve[CURVE_KART]->addPoint(m_kart->getXYZ()+eps);
    Vec3 forw(0, 0, 50);
    m_curve[CURVE_KART]->addPoint(m_kart->getTrans()(forw)+eps);
#endif
    unsigned int sector = m_next_node_index[m_track_node];
    int target_sector;

    Vec3 direction;
    Vec3 step_track_coord;

    // The original while(1) loop is replaced with a for loop to avoid
    // infinite loops (which we had once or twice). Usually the number
    // of iterations in the while loop is less than 7.
    for(unsigned int i=0; i<100; i++)
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        target_sector = m_next_node_index[sector];

        //direction is a vector from our kart to the sectors we are testing
        direction = QuadGraph::get()->getQuadOfNode(target_sector).getCenter()
                  - m_kart->getXYZ();

        float len=direction.length_2d();
        unsigned int steps = (unsigned int)( len / m_kart_length );
        if( steps < 3 ) steps = 3;

        // That shouldn't happen, but since we had one instance of
        // STK hanging, add an upper limit here (usually it's at most
        // 20 steps)
        if( steps>1000) steps = 1000;

        //Protection against having vel_normal with nan values
        if(len>0.0f) {
            direction*= 1.0f/len;
        }

        Vec3 step_coord;
        //Test if we crash if we drive towards the target sector
        for(unsigned int i = 2; i < steps; ++i )
        {
            step_coord = m_kart->getXYZ()+direction*m_kart_length * float(i);

            QuadGraph::get()->spatialToTrack(&step_track_coord, step_coord,
                                             sector );
 
            float distance = fabsf(step_track_coord[0]);

            //If we are outside, the previous sector is what we are looking for
            if ( distance + m_kart_width * 0.5f 
                 > QuadGraph::get()->getNode(sector).getPathWidth() )
            {
                *result = QuadGraph::get()->getQuadOfNode(sector).getCenter();
                return;
            }
        }
        sector = target_sector;
    }   // for i<100
    *result = QuadGraph::get()->getQuadOfNode(sector).getCenter();
}   // findNonCrashingPoint

//-----------------------------------------------------------------------------
/** Determines the direction of the track ahead of the kart: 0 indicates 
 *  straight, +1 right turn, -1 left turn.
 */
void SkiddingAI::determineTrackDirection()
{
    const QuadGraph *qg = QuadGraph::get();
    unsigned int succ   = m_successor_index[m_track_node];
    float angle_to_track = qg->getNode(m_track_node).getAngleToSuccessor(succ)
                         - m_kart->getHeading();
    angle_to_track = normalizeAngle(angle_to_track);

    // In certain circumstances (esp. S curves) it is possible that the
    // kart is not facing in the direction of the track. In this case
    // determining the curve radius based on the direction the kart is
    // facing results in very incorrect results (example: if the kart is
    // in a tight curve, but already facing towards the last point of the
    // curve - in this case a huge curve radius will be computes (since
    // the kart is nearly going straight), while in fact the kart would
    // go across the circle and not along, bumping into the track).
    // To avoid this we set the direction to undefined in this case,
    // which causes the kart to brake (which will allow the kart to
    // quicker be aligned with the track again).
    if(fabsf(angle_to_track) > 0.22222f * M_PI)
    {
        m_current_track_direction = GraphNode::DIR_UNDEFINED;
        return;
    }

    unsigned int next   = qg->getNode(m_track_node).getSuccessor(succ);

    qg->getNode(next).getDirectionData(m_successor_index[next], 
                                       &m_current_track_direction, 
                                       &m_last_direction_node);

#ifdef AI_DEBUG
    m_curve[CURVE_QG]->clear();
    for(unsigned int i=m_track_node; i<=m_last_direction_node; i++)
    {
        m_curve[CURVE_QG]->addPoint(qg->getNode(i).getCenter());
    }
#endif
    m_controls->m_skid = false;

    if(m_current_track_direction==GraphNode::DIR_LEFT  ||
       m_current_track_direction==GraphNode::DIR_RIGHT   )
    {
        handleCurve();
    }   // if(m_current_track_direction == DIR_LEFT || DIR_RIGHT   )


    return;
}   // determineTrackDirection

// ----------------------------------------------------------------------------
/** If the kart is at/in a curve, determine the turn radius.
 */
void SkiddingAI::handleCurve()
{
    // Ideally we would like to have a circle that:
    // 1) goes through the kart position
    // 2) has the current heading of the kart as tangent in that point
    // 3) goes through the last point
    // 4) has a tangent at the last point that faces towards the next node
    // Unfortunately conditions 1 to 3 already fully determine the circle,
    // i.e. it is not always possible to find an appropriate circle.
    // Using the first three conditions is mostly a good choice (since the
    // kart will already point towards the direction of the circle), and
    // the case that the kart is facing wrong was already tested for before

    const QuadGraph *qg = QuadGraph::get();
    Vec3 xyz            = m_kart->getXYZ();
    Vec3 tangent        = m_kart->getTrans()(Vec3(0,0,1)) - xyz;
    Vec3 last_xyz       = qg->getNode(m_last_direction_node).getCenter();

    determineTurnRadius(xyz, tangent, last_xyz,
                        &m_curve_center, &m_current_curve_radius);

#ifdef ADJUST_TURN_RADIUS_TO_AVOID_CRASH_INTO_TRACK
    for(unsigned int i=next; i<=last; i++)
    {
        // Pick either the lower left or right point:
        int index = m_current_track_direction==GraphNode::DIR_LEFT
            ? 0 : 1;
        float r = (center - qg->getQuadOfNode(i)[index]).length();
        if(m_current_curve_radius < r)
        {
            determineTurnRadius(xyz, tangent, qg->getQuadOfNode(i)[index],
                &center, &m_current_curve_radius);
            break;
        }
    }
#endif
#if defined(AI_DEBUG) && defined(AI_DEBUG_CIRCLES)
    m_curve[CURVE_PREDICT1]->makeCircle(m_curve_center, 
                                        m_current_curve_radius);
    m_curve[CURVE_PREDICT1]->addPoint(last_xyz);
    m_curve[CURVE_PREDICT1]->addPoint(m_curve_center);
    m_curve[CURVE_PREDICT1]->addPoint(xyz);
#endif

}   // handleCurve
// ----------------------------------------------------------------------------
/** Determines if the kart should skid. The base implementation enables
 *  skidding 
 *  \param steer_fraction The steering fraction as computed by the 
 *          AIBaseController.
 *  \return True if the kart should skid.
 */
bool SkiddingAI::doSkid(float steer_fraction)
{
    // No skidding on straights
    if(m_current_track_direction==GraphNode::DIR_STRAIGHT)
        return false;

    const float MIN_SKID_SPEED = 5.0f;
    const QuadGraph *qg = QuadGraph::get();
    Vec3 last_xyz       = qg->getNode(m_last_direction_node).getCenter();

    // Only try skidding when a certain minimum speed is reached.
    if(m_kart->getSpeed()<MIN_SKID_SPEED) return false;

    // Estimate how long it takes to finish the curve
    Vec3 diff_kart = m_kart->getXYZ() - m_curve_center;
    Vec3 diff_last = last_xyz         - m_curve_center;
    float angle_kart = atan2(diff_kart.getX(), diff_kart.getZ());
    float angle_last = atan2(diff_last.getX(), diff_last.getZ());
    float angle = m_current_track_direction == GraphNode::DIR_RIGHT
                ? angle_last - angle_kart
                : angle_kart - angle_last;
    angle = normalizeAngle(angle);
    float length = m_current_curve_radius*fabsf(angle);
    float duration = length / m_kart->getSpeed();
    duration *= 1.5f;
    const Skidding *skidding = m_kart->getSkidding();
    if(m_controls->m_skid && duration < 1.0f)
    {
        if(m_ai_debug)
            printf("[AI] skid : '%s' too short, stop skid.\n",
            m_kart->getIdent().c_str());
        return false;
    }

    else if(skidding->getNumberOfBonusTimes()>0 &&
        skidding->getTimeTillBonus(0) < duration)
    {
#ifdef DEBUG
        if(m_ai_debug)
            printf("[AI] skid: %s start skid, duration %f.\n",
            m_kart->getIdent().c_str(), duration);
#endif
        return true;

    }  // if curve long enough for skidding

    return false;
}   // doSkid

//-----------------------------------------------------------------------------
/** Converts the steering angle to a lr steering in the range of -1 to 1. 
 *  If the steering angle is too great, it will also trigger skidding. This 
 *  function uses a 'time till full steer' value specifying the time it takes
 *  for the wheel to reach full left/right steering similar to player karts 
 *  when using a digital input device. The parameter is defined in the kart 
 *  properties and helps somewhat to make AI karts more 'pushable' (since
 *  otherwise the karts counter-steer to fast).
 *  It also takes the effect of a plunger into account by restricting the
 *  actual steer angle to 50% of the maximum.
 *  \param angle Steering angle.
 *  \param dt Time step.
 */
void SkiddingAI::setSteering(float angle, float dt)
{
    float steer_fraction = angle / m_kart->getMaxSteerAngle();

    m_controls->m_skid   = doSkid(steer_fraction);

    // Adjust steer fraction in case to be in [-1,1]
    if     (steer_fraction >  1.0f) steer_fraction =  1.0f;
    else if(steer_fraction < -1.0f) steer_fraction = -1.0f;

    // Restrict steering when a plunger is in the face
    if(m_kart->hasViewBlockedByPlunger())
    {
        if     (steer_fraction >  0.5f) steer_fraction =  0.5f;
        else if(steer_fraction < -0.5f) steer_fraction = -0.5f;
    }
    
    const Skidding *skidding = m_kart->getSkidding();

    // If we are supposed to skid, but the current steering is still 
    // in the wrong direction, don't start to skid just now, since then
    // we can't turn into the direction we want to anymore (see 
    // Skidding class)
    Skidding::SkidState ss = skidding->getSkidState();
    if(ss==Skidding::SKID_ACCUMULATE_LEFT  && steer_fraction>0 ||
       ss==Skidding::SKID_ACCUMULATE_RIGHT && steer_fraction<0    )
    {
        m_controls->m_skid = false;
#ifdef DEBUG
        if(m_ai_debug)
            printf("[AI] skid : '%s' wrong steering, stop skid.\n",
                    m_kart->getIdent().c_str());
#endif
    }

    if(m_controls->m_skid && ( 
        skidding->getSkidState()==Skidding::SKID_ACCUMULATE_LEFT ||
        skidding->getSkidState()==Skidding::SKID_ACCUMULATE_RIGHT))
    {
        steer_fraction = 
            skidding->getSteeringWhenSkidding(steer_fraction);
        if(steer_fraction<-1.0f)
            steer_fraction = -1.0f;
        else if(steer_fraction>1.0f)
            steer_fraction = 1.0f;
    }

    float old_steer      = m_controls->m_steer;

    // The AI has its own 'time full steer' value (which is the time
    float max_steer_change = dt/m_kart->getKartProperties()->getTimeFullSteerAI();
    if(old_steer < steer_fraction)
    {
        m_controls->m_steer = (old_steer+max_steer_change > steer_fraction) 
                           ? steer_fraction : old_steer+max_steer_change;
    }
    else
    {
        m_controls->m_steer = (old_steer-max_steer_change < steer_fraction) 
                           ? steer_fraction : old_steer-max_steer_change;
    }


}   // setSteering

// ----------------------------------------------------------------------------
/** Determine the center point and radius of a circle given two points on
 *  the ccircle and the tangent at the first point. This is done as follows:
 *  1) Determine the line going through the center point start+end, which is 
 *     orthogonal to the vector from start to end. This line goes through the
 *     center of the circle.
 *  2) Determine the line going through the first point and is orthogonal
 *     to the given tangent.
 *  3) The intersection of these two lines is the center of the circle.
 *  \param start First point.
 *  \param tangent Tangent at first point.
 *  \param end Second point on circle.
 *  \return center Center point of the circle.
 *  \return radius Radius of the circle.
 */
void  SkiddingAI::determineTurnRadius(const Vec3 &start,
                                      const Vec3 &tangent,
                                      const Vec3 &end,
                                      Vec3 *center,
                                      float *radius)
{
    // 1) Line through middle of start+end
    Vec3 mid = 0.5f*(start+end);
    Vec3 direction = end-start;

    Vec3 orthogonal(direction.getZ(), 0, -direction.getX());
    Vec3  q1 = mid + orthogonal;
    irr::core::line2df line1(mid.getX(), mid.getZ(),
                             q1.getX(),  q1.getZ()  );

    Vec3 ortho_tangent(tangent.getZ(), 0, -tangent.getX());
    Vec3  q2 = start + ortho_tangent;
    irr::core::line2df line2(start.getX(), start.getZ(),
                             q2.getX(),    q2.getZ());


    irr::core::vector2df result;
    if(line1.intersectWith(line2, result, /*checkOnlySegments*/false))
    {
        *center = Vec3(result.X, start.getY(), result.Y);
        *radius = (start - *center).length();
    }
    else
    {
        // No intersection. In this case assume that the two points are
        // on a semicircle, in which case the center is at 0.5*(start+end):
        *center = 0.5f*(start+end);
        *radius = 0.5f*(end-start).length();
    }
    return;
}   // determineTurnRadius
