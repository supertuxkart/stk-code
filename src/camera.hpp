//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

#include "vec3.hpp"

class ssgContext;
class Kart;

class Camera
{
public:
    enum Mode {
        CM_NORMAL,        // Normal camera mode
        CM_CLOSEUP,       // Normal camera, closer to kart
        CM_DRIFTING,      // FIXME: drifting behind when accelerating = not yet implemented
        CM_LEADER_MODE,   // for deleted player karts in follow the leader
        CM_REVERSE,       // Camera is pointing backwards
        CM_FINAL,         // Final camera to show the end of the race
        CM_SIMPLE_REPLAY
    };
protected:
    ssgContext *m_context ;
//    sgCoord     m_current_pos;
    Vec3        m_xyz;                  // current position of camera
    Vec3        m_hpr;                  // heading, pitch, roll of camera
    const Kart *m_kart;                 // the kart the camera is attached to
    Mode        m_mode;                 // CM_ value, see above
    float       m_x, m_y, m_w, m_h;     // window to us
    float       m_current_speed;        // current speed of camera
    float       m_last_pitch;           // for tiling the camera when going downhill
    float       m_distance;             // distance between camera and kart
    Vec3        m_velocity;             // camera velocity for final mode
    Vec3        m_angular_velocity;     // camera angular velocity for final mode
    float       m_final_time;           // time when final camera mode started

private:
    void finalCamera      (float dt);   // handle the final camera
public:
         Camera           (int camera_index, const Kart* kart);
    void setMode          (Mode mode_);    /** Set the camera to the given mode */
    void setScreenPosition(int pos);
    void reset            ();
    void update           (float dt);
    void apply            ();
} ;

#endif

/* EOF */
