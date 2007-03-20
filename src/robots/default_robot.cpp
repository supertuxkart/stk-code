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
//#define AI_DEBUG

#ifdef AI_DEBUG
#define SHOW_FUTURE_PATH //If defined, it will put a bunch of spheres when it
//checks for crashes with the outside of the track.
#define ERASE_PATH   //If not defined, the spheres drawn in the future path
//won't be erased the next time the function is called.
#define SHOW_NON_CRASHING_HINT //If defined, draws a green sphere where the
//nfarthest non-crashing hint is.
#include <plib/ssgAux.h>
#endif

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>
#include "constants.hpp"
#include "world.hpp"

#include "default_robot.hpp"

DefaultRobot::DefaultRobot(const KartProperties *kart_properties, int position,
                   sgCoord init_pos) :
    AutoKart(kart_properties, position, init_pos)
{
    m_time_since_last_shot = 0.0f;
    m_future_hint = 0;
    m_next_curve_hint = -1;
    m_next_straight_hint = -1;
    m_on_curve = false;
    m_handle_curve = false;
    start_kart_crash_direction = 0;

    m_crash_time = 0.0;
    reset();
}

//-----------------------------------------------------------------------------

//TODO: if the AI is crashing constantly, make it move backwards in a straight line, then move forward while turning.
//TODO: rotation should be dependant on how much each kart can rotate, so we don't have crazy cars.
void DefaultRobot::update (float delta)
{
    if( world->getPhase() == World::START_PHASE )
    {
        handle_race_start();
        return;
    }
    

    /*Get information about the track*/
    //Detect if we are going to crash with the track and/or karts
    const int STEPS = calc_steps();
    check_crashes(STEPS, m_curr_pos.xyz);

    //Store the absolute distance to the center of the track
    const float DIST_TO_CENTER = m_curr_track_coords[0] > 0.0f ?
                                 m_curr_track_coords[0] : -m_curr_track_coords[0];

    //Store the number of points the driveline has
    const size_t DRIVELINE_SIZE = world->m_track->m_driveline.size();

    const size_t NEXT_HINT = m_track_hint + 1 < DRIVELINE_SIZE ? m_track_hint + 1 : 0;

    const float KART_LENGTH = 1.5f;

    //The m_crash_time measures if a kart has been crashing for too long
    m_crash_time += m_collided ? 3.0f * delta : -0.25f * delta;
    if(m_crash_time < 0.0f) m_crash_time = 0.0f;

    //Find if any player is ahead of this kart
    bool player_winning = false;
    for(int i = 0; i < world->m_race_setup.getNumPlayers(); ++i)
        if(m_race_position > world->getPlayerKart(i)->getPosition())
            player_winning = true;

    float difficulty_multiplier = 1.0f;
    if(!player_winning)
        switch(world->m_race_setup.m_difficulty)
        {
        case RD_EASY:
            difficulty_multiplier = 0.9f;
            break;

        case RD_MEDIUM:
            difficulty_multiplier = 0.95f;
            break;

        default:
            break;
        }

    /*Steer based on the information we just gathered*/
    float steer_angle = 0.0f;

    //If the kart is outside of the track steer back to it
    if(DIST_TO_CENTER + KART_LENGTH * 0.5f > world->m_track->getWidth()[m_track_hint])
    {
        steer_angle = steer_to_point(world->m_track->m_driveline[NEXT_HINT]);

#ifdef AI_DEBUG
        std::cout << "Action: Steer towards center." << std::endl;
#endif
    }

#if 0
    //Since there won't be any special handling for tight curves, this
    //won't be needed.
    //If there is a tight curve ahead, try to steer like a racing curve
    else if(handle_tight_curves())
    {
        steer_angle = steer_for_tight_curve();
#ifdef AI_DEBUG
        std::cout << "Action: Handle tight curves." << std::endl;
#endif

    }
#endif

    //If it seems like the kart will crash with a curve, try to remain
    //in the same direction of the track
    else if(crashes.m_road)// && world->m_race_setup.m_difficulty != RD_HARD)
    {
        steer_angle = steer_to_angle(m_track_hint, 0.0f);

#ifdef AI_DEBUG
        std::cout << "Action: Avoid crashing with the track." << std::endl;
#endif
    }
    //If we are going to crash against a kart, avoid it
    else if(crashes.m_kart != -1)
    {
        if(start_kart_crash_direction == 1) //-1 = left, 1 = right, 0 = no crash.
        {
            steer_angle = steer_to_angle(NEXT_HINT, -90.0f);
            start_kart_crash_direction = 0;
        }
        else if(start_kart_crash_direction == -1)
        {
            steer_angle = steer_to_angle(NEXT_HINT, 90.0f);
            start_kart_crash_direction = 0;
        }
        else
        {
            if(m_curr_track_coords[0] > world->getKart(crashes.m_kart)->
               getDistanceToCenter())
            {
                steer_angle = steer_to_angle(NEXT_HINT, -90.0f);
                start_kart_crash_direction = 1;
            }
            else
            {
                steer_angle = steer_to_angle(NEXT_HINT, 90.0f);
                start_kart_crash_direction = -1;
            }
        }

#ifdef AI_DEBUG
        std::cout << "Action: Avoid crashing with karts." << std::endl;
#endif

    }
    else
    {
        switch(world->m_race_setup.m_difficulty)
        {
            //Steer to the farest point that the kart can drive to in a
            //straight line without crashing with the track.
        case RD_HARD:
            {
                //Find a suitable point to drive to in a straight line
                sgVec2 straight_point;
                find_non_crashing_point(straight_point);

                steer_angle = steer_to_point(straight_point);
            }
            break;

            //Remain parallel to the track
        case RD_MEDIUM:
            steer_angle = steer_to_angle(NEXT_HINT, 0.0f);
            break;

            //Just drive in a straight line
        case RD_EASY:
            steer_angle = 0.0f;
            break;
        }

#ifdef AI_DEBUG
        std::cout << "Action: Fallback."  << std::endl;
#endif

    }

    /*Handle braking*/
    m_controls.brake = false;
    //At the moment this makes the AI brake too much
#if 0
    float time = (m_velocity.xyz[1]/ -guess_accel(-1.0f));

    //Braking distance is in openGL units.
    float braking_distance = m_velocity.xyz[1] * time - (-guess_accel(-1.0f) / 2) * time * time;
    if(crashes.m_road && braking_distance > crashes.m_road) brake = true;
#endif

    /*Handle acceleration*/
    if(m_starting_delay < 0.0f)
    {
        m_controls.accel = 1.0f * difficulty_multiplier;
    }
    else
    {
        m_controls.accel = 0.0f;
        m_starting_delay -= delta;
    }

    /*Handle rescue*/
    //If we have been crashing constantly, asume
    //the kart is stuck and ask for a rescue.
    if(m_crash_time > 5.0f)
    {
        m_rescue = true;
    }

    /*Handle wheelies*/
    if(world->m_race_setup.m_difficulty != RD_EASY)
        m_controls.wheelie = do_wheelie(STEPS);

    /*Handle specials*/
    m_time_since_last_shot += delta;
    if ( m_collectable.getType() != COLLECT_NOTHING )
    {
        switch(world->m_race_setup.m_difficulty)
        {
        case RD_EASY:
            if (m_time_since_last_shot > 10.0f)
            {
                m_collectable.use() ;
                m_time_since_last_shot = 0.0f;
            }
            break;
        case RD_MEDIUM:
        case RD_HARD:
            switch(m_collectable.getType())
            {
            case COLLECT_ZIPPER:
                {
                    float angle = fabsf(world->m_track->m_angle[m_track_hint]-
                                        m_curr_pos.hpr[0]);
                    if(m_time_since_last_shot > 10.0f && angle < 30.0f &&
                       !crashes.m_road && STEPS > 2)
                    {
                        m_collectable.use();
                        m_time_since_last_shot = 0.0f;
                    }
                }
                break;

            case COLLECT_MISSILE:
            case COLLECT_HOMING_MISSILE:
                if (m_time_since_last_shot > 5.0f && crashes.m_kart != -1)
                {
                    if(sgDistanceVec2(m_curr_pos.xyz,
                                      world->getKart(crashes.m_kart)->getCoord()->xyz) >
                       KART_LENGTH * 2.5f)
                    {
                        m_collectable.use() ;
                        m_time_since_last_shot = 0.0f;
                    }
                }
                break;

            case COLLECT_SPARK:
                if (m_time_since_last_shot > 3.0f && crashes.m_kart != -1)
                {
                    m_collectable.use() ;
                    m_time_since_last_shot = 0.0f;
                }
                break;
                /*TODO: teach AI to use the magnet*/
            default:
                m_collectable.use() ;
                m_time_since_last_shot = 0.0f;
                break;
            }
            break;
        }
    }   // if COLLECT_NOTHING
    
    
    m_controls.lr = angle_to_control(steer_angle) * difficulty_multiplier;


    /*And obviously general kart stuff*/
    Kart::update(delta);
}   // update

//-----------------------------------------------------------------------------
bool DefaultRobot::do_wheelie ( const int STEPS )
{
    if(crashes.m_road) return false;
    if(crashes.m_kart != -1) return false;

    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord, step_track_coord;
    float distance;

    sgNormalizeVec2(vel_normal, m_abs_velocity);

    //FIXME: instead of using 1.35 and 1.5, it should find out how much time it
    //will pass to stop doing the wheelie completely from the current state.
    const float CHECK_DIST = world->m_race_setup.m_difficulty == RD_HARD ? 1.35f :
                             1.5f;

    /* The following method of finding if a position is outside of the track
       is less accurate than calling findRoadSector(), but a lot faster.
     */
    const int WHEELIE_STEPS = int((m_velocity.xyz[1] * CHECK_DIST)/ KART_LENGTH);
    for(int i = WHEELIE_STEPS; i > STEPS - 1; --i)
    {
        sgAddScaledVec2(step_coord, m_curr_pos.xyz, vel_normal, KART_LENGTH * i);

        world->m_track->spatialToTrack( step_track_coord, step_coord,
            m_future_hint );

        distance = step_track_coord[0] > 0.0f ?  step_track_coord[0]
                   : -step_track_coord[0];

        if (distance > world->m_track->getWidth()[m_track_hint]) return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
void DefaultRobot::handle_race_start()
{

    //FIXME: make karts able to get a penalty for accelerating too soon
    //like players, should happen to about 20% of the karts in easy,
    //5% in medium and less than 1% of the karts in hard.
    if(m_starting_delay <  0.0f)
    {
        srand((unsigned)time(0));

        //Each kart starts at a different, random time, and the time is
        //smaller depending on the difficulty.
        switch(world->m_race_setup.m_difficulty)
        {
        case RD_EASY:
            m_starting_delay = (float)rand()/RAND_MAX * 0.5f;
            break;
        case RD_MEDIUM:
            m_starting_delay = (float)rand()/RAND_MAX * 0.3f;
            break;
        case RD_HARD:
            m_starting_delay = (float)rand()/RAND_MAX * 0.15f;
            break;
        }

        //This is used instead of AutoKart::update() because the AI doesn't
        //needs more. Without this, karts behave a look a little erratic before
        //the race starts.
        placeModel();

    }
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_angle (const size_t HINT, const float ANGLE)
{
    float dist1 = sgDistanceVec2(world->m_track->m_driveline[HINT], m_curr_pos.xyz);
    float dist2 = sgDistanceVec2(world->m_track->m_driveline[m_track_hint], m_curr_pos.xyz);
    float angle = (world->m_track->m_angle[HINT] * dist1 +
                   world->m_track->m_angle[m_track_hint] * dist2) / (dist1 + dist2);

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - m_curr_pos.hpr[0];
    remove_angle_excess(steer_angle);

    steer_angle += ANGLE;
    remove_angle_excess(steer_angle);

    return steer_angle;
}

//-----------------------------------------------------------------------------
float DefaultRobot::steer_to_point(const sgVec2 POINT)
{
    const SGfloat ADJACENT_LINE = POINT[0] - m_curr_pos.xyz[0];
    const SGfloat OPPOSITE_LINE = POINT[1] - m_curr_pos.xyz[1];
    SGfloat theta = sgATan(OPPOSITE_LINE/ADJACENT_LINE);

    //The real value depends on the side of the track that the kart is
    theta += ADJACENT_LINE < 0.0f ? 90.0f : -90.0f;

    float steer_angle = theta - getCoord()->hpr[0];
    remove_angle_excess(steer_angle);

    return steer_angle;
}

//-----------------------------------------------------------------------------
void DefaultRobot::check_crashes(const int STEPS, sgVec3 pos)
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

    crashes.clear();

    const size_t NUM_KARTS = world->getNumKarts();
    sgNormalizeVec2(vel_normal, m_abs_velocity);

    for(int i = 1; STEPS > i; ++i)
    {
        sgAddScaledVec2(step_coord, pos, vel_normal, KART_LENGTH * i);

        /*Find if we crash with any kart*/
        if(crashes.m_kart == -1) //Don't find a kart to dodge if we got one
            for (unsigned int j = 0; j < NUM_KARTS; ++j)
            {
                if(world->getKart(j) == this) continue;

                kart_distance = sgDistanceVec2(step_coord,
                                               world->getKart(j)->getCoord()->xyz);

                {
                    if(kart_distance < KART_LENGTH + 0.125f * i)
                        if(m_velocity.xyz[1] > world->getKart(j)->
                           getVelocity()->xyz[1] * 0.75f) crashes.m_kart = j;
                }
            }

        /*Find if we crash with the drivelines*/
        int hint = world->m_track->findRoadSector(step_coord);
        
        const int UNKNOWN_SECTOR = -1;

#ifdef SHOW_FUTURE_PATH

        ssgaSphere *sphere = new ssgaSphere;

#ifdef ERASE_PATH
        static ssgaSphere *last_sphere = 0;

        if(last_sphere) world->m_scene->removeKid(last_sphere);

        last_sphere = sphere;
#endif

        sgVec3 center;
        center[0] = step_coord[0];
        center[1] = step_coord[1];
        center[2] = pos[2];
        sphere->setCenter(center);
        sphere->setSize(KART_LENGTH);
        if(hint != UNKNOWN_SECTOR)
        {
            sgVec4 colour;
            colour[0] = colour[3] = 255;
            colour[1] = colour[2] = 0;
            sphere->setColour(colour);
        }
        else if(i == 1)
        {
            sgVec4 colour;
            colour[0] = colour[1] = colour[2] = 0;
            colour[3] = 255;
            sphere->setColour(colour);
        }
        world->m_scene->addKid(sphere);
#endif
        if (hint != UNKNOWN_SECTOR)
        {
            m_future_hint = hint;
            crashes.m_road = true;
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//Find the hint that at the longest distance from the kart, that can be
//driven to without crashing with the track, then find towards which of
//the two edges of the track is closest to the next curve after wards,
//and return the position of that edge.
void DefaultRobot::find_non_crashing_point(sgVec2 result)
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;

    const unsigned int DRIVELINE_SIZE = world->m_track->m_driveline.size();

    unsigned int hint = m_track_hint + 1 < DRIVELINE_SIZE ? m_track_hint + 1 : 0;
    int target_hint;

    sgVec2 direction, step_track_coord;
    SGfloat distance;
    int steps;

    //We exit from the function when we have found a solution
    while(1)
    {
        //target_hint is the hint at the longest distance that we can drive
        //to without crashing with the track.
        target_hint = hint + 1 < DRIVELINE_SIZE ? hint + 1 : 0;
        /*
                target_hint = next_hint + count < DRIVELINE_SIZE ?
                    next_hint + count : next_hint + count - DRIVELINE_SIZE;*/

        //direction is a vector from our kart to the hints we are testing
        sgSubVec2(direction, world->m_track->m_driveline[target_hint], m_curr_pos.xyz);

        steps = int(sgLengthVec2(direction) / KART_LENGTH);
        if(steps < 1) steps = 1;

        sgVec2 step_coord;

        sgNormalizeVec2(direction);

        //Test if we crash if we drive towards the target hint
        for(int i = 2; i < steps; ++i)
        {

            sgAddScaledVec2(step_coord, m_curr_pos.xyz, direction, KART_LENGTH * i);

            world->m_track->spatialToTrack( step_track_coord, step_coord,
                hint );

            distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                       : -step_track_coord[0];

            //If we are outside, the previous hint is what we are looking for
            if (distance + KART_LENGTH * 0.5f > world->m_track->getWidth()[hint])
            {
                sgCopyVec2(result, world->m_track->m_driveline[hint]);

#ifdef SHOW_NON_CRASHING_HINT
                ssgaSphere *sphere = new ssgaSphere;

                static ssgaSphere *last_sphere = 0;

                if(last_sphere) world->m_scene->removeKid(last_sphere);

                last_sphere = sphere;

                sgVec3 center;
                center[0] = result[0];
                center[1] = result[1];
                center[2] = m_curr_pos.xyz[2];
                sphere->setCenter(center);
                sphere->setSize(0.5f);

                sgVec4 colour;
                colour[1] = colour[3] = 255;
                colour[0] = colour[2] = 0;
                sphere->setColour(colour);

                world->m_scene->addKid(sphere);
#endif

                return;
            }
        }
        hint = target_hint;
    }
}

//-----------------------------------------------------------------------------
void DefaultRobot::reset()
{
    m_future_hint = 0;
    m_starting_delay = -1.0f;
    m_next_curve_hint = -1;
    m_next_straight_hint = -1;
    m_on_curve = false;
    m_handle_curve = false;

    m_crash_time = 0.0f;

    Kart::reset();
}

//-----------------------------------------------------------------------------
inline void DefaultRobot::remove_angle_excess(float &angle)
{
    if (angle > 180.0f) angle -= 360.0f;
    else if (angle < -180.0f ) angle += 360.0f;
}

//-----------------------------------------------------------------------------
int DefaultRobot::calc_steps()
{
    //calc_steps divides the velocity vector by the lenght of the kart and get the number
    //of steps to use for the sight line of the kart based on that

    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;

    int min_steps = 2;

    switch(world->m_race_setup.m_difficulty)
    {
    case RD_EASY:
        min_steps = 0;
        break;
    case RD_MEDIUM:
        min_steps = 1;
        break;
    case RD_HARD:
        min_steps = 2;
        break;
    }

    int steps = int(m_velocity.xyz[1] / KART_LENGTH);
    if(steps < min_steps) steps = min_steps;

    return steps;
}

/** Translates coordinates from an angle(in degrees) to values within the range
 *  of -1.0 to 1.0 to use the same format as the KartControl::lr variable.
 */
float DefaultRobot::angle_to_control(float angle)
{
    angle *= 180.0f / (getMaxSteerAngle() * M_PI) ;

    if(angle > 1.0f) return 1.0f;
    else if(angle < -1.0f) return -1.0f;

    return angle;
}
