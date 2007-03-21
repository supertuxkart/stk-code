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

    void remove_angle_excess (float &angle);
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
