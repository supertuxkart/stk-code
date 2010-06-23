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
#include "irrlicht.h"
using namespace irr;

class Kart;

/**
  * \brief Handles the game camera
  * \ingroup graphics
  */
class Camera
{
public:
    enum Mode {
        CM_NORMAL,        //!< Normal camera mode
        CM_CLOSEUP,       //!< Closer to kart
        CM_REVERSE,       //!< Looking backwards
        CM_LEADER_MODE,   //!< for deleted player karts in follow the leader
        CM_FINAL,         //!< Final camera
        CM_SIMPLE_REPLAY
    };

private:
    /** The camera scene node. */
    scene::ICameraSceneNode *m_camera;

    /** Camera's mode. */
    Mode            m_mode;

    /** The index of this camera which is the index of the kart it is 
     *  attached to. */
    unsigned int    m_index;

    /** Current ambient light for this camera. */
    video::SColor   m_ambient_light;

    /** Distance between the camera and the kart. */
    float           m_distance;

    /** The speed at which the camera changes position. */
    float           m_position_speed;

    /** The speed at which the camera target changes position. */
    float           m_target_speed;

    /** Factor of the effects of steering in camera aim. */
    float           m_rotation_range;

    /** The kart that the camera follows. */
    const Kart     *m_kart;

    /** The list of viewports for this cameras. */
    core::recti     m_viewport;

    /** The scaling necessary for each axis. */
    core::vector2df m_scaling;

    /** Field of view for the camera. */
    float           m_fov;

    /** Aspect ratio for camera. */
    float           m_aspect;

    /** Linear velocity of the camera, only used for end camera. */
    core::vector3df m_lin_velocity;

    /** Velocity of the target of the camera, only used for end camera. */
    core::vector3df m_target_velocity;

    void setupCamera();
    void smoothMoveCamera(float dt, const Vec3 &wanted_position,
                          const Vec3 &wanted_target);
    void computeNormalCameraPosition(Vec3 *wanted_position,
                                     Vec3 *wanted_target);
public:
         Camera            (int camera_index, const Kart* kart);
        ~Camera            ();
    void setMode           (Mode mode_);    /** Set the camera to the given mode */
    Mode getMode();
    /** Returns the camera index (or player kart index, which is the same). */
    int  getIndex() const  {return m_index;}
    void reset             ();
    void setInitialTransform();
    void activate();
    void update            (float dt);

    /** Sets the ambient light for this camera. */
    void setAmbientLight(const video::SColor &color) { m_ambient_light=color; }

    /** Returns the current ambient light. */
    const video::SColor &getAmbientLight() const {return m_ambient_light; }

    /** Returns the viewport of this camera. */
    const core::recti& getViewport() const {return m_viewport; }

    /** Returns the scaling in x/y direction for this camera. */
    const core::vector2df& getScaling() const {return m_scaling; }

    /** Returns the camera scene node. */
    scene::ICameraSceneNode *getCameraSceneNode() 
    {
        return m_camera;
    }
} ;

#endif

/* EOF */
