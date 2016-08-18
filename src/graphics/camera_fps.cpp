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

#include "graphics/camera_fps.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/skidding.hpp"

#include "vector3d.h"

using namespace irr;

// ============================================================================
CameraFPS::CameraFPS(int camera_index, AbstractKart* kart)
         : Camera(Camera::CM_TYPE_FPS, camera_index, kart)
{
    m_attached      = false;

    // TODO: Put these values into a config file
    //       Global or per split screen zone?
    //       Either global or per user (for instance, some users may not like
    //       the extra camera rotation so they could set m_rotation_range to
    //       zero to disable it for themselves).
    m_position_speed = 8.0f;
    m_target_speed   = 10.0f;
    m_rotation_range = 0.4f;
    m_rotation_range = 0.0f;
    m_lin_velocity = core::vector3df(0, 0, 0);
    m_target_velocity = core::vector3df(0, 0, 0);
    m_target_direction = core::vector3df(0, 0, 1);
    m_target_up_vector = core::vector3df(0, 1, 0);
    m_direction_velocity = core::vector3df(0, 0, 0);

    m_local_position = core::vector3df(0, 0, 0);
    m_local_direction = core::vector3df(0, 0, 1);
    m_local_up = core::vector3df(0, 1, 0);

    m_angular_velocity = 0;
    m_target_angular_velocity = 0;
    m_max_velocity = 15;
    reset();
}   // Camera

// ----------------------------------------------------------------------------
/** Removes the camera scene node from the scene.
 */
CameraFPS::~CameraFPS()
{
}   // ~Camera

//-----------------------------------------------------------------------------
/** Applies mouse movement to the first person camera.
 *  \param x The horizontal difference of the mouse position.
 *  \param y The vertical difference of the mouse position.
 */
void CameraFPS::applyMouseMovement (float x, float y)
{
    core::vector3df direction(m_target_direction);
    core::vector3df up(m_camera->getUpVector());

    // Set local values if the camera is attached to the kart
    if (m_attached)
        up = m_local_up;

    direction.normalize();
    up.normalize();

    core::vector3df side(direction.crossProduct(up));
    side.normalize();
    core::quaternion quat;
    quat.fromAngleAxis(y, side);

    core::quaternion quat_x;
    quat_x.fromAngleAxis(x, up);
    quat *= quat_x;

    direction = quat * direction;
    // Try to prevent toppling over
    // If the camera would topple over with the next movement, the vertical
    // movement gets reset close to the up vector
    if ((direction - up).getLengthSQ() + (m_target_direction - up).getLengthSQ()
        <= (direction - m_target_direction).getLengthSQ())
        direction = quat_x * ((m_target_direction - up).setLength(0.02f) + up);
    // Prevent toppling under
    else if ((direction + up).getLengthSQ() + (m_target_direction + up).getLengthSQ()
        <= (direction - m_target_direction).getLengthSQ())
        direction = quat_x * ((m_target_direction + up).setLength(0.02f) - up);
    m_target_direction = direction;

    // Don't do that because it looks ugly and is bad to handle ;)
    /*side = direction.crossProduct(up);
    // Compute new up vector
    up = side.crossProduct(direction);
    up.normalize();
    cam->setUpVector(up);*/
}   // applyMouseMovement

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void CameraFPS::update(float dt)
{
    Camera::update(dt);
    
    // To view inside tunnels in top mode, increase near value
    m_camera->setNearValue(1.0f);

    core::vector3df direction(m_camera->getTarget() - m_camera->getPosition());
    core::vector3df up(m_camera->getUpVector());
    core::vector3df side(direction.crossProduct(up));
    core::vector3df pos = m_camera->getPosition();

    // Set local values if the camera is attached to the kart
    if (m_attached)
    {
        direction = m_local_direction;
        up = m_local_up;
        pos = m_local_position;
    }

    // Update smooth movement
    if (m_smooth)
    {
        // Angular velocity
        if (m_angular_velocity < m_target_angular_velocity)
        {
            m_angular_velocity += UserConfigParams::m_fpscam_angular_velocity;
            if (m_angular_velocity > m_target_angular_velocity)
                m_angular_velocity = m_target_angular_velocity;
        }
        else if (m_angular_velocity > m_target_angular_velocity)
        {
            m_angular_velocity -= UserConfigParams::m_fpscam_angular_velocity;
            if (m_angular_velocity < m_target_angular_velocity)
                m_angular_velocity = m_target_angular_velocity;
        }

        // Linear velocity
        core::vector3df diff(m_target_velocity - m_lin_velocity);
        if (diff.X != 0 || diff.Y != 0 || diff.Z != 0)
        {
            if (diff.getLengthSQ() > 1) diff.setLength(1);
            m_lin_velocity += diff;
        }

        // Camera direction
        diff = m_target_direction - direction;
        if (diff.X != 0 || diff.Y != 0 || diff.Z != 0)
        {
            diff.setLength(UserConfigParams::m_fpscam_direction_speed);
            m_direction_velocity += diff;
            if (m_direction_velocity.getLengthSQ() >
                UserConfigParams::m_fpscam_smooth_direction_max_speed *
                UserConfigParams::m_fpscam_smooth_direction_max_speed)
            {
                m_direction_velocity.setLength(
                    UserConfigParams::m_fpscam_smooth_direction_max_speed);
            }
            direction += m_direction_velocity;
            m_target_direction = direction;
        }   // if diff is no 0

        // Camera rotation
        diff = m_target_up_vector - up;
        if (diff.X != 0 || diff.Y != 0 || diff.Z != 0)
        {
            if (diff.getLengthSQ() >
                UserConfigParams::m_fpscam_angular_velocity *
                UserConfigParams::m_fpscam_angular_velocity)
            {
                diff.setLength(UserConfigParams::m_fpscam_angular_velocity);
            }
            up += diff;
        }
    }
    else
    {
        direction = m_target_direction;
        up = m_target_up_vector;
        side = direction.crossProduct(up);
    }

    // Rotate camera
    core::quaternion quat;
    quat.fromAngleAxis(m_angular_velocity * dt, direction);
    up = quat * up;
    m_target_up_vector = quat * up;
    direction.normalize();
    up.normalize();
    side.normalize();

    // Top vector is the real up vector, not the one used by the camera
    core::vector3df top(side.crossProduct(direction));

    // Move camera
    core::vector3df movement(direction * m_lin_velocity.Z +
        top * m_lin_velocity.Y + side * m_lin_velocity.X);
    pos = pos + movement * dt;

    if (m_attached)
    {
        // Save current values
        m_local_position = pos;
        m_local_direction = direction;
        m_local_up = up;

        // Move the camera with the kart
        btTransform t = m_kart->getTrans();
        if (stk_config->m_camera_follow_skid &&
            m_kart->getSkidding()->getVisualSkidRotation() != 0)
        {
            // If the camera should follow the graphical skid, add the
            // visual rotation to the relative vector:
            btQuaternion q(m_kart->getSkidding()->getVisualSkidRotation(), 0, 0);
            t.setBasis(t.getBasis() * btMatrix3x3(q));
        }
        pos = Vec3(t(Vec3(pos))).toIrrVector();

        btQuaternion q = t.getRotation();
        btMatrix3x3 mat(q);
        direction = Vec3(mat * Vec3(direction)).toIrrVector();
        up = Vec3(mat * Vec3(up)).toIrrVector();
    }

    // Set camera attributes
    m_camera->setPosition(pos);
    m_camera->setTarget(pos + direction);
    m_camera->setUpVector(up);
}   // update

// ----------------------------------------------------------------------------
/** Sets the angular velocity for this camera. */
void CameraFPS::setAngularVelocity(float vel)
{
    if (m_smooth)
        m_target_angular_velocity = vel;
    else
        m_angular_velocity = vel;
}   // setAngularVelocity

// ----------------------------------------------------------------------------
/** Returns the current target angular velocity. */
float CameraFPS::getAngularVelocity()
{
    if (m_smooth)
        return m_target_angular_velocity;
    else
        return m_angular_velocity;
}   // getAngularVelocity

// ----------------------------------------------------------------------------
/** Sets the linear velocity for this camera. */
void CameraFPS::setLinearVelocity(core::vector3df vel)
{
    if (m_smooth)
        m_target_velocity = vel;
    else
        m_lin_velocity = vel;
}   // setLinearVelocity

// ----------------------------------------------------------------------------
/** Returns the current linear velocity. */
const core::vector3df &CameraFPS::getLinearVelocity()
{
    if (m_smooth)
        return m_target_velocity;
    else
        return m_lin_velocity;
}   // getLinearVelocity

