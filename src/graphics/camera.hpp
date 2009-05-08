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

#ifndef HEADER_CAMERA_HPP
#define HEADER_CAMERA_HPP

#include "utils/vec3.hpp"
#ifdef HAVE_IRRLICHT
#include "irrlicht.h"
using namespace irr;
#else
class ssgContext;
#endif
class Kart;

class Camera
{
public:
    enum Mode {
        CM_NORMAL,        // Normal camera mode
        CM_CLOSEUP,       // Normal camera, closer to kart
        CM_LEADER_MODE,   // for deleted player karts in follow the leader
        CM_FINAL,         // Final camera to show the end of the race
        CM_SIMPLE_REPLAY
    };

protected:
#ifdef HAVE_IRRLICHT
    scene::ICameraSceneNode 
               *m_camera;
#endif
    Mode        m_mode;             // Camera's mode
    Vec3        m_position;         // The ultimate position which the camera wants to obtain
    Vec3        m_temp_position;    // The position the camera currently has
    Vec3        m_target;           // The ultimate target which the camera wants to obtain
    Vec3        m_temp_target;      // The target the camera currently has

    int         m_index;

    float       m_distance;         // Distance between the camera and the kart
    float       m_angle_up;         // Angle between the ground and the camera (with the kart as the vertex of the angle)
    float       m_angle_around;     // Angle around the kart (should actually match the rotation of the kart)
    float       m_position_speed;   // The speed at which the camera changes position
    float       m_target_speed;     // The speed at which the camera changes targets
    float       m_rotation_range;   // Factor of the effects of steering in camera aim

    float       m_x, m_y, m_w, m_h; 

    const Kart *m_kart;             // The kart that the camera follows

private:
    
public:
         Camera            (int camera_index, const Kart* kart);
        ~Camera            ();
    void setMode           (Mode mode_);    /** Set the camera to the given mode */
    Mode getMode();
    void setScreenPosition (int pos);
    void reset             ();
    void setInitialTransform();
    void update            (float dt);
    void apply             ();
} ;

#endif

/* EOF */
