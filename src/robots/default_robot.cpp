//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
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

#ifdef AI_DEBUG
#define SHOW_FUTURE_PATH //If defined, it will put a bunch of spheres when it
//checks for crashes with the outside of the track.
#define ERASE_PATH   //If not defined, the spheres drawn in the future path
//won't be erased the next time the function is called.
#define SHOW_NON_CRASHING_POINT //If defined, draws a green sphere where the
//n farthest non-crashing point is.
#include <plib/ssgAux.h>
#endif

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>
#include "constants.hpp"
#include "scene.hpp"
#include "world.hpp"
#include "race_manager.hpp"

#include "default_robot.hpp"

DefaultRobot::DefaultRobot(const std::string& kart_name,
                           int position, const btTransform& init_pos ) :
    AutoKart( kart_name, position, init_pos )
{
    reset();

    switch( race_manager->getDifficulty())
    {
    case RaceManager::RD_EASY:
        m_wait_for_players = true;
        m_max_handicap_accel = 0.9f;
        m_fallback_tactic = FT_AVOID_TRACK_CRASH;
        m_use_wheelies = false;
        m_wheelie_check_dist = 0.0f;
        m_item_tactic = IT_TEN_SECONDS;
        m_max_start_delay = 0.5f;
        m_min_steps = 0;
        break;
    case RaceManager::RD_MEDIUM:
        m_wait_for_players = true;
        m_max_handicap_accel = 0.95f;
        m_fallback_tactic = FT_PARALLEL;
        m_use_wheelies = true;
        m_wheelie_check_dist = 0.8f;
        m_item_tactic = IT_CALCULATE;
        m_max_start_delay = 0.4f;
        m_min_steps = 1;
        break;
    case RaceManager::RD_HARD:
    case RaceManager::RD_SKIDDING:
        m_wait_for_players = false;
        m_max_handicap_accel = 1.0f;
        m_fallback_tactic = FT_FAREST_POINT;
        m_use_wheelies = true;
        m_wheelie_check_dist = 1.0f;
        m_item_tactic = IT_CALCULATE;
        m_max_start_delay = 0.1f;
        m_min_steps = 2;
        break;
    }
}

//-----------------------------------------------------------------------------
//TODO: if the AI is crashing constantly, make it move backwards in a straight
//line, then move forward while turning.
void DefaultRobot::update( float delta )
{
    if( world->isStartPhase())
    {
        handle_race_start();
        AutoKart::update( delta );
        return;
    }

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    int steps = 0;

    // This should not happen (anymore :) ), but it keeps the game running
    // in case that m_future_sector becomes undefined.
    if(m_future_sector == Track::UNKNOWN_SECTOR )
    {
#ifdef DEBUG
        fprintf(stderr,"DefaultRobot: m_future_sector is undefined.\n");
        fprintf(stderr,"This shouldn't happen, but can be ignored.\n");
#endif
        forceRescue();
        m_future_sector = 0;
    }
    else
    {
        steps = calc_steps();
    }

    check_crashes( steps, getXYZ() );
    find_curve();

    /*Response handling functions*/
    handle_acceleration( delta );
    handle_steering();
    handle_items( delta, steps );
    handle_rescue( delta );
    handle_wheelie( steps );
    handle_braking();
    //TODO:handle jumping

    /*And obviously general kart stuff*/
    AutoKart::update( delta );
    m_collided = false;
}   // update

//-----------------------------------------------------------------------------
void DefaultRobot::handle_wheelie( const int STEPS )
{
    if( m_use_wheelies )
    {
        m_controls.wheelie = do_wheelie( STEPS );
    }
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_braking()
{
    // In follow the leader mode, the kart should brake if they are ahead of
    // the leader (and not the leader, i.e. don't have initial position 1)
    if(race_manager->getMinorMode()==RaceManager::RM_FOLLOW_LEADER &&
        getPosition()<world->getKart(0)->getPosition()             &&
        getInitialPosition()>1                                       )
    {
        m_controls.brake = true;
        return;
    }
    const float MIN_SPEED = world->m_track->getWidth()[m_track_sector];

    //We may brake if we are about to get out of the road, but only if the
    //kart is on top of the road, and if we won't slow down below a certain
    //limit.
    if ( m_crashes.m_road && m_on_road && getVelocityLC().getY() > MIN_SPEED)
    {
        float kart_ang_diff = world->m_track->m_angle[m_track_sector] -
                              RAD_TO_DEGREE(getHPR().getHeading());
        kart_ang_diff = normalize_angle(kart_ang_diff);
        kart_ang_diff = fabsf(kart_ang_diff);

        const float MIN_TRACK_ANGLE = 20.0f;
        const float CURVE_INSIDE_PERC = 0.25f;

        //Brake only if the road does not goes somewhat straight.
        if(m_curve_angle > MIN_TRACK_ANGLE) //Next curve is left
        {
            //Avoid braking if the kart is in the inside of the curve, but
            //if the curve angle is bigger than what the kart can steer, brake
            //even if we are in the inside, because the kart would be 'thrown'
            //out of the curve.
            if(!(getDistanceToCenter() > world->m_track->
                getWidth()[m_track_sector] * -CURVE_INSIDE_PERC ||
                m_curve_angle > getMaxSteerAngle()))
            {
                m_controls.brake = false;
                return;
            }
        }
        else if( m_curve_angle < -MIN_TRACK_ANGLE ) //Next curve is right
        {
            if(!(getDistanceToCenter() < world->m_track->
                getWidth()[m_track_sector] * CURVE_INSIDE_PERC ||
                m_curve_angle < -getMaxSteerAngle()))
            {
                m_controls.brake = false;
                return;
            }
        }

        //Brake if the kart's speed is bigger than the speed we need
        //to go through the curve at the widest angle, or if the kart
        //is not going straight in relation to the road.
        float angle_adjust = world->m_track->getAIAngleAdjustment();
        float speed_adjust = world->m_track->getAICurveSpeedAdjustment();
        if(getVelocityLC().getY() > speed_adjust*m_curve_target_speed ||
           kart_ang_diff          > angle_adjust*MIN_TRACK_ANGLE         )
        {
#ifdef AI_DEBUG
        std::cout << "BRAKING" << std::endl;
#endif
            m_controls.brake = true;
            return;
        }

    }

    m_controls.brake = false;
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_steering()
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)world->m_track->m_driveline.size();
    const size_t NEXT_SECTOR = (unsigned int)m_track_sector + 1 < DRIVELINE_SIZE ?
        m_track_sector + 1 : 0;
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if( fabsf(getDistanceToCenter()) + 0.5 >
        world->m_track->getWidth()[m_track_sector] )
    {
        steer_angle = steer_to_point( world->m_track->
            m_driveline[NEXT_SECTOR] );

#ifdef AI_DEBUG
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
            steer_angle = steer_to_angle( NEXT_SECTOR, -90.0f );
            m_start_kart_crash_direction = 0;
        }
        else if(m_start_kart_crash_direction == -1)
        {
            steer_angle = steer_to_angle( NEXT_SECTOR, 90.0f );
            m_start_kart_crash_direction = 0;
        }
        else
        {
            if(getDistanceToCenter() > world->getKart(m_crashes.m_kart)->
               getDistanceToCenter())
            {
                steer_angle = steer_to_angle( NEXT_SECTOR, -90.0f );
                m_start_kart_crash_direction = 1;
            }
            else
            {
                steer_angle = steer_to_angle( NEXT_SECTOR, 90.0f );
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
        switch( m_fallback_tactic )
        {
        case FT_FAREST_POINT:
            {
                sgVec2 straight_point;
                find_non_crashing_point( straight_point );
                steer_angle = steer_to_point( straight_point );
            }
            break;

        case FT_PARALLEL:
            steer_angle = steer_to_angle( NEXT_SECTOR, 0.0f );
            break;

        case FT_AVOID_TRACK_CRASH:
            if( m_crashes.m_road )
            {
                steer_angle = steer_to_angle( m_track_sector, 0.0f );
            }
            else steer_angle = 0.0f;

            break;
        }

#ifdef AI_DEBUG
        std::cout << "- Fallback."  << std::endl;
#endif

    }
    // avoid steer vibrations
    if (fabsf(steer_angle) < 2.0f)
        steer_angle = 0.f;

    m_controls.lr = angle_to_control( steer_angle );
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_items( const float DELTA, const int STEPS )
{
    m_controls.fire = false;

    if(isRescue() )
    {
        return;
    }

    m_time_since_last_shot += DELTA;
    if( m_collectable.getType() != COLLECT_NOTHING )
    {
        switch( m_item_tactic )
        {
        case IT_TEN_SECONDS:
            if( m_time_since_last_shot > 10.0f )
            {
                m_controls.fire = true;
                m_time_since_last_shot = 0.0f;
            }
            break;
        case IT_CALCULATE:
            switch( m_collectable.getType() )
            {
            case COLLECT_ZIPPER:
                {
                    const float ANGLE_DIFF = fabsf( normalize_angle(
                        world->m_track->m_angle[m_track_sector]-
                        RAD_TO_DEGREE(getHPR().getHeading()) ) );

                    if( m_time_since_last_shot > 10.0f && ANGLE_DIFF <
                        15.0f && !m_crashes.m_road && STEPS > 8 )
                    {
                        m_controls.fire = true;
                        m_time_since_last_shot = 0.0f;
                    }
                }
                break;

            case COLLECT_MISSILE:
            case COLLECT_HOMING:
                if( m_time_since_last_shot > 5.0f && m_crashes.m_kart != -1 )
                {
		  if( (getXYZ()-world->getKart(m_crashes.m_kart)->getXYZ() ).length_2d() >
		      m_kart_properties->getKartLength() * 2.5f )
                    {
                        m_controls.fire = true;
                        m_time_since_last_shot = 0.0f;
                    }
                }
                break;

            case COLLECT_BOWLING:
                if ( m_time_since_last_shot > 3.0f && m_crashes.m_kart != -1 )
                {
                    m_controls.fire = true;
                    m_time_since_last_shot = 0.0f;
                }
                break;
            default:
                m_controls.fire = true;
                m_time_since_last_shot = 0.0f;
                return;
            }
            break;
        }
    }
    return;
}   // handle_items

//-----------------------------------------------------------------------------
void DefaultRobot::handle_acceleration( const float DELTA )
{
    //Do not accelerate until we have delayed the start enough
    if( m_time_till_start > 0.0f )
    {
        m_time_till_start -= DELTA;
        m_controls.accel = 0.0f;
        return;
    }

    if( m_controls.brake == true )
    {
        m_controls.accel = 0.0f;
        return;
    }

    if( m_wait_for_players )
    {
        //Find if any player is ahead of this kart
        bool player_winning = false;
        for(unsigned int i = 0; i < race_manager->getNumPlayers(); ++i )
            if( m_race_position > world->getPlayerKart(i)->getPosition() )
            {
                player_winning = true;
                break;
            }

        if( player_winning )
        {
            m_controls.accel = m_max_handicap_accel;
            return;
        }
    }

    m_controls.accel = 1.0f;
}

//-----------------------------------------------------------------------------
bool DefaultRobot::do_wheelie ( const int STEPS )
{
    if( m_crashes.m_road ) return false;
    if( m_crashes.m_kart != -1 ) return false;

    //We have to be careful with normalizing, because if the source argument
    //has both the X and Y axis set to 0, it returns nan to the destiny.
    const Vec3 &VEL      = getVelocity();
    Vec3        vel_normal(VEL.getX(), VEL.getY(), 0.0);
    float       len      = vel_normal.length();
    // Too slow for wheelies, and it avoids normalisation problems.
    if(len<getMaxSpeed()*getWheelieMaxSpeedRatio()) return false;
    vel_normal/=len;

    Vec3 step_coord;
    Vec3 step_track_coord;
    float distance;

    //FIXME: instead of using 1.5, it should find out how much time it
    //will pass to stop doing the wheelie completely from the current state.
    const float CHECK_DIST = 1.5f * m_wheelie_check_dist;

    /* The following method of finding if a position is outside of the track
       is less accurate than calling findRoadSector(), but a lot faster.
     */
    const int WHEELIE_STEPS = int(( getVelocityLC().getY() * CHECK_DIST )/
        m_kart_properties->getKartLength() );

    for( int i = WHEELIE_STEPS; i > STEPS - 1; --i )
    {
        step_coord = getXYZ()+vel_normal* m_kart_properties->getKartLength() * float(i);

        world->m_track->spatialToTrack(step_track_coord, step_coord,
                                       m_future_sector );

        distance = step_track_coord[0] > 0.0f ?  step_track_coord[0]
                   : -step_track_coord[0];

        if( distance > world->m_track->getWidth()[m_track_sector] )
        {
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_race_start()
{
    //FIXME: make karts able to get a penalty for accelerating too soon
    //like players, should happen to about 20% of the karts in easy,
    //5% in medium and less than 1% of the karts in hard.
    if( m_time_till_start <  0.0f )
    {
        srand(( unsigned ) time( 0 ));

        //Each kart starts at a different, random time, and the time is
        //smaller depending on the difficulty.
        m_time_till_start = ( float ) rand() / RAND_MAX * m_max_start_delay;
    }
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_rescue(const float DELTA)
{
    //TODO: check if we collided against a dynamic object (ej.:kart) or
    //against the track's static object.
    //The m_crash_time measures if a kart has been crashing for too long

    m_crash_time += (m_collided && isOnGround()) ? 3.0f * DELTA : -0.25f * DELTA;
    if( m_crash_time < 0.0f ) m_crash_time = 0.0f;

    //Reaction to being stuck
    if( m_crash_time > 3.0f )
    {
        forceRescue();
        m_crash_time = 0.0f;
    }


    // check if kart is stuck
    if(getVehicle()->getRigidBody()->getLinearVelocity().length()<2.0f &&
       !isRescue() && !world->isStartPhase())
    {
        m_time_since_stuck += DELTA;
        if(m_time_since_stuck > 2.0f)
        {
            forceRescue();
            m_time_since_stuck=0.0f;
        }   // m_time_since_stuck > 2.0f
    }
    else
    {
        m_time_since_stuck = 0.0f;
    }
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_angle (const size_t SECTOR, const float ANGLE)
{
    float angle = world->m_track->m_angle[SECTOR];

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - RAD_TO_DEGREE(getHPR().getHeading());

    steer_angle += ANGLE;
    steer_angle = normalize_angle( steer_angle );


    return steer_angle;
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_point( const sgVec2 POINT )
{
    const SGfloat ADJACENT_LINE = POINT[0] - getXYZ().getX();
    const SGfloat OPPOSITE_LINE = POINT[1] - getXYZ().getY();
    SGfloat theta;

    //Protection from division by zero
    if( ADJACENT_LINE > 0.0000001 || ADJACENT_LINE < -0.0000001 )
    {
        theta = sgATan( OPPOSITE_LINE / ADJACENT_LINE );
    }
    else theta = 0;

    //The real value depends on the side of the track that the kart is
    theta += ADJACENT_LINE < 0.0f ? 90.0f : -90.0f;

    float steer_angle = theta - RAD_TO_DEGREE(getHPR().getHeading());
    steer_angle = normalize_angle( steer_angle );

    return steer_angle;
}

//-----------------------------------------------------------------------------
void DefaultRobot::check_crashes( const int STEPS, const Vec3& pos )
{
    //Right now there are 2 kind of 'crashes': with other karts and another
    //with the track. The sight line is used to find if the karts crash with
    //each other, but the first step is twice as big as other steps to avoid
    //having karts too close in any direction. The crash with the track can
    //tell when a kart is going to get out of the track so it steers.

    Vec3 vel_normal;
	//in this func we use it as a 2d-vector, but later on it is passed
	//to world->m_track->findRoadSector, there it is used as a 3d-vector
	//to find distance to plane, so z must be initialized to zero
    Vec3 step_coord;
    SGfloat kart_distance;

    step_coord.setZ(0.0);

    m_crashes.clear();

    const size_t NUM_KARTS = race_manager->getNumKarts();

    //Protection against having vel_normal with nan values
    const Vec3 &VEL = getVelocity();
    vel_normal.setValue(VEL.getX(), VEL.getY(), 0.0);
    float len=vel_normal.length();
    if(len>0.0f)
    {
        vel_normal/=len;
    }
    else
    {
        vel_normal.setValue(0.0, 0.0, 0.0);
    }

    for(int i = 1; STEPS > i; ++i)
    {
	step_coord = pos + vel_normal* m_kart_properties->getKartLength() * float(i);

        /* Find if we crash with any kart, as long as we haven't found one
         * yet
         */
        if( m_crashes.m_kart == -1 )
        {
            for( unsigned int j = 0; j < NUM_KARTS; ++j )
            {
                const Kart* kart=world->getKart(j);
                if(kart==this||kart->isEliminated()) continue;   // ignore eliminated karts

		kart_distance = (step_coord-world->getKart(j)->getXYZ()).length_2d();

                if( kart_distance < m_kart_properties->getKartLength() + 0.125f * i )
                    if( getVelocityLC().getY() > world->getKart(j)->
                       getVelocityLC().getY() * 0.75f ) m_crashes.m_kart = j;
            }
        }

        /*Find if we crash with the drivelines*/
        world->m_track->findRoadSector(step_coord, &m_sector);

#ifdef SHOW_FUTURE_PATH

        ssgaSphere *sphere = new ssgaSphere;

#ifdef ERASE_PATH
        static ssgaSphere *last_sphere = 0;

        if( last_sphere ) scene->remove( last_sphere );

        last_sphere = sphere;
#endif

        sgVec3 center;
        center[0] = step_coord[0];
        center[1] = step_coord[1];
        center[2] = pos[2];
        sphere->setCenter( center );
        sphere->setSize( m_kart_properties->getKartLength() );
        if( m_sector == Track::UNKNOWN_SECTOR )
        {
            sgVec4 colour;
            colour[0] = colour[3] = 255;
            colour[1] = colour[2] = 0;
            sphere->setColour(colour);
        }
        else if( i == 1 )
        {
            sgVec4 colour;
            colour[0] = colour[1] = colour[2] = 0;
            colour[3] = 255;
            sphere->setColour( colour );
        }
        scene->add( sphere );
#endif

        m_future_location[0] = step_coord[0]; m_future_location[1] =
            step_coord[1];

        if( m_sector == Track::UNKNOWN_SECTOR )
        {
            m_future_sector = world->getTrack()->findOutOfRoadSector( step_coord,
                Track::RS_DONT_KNOW, m_future_sector );
            m_crashes.m_road = true;
            break;
        }
        else
        {
            m_future_sector = m_sector;
        }


    }
}

//-----------------------------------------------------------------------------
/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve after wards,
 *  and return the position of that edge.
 */
void DefaultRobot::find_non_crashing_point( sgVec2 result )
{
    const unsigned int DRIVELINE_SIZE = (unsigned int)world->m_track->m_driveline.size();

    unsigned int sector = (unsigned int)m_track_sector + 1 < DRIVELINE_SIZE ?
        m_track_sector + 1 : 0;
    int target_sector;

    Vec3 direction;
    Vec3 step_track_coord;
    SGfloat distance;
    int steps;

    //We exit from the function when we have found a solution
    while( 1 )
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        target_sector = sector + 1 < DRIVELINE_SIZE ? sector + 1 : 0;

        //direction is a vector from our kart to the sectors we are testing
	direction = world->m_track->m_driveline[target_sector] - getXYZ();

        float len=direction.length_2d();
        steps = int( len / m_kart_properties->getKartLength() );
        if( steps < 3 ) steps = 3;

        //Protection against having vel_normal with nan values
        if(len>0.0f) {
            direction*= 1.0f/len;
        }

        Vec3 step_coord;
        //Test if we crash if we drive towards the target sector
        for( int i = 2; i < steps; ++i )
        {
            step_coord = getXYZ()+direction*m_kart_properties->getKartLength() * float(i);

            world->m_track->spatialToTrack( step_track_coord, step_coord,
                sector );

            distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                       : -step_track_coord[0];

            //If we are outside, the previous sector is what we are looking for
            if ( distance + m_kart_properties->getKartLength() * 0.5f > world->
                m_track->getWidth()[sector] )
            {
                sgCopyVec2( result, world->m_track->m_driveline[sector] );

#ifdef SHOW_NON_CRASHING_POINT
                ssgaSphere *sphere = new ssgaSphere;

                static ssgaSphere *last_sphere = 0;

                if(last_sphere) scene->remove( last_sphere );

                last_sphere = sphere;

                sgVec3 center;
                center[0] = result[0];
                center[1] = result[1];
                center[2] = m_curr_pos.xyz[2];
                sphere->setCenter( center );
                sphere->setSize( 0.5f );

                sgVec4 colour;
                colour[1] = colour[3] = 255;
                colour[0] = colour[2] = 0;
                sphere->setColour( colour );

                scene->add( sphere );
#endif

                return;
            }
        }
        sector = target_sector;
    }
}

//-----------------------------------------------------------------------------
void DefaultRobot::reset()
{
    m_time_since_last_shot = 0.0f;
    m_start_kart_crash_direction = 0;

    m_sector      = Track::UNKNOWN_SECTOR;
    m_inner_curve = 0;
    m_curve_target_speed = getMaxSpeed();
    m_curve_angle = 0.0;

    m_future_location[0] = 0.0;
    m_future_location[1] = 0.0;

    m_future_sector = 0;
    m_time_till_start = -1.0f;
    m_crash_time = 0.0f;
    m_collided = false;


    m_time_since_stuck     = 0.0f;

    AutoKart::reset();
}   // reset

//-----------------------------------------------------------------------------
inline float DefaultRobot::normalize_angle( float angle )
{
    while( angle > 360.0 ) angle -= 360;
    while( angle < -360.0 ) angle += 360;

    if( angle > 180.0 ) angle -= 360.0;
    else if( angle < -180.0 ) angle += 360.0;

    return angle;
}

//-----------------------------------------------------------------------------
/** calc_steps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart.
 *  The calling sequence guarantees that m_future_sector is not UNKNOWN.
 */
int DefaultRobot::calc_steps()
{
    int steps = int( getVelocityLC().getY() / m_kart_properties->getKartLength() );
    if( steps < m_min_steps ) steps = m_min_steps;

    //Increase the steps depending on the width, if we steering hard,
    //mostly for curves.
    if( fabsf(m_controls.lr) > 0.95 )
    {
        const int WIDTH_STEPS = 
            (int)( world->m_track->getWidth()[m_future_sector] 
                   /( m_kart_properties->getKartLength() * 2.0 ) );

        steps += WIDTH_STEPS;
    }

    return steps;
}

//-----------------------------------------------------------------------------
/** Translates coordinates from an angle(in degrees) to values within the range
 *  of -1.0 to 1.0 to use the same format as the KartControl::lr variable.
 */
float DefaultRobot::angle_to_control( float angle ) const
{
    angle *= 180.0f / ( getMaxSteerAngle() * M_PI ) ;

    if(angle > 1.0f) return 1.0f;
    else if(angle < -1.0f) return -1.0f;

    return angle;
}

//-----------------------------------------------------------------------------
/** Finds the approximate radius of a track's curve. It needs two arguments,
 *  the number of the drivepoint that marks the beginning of the curve, and
 *  the number of the drivepoint that marks the ending of the curve.
 *
 *  Based on that you can construct any circle out of 3 points in it, we use
 *  the two arguments to use the drivelines as the first and last point; the
 *  middle sector is averaged.
 */
float DefaultRobot::get_approx_radius(const int START, const int END) const
{
    const int MIDDLE = (START + END) / 2;

    //If the START and END sectors are very close, their average will be one
    //of them, and using twice the same point just generates a huge radius
    //(too big to be of any use) but it also can generate a division by zero,
    //so here is some special handling for that case.
    if (MIDDLE == START || MIDDLE == END ) return 99999.0f;

    float X1, Y1, X2, Y2, X3, Y3;

    //The next line is just to avoid compiler warnings.
    X1 = X2 = X3 = Y1 = Y2 = Y3 = 0.0;


    if(m_inner_curve == -1)
    {

    X1 = world->m_track->m_left_driveline[START][0];
    Y1 = world->m_track->m_left_driveline[START][1];

    X2 = world->m_track->m_left_driveline[MIDDLE][0];
    Y2 = world->m_track->m_left_driveline[MIDDLE][1];

    X3 = world->m_track->m_left_driveline[END][0];
    Y3 = world->m_track->m_left_driveline[END][1];
    }else if (m_inner_curve == 0)
    {
    X1 = world->m_track->m_driveline[START][0];
    Y1 = world->m_track->m_driveline[START][1];

    X2 = world->m_track->m_driveline[MIDDLE][0];
    Y2 = world->m_track->m_driveline[MIDDLE][1];

    X3 = world->m_track->m_driveline[END][0];
    Y3 = world->m_track->m_driveline[END][1];
    }else if (m_inner_curve == 1)
    {
    X1 = world->m_track->m_right_driveline[START][0];
    Y1 = world->m_track->m_right_driveline[START][1];

    X2 = world->m_track->m_right_driveline[MIDDLE][0];
    Y2 = world->m_track->m_right_driveline[MIDDLE][1];

    X3 = world->m_track->m_right_driveline[END][0];
    Y3 = world->m_track->m_right_driveline[END][1];
    }

    const float A = X2 - X1;
    const float B = Y2 - Y1;
    const float C = X3 - X1;
    const float D = Y3 - Y1;

    const float E = A * ( X1 + X2) + B * (Y1 + Y2);
    const float F = C * ( X1 + X3) + D * (Y1 + Y3);

    const float G = 2.0f * ( A*( Y3-Y2 ) - B*( X3 - X2 ) );

    const float pX = ( D*E - B*F) / G;
    const float pY = ( A*F - C*E) / G;

    const float radius = sqrt( ( X1 - pX) * ( X1 - pX) + (Y1 - pY) * (Y1 - pY) );

    return radius;
}

//-----------------------------------------------------------------------------
/**Find_curve() gathers info about the closest sectors ahead: the curve
 * angle, the direction of the next turn, and the optimal speed at which the
 * curve can be travelled at it's widest angle.
 *
 * The number of sectors that form the curve is dependant on the kart's speed.
 */
void DefaultRobot::find_curve()
{
    const int DRIVELINE_SIZE = (unsigned int)world->m_track->m_driveline.size();
    float total_dist = 0.0f;
    int next_hint = m_track_sector;
    int i;

    for(i = m_track_sector; total_dist < getVelocityLC().getY(); i = next_hint)
    {
        next_hint = i + 1 < DRIVELINE_SIZE ? i + 1 : 0;
        total_dist += sgDistanceVec2(world->m_track->m_driveline[i], world->m_track->m_driveline[next_hint]);
    }


    m_curve_angle = normalize_angle(world->m_track->m_angle[i] - world->m_track->m_angle[m_track_sector]);
    m_inner_curve = m_curve_angle > 0.0 ? -1 : 1;
    // FIXME: 0.9 is the tire grip - but this was never used. For now this
    // 0.9 is left in place to reproduce the same results and AI behaviour,
    // but this function should be updated to bullet physics
    m_curve_target_speed = sgSqrt(get_approx_radius(m_track_sector, i) * world->m_track->getGravity() * 0.9f);
}
