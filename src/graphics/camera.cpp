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

#include "graphics/camera.hpp"

#include <cmath>

#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "physics/btKart.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/aligned_array.hpp"
#include "utils/constants.hpp"
#include "utils/vs.hpp"

#include "ICameraSceneNode.h"
#include "ISceneManager.h"

AlignedArray<Camera::EndCameraInformation> Camera::m_end_cameras;
std::vector<Camera*>                       Camera::m_all_cameras;

Camera* Camera::s_active_camera = NULL;

// ============================================================================
Camera::Camera(int camera_index, AbstractKart* kart) : m_kart(NULL)
{
    m_smooth        = false;
    m_attached      = false;
    m_mode          = CM_NORMAL;
    m_index         = camera_index;
    m_original_kart = kart;
    m_camera        = irr_driver->addCameraSceneNode();
    m_previous_pv_matrix = core::matrix4();

#ifdef DEBUG
    if (kart != NULL)
        m_camera->setName(core::stringc("Camera for ") + kart->getKartProperties()->getName());
    else
        m_camera->setName("Camera");
#endif

    setupCamera();
    if (kart != NULL)
    {
        m_distance = kart->getKartProperties()->getCameraDistance();
        setKart(kart);
    }
    else
    {
        m_distance = 1000.0f;
    }
    m_ambient_light = World::getWorld()->getTrack()->getDefaultAmbientColor();

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
Camera::~Camera()
{
    irr_driver->removeCameraSceneNode(m_camera);

    if (s_active_camera == this)
        s_active_camera = NULL;
}   // ~Camera

//-----------------------------------------------------------------------------
/** Applies mouse movement to the first person camera.
 *  \param x The horizontal difference of the mouse position.
 *  \param y The vertical difference of the mouse position.
 */
void Camera::applyMouseMovement (float x, float y)
{
    vector3df direction(m_target_direction);
    vector3df up(m_camera->getUpVector());

    // Set local values if the camera is attached to the kart
    if (m_attached)
        up = m_local_up;

    direction.normalize();
    up.normalize();

    vector3df side(direction.crossProduct(up));
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
}

//-----------------------------------------------------------------------------
/** Changes the owner of this camera to the new kart.
 *  \param new_kart The new kart to use this camera.
 */
void Camera::setKart(AbstractKart *new_kart)
{
    m_kart = new_kart;
#ifdef DEBUG
    if(new_kart)
    {
        std::string name = new_kart->getIdent()+"'s camera";
        getCameraSceneNode()->setName(name.c_str() );
    }
#endif

}   // setKart

//-----------------------------------------------------------------------------
/** This function clears all end camera data structure. This is necessary
 *  since all end cameras are shared between all camera instances (i.e. are
 *  static), otherwise (if no end camera is defined for a track) the old
 *  end camera structure would be used.
 */
void Camera::clearEndCameras()
{
    m_end_cameras.clear();
}   // clearEndCameras

//-----------------------------------------------------------------------------
/** Reads the information about the end camera. This information is shared
 *  between all cameras, so this is a static function.
 *  \param node The XML node containing all end camera informations
 */
void Camera::readEndCamera(const XMLNode &root)
{
    m_end_cameras.clear();
    for(unsigned int i=0; i<root.getNumNodes(); i++)
    {
        unsigned int index = i;
        // In reverse mode, reverse the order in which the
        // end cameras are read.
        if(QuadGraph::get()->isReverse())
            index = root.getNumNodes() - 1 - i;
        const XMLNode *node = root.getNode(index);
        EndCameraInformation eci;
        if(!eci.readXML(*node)) continue;
        m_end_cameras.push_back(eci);
    }   // for i<getNumNodes()
}   // readEndCamera

//-----------------------------------------------------------------------------
/** Sets up the viewport, aspect ratio, field of view, and scaling for this
 *  camera.
 */
void Camera::setupCamera()
{
    m_aspect = (float)(irr_driver->getActualScreenSize().Width)/irr_driver->getActualScreenSize().Height;
    switch(race_manager->getNumLocalPlayers())
    {
    case 1: m_viewport = core::recti(0, 0,
                                     irr_driver->getActualScreenSize().Width,
                                     irr_driver->getActualScreenSize().Height);
            m_scaling  = core::vector2df(1.0f, 1.0f);
            m_fov      = DEGREE_TO_RAD*stk_config->m_camera_fov[0];
            break;
    case 2: m_viewport = core::recti(0,
                                     m_index==0 ? 0
                                                : irr_driver->getActualScreenSize().Height>>1,
                                     irr_driver->getActualScreenSize().Width,
                                     m_index==0 ? irr_driver->getActualScreenSize().Height>>1
                                                : irr_driver->getActualScreenSize().Height);
            m_scaling  = core::vector2df(1.0f, 0.5f);
            m_aspect  *= 2.0f;
            m_fov      = DEGREE_TO_RAD*stk_config->m_camera_fov[1];
            break;
    case 3:
            /*
            if(m_index<2)
            {
                m_viewport = core::recti(m_index==0 ? 0
                                                    : irr_driver->getActualScreenSize().Width>>1,
                                         0,
                                         m_index==0 ? irr_driver->getActualScreenSize().Width>>1
                                                    : irr_driver->getActualScreenSize().Width,
                                         irr_driver->getActualScreenSize().Height>>1);
                m_scaling  = core::vector2df(0.5f, 0.5f);
                m_fov      = DEGREE_TO_RAD*50.0f;
            }
            else
            {
                m_viewport = core::recti(0, irr_driver->getActualScreenSize().Height>>1,
                                         irr_driver->getActualScreenSize().Width,
                                         irr_driver->getActualScreenSize().Height);
                m_scaling  = core::vector2df(1.0f, 0.5f);
                m_fov      = DEGREE_TO_RAD*65.0f;
                m_aspect  *= 2.0f;
            }
            break;*/
    case 4:
            { // g++ 4.3 whines about the variables in switch/case if not {}-wrapped (???)
            const int x1 = (m_index%2==0 ? 0 : irr_driver->getActualScreenSize().Width>>1);
            const int y1 = (m_index<2    ? 0 : irr_driver->getActualScreenSize().Height>>1);
            const int x2 = (m_index%2==0 ? irr_driver->getActualScreenSize().Width>>1  : irr_driver->getActualScreenSize().Width);
            const int y2 = (m_index<2    ? irr_driver->getActualScreenSize().Height>>1 : irr_driver->getActualScreenSize().Height);
            m_viewport = core::recti(x1, y1, x2, y2);
            m_scaling  = core::vector2df(0.5f, 0.5f);
            m_fov      = DEGREE_TO_RAD*stk_config->m_camera_fov[3];
            }
            break;
    default:
            if(UserConfigParams::logMisc())
                Log::warn("Camera", "Incorrect number of players: '%d' - assuming 1.",
                          race_manager->getNumLocalPlayers());
            m_viewport = core::recti(0, 0,
                                     irr_driver->getActualScreenSize().Width,
                                     irr_driver->getActualScreenSize().Height);
            m_scaling  = core::vector2df(1.0f, 1.0f);
            m_fov      = DEGREE_TO_RAD*75.0f;
            break;
    }   // switch
    m_camera->setFOV(m_fov);
    m_camera->setAspectRatio(m_aspect);
    m_camera->setFarValue(World::getWorld()->getTrack()->getCameraFar());
}   // setupCamera

// ----------------------------------------------------------------------------
/** Sets the mode of the camera.
 *  \param mode Mode the camera should be switched to.
 */
void Camera::setMode(Mode mode)
{
    // If we switch from reverse view, move the camera immediately to the
    // correct position.
    if((m_mode==CM_REVERSE && mode==CM_NORMAL) || (m_mode==CM_FALLING && mode==CM_NORMAL))
    {
        Vec3 start_offset(0, 1.6f, -3);
        Vec3 current_position = m_kart->getTrans()(start_offset);
        m_camera->setPosition(  current_position.toIrrVector());
        m_camera->setTarget(m_camera->getPosition());
    }
    if(mode==CM_FINAL)
    {
        if(m_end_cameras.size()>0)
            m_camera->setPosition(m_end_cameras[0].m_position.toIrrVector());
        m_next_end_camera    = m_end_cameras.size()>1 ? 1 : 0;
        m_current_end_camera = 0;
        m_camera->setFOV(m_fov);
        handleEndCamera(0);
    }   // mode==CM_FINAL

    m_mode = mode;
}   // setMode

// ----------------------------------------------------------------------------
/** Returns the current mode of the camera.
 */
Camera::Mode Camera::getMode()
{
    return m_mode;
}

//-----------------------------------------------------------------------------
/** Reset is called when a new race starts. Make sure that the camera
    is aligned neutral, and not like in the previous race
*/
void Camera::reset()
{
    m_kart = m_original_kart;
    setMode(CM_NORMAL);

    if (m_kart != NULL)
        setInitialTransform();
}   // reset

//-----------------------------------------------------------------------------
/** Saves the current kart position as initial starting position for the
 *  camera.
 */
void Camera::setInitialTransform()
{
    if (m_kart == NULL) return;
    Vec3 start_offset(0, 1.6f, -3);
    Vec3 current_position = m_kart->getTrans()(start_offset);
    m_camera->setPosition(  current_position.toIrrVector());
    // Reset the target from the previous target (in case of a restart
    // of a race) - otherwise the camera will initially point in the wrong
    // direction till smoothMoveCamera has corrected this. Setting target
    // to position doesn't make sense, but smoothMoves will adjust the
    // value before the first frame is rendered
    m_camera->setTarget(m_camera->getPosition());
    m_camera->setRotation(core::vector3df(0, 0, 0));
    m_camera->setRotation( core::vector3df( 0.0f, 0.0f, 0.0f ) );
    m_camera->setFOV(m_fov);

    assert(!std::isnan(m_camera->getPosition().X));
    assert(!std::isnan(m_camera->getPosition().Y));
    assert(!std::isnan(m_camera->getPosition().Z));
}   // setInitialTransform

//-----------------------------------------------------------------------------
/** Moves the camera smoothly from the current camera position (and target)
 *  to the new position and target.
 *  \param wanted_position The position the camera wanted to reach.
 *  \param wanted_target The point the camera wants to point to.
 */
void Camera::smoothMoveCamera(float dt)
{
    Kart *kart = dynamic_cast<Kart*>(m_kart);
    if (kart->isFlying())
    {
        Vec3 vec3 = m_kart->getXYZ() + Vec3(sin(m_kart->getHeading()) * -4.0f, 0.5f, cos(m_kart->getHeading()) * -4.0f);
        m_camera->setTarget(m_kart->getXYZ().toIrrVector());
        m_camera->setPosition(vec3.toIrrVector());
        return;
    }


    core::vector3df current_position  =  m_camera->getPosition();
    // Smoothly interpolate towards the position and target
    const KartProperties *kp = m_kart->getKartProperties();
    float max_increase_with_zipper = kp->getZipperMaxSpeedIncrease();
    float max_speed_without_zipper = kp->getEngineMaxSpeed();
    float current_speed = m_kart->getSpeed();

    const Skidding *ks = m_kart->getSkidding();
    float skid_factor = ks->getVisualSkidRotation();

    float skid_angle = asin(skid_factor);
    float ratio = (current_speed - max_speed_without_zipper) / max_increase_with_zipper;
    ratio = ratio > -0.12f ? ratio : -0.12f;
    float camera_distance = -3 * (0.5f + ratio);// distance of camera from kart in x and z plane
    if (camera_distance > -2.0f) camera_distance = -2.0f;
    Vec3 camera_offset(camera_distance * sin(skid_angle / 2),
                       1.1f * (1 + ratio / 2),
                       camera_distance * cos(skid_angle / 2));// defines how far camera should be from player kart.
    Vec3 m_kart_camera_position_with_offset = m_kart->getTrans()(camera_offset);



    core::vector3df current_target = m_kart->getXYZ().toIrrVector();// next target
    current_target.Y += 0.5f;
    core::vector3df wanted_position = m_kart_camera_position_with_offset.toIrrVector();// new required position of camera

    if ((m_kart->getSpeed() > 5 ) || (m_kart->getSpeed() < 0 ))
    {
        current_position += ((wanted_position - current_position) * dt
                          * (m_kart->getSpeed()>0 ? m_kart->getSpeed()/3 + 1.0f
                                                 : -1.5f * m_kart->getSpeed() + 2.0f));
    }
    else
    {
        current_position += (wanted_position - current_position) * dt * 5;
    }

    // Avoid camera crash: if the speed is negative, the current_position
    // can oscillate between plus and minus, getting bigger and bigger. If
    // this happens often enough, floating point overflow happens (large
    // negative speeds can happen when the kart is tumbling/falling)
    // To avoid this, we just move the camera to the wanted position if
    // the distance becomes too large (see #1356).
    if( (current_position - wanted_position).getLengthSQ() > 100)
    {
        Log::debug("camera", "Resetting camera position to avoid crash");
        current_position = wanted_position;
    }

    if(m_mode!=CM_FALLING)
        m_camera->setPosition(current_position);
    m_camera->setTarget(current_target);//set new target

    assert(!std::isnan(m_camera->getPosition().X));
    assert(!std::isnan(m_camera->getPosition().Y));
    assert(!std::isnan(m_camera->getPosition().Z));

}   // smoothMoveCamera

//-----------------------------------------------------------------------------
/** Determine the camera settings for the current frame.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
 */
void Camera::getCameraSettings(float *above_kart, float *cam_angle,
                               float *sideway, float *distance,
                               bool *smoothing)
{
    const KartProperties *kp = m_kart->getKartProperties();

    switch(m_mode)
    {
    case CM_NORMAL:
    case CM_FALLING:
        {
            if(UserConfigParams::m_camera_debug==2)
            {
                *above_kart = 0;
                *cam_angle  = 0;
                *distance   = -m_kart->getKartModel()->getLength()-1.0f;
            }
            else if(UserConfigParams::m_camera_debug==4)
            {
                *above_kart = 0;
                *cam_angle  = 0;
                *distance   = -0.5f*m_kart->getKartModel()->getLength()-1.0f;
                *smoothing  = false;
            }
            else
            {
                *above_kart    = 0.75f;
                *cam_angle     = kp->getCameraForwardUpAngle() * DEGREE_TO_RAD;
                *distance      = -m_distance;
            }
            float steering = m_kart->getSteerPercent()
                           * (1.0f + (m_kart->getSkidding()->getSkidFactor()
                                      - 1.0f)/2.3f );
            // quadratically to dampen small variations (but keep sign)
            float dampened_steer = fabsf(steering) * steering;
            *sideway             = -m_rotation_range*dampened_steer*0.5f;
            *smoothing           = true;
            break;
        }   // CM_FALLING
    case CM_REVERSE: // Same as CM_NORMAL except it looks backwards
        {
            *above_kart = 0.75f;
            *cam_angle  = kp->getCameraBackwardUpAngle() * DEGREE_TO_RAD;
            *sideway    = 0;
            *distance   = 2.0f*m_distance;
            *smoothing  = false;
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
            break;
        }
    case CM_LEADER_MODE:
        {
            *above_kart = 0.0f;
            *cam_angle  = 40*DEGREE_TO_RAD;
            *sideway    = 0;
            *distance   = 2.0f*m_distance;
            *smoothing  = true;
            break;
        }
    case CM_FINAL:
    case CM_SIMPLE_REPLAY:
        // TODO: Implement
        break;
    }

}   // getCameraSettings

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void Camera::update(float dt)
{
    if (m_kart == NULL)
    {
        if (race_manager->getNumLocalPlayers() < 2)
        {
            Vec3 pos(m_camera->getPosition());
            SFXManager::get()->positionListener(pos,
                Vec3(m_camera->getTarget()) - pos,
                Vec3(0, 1, 0));
        }

        return; // cameras not attached to kart must be positioned manually
    }

    if (race_manager->getNumLocalPlayers() < 2)
    {
        Vec3 heading(sin(m_kart->getHeading()), 0.0f, cos(m_kart->getHeading()));
        SFXManager::get()->positionListener(m_kart->getXYZ(),
            heading,
            Vec3(0, 1, 0));
    }
    
    float above_kart, cam_angle, side_way, distance;
    bool  smoothing;

    // The following settings give a debug camera which shows the track from
    // high above the kart straight down.
    if (UserConfigParams::m_camera_debug==1)
    {
        core::vector3df xyz = m_kart->getXYZ().toIrrVector();
        m_camera->setTarget(xyz);
        xyz.Y = xyz.Y+55;
        xyz.Z -= 5.0f;
        m_camera->setPosition(xyz);
        // To view inside tunnels (FIXME 27>15 why??? makes no sense
        // - the kart should not be visible, but it works)
        m_camera->setNearValue(27.0);
    }
    else if (UserConfigParams::m_camera_debug==5)
    {
        core::vector3df xyz = m_kart->getXYZ().toIrrVector();
        Vec3 offset(3, 0, 0);
        offset = m_kart->getTrans()(offset);
        m_camera->setTarget(xyz);
        m_camera->setPosition(offset.toIrrVector());
    }
    // Update the first person camera
    else if (UserConfigParams::m_camera_debug == 3)
    {
        vector3df direction(m_camera->getTarget() - m_camera->getPosition());
        vector3df up(m_camera->getUpVector());
        vector3df side(direction.crossProduct(up));
        vector3df pos = m_camera->getPosition();

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
                if (diff.getLengthSQ() > 1)
                    diff.setLength(1);
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
                    m_direction_velocity.setLength(
                        UserConfigParams::m_fpscam_smooth_direction_max_speed);
                direction += m_direction_velocity;
                m_target_direction = direction;
            }

            // Camera rotation
            diff = m_target_up_vector - up;
            if (diff.X != 0 || diff.Y != 0 || diff.Z != 0)
            {
                if (diff.getLengthSQ() >
                    UserConfigParams::m_fpscam_angular_velocity *
                    UserConfigParams::m_fpscam_angular_velocity)
                    diff.setLength(UserConfigParams::m_fpscam_angular_velocity);
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
        vector3df top(side.crossProduct(direction));

        // Move camera
        vector3df movement(direction * m_lin_velocity.Z +
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
    }
    else if (m_mode==CM_FINAL)
    {
        handleEndCamera(dt);
    }
    // If an explosion is happening, stop moving the camera,
    // but keep it target on the kart.
    else if (dynamic_cast<ExplosionAnimation*>(m_kart->getKartAnimation()))
    {
        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance, &smoothing);
        // The camera target needs to be 'smooth moved', otherwise
        // there will be a noticable jump in the first frame

        // Aim at the usual same position of the kart (i.e. slightly
        // above the kart).
        // Note: this code is replicated from smoothMoveCamera so that
        // the camera keeps on pointing to the same spot.
        core::vector3df current_target = (m_kart->getXYZ().toIrrVector()+core::vector3df(0, above_kart, 0));
        m_camera->setTarget(current_target);
    }
    else
    {
        getCameraSettings(&above_kart, &cam_angle, &side_way, &distance, &smoothing);
        positionCamera(dt, above_kart, cam_angle, side_way, distance, smoothing);
    }
}   // update

// ----------------------------------------------------------------------------
/** Actually sets the camera based on the given parameter.
 *  \param above_kart How far above the camera should aim at.
 *  \param cam_angle  Angle above the kart plane for the camera.
 *  \param sideway Sideway movement of the camera.
 *  \param distance Distance from kart.
*/
void Camera::positionCamera(float dt, float above_kart, float cam_angle,
                           float side_way, float distance, float smoothing)
{
    Vec3 wanted_position;
    Vec3 wanted_target = m_kart->getXYZ();
    if(UserConfigParams::m_camera_debug==2)
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
    btTransform t=m_kart->getTrans();
    if(stk_config->m_camera_follow_skid &&
        m_kart->getSkidding()->getVisualSkidRotation()!=0)
    {
        // If the camera should follow the graphical skid, add the
        // visual rotation to the relative vector:
        btQuaternion q(m_kart->getSkidding()->getVisualSkidRotation(), 0, 0);
        t.setBasis(t.getBasis() * btMatrix3x3(q));
    }
    if (UserConfigParams::m_camera_debug == 2)
    {
        wanted_position = t(relative_position);
        // Make sure that the Y position is a the same height as the wheel.
        wanted_position.setY(wanted_target.getY());
    }
    else
        wanted_position = t(relative_position);

    if (smoothing && UserConfigParams::m_camera_debug==0)
    {
        smoothMoveCamera(dt);
    }
    else
    {
        if (m_mode!=CM_FALLING)
            m_camera->setPosition(wanted_position.toIrrVector());
        m_camera->setTarget(wanted_target.toIrrVector());

        if (race_manager->getNumLocalPlayers() < 2)
        {
            SFXManager::get()->positionListener(m_camera->getPosition(),
                                      wanted_target - m_camera->getPosition(),
                                      Vec3(0, 1, 0));
        }
    }

    Kart *kart = dynamic_cast<Kart*>(m_kart);
    if (kart && !kart->isFlying())
    {
        // Rotate the up vector (0,1,0) by the rotation ... which is just column 1
        Vec3 up = m_kart->getTrans().getBasis().getColumn(1);
        float f = 0.04f;  // weight for new up vector to reduce shaking
        m_camera->setUpVector(        f  * up.toIrrVector() +
                              (1.0f - f) * m_camera->getUpVector());
    }   // kart && !flying
    else
        m_camera->setUpVector(core::vector3df(0, 1, 0));
}   // positionCamera

// ----------------------------------------------------------------------------
/** This function handles the end camera. It adjusts the camera position
 *  according to the current camera type, and checks if a switch to the
 *  next camera should be made.
 *  \param dt Time step size.
*/
void Camera::handleEndCamera(float dt)
{
    // First test if the kart is close enough to the next end camera, and
    // if so activate it.
    if( m_end_cameras.size()>0 &&
        m_end_cameras[m_next_end_camera].isReached(m_kart->getXYZ()))
    {
        m_current_end_camera = m_next_end_camera;
        if(m_end_cameras[m_current_end_camera].m_type
            ==EndCameraInformation::EC_STATIC_FOLLOW_KART)
        {
            m_camera->setPosition(
                m_end_cameras[m_current_end_camera].m_position.toIrrVector()
                );
        }
        m_camera->setFOV(m_fov);
        m_next_end_camera++;
        if(m_next_end_camera>=(unsigned)m_end_cameras.size())
            m_next_end_camera = 0;
    }

    EndCameraInformation::EndCameraType info
        = m_end_cameras.size()==0 ? EndCameraInformation::EC_AHEAD_OF_KART
                                  : m_end_cameras[m_current_end_camera].m_type;

    switch(info)
    {
    case EndCameraInformation::EC_STATIC_FOLLOW_KART:
        {
            // Since the camera has no parents, we can use the relative
            // position here (otherwise we need to call updateAbsolutePosition
            // after changing the relative position in order to get the right
            // position here).
            const core::vector3df &cp = m_camera->getPosition();
            const Vec3            &kp = m_kart->getXYZ();
            // Estimate the fov, assuming that the vector from the camera to
            // the kart and the kart length are orthogonal to each other
            // --> tan (fov) = kart_length / camera_kart_distance
            // In order to show a little bit of the surrounding of the kart
            // the kart length is multiplied by 6 (experimentally found)
            float fov = 6*atan2(m_kart->getKartLength(),
                                (cp-kp.toIrrVector()).getLength());
            m_camera->setFOV(fov);
            m_camera->setTarget(m_kart->getXYZ().toIrrVector());
            break;
        }
    case EndCameraInformation::EC_AHEAD_OF_KART:
        {
            float cam_angle = m_kart->getKartProperties()->getCameraBackwardUpAngle() * DEGREE_TO_RAD;

            positionCamera(dt, /*above_kart*/0.75f,
                           cam_angle, /*side_way*/0,
                           2.0f*m_distance, /*smoothing*/false);
            break;
        }
    default: break;
    }   // switch

}   // handleEndCamera

// ----------------------------------------------------------------------------
/** Sets viewport etc. for this camera. Called from irr_driver just before
 *  rendering the view for this kart.
 */
void Camera::activate(bool alsoActivateInIrrlicht)
{
    s_active_camera = this;
    if (alsoActivateInIrrlicht)
    {
        irr::scene::ISceneManager *sm = irr_driver->getSceneManager();
        sm->setActiveCamera(m_camera);
        irr_driver->getVideoDriver()->setViewPort(m_viewport);
    }
}   // activate

// ----------------------------------------------------------------------------
/** Sets the angular velocity for this camera. */
void Camera::setAngularVelocity(float vel)
{
    if (m_smooth)
        m_target_angular_velocity = vel;
    else
        m_angular_velocity = vel;
}   // setAngularVelocity

// ----------------------------------------------------------------------------
/** Returns the current target angular velocity. */
float Camera::getAngularVelocity()
{
    if (m_smooth)
        return m_target_angular_velocity;
    else
        return m_angular_velocity;
}   // getAngularVelocity

// ----------------------------------------------------------------------------
/** Sets the linear velocity for this camera. */
void Camera::setLinearVelocity(core::vector3df vel)
{
    if (m_smooth)
        m_target_velocity = vel;
    else
        m_lin_velocity = vel;
}   // setLinearVelocity

// ----------------------------------------------------------------------------
/** Returns the current linear velocity. */
const core::vector3df &Camera::getLinearVelocity()
{
    if (m_smooth)
        return m_target_velocity;
    else
        return m_lin_velocity;
}   // getLinearVelocity

