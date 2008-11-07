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

#ifndef HEADER_DEFAULT_H
#define HEADER_DEFAULT_H

#include "karts/auto_kart.hpp"

class DefaultRobot : public AutoKart
{
private:
    enum FallbackTactic
    {
        FT_AVOID_TRACK_CRASH, //Only steer to avoid getting out of the road,
                              //otherwise, don't steer at all
        FT_PARALLEL,    //Stay parallel to the road
        FT_FAREST_POINT //Drive towards the farest non-crashing point that
                        //the kart can drive to in a straight line without
                        //crashing with the track.
    };

    enum ItemTactic
    {
        IT_TEN_SECONDS, //Fire after 10 seconds have passed, since the item
                        //was grabbed.
        IT_CALCULATE //Aim carefully, check for enough space for boosters,
                     //and that other conditions are meet before firing.
    };

    class CrashTypes
    {
        public:

        bool m_road; //true if we are going to 'crash' with the bounds of the road
        int m_kart; //-1 if no crash, pos numbers are the kart it crashes with
        CrashTypes() : m_road(false), m_kart(-1) {};
        void clear() {m_road = false; m_kart = -1;}
    } m_crashes;

    /*Difficulty handling variables*/
    float m_max_start_delay; //Delay before accelerating at the start of each
                             //race
    int m_min_steps; //Minimum number of steps to check. If 0, the AI doesn't
                     //even has check around the kart, if 1, it checks around
                     //the kart always, and more than that will check the
                     //remaining number of steps in front of the kart, always
    bool  m_wait_for_players; //If true, the acceleration is decreased when
                              //the AI is in a better position than all the
                              //human players.
    float m_max_handicap_accel; //The allowed maximum speed, in percentage,
                                //from 0.0 to 1.0. Used only when
                                //m_wait_for_players == true.
    FallbackTactic m_fallback_tactic; //General steering procedure. Used
                                      //mostly on straight lines and on curves
                                      //that re too small to need special
                                      //handling.
    bool m_use_wheelies; //Is the AI allowed to use wheelies?
    float m_wheelie_check_dist; //How far to check for the space needed for
                                //wheelies, in percentage. Used only when
                                //m_use_wheelies == true.
    ItemTactic m_item_tactic; //How are items going to be used?

    /*General purpose variables*/
    //The crash percentage is how much of the time the AI has been crashing,
    //if the AI has been crashing for some time, use the rescue.
    float m_crash_time;
    int   m_collided;           // true if the kart collided with the track

    float m_time_since_last_shot;
    int   m_future_sector;
    sgVec2 m_future_location;

    float m_time_till_start; //Used to simulate a delay at the start of the
                             //race, since human players don't accelerate
                             //at the same time and rarely repeat the a
                             //previous timing.

    int m_inner_curve;//-1 left, 1 = right, 0 = center
    float m_curve_target_speed;
    float m_curve_angle;

    float m_time_since_stuck;

    int m_start_kart_crash_direction; //-1 = left, 1 = right, 0 = no crash.

    /** Length of the kart, storing it here saves many function calls. */
    float m_kart_length;

    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable, except handle_race_start() that isn't associated with any
     *specific action (more like, associated with inaction).
     */
    void handle_race_start();
    void handle_acceleration(const float DELTA);
    void handle_steering();
    void handle_items(const float DELTA, const int STEPS);
    void handle_rescue(const float DELTA);
    void handle_braking();
    void handle_wheelie(const int STEPS);

    /*Lower level functions not called directly from update()*/
    float steer_to_angle(const size_t SECTOR, const float ANGLE);
    float steer_to_point(const sgVec2 POINT);

    bool do_wheelie(const int STEPS);
    void check_crashes(const int STEPS, const Vec3& pos);
    void find_non_crashing_point(sgVec2 result);

    float normalize_angle (float angle);
    int calc_steps();

    float angle_to_control(float angle) const;
    float get_approx_radius(const int START, const int END) const;
    void find_curve();
    int m_sector;

public:
    DefaultRobot(const std::string& kart_name, int position,
                 const btTransform& init_pos);

    void update      (float delta) ;
    void reset       ();
    virtual void crashed(Kart *k) {if(k) m_collided = true;};
};

#endif

/* EOF */
