//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_CAMERA_FPS_HPP
#define HEADER_CAMERA_FPS_HPP

#include "graphics/camera.hpp"

#include "utils/cpp2011.hpp"

class AbstractKart;

/**
  * \brief Handles the game camera
  * \ingroup graphics
  */
class CameraFPS : public Camera
{
private:
    /** The speed at which the camera changes position. */
    float           m_position_speed;

    /** The speed at which the camera target changes position. */
    float           m_target_speed;

    /** Factor of the effects of steering in camera aim. */
    float           m_rotation_range;

    /** Smooth acceleration with the first person camera. */
    bool m_smooth;

    /** Attache the first person camera to a kart.
        That means moving the kart also moves the camera. */
    bool m_attached;

    /** The speed at which the up-vector rotates, only used for the first person camera. */
    float m_angular_velocity;

    /** Target angular velocity. Used for smooth movement in fps perpective. */
    float m_target_angular_velocity;

    /** Maximum velocity for fps camera. */
    float m_max_velocity;

    /** Linear velocity of the camera, used for end and first person camera.
        It's stored relative to the camera direction for the first person view. */
    core::vector3df m_lin_velocity;

    /** Velocity of the target of the camera, used for end and first person camera. */
    core::vector3df m_target_velocity;

    /** The target direction for the camera, only used for the first person camera. */
    core::vector3df m_target_direction;

    /** The speed at which the direction changes, only used for the first person camera. */
    core::vector3df m_direction_velocity;

    /** The up vector the camera should have, only used for the first person camera. */
    core::vector3df m_target_up_vector;

    /** Save the local position if the first person camera is attached to the kart. */
    core::vector3df m_local_position;

    /** Save the local direction if the first person camera is attached to the kart. */
    core::vector3df m_local_direction;

    /** Save the local up vector if the first person camera is attached to the kart. */
    core::vector3df m_local_up;



    void positionCamera(float dt, float above_kart, float cam_angle,
                        float side_way, float distance, float smoothing);

    friend class Camera;
             CameraFPS(int camera_index, AbstractKart* kart);
    virtual ~CameraFPS();
public:
    // ------------------------------------------------------------------------
    static bool isFPS() { return true; }
    // ------------------------------------------------------------------------

    virtual void update(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Applies mouse movement to the first person camera. */
    void applyMouseMovement (float x, float y);

    // ------------------------------------------------------------------------
    /** Sets if the first person camera should be moved smooth. */
    void setSmoothMovement (bool value) { m_smooth = value; }

    // ------------------------------------------------------------------------
    /** If the first person camera should be moved smooth. */
    bool getSmoothMovement () { return m_smooth; }

    // ------------------------------------------------------------------------
    /** Sets if the first person camera should be moved with the kart. */
    void setAttachedFpsCam (bool value) { m_attached = value; }

    // ------------------------------------------------------------------------
    /** If the first person camera should be moved with the kart. */
    bool getAttachedFpsCam () { return m_attached; }

    // ------------------------------------------------------------------------
    /** Sets the angular velocity for this camera. */
    void setMaximumVelocity (float vel) { m_max_velocity = vel; }

    // ------------------------------------------------------------------------
    /** Returns the current angular velocity. */
    float getMaximumVelocity () { return m_max_velocity; }

    // ------------------------------------------------------------------------
    /** Sets the vector, the first person camera should look at. */
    void setDirection (core::vector3df target) { m_target_direction = target; }

    // ------------------------------------------------------------------------
    /** Gets the vector, the first person camera should look at. */
    const core::vector3df &getDirection () { return m_target_direction; }

    // ------------------------------------------------------------------------
    /** Sets the up vector, the first person camera should use. */
    void setUpVector (core::vector3df target) { m_target_up_vector = target; }

    // ------------------------------------------------------------------------
    /** Gets the up vector, the first person camera should use. */
    const core::vector3df &getUpVector () { return m_target_up_vector; }

    // ------------------------------------------------------------------------
    /** Sets the angular velocity for this camera. */
    void setAngularVelocity (float vel);

    // ------------------------------------------------------------------------
    /** Returns the current target angular velocity. */
    float getAngularVelocity ();

    // ------------------------------------------------------------------------
    /** Sets the linear velocity for this camera. */
    void setLinearVelocity (core::vector3df vel);

    // ------------------------------------------------------------------------
    /** Returns the current linear velocity. */
    const core::vector3df &getLinearVelocity ();

};   // class CameraFPS

#endif

/* EOF */
