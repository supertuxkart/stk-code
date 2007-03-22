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

#ifndef HEADER_DEFAULT_H
#define HEADER_DEFAULT_H

#include "auto_kart.hpp"

class DefaultRobot : public AutoKart
{
private:
    enum StraightTactic
    {
        ST_DONT_STEER,  //In straight roads, don't steer at all
        ST_PARALLEL,    //Stay parallel to the road
        ST_FAREST_POINT //Drive towards the farest non-crashing point
    };

    enum ItemTactic
    {
        IT_TEN_SECONDS, //Fire after 10 seconds have passed, since the item
                        //was grabbed.
        IT_CALCULATE //Aim carefully, check for enough space for boosters,
                     //and that other conditions are meet before firing.
    };

    /*Difficulty handling variables*/
    float m_max_speed; //The allowed maximum speed, in percentage,
                       //from 0.0 to 1.0
    StraightTactic m_straights_tactic; //How to steer on straight roads
    bool m_use_wheelies; //Is the AI allowed to use wheelies?
    float m_wheelie_check_dist; //How far to check for the space needed for
                                //wheelies, in percentage
    ItemTactic m_item_tactic; //How are items going to be used?
    float m_max_start_delay; //Delay before accelerating at the start of each
                             //race
    int m_min_steps; //Minimum number of steps to check. If 0, the AI doesn't
                     //even has check around the kart, if 1, it checks around
                     //the kart always, and more than that will check the
                     //remaining number of steps in front of the kart, always


    /*General purpose variables*/
    //The crash percentage is how much of the time the AI has been crashing,
    //if the AI has been crashing for some time, use the rescue.
    float m_crash_time;

    float m_time_since_last_shot;
    size_t m_future_sector;

    float m_starting_delay;

    int m_next_curve_sector;
    int m_next_straight_sector;
    bool m_on_curve;
    bool m_handle_curve;

    class CrashTypes
    {
        public:

        bool m_road; //true if we are going to 'crash' with the bounds of the road
        int m_kart; //-1 if no crash, pos numbers are the kart it crashes with
        CrashTypes() : m_road(false), m_kart(-1) {};
        void clear() {m_road = false; m_kart = -1;}
    }
    crashes;

    int start_kart_crash_direction; //-1 = left, 1 = right, 0 = no crash.

    void handle_race_start();

    float steer_to_angle(const size_t SECTOR, const float ANGLE);
    float steer_to_point(const sgVec2 POINT);

    bool do_wheelie(const int STEPS);
    void check_crashes(const int STEPS, sgVec3 pos);
    void find_non_crashing_point(sgVec2 result);

    float normalize_angle (float angle);
    int calc_steps();

    float angle_to_control(float angle);

public:
    DefaultRobot(const KartProperties *kart_properties, int position,
             sgCoord init_pos);

    void update      (float delta) ;
    void reset       ();
    int  isPlayerKart() const {return 0;}
};

#endif

/* EOF */
