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

#include "graphics/camera.hpp"

#include "audio/sound_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/coord.hpp"

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode     = CM_NORMAL;
    m_index    = camera_index;
    m_camera   = irr_driver->addCameraSceneNode();
    setupCamera();
    m_distance = kart->getKartProperties()->getCameraDistance() * 0.5f;
    m_kart     = kart;
    m_ambient_light = World::getWorld()->getTrack()->getDefaultAmbientColor();

    // TODO: Put these values into a config file
    //       Global or per split screen zone?
    //       Either global or per user (for instance, some users may not like 
    //       the extra camera rotation so they could set m_rotation_range to 
    //       zero to disable it for themselves). 
    m_position_speed = 8.0f;
    m_target_speed   = 10.0f;
    m_rotation_range = 0.4f;

}   // Camera

// ----------------------------------------------------------------------------
/** Removes the camera scene node from the scene. 
 */
Camera::~Camera()
{
    irr_driver->removeCameraSceneNode(m_camera);
}   // ~Camera

//-----------------------------------------------------------------------------
/** Sets up the viewport, aspect ratio, field of view, and scaling for this
 *  camera.
 */
void Camera::setupCamera()
{
    m_aspect = (float)(UserConfigParams::m_width)/UserConfigParams::m_height;
    switch(race_manager->getNumLocalPlayers())
    {
    case 1: m_viewport = core::recti(0, 0, 
                                     UserConfigParams::m_width, 
                                     UserConfigParams::m_height);
            m_scaling  = core::vector2df(1.0f, 1.0f);
            m_fov      = DEGREE_TO_RAD*75.0f;
            break;
    case 2: m_viewport = core::recti(0, 
                                     m_index==0 ? 0 
                                                : UserConfigParams::m_height>>1,
                                     UserConfigParams::m_width, 
                                     m_index==0 ? UserConfigParams::m_height>>1
                                                : UserConfigParams::m_height);
            m_scaling  = core::vector2df(1.0f, 0.5f);
            m_aspect  *= 2.0f;
            m_fov      = DEGREE_TO_RAD*65.0f;
            break;
    case 3:
            /*
            if(m_index<2)
            {
                m_viewport = core::recti(m_index==0 ? 0 
                                                    : UserConfigParams::m_width>>1,
                                         0, 
                                         m_index==0 ? UserConfigParams::m_width>>1 
                                                    : UserConfigParams::m_width, 
                                         UserConfigParams::m_height>>1);
                m_scaling  = core::vector2df(0.5f, 0.5f);
                m_fov      = DEGREE_TO_RAD*50.0f;
            }
            else
            {
                m_viewport = core::recti(0, UserConfigParams::m_height>>1, 
                                         UserConfigParams::m_width, 
                                         UserConfigParams::m_height);
                m_scaling  = core::vector2df(1.0f, 0.5f);
                m_fov      = DEGREE_TO_RAD*65.0f;
                m_aspect  *= 2.0f;
            }
            break;*/
    case 4:
            { // g++ 4.3 whines about the variables in switch/case if not {}-wrapped (???)
            const int x1 = (m_index%2==0 ? 0 : UserConfigParams::m_width>>1);
            const int y1 = (m_index<2    ? 0 : UserConfigParams::m_height>>1);
            const int x2 = (m_index%2==0 ? UserConfigParams::m_width>>1  : UserConfigParams::m_width);
            const int y2 = (m_index<2    ? UserConfigParams::m_height>>1 : UserConfigParams::m_height);
            m_viewport = core::recti(x1, y1, x2, y2);
            std::cout << "Viewport : " << m_viewport.UpperLeftCorner.X << ", " << m_viewport.UpperLeftCorner.Y << "; size : "
                      << m_viewport.getWidth() << "x" << m_viewport.getHeight() << "\n";
            m_scaling  = core::vector2df(0.5f, 0.5f);
            m_fov      = DEGREE_TO_RAD*50.0f;
            }
            break;
    default:fprintf(stderr, "Incorrect number of players: '%d' - assuming 1.\n",
                    race_manager->getNumLocalPlayers());
            m_viewport = core::recti(0, 0, 
                                     UserConfigParams::m_width, 
                                     UserConfigParams::m_height);
            m_scaling  = core::vector2df(1.0f, 1.0f);
            m_fov      = DEGREE_TO_RAD*75.0f;
            break;
    }   // switch
    m_camera->setFOV(m_fov);
    m_camera->setAspectRatio(m_aspect);
}   // setupCamera

// ----------------------------------------------------------------------------
/** Sets the mode of the camera.
 *  \param mode Mode the camera should be switched to.
 */
void Camera::setMode(Mode mode)
{
    World *world = World::getWorld();
    // If we switch from reverse view, move the camera immediately to the 
    // correct position. 
    if(m_mode==CM_REVERSE && mode==CM_NORMAL)
    {
        Vec3 wanted_position, wanted_target;
        computeNormalCameraPosition(&wanted_position, &wanted_target);
        m_camera->setPosition(wanted_position.toIrrVector());
        m_camera->setTarget(wanted_target.toIrrVector());
    }
    if(mode==CM_FINAL)
    {
        const Track* track       = world->getTrack();
        core::vector3df wanted_position(track->getCameraPosition().toIrrVector());
        core::vector3df curr_position(m_camera->getPosition());
        m_lin_velocity           = (wanted_position-curr_position)
                                 / stk_config->m_final_camera_time;
        float distance           = (m_kart->getXYZ() - curr_position).length();
        core::vector3df hpr      = track->getCameraHPR().toIrrHPR();
        core::vector3df target   = hpr.rotationToDirection(core::vector3df(0, 0, 1)*distance)
                                 + wanted_position;
        core::vector3df curr_hpr = m_camera->getRotation();
        m_target_velocity        = (target - m_camera->getTarget())
                                 / stk_config->m_final_camera_time;
        m_final_time         = 0.0f;
     }

    // If the camera is set to final mode but there is no camera
    // end position defined, ignore this request and leave the camera
    // in normal mode.
    if(mode!=CM_FINAL || world->getTrack()->hasFinalCamera())
    {
        m_mode = mode;
    }
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
    setMode(CM_NORMAL);
}   // reset

//-----------------------------------------------------------------------------
/** Saves the current kart position as initial starting position for the
 *  camera.
 */
void Camera::setInitialTransform()
{
    m_camera->setPosition( m_kart->getXYZ().toIrrVector() - core::vector3df(0, -25, 50) );
}   // setInitialTransform

//-----------------------------------------------------------------------------
/** Moves the camera smoothly from the current camera position (and target)
 *  to the new position and target.
 *  \param wanted_position The position the camera wanted to reach.
 *  \param wanted_target The point the camera wants to point to.
 */
void Camera::smoothMoveCamera(float dt, const Vec3 &wanted_position,
                              const Vec3 &wanted_target)
{
    // Smoothly interpolate towards the position and target
    core::vector3df current_position = m_camera->getPosition();
    core::vector3df current_target   = m_camera->getTarget();
    current_target   += ((wanted_target.toIrrVector()   - current_target  ) * m_target_speed  ) * dt;
    current_position += ((wanted_position.toIrrVector() - current_position) * m_position_speed) * dt;
    m_camera->setPosition(current_position);
    m_camera->setTarget(current_target);

    if(race_manager->getNumLocalPlayers() < 2)
        sound_manager->positionListener(current_position, 
                                        current_target - current_position);

    // The following settings give a debug camera which shows the track from
    // high above the kart straight down.
#undef DEBUG_CAMERA
#ifdef DEBUG_CAMERA
    core::vector3df xyz = RaceManager::getKart(0)->getXYZ().toIrrVector();
    m_camera->setTarget(xyz);
    xyz.Y = xyz.Y+30;
    m_camera->setPosition(xyz);
#endif
}   // smoothMoveCamera

//-----------------------------------------------------------------------------
/** Computes the wanted camera position and target for normal camera mode.
 *  Besides being used in update(dt), it is also used when switching the
 *  camera from reverse mode to normal mode - in which case we don't want
 *  to have a smooth camera.
 *  \param wanted_position The position the camera should be.
 *  \param wanted_target The target position the camera should target.
 */
void Camera::computeNormalCameraPosition(Vec3 *wanted_position,
                                         Vec3 *wanted_target)
{
    *wanted_target = m_kart->getXYZ();
    wanted_target->setZ(wanted_target->getZ()+ 0.75f);
    // This first line moves the camera around behind the kart, pointing it 
    // towards where the kart is turning (and turning even more while skidding).
    float steering = m_kart->getSteerPercent() * (1.0f + (m_kart->getSkidding() - 1.0f)/2.3f ); // dampen skidding effect
    float dampened_steer =  fabsf(steering) * steering; // quadratically to dampen small variations (but keep sign)
    float angle_around = m_kart->getHPR().getX() + m_rotation_range * dampened_steer * 0.5f;
    float angle_up     = m_kart->getHPR().getY() - 30.0f*DEGREE_TO_RAD;      
    wanted_position->setX( sin(angle_around));
    wanted_position->setY(-cos(angle_around));
    wanted_position->setZ(-sin(angle_up)    );
    *wanted_position *= m_distance;
    *wanted_position += *wanted_target;
}   // computeNormalCameraPosition

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void Camera::update(float dt)
{
    Vec3 wanted_position;
    Vec3 wanted_target = m_kart->getXYZ();

    // Each case should set wanted_position and wanted_target according to 
    // what is needed for that mode. Yes, there is a lot of duplicate code 
    // but it is (IMHO) much easier to follow this way.
    switch(m_mode)
    {
    case CM_NORMAL:
        {
            computeNormalCameraPosition(&wanted_position, &wanted_target);
            smoothMoveCamera(dt, wanted_position, wanted_target);
            break;
        }
    case CM_REVERSE: // Same as CM_NORMAL except it looks backwards
        {
            wanted_target.setZ(wanted_target.getZ()+ 0.75f);
            float angle_around = m_kart->getHPR().getX() - m_rotation_range * m_kart->getSteerPercent() * m_kart->getSkidding();
            float angle_up     = m_kart->getHPR().getY() + 30.0f*DEGREE_TO_RAD;
            wanted_position.setX(-sin(angle_around));
            wanted_position.setY( cos(angle_around));
            wanted_position.setZ( sin(angle_up)    );
            wanted_position *= m_distance * 2.0f;
            wanted_position += wanted_target;
            smoothMoveCamera(dt, wanted_position, wanted_target);
            m_camera->setPosition(wanted_position.toIrrVector());
            m_camera->setTarget(wanted_target.toIrrVector());
            break;
        }
    case CM_CLOSEUP: // Lower to the ground and closer to the kart
        {
            wanted_target.setZ(wanted_target.getZ()+0.75f);
            float angle_around = m_kart->getHPR().getX() + m_rotation_range * m_kart->getSteerPercent() * m_kart->getSkidding();
            float angle_up     = m_kart->getHPR().getY() - 20.0f*DEGREE_TO_RAD;
            wanted_position.setX( sin(angle_around));
            wanted_position.setY(-cos(angle_around));
            wanted_position.setZ(-sin(angle_up)    );
            wanted_position *= m_distance * 0.5f;
            wanted_position += wanted_target;
            smoothMoveCamera(dt, wanted_position, wanted_target);
            break;
        }
    case CM_LEADER_MODE:
        {
            World *world  = World::getWorld();
            Kart *kart    = world->getKart(0);
            wanted_target = kart->getXYZ().toIrrVector();
            // Follows the leader kart, higher off of the ground, further from the kart,
            // and turns in the opposite direction from the kart for a nice effect. :)
            float angle_around = kart->getHPR().getX();
            float angle_up     = kart->getHPR().getY() + 40.0f*DEGREE_TO_RAD;
            wanted_position.setX(sin(angle_around));
            wanted_position.setY(cos(angle_around));
            wanted_position.setZ(sin(angle_up)    );
            wanted_position *= m_distance * 2.0f;
            wanted_position += wanted_target;
            smoothMoveCamera(dt, wanted_position, wanted_target);
            break;
        }
    case CM_FINAL:
        {
            m_final_time +=dt;
            if(m_final_time < stk_config->m_final_camera_time)
            {
                core::vector3df new_pos = m_camera->getPosition()+m_lin_velocity*dt;
                m_camera->setPosition(new_pos);
                core::vector3df new_target = m_camera->getTarget()+m_target_velocity*dt;
                m_camera->setTarget(new_target);
            }
            return;
            break;
        }
    case CM_SIMPLE_REPLAY:
        // TODO: Implement
        break;
    }

}   // update

// ----------------------------------------------------------------------------
/** Sets viewport etc. for this camera. Called from irr_driver just before
 *  rendering the view for this kart.
 */
void Camera::activate()
{
    irr::scene::ISceneManager *sm = irr_driver->getSceneManager();
    sm->setActiveCamera(m_camera);
    sm->setAmbientLight(m_ambient_light);
    irr_driver->getVideoDriver()->setViewPort(m_viewport);

}   // activate
