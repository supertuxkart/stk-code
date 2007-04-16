//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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
#include "world.hpp"

#include "default_robot.hpp"

DefaultRobot::DefaultRobot( const KartProperties *kart_properties,
                           int position, sgCoord init_pos ) :
    AutoKart( kart_properties, position, init_pos )
{
    m_time_since_last_shot = 0.0f;
    m_start_kart_crash_direction = 0;
    reset();

    switch( world->m_race_setup.m_difficulty )
    {
    case RD_EASY:
        m_wait_for_players = true;
        m_max_handicap_accel = 0.9f;
        m_fallback_tactic = FT_AVOID_TRACK_CRASH;
        m_use_wheelies = false;
        m_wheelie_check_dist = 0.0f;
        m_item_tactic = IT_TEN_SECONDS;
        m_max_start_delay = 0.5f;
        m_min_steps = 0;
        break;
    case RD_MEDIUM:
        m_wait_for_players = true;
        m_max_handicap_accel = 0.95f;
        m_fallback_tactic = FT_PARALLEL;
        m_use_wheelies = true;
        m_wheelie_check_dist = 0.8f;
        m_item_tactic = IT_CALCULATE;
        m_max_start_delay = 0.4f;
        m_min_steps = 1;
        break;
    case RD_HARD:
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
    if( world->getPhase() == World::START_PHASE )
    {
        handle_race_start();
        AutoKart::update( delta );
        return;
    }

    /*Get information that is needed by more than 1 of the handling funcs*/
    //Detect if we are going to crash with the track and/or kart
    const int STEPS = calc_steps();
    check_crashes( STEPS, m_curr_pos.xyz );

    /*Response handling functions*/
    handle_acceleration( delta );
    handle_steering();
    handle_items( delta, STEPS );
    handle_rescue( delta );
    handle_wheelie( STEPS );
    handle_braking( STEPS );
    //TODO:handle jumping

    /*And obviously general kart stuff*/
    AutoKart::update( delta );
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
void DefaultRobot::handle_braking( const int STEPS )
{
    //We brake if we are about to get out of the road, but only if the
    //kart is on top of the road and if it doesn't slows the kart too
    //much
    if ( m_crashes.m_road && m_on_road && STEPS > 5 )
    {
        m_controls.brake = true;
#ifdef AI_DEBUG
        std::cout << "BRAKING" << std::endl;
#endif
    }
    else
        m_controls.brake = false;
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_steering()
{
    const unsigned int DRIVELINE_SIZE = world->m_track->m_driveline.size();
    const size_t NEXT_SECTOR = (uint)m_track_sector + 1 < DRIVELINE_SIZE ?
        m_track_sector + 1 : 0;
    float steer_angle = 0.0f;

    /*The AI responds based on the information we just gathered, using a
     *finite state machine.
     */
    //Reaction to being outside of the road
    if( fabsf(m_curr_track_coords[0]) + 0.5 >
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
            if(m_curr_track_coords[0] > world->getKart(m_crashes.m_kart)->
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

    m_controls.lr = angle_to_control( steer_angle );
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_items( const float DELTA, const int STEPS )
{
    if( m_rescue )
    {
        m_controls.fire = false;
        return;
    }

    //FIXME: find real kart length
    const float KART_LENGTH = 1.5f;

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
            return;
        case IT_CALCULATE:
            switch( m_collectable.getType() )
            {
            case COLLECT_ZIPPER:
                {
                    const float ANGLE_DIFF = fabsf( world->m_track->
                        m_angle[m_track_sector]- m_curr_pos.hpr[0] );
                    if( m_time_since_last_shot > 10.0f && ANGLE_DIFF <
                        15.0f && !m_crashes.m_road && STEPS > 8 )
                    {
                        m_controls.fire = true;
                        m_time_since_last_shot = 0.0f;
                    }
                }
                return;

            case COLLECT_MISSILE:
            case COLLECT_HOMING_MISSILE:
                if( m_time_since_last_shot > 5.0f && m_crashes.m_kart != -1 )
                {
                    if( sgDistanceVec2( m_curr_pos.xyz,
                        world->getKart(m_crashes.m_kart)->getCoord()->xyz ) >
                        KART_LENGTH * 2.5f )
                    {
                        m_controls.fire = true;
                        m_time_since_last_shot = 0.0f;
                    }
                }
                return;

            case COLLECT_SPARK:
                if ( m_time_since_last_shot > 3.0f && m_crashes.m_kart != -1 )
                {
                    m_controls.fire = true;
                    m_time_since_last_shot = 0.0f;
                }
                return;
                /*TODO: teach AI to use the magnet*/
            default:
                m_controls.fire = true;
                m_time_since_last_shot = 0.0f;
                return;
            }
            break;
        }
    }


    m_controls.fire = false;
}

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
        for( int i = 0; i < world->m_race_setup.getNumPlayers(); ++i )
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

    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord, step_track_coord;
    float distance;

    //We have to be careful with normalizing, because if the source argument
    //has both the X and Y axis set to 0, it returns nan to the destiny.
    if( m_abs_velocity[0] == 0.0 && m_abs_velocity[1] == 0.0 ) return false;
    sgNormalizeVec2( vel_normal, m_abs_velocity );

    //FIXME: instead of using 1.5, it should find out how much time it
    //will pass to stop doing the wheelie completely from the current state.
    const float CHECK_DIST = 1.5f * m_wheelie_check_dist;

    /* The following method of finding if a position is outside of the track
       is less accurate than calling findRoadSector(), but a lot faster.
     */
    const int WHEELIE_STEPS = int(( m_velocity.xyz[1] * CHECK_DIST )/
        KART_LENGTH );

    for( int i = WHEELIE_STEPS; i > STEPS - 1; --i )
    {
        sgAddScaledVec2( step_coord, m_curr_pos.xyz, vel_normal,
            KART_LENGTH * i );

        world->m_track->spatialToTrack( step_track_coord, step_coord,
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
    m_crash_time += (m_collided && m_on_ground) ? 3.0f * DELTA : -0.25f * DELTA;

    if( m_crash_time < 0.0f ) m_crash_time = 0.0f;

    //Reaction to being stuck
    if( m_crash_time > 3.0f )
    {
        m_rescue = true;
        m_crash_time = 0.0f;
    }
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_angle (const size_t SECTOR, const float ANGLE)
{
    float angle = world->m_track->m_angle[SECTOR];

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - m_curr_pos.hpr[0];

    steer_angle += ANGLE;
    steer_angle = normalize_angle( steer_angle );


    return steer_angle;
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_point( const sgVec2 POINT )
{
    const SGfloat ADJACENT_LINE = POINT[0] - m_curr_pos.xyz[0];
    const SGfloat OPPOSITE_LINE = POINT[1] - m_curr_pos.xyz[1];
    SGfloat theta;

    //Protection from division by zero
    if( ADJACENT_LINE > 0.0000001 || ADJACENT_LINE < -0.0000001 )
    {
        theta = sgATan( OPPOSITE_LINE / ADJACENT_LINE );
    }
    else theta = 0;

    //The real value depends on the side of the track that the kart is
    theta += ADJACENT_LINE < 0.0f ? 90.0f : -90.0f;

    float steer_angle = theta - getCoord()->hpr[0];
    steer_angle = normalize_angle( steer_angle );

    return steer_angle;
}

//-----------------------------------------------------------------------------
void DefaultRobot::check_crashes( const int STEPS, sgVec3 const pos )
{
    //Right now there are 2 kind of 'crashes': with other karts and another
    //with the track. The sight line is used to find if the karts crash with
    //each other, but the first step is twice as big as other steps to avoid
    //having karts too close in any direction. The crash with the track can
    //tell when a kart is going to get out of the track so it steers.

    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord;
    SGfloat kart_distance;

    m_crashes.clear();

    const size_t NUM_KARTS = world->getNumKarts();

    //Protection against having vel_normal with nan values
    if( m_abs_velocity[0] == 0.0 && m_abs_velocity[1] == 0.0 )
    {
        vel_normal[0] = vel_normal[1] = 0.0;
    }
    else sgNormalizeVec2( vel_normal, m_abs_velocity );

    for(int i = 1; STEPS > i; ++i)
    {
        sgAddScaledVec2( step_coord, pos, vel_normal, KART_LENGTH * i );

        /* Find if we crash with any kart, as long as we haven't found one
         * yet
         */
        if( m_crashes.m_kart == -1 )
        {
            for( unsigned int j = 0; j < NUM_KARTS; ++j )
            {
                if(world->getKart(j) == this) continue;

                kart_distance = sgDistanceVec2( step_coord,
                    world->getKart(j)->getCoord()->xyz );

                if( kart_distance < KART_LENGTH + 0.125f * i )
                    if( m_velocity.xyz[1] > world->getKart(j)->
                       getVelocity()->xyz[1] * 0.75f ) m_crashes.m_kart = j;
            }
        }

        /*Find if we crash with the drivelines*/
        int sector = world->m_track->findRoadSector( step_coord );

#ifdef SHOW_FUTURE_PATH

        ssgaSphere *sphere = new ssgaSphere;

#ifdef ERASE_PATH
        static ssgaSphere *last_sphere = 0;

        if( last_sphere ) world->m_scene->removeKid( last_sphere );

        last_sphere = sphere;
#endif

        sgVec3 center;
        center[0] = step_coord[0];
        center[1] = step_coord[1];
        center[2] = pos[2];
        sphere->setCenter( center );
        sphere->setSize( KART_LENGTH );
        if( sector == Track::UNKNOWN_SECTOR )
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
        world->m_scene->addKid( sphere );
#endif

        m_future_location[0] = step_coord[0]; m_future_location[1] =
            step_coord[1];

        if( sector == Track::UNKNOWN_SECTOR )
        {
            m_future_sector = world->m_track->findOutOfRoadSector( step_coord,
                Track::RS_DONT_KNOW, m_future_sector );
            m_crashes.m_road = true;
            break;
        }
        else
        {
            m_future_sector = sector;
        }


    }
}

/** Find the sector that at the longest distance from the kart, that can be
 *  driven to without crashing with the track, then find towards which of
 *  the two edges of the track is closest to the next curve after wards,
 *  and return the position of that edge.
 */
void DefaultRobot::find_non_crashing_point( sgVec2 result )
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;

    const unsigned int DRIVELINE_SIZE = world->m_track->m_driveline.size();

    unsigned int sector = (uint)m_track_sector + 1 < DRIVELINE_SIZE ?
        m_track_sector + 1 : 0;
    int target_sector;

    sgVec2 direction, step_track_coord;
    SGfloat distance;
    int steps;

    //We exit from the function when we have found a solution
    while( 1 )
    {
        //target_sector is the sector at the longest distance that we can drive
        //to without crashing with the track.
        target_sector = sector + 1 < DRIVELINE_SIZE ? sector + 1 : 0;

        //direction is a vector from our kart to the sectors we are testing
        sgSubVec2( direction, world->m_track->m_driveline[target_sector],
            m_curr_pos.xyz );

        steps = int( sgLengthVec2( direction ) / KART_LENGTH );
        if( steps < 1 ) steps = 1;

        sgVec2 step_coord;

        //Protection against having vel_normal with nan values
        if( direction[0] != 0.0 && direction[1] != 0.0 )
        {
            sgNormalizeVec2( direction );
        }

        //Test if we crash if we drive towards the target sector
        for( int i = 2; i < steps; ++i )
        {
            sgAddScaledVec2( step_coord, m_curr_pos.xyz, direction,
                KART_LENGTH * i );

            world->m_track->spatialToTrack( step_track_coord, step_coord,
                sector );

            distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                       : -step_track_coord[0];

            //If we are outside, the previous sector is what we are looking for
            if ( distance + KART_LENGTH * 0.5f > world->
                m_track->getWidth()[sector] )
            {
                sgCopyVec2( result, world->m_track->m_driveline[sector] );

#ifdef SHOW_NON_CRASHING_POINT
                ssgaSphere *sphere = new ssgaSphere;

                static ssgaSphere *last_sphere = 0;

                if(last_sphere) world->m_scene->removeKid( last_sphere );

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

                world->m_scene->addKid( sphere );
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
    m_inner_curve = 0;
    m_future_location[0] = 0.0;
    m_future_location[1] = 0.0;

    m_future_sector = 0;
    m_time_till_start = -1.0f;

    m_crash_time = 0.0f;

    AutoKart::reset();
}

//-----------------------------------------------------------------------------
inline float DefaultRobot::normalize_angle( float angle )
{
    while( angle > 360.0 ) angle -= 360;
    while( angle < -360.0 ) angle += 360;

    if( angle > 180.0 ) angle -= 360.0;
    else if( angle < -180.0 ) angle += 360.0;

    return angle;
}

/** calc_steps() divides the velocity vector by the lenght of the kart,
 *  and gets the number of steps to use for the sight line of the kart..
 */
int DefaultRobot::calc_steps()
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;
    int steps = int( m_velocity.xyz[1] / KART_LENGTH );
    if( steps < m_min_steps ) steps = m_min_steps;

    //Increase the steps depending on the width, if we steering hard,
    //mostly for curves.
    if( fabsf(m_controls.lr) > 0.95 )
    {
        const int WIDTH_STEPS = (int)( world->m_track->getWidth(
            )[m_future_sector] / ( KART_LENGTH * 2.0 ));

        steps += WIDTH_STEPS;
    }

    return steps;
}

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

