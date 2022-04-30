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

#include "graphics/camera_normal.hpp"

#include "audio/sfx_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/multitouch_device.hpp"
#include "modes/soccer_world.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "tracks/track.hpp"

// ============================================================================
/** Constructor for the normal camera. This is the only camera constructor
 *  except for the base class that takes a camera type as parameter. This is
 *  because debug and end camera use the normal camera as their base class.
 *  \param type The type of the camera that is created (can be CM_TYPE_END
 *         or CM_TYPE_DEBUG).
 *  \param camera_index Index of this camera.
 *  \param Kart Pointer to the kart for which this camera is used.
 */
CameraNormal::CameraNormal(Camera::CameraType type,  int camera_index, 
                           AbstractKart* kart) 
            : Camera(type, camera_index, kart), m_camera_offset(0, 0, -15.0f)
{
    m_distance = kart ? UserConfigParams::m_camera_distance : 1000.0f;
    m_ambient_light = Track::getCurrentTrack()->getDefaultAmbientColor();

    // TODO: Put these values into a config file
    //       Global or per split screen zone?
    //       Either global or per user (for instance, some users may not like
    //       the extra camera rotation so they could set m_rotation_range to
    //       zero to disable it for themselves).
    m_position_speed = 8.0f;
    m_target_speed   = 10.0f;
    m_rotation_range = 0.4f;
    m_rotation_range = 0.0f;
    m_kart_position = btVector3(0, 0, 0);
    m_kart_rotation = btQuaternion(0, 0, 0, 1);
    reset();
    m_camera->setNearValue(1.0f);

    if (kart)
    {
        btTransform btt = kart->getSmoothedTrans();
        m_kart_position = btt.getOrigin();
        m_kart_rotation = btt.getRotation();
    }
}   // Camera

//-----------------------------------------------------------------------------
/** Moves the camera smoothly from the current camera position (and target)
 *  to the new position and target.
 *  \param dt Delta time, 
 *  \param if false, the camera instantly moves to the endpoint, or else it smoothly moves
 */
void CameraNormal::moveCamera(float dt, bool smooth, float cam_angle, float distance)
{
    if(!m_kart) return;

    Kart *kart = dynamic_cast<Kart*>(m_kart);
    if (kart->isFlying())
    {
        Vec3 vec3 = m_kart->getSmoothedXYZ() + Vec3(sinf(m_kart->getHeading()) * -4.0f,
            0.5f,
            cosf(m_kart->getHeading()) * -4.0f);
        m_camera->setTarget(m_kart->getSmoothedXYZ().toIrrVector());
        m_camera->setPosition(vec3.toIrrVector());
        return;
    }   // kart is flying

    core::vector3df current_position = m_camera->getPosition();
    // Smoothly interpolate towards the position and target
    const KartProperties *kp = m_kart->getKartProperties();
    float max_speed_without_zipper = kp->getEngineMaxSpeed();
    float current_speed = m_kart->getSpeed();

    const Skidding *ks = m_kart->getSkidding();
    float skid_factor = ks->getVisualSkidRotation();

    float skid_angle = asinf(skid_factor);
    float ratio = current_speed / max_speed_without_zipper;

    ratio = ratio > -0.12f ? ratio : -0.12f;

    // distance of camera from kart in x and z plane
    float camera_distance = -1.25f - 2.5f * ratio;
    float min_distance = (distance * 2.0f);
    if (distance > 0) camera_distance += distance + 1; // note that distance < 0
    if (camera_distance > min_distance) camera_distance = min_distance; // don't get too close to the kart

    float tan_up = 0;
    if (cam_angle > 0) tan_up = tanf(cam_angle) * distance;

    // Defines how far camera should be from player kart.
    Vec3 wanted_camera_offset(camera_distance * sinf(skid_angle / 2),
        (0.85f + ratio / 2.5f) - tan_up,
        camera_distance * cosf(skid_angle / 2));

    float delta = 1;
    float delta2 = 1;
    if (smooth)
    {
        delta = (dt*5.0f);
        if (delta < 0.0f)
            delta = 0.0f;
        else if (delta > 1.0f)
            delta = 1.0f;

        delta2 = dt * 8.0f;
        if (delta2 < 0)
            delta2 = 0;
        else if (delta2 > 1)
            delta2 = 1;
    }

    btTransform btt = m_kart->getSmoothedTrans();
    m_kart_position = btt.getOrigin();
    btQuaternion q1, q2;
    q1 = m_kart_rotation.normalized();
    q2 = btt.getRotation().normalized();
    if (dot(q1, q2) < 0.0f)
        q2 = -q2;

    m_kart_rotation = q1.slerp(q2, delta2);

    btt.setOrigin(m_kart_position);
    btt.setRotation(q1);

    Vec3 kart_camera_position_with_offset = btt(m_camera_offset);
    m_camera_offset += (wanted_camera_offset - m_camera_offset) * delta;

    // next target
    Vec3 current_target = btt(Vec3(0, 0.5f, 0));
    // new required position of camera
    current_position = kart_camera_position_with_offset.toIrrVector();

    //Log::info("CAM_DEBUG", "OFFSET: %f %f %f TRANSFORMED %f %f %f TARGET %f %f %f",
    //    wanted_camera_offset.x(), wanted_camera_offset.y(), wanted_camera_offset.z(),
    //    kart_camera_position_with_offset.x(), kart_camera_position_with_offset.y(),
    //    kart_camera_position_with_offset.z(), current_target.x(), current_target.y(),
    //    current_target.z());

    if(getMode()!=CM_FALLING)
        m_camera->setPosition(current_position);
    m_camera->setTarget(current_target.toIrrVector());//set new target

    assert(!std::isnan(m_camera->getPosition().X));
    assert(!std::isnan(m_camera->getPosition().Y));
    assert(!std::isnan(m_camera->getPosition().Z));

}   // moveCamera

//-----------------------------------------------------------------------------
void CameraNormal::snapToPosition()
{
    moveCamera(1.0f, false, 0, 0);
}   // snapToPosition

//-----------------------------------------------------------------------------
/** Determine the camera settings for the current frame.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
 *  \param cam_roll_angle Roll camera for gyroscope steering effect.
 */
void CameraNormal::getCameraSettings(float *above_kart, float *cam_angle,
                                     float *sideway, float *distance,
                                     bool *smoothing, float *cam_roll_angle)
{
    switch(getMode())
    {
    case CM_NORMAL:
    case CM_FALLING:
        {
            *above_kart = 0.75f;
            *cam_angle = UserConfigParams::m_camera_forward_up_angle * DEGREE_TO_RAD;
            *distance = -m_distance;
            float steering = m_kart->getSteerPercent()
                           * (1.0f + (m_kart->getSkidding()->getSkidFactor()
                                      - 1.0f)/2.3f );
            // quadratically to dampen small variations (but keep sign)
            float dampened_steer = fabsf(steering) * steering;
            *sideway             = -m_rotation_range*dampened_steer*0.5f;
            *smoothing           = UserConfigParams::m_camera_forward_smoothing;
            *cam_roll_angle      = 0.0f;
            if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            {
                MultitouchDevice* device = input_manager->getDeviceManager()->getMultitouchDevice();
                if (device)
                {
                    *cam_roll_angle = device->getOrientation();
                }
            }
            break;
        }   // CM_FALLING
    case CM_REVERSE: // Same as CM_NORMAL except it looks backwards
        {
            *above_kart = 0.75f;
            *cam_angle  = UserConfigParams::m_camera_backward_up_angle * DEGREE_TO_RAD;
            *sideway    = 0;
            *distance   = UserConfigParams::m_camera_backward_distance;
            *smoothing  = false;
            *cam_roll_angle = 0.0f;
            if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            {
                MultitouchDevice* device = input_manager->getDeviceManager()->getMultitouchDevice();
                if (device)
                {
                    *cam_roll_angle = -device->getOrientation();
                }
            }
            break;
        }
    case CM_CLOSEUP: // Lower to the ground and closer to the kart
        {
            *above_kart = 0.75f;
            *cam_angle  = 20.0f*DEGREE_TO_RAD;
            *sideway    = m_rotation_range
                        * m_kart->getSteerPercent()
                        * m_kart->getSkidding()->getSkidFactor();
            *distance   = -0.5f*m_distance;
            *smoothing  = false;
            *cam_roll_angle = 0.0f;
            if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            {
                MultitouchDevice* device = input_manager->getDeviceManager()->getMultitouchDevice();
                if (device)
                {
                    *cam_roll_angle = -device->getOrientation();
                }
            }
            break;
        }
    case CM_LEADER_MODE:
        {
            *above_kart = 0.0f;
            *cam_angle  = 40*DEGREE_TO_RAD;
            *sideway    = 0;
            *distance   = 2.0f*m_distance;
            *smoothing  = true;
            *cam_roll_angle = 0.0f;
            break;
        }
    case CM_SPECTATOR_SOCCER:
        {
            *above_kart = 0.0f;
            *cam_angle  = UserConfigParams::m_spectator_camera_angle*DEGREE_TO_RAD;
            *sideway    = 0;
            *distance   = -UserConfigParams::m_spectator_camera_distance;
            *smoothing  = true;
            *cam_roll_angle = 0.0f;
            break;
        }
    case CM_SPECTATOR_TOP_VIEW:
        {
            *above_kart = 0.0f;
            *cam_angle  = 0;
            *sideway    = 0;
            *distance   = UserConfigParams::m_spectator_camera_distance;
            *smoothing  = true;
            *cam_roll_angle = 0.0f;
            break;
        }
    case CM_SIMPLE_REPLAY:
        // TODO: Implement
        break;
    }

}   // getCameraSettings

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void CameraNormal::update(float dt)
{
    Camera::update(dt);
    if(!m_kart) return;

    m_camera->setNearValue(1.0f);

    // If an explosion is happening, stop moving the camera,
    // but keep it target on the kart.
    ExplosionAnimation* ea =
        dynamic_cast<ExplosionAnimation*>(m_kart->getKartAnimation());
    if (ea && !ea->hasResetAlready())
    {
        float above_kart, cam_angle, side_way, distance, cam_roll_angle;
        bool  smoothing;

        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance, &smoothing, &cam_roll_angle);
        // The camera target needs to be 'smooth moved', otherwise
        // there will be a noticable jump in the first frame

        // Aim at the usual same position of the kart (i.e. slightly
        // above the kart).
        // Note: this code is replicated from smoothMoveCamera so that
        // the camera keeps on pointing to the same spot.
        core::vector3df current_target = (m_kart->getSmoothedXYZ().toIrrVector()
                                       +  core::vector3df(0, above_kart, 0));
        m_camera->setTarget(current_target);
    }
    else // no kart animation
    {
        float above_kart, cam_angle, side_way, distance, cam_roll_angle;
        bool  smoothing;
        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance, &smoothing, &cam_roll_angle);
        positionCamera(dt, above_kart, cam_angle, side_way, distance, smoothing, cam_roll_angle);
    }
}   // update


// ----------------------------------------------------------------------------
/** Actually sets the camera based on the given parameter.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
 *  \param cam_roll_angle Roll camera for gyroscope steering effect.
*/
void CameraNormal::positionCamera(float dt, float above_kart, float cam_angle,
                           float side_way, float distance, float smoothing,
                           float cam_roll_angle)
{
    Vec3 wanted_position;
    Vec3 wanted_target = m_kart->getSmoothedTrans()(Vec3(0, above_kart, 0));

    float tan_up = tanf(cam_angle);

    Camera::Mode mode = getMode();
    if (UserConfigParams::m_reverse_look_use_soccer_cam && getMode() == CM_REVERSE) mode=CM_SPECTATOR_SOCCER;

    switch(mode)
    {
    case CM_SPECTATOR_SOCCER:
        {
            SoccerWorld *soccer_world = dynamic_cast<SoccerWorld*> (World::getWorld());
            if (soccer_world)
            {
                Vec3 ball_pos = soccer_world->getBallPosition();
                Vec3 to_target=(ball_pos-wanted_target);
                wanted_position = wanted_target + Vec3(0,  fabsf(distance)*tan_up+above_kart, 0) + (to_target.normalize() * distance * (getMode() == CM_REVERSE ? -1:1));
                m_camera->setPosition(wanted_position.toIrrVector());
                m_camera->setTarget(wanted_target.toIrrVector());
                return;
            }
            break;
        }
    case CM_SPECTATOR_TOP_VIEW:
        {
            SoccerWorld *soccer_world = dynamic_cast<SoccerWorld*> (World::getWorld());
            if (soccer_world) wanted_target = soccer_world->getBallPosition();
            wanted_position = wanted_target + Vec3(0,  distance+above_kart, 0);
            m_camera->setPosition(wanted_position.toIrrVector());
            m_camera->setTarget(wanted_target.toIrrVector());
            return;
        }
    default: break;
    }

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
    wanted_position = t(relative_position);

    if (smoothing)
    {
        moveCamera(dt, true, cam_angle, distance);
    }
    else
    {
        if (getMode()!=CM_FALLING)
            m_camera->setPosition(wanted_position.toIrrVector());
        m_camera->setTarget(wanted_target.toIrrVector());

        if (RaceManager::get()->getNumLocalPlayers() < 2)
        {
            SFXManager::get()->positionListener(m_camera->getPosition(),
                                      wanted_target - m_camera->getPosition(),
                                      Vec3(0, 1, 0));
        }
    }

    Kart *kart = dynamic_cast<Kart*>(m_kart);

    // Rotate the up vector (0,1,0) by the rotation ... which is just column 1
    const Vec3& up = m_kart->getSmoothedTrans().getBasis().getColumn(1);
    const irr::core::vector3df straight_up = irr::core::vector3df(0, 1, 0);

    if (kart && !kart->isFlying())
    {
        float f = 0.04f;  // weight for new up vector to reduce shaking
        m_camera->setUpVector(        f  * up.toIrrVector() +
                              (1.0f - f) * m_camera->getUpVector());
    }   // kart && !flying
    else
        m_camera->setUpVector(straight_up);

    if (cam_roll_angle != 0.0f)
    {
        btQuaternion q(m_kart->getSmoothedTrans().getBasis().getColumn(2),
            -cam_roll_angle);
        q *= m_kart->getSmoothedTrans().getRotation();
        btMatrix3x3 m(q);
        m_camera->setUpVector(((Vec3)m.getColumn(1)).toIrrVector());
    }
}   // positionCamera
