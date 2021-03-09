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

#include "graphics/camera_debug.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/skidding.hpp"
#include "physics/btKart.hpp"

CameraDebug::CameraDebugType CameraDebug::m_default_debug_Type = 
                                            CameraDebug::CM_DEBUG_TOP_OF_KART;

// ============================================================================
CameraDebug::CameraDebug(int camera_index, AbstractKart* kart)
           : CameraNormal(Camera::CM_TYPE_DEBUG, camera_index, kart)
{
    reset();
}   // Camera

// ----------------------------------------------------------------------------
/** Removes the camera scene node from the scene.
 */
CameraDebug::~CameraDebug()
{
}   // ~CameraDebug

//-----------------------------------------------------------------------------
/** Determine the camera settings for the current frame.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
 */
void CameraDebug::getCameraSettings(float *above_kart, float *cam_angle,
                                    float *sideway, float *distance    )
{
    // Set some default values
    float steering = m_kart->getSteerPercent()
                   * (1.0f + (m_kart->getSkidding()->getSkidFactor()
                              - 1.0f) / 2.3f);
    // quadratically to dampen small variations (but keep sign)
    float dampened_steer = fabsf(steering) * steering;
    *sideway = -m_rotation_range*dampened_steer*0.5f;
    *above_kart = 0;
    *cam_angle  = 0;
    
    switch(m_default_debug_Type)
    {
    case CM_DEBUG_BEHIND_KART:
        *distance   = -0.5f*m_kart->getKartModel()->getLength()-1.0f;
        break;
    case CM_DEBUG_GROUND:
        *distance   = -m_kart->getKartModel()->getLength()-1.0f;
        break;
    case CM_DEBUG_SIDE_OF_KART:
    case CM_DEBUG_INV_SIDE_OF_KART:
    case CM_DEBUG_FRONT_OF_KART:
    case CM_DEBUG_TOP_OF_KART:
        *above_kart    = 0.75f;
        *cam_angle     = UserConfigParams::m_camera_forward_up_angle * DEGREE_TO_RAD;
        *distance      = -m_distance;
        break;
    }   // switch 

}   // getCameraSettings

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void CameraDebug::update(float dt)
{
    Camera::update(dt);

    m_camera->setNearValue(1.0f);

    float above_kart, cam_angle, side_way, distance;

    // The following settings give a debug camera which shows the track from
    // high above the kart straight down.
    if (m_default_debug_Type==CM_DEBUG_TOP_OF_KART)
    {
        core::vector3df xyz = m_kart->getSmoothedXYZ().toIrrVector();
        m_camera->setTarget(xyz);
#define CLOSE_TO_KART
#ifdef CLOSE_TO_KART
        // Better for debugging physics/collision issues
        xyz.Y = xyz.Y+7;
        m_camera->setNearValue(7.0f);
#else
        // Very high few, better for debugging AI behaviour
        xyz.Y = xyz.Y+55;
        xyz.Z -= 5.0f;
        m_camera->setNearValue(27.0f);
#endif
        m_camera->setPosition(xyz);
    }
    else if (m_default_debug_Type==CM_DEBUG_SIDE_OF_KART)
    {
        core::vector3df xyz = m_kart->getSmoothedXYZ().toIrrVector();
        Vec3 offset(3, 0, 0);
        offset = m_kart->getSmoothedTrans()(offset);
        m_camera->setTarget(xyz);
        m_camera->setPosition(offset.toIrrVector());
    }
    else if (m_default_debug_Type==CM_DEBUG_INV_SIDE_OF_KART)
    {
        core::vector3df xyz = m_kart->getSmoothedXYZ().toIrrVector();
        Vec3 offset(-3, 0, 0);
        offset = m_kart->getSmoothedTrans()(offset);
        m_camera->setTarget(xyz);
        m_camera->setPosition(offset.toIrrVector());
    }
    else if (m_default_debug_Type==CM_DEBUG_FRONT_OF_KART)
    {
        core::vector3df xyz = m_kart->getSmoothedXYZ().toIrrVector();
        Vec3 offset(0, 1, 2);
        offset = m_kart->getSmoothedTrans()(offset);
        m_camera->setTarget(xyz);
        m_camera->setPosition(offset.toIrrVector());
    }
    // If an explosion is happening, stop moving the camera,
    // but keep it target on the kart.
    else if (dynamic_cast<ExplosionAnimation*>(m_kart->getKartAnimation()))
    {
        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance);
        // The camera target needs to be 'smooth moved', otherwise
        // there will be a noticable jump in the first frame

        // Aim at the usual same position of the kart (i.e. slightly
        // above the kart).
        // Note: this code is replicated from smoothMoveCamera so that
        // the camera keeps on pointing to the same spot.
        core::vector3df current_target = (m_kart->getSmoothedXYZ().toIrrVector()
                                         +core::vector3df(0, above_kart, 0));
        m_camera->setTarget(current_target);
    }
    else
    {
        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance);
        positionCamera(dt, above_kart, cam_angle, side_way, distance);
    }
}   // update

// ----------------------------------------------------------------------------
/** Actually sets the camera based on the given parameter.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
*/
void CameraDebug::positionCamera(float dt, float above_kart, float cam_angle,
                                 float side_way, float distance              )
{
    Vec3 wanted_position;
    Vec3 wanted_target = m_kart->getSmoothedXYZ();
    if(m_default_debug_Type==CM_DEBUG_GROUND)
    {
        const btWheelInfo &w = m_kart->getVehicle()->getWheelInfo(2);
        wanted_target.setY(w.m_raycastInfo.m_contactPointWS.getY());
    }
    else
        wanted_target.setY(wanted_target.getY()+above_kart);
    float tan_up = tan(cam_angle);
    Vec3 relative_position(side_way,
                           fabsf(distance)*tan_up+above_kart,
                           distance);
    btTransform t=m_kart->getSmoothedTrans();
    if(stk_config->m_camera_follow_skid &&
        m_kart->getSkidding()->getVisualSkidRotation()!=0)
    {
        // If the camera should follow the graphical skid, add the
        // visual rotation to the relative vector:
        btQuaternion q(m_kart->getSkidding()->getVisualSkidRotation(), 0, 0);
        t.setBasis(t.getBasis() * btMatrix3x3(q));
    }
    if (m_default_debug_Type == CM_DEBUG_GROUND)
    {
        wanted_position = t(relative_position);
        // Make sure that the Y position is a the same height as the wheel.
        wanted_position.setY(wanted_target.getY());
    }
    else
        wanted_position = t(relative_position);

    if (getMode() != CM_FALLING)
        m_camera->setPosition(wanted_position.toIrrVector());
    m_camera->setTarget(wanted_target.toIrrVector());

    Kart *kart = dynamic_cast<Kart*>(m_kart);
    if (kart && !kart->isFlying())
    {
        // Rotate the up vector (0,1,0) by the rotation ... which is just column 1
        Vec3 up = m_kart->getSmoothedTrans().getBasis().getColumn(1);
        float f = 0.04f;  // weight for new up vector to reduce shaking
        m_camera->setUpVector(        f  * up.toIrrVector() +
                              (1.0f - f) * m_camera->getUpVector());
    }   // kart && !flying
    else
        m_camera->setUpVector(core::vector3df(0, 1, 0));
}   // positionCamera

