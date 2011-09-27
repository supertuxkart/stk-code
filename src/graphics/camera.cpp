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

#if defined(WIN32) && !defined(__CYGWIN__)
#  define isnan _isnan
#else
#  include <math.h>
#endif

#include "audio/music_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/xml_node.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/aligned_array.hpp"
#include "utils/constants.hpp"

#include <ICameraSceneNode.h>
#include <ISceneManager.h>

AlignedArray<Camera::EndCameraInformation> Camera::m_end_cameras;

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode     = CM_NORMAL;
    m_index    = camera_index;
    m_camera   = irr_driver->addCameraSceneNode();
    
#ifdef DEBUG
    m_camera->setName(core::stringc("Camera for ") + kart->getKartProperties()->getName());
#endif
    
    setupCamera();
    m_distance = kart->getKartProperties()->getCameraDistance();
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
    // TODO: Make this per user too if the one above goes that way.
    switch(UserConfigParams::m_camera_style)
    {
        case 1: m_camera_style = CS_CLASSIC; break;
        case 0:
        default: m_camera_style = CS_MODERN; break;
    }
    reset();
}   // Camera

// ----------------------------------------------------------------------------
/** Removes the camera scene node from the scene. 
 */
Camera::~Camera()
{
    irr_driver->removeCameraSceneNode(m_camera);
}   // ~Camera

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
        const XMLNode *node = root.getNode(i);
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
            m_scaling  = core::vector2df(0.5f, 0.5f);
            m_fov      = DEGREE_TO_RAD*50.0f;
            }
            break;
    default:
            if(UserConfigParams::logMisc())
                fprintf(stderr, "Incorrect number of players: '%d' - assuming 1.\n",
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
    if(m_mode==CM_REVERSE && mode==CM_NORMAL)
    {
        Vec3 wanted_position, wanted_target;
        computeNormalCameraPosition(&wanted_position, &wanted_target);
        m_camera->setPosition(wanted_position.toIrrVector());
        m_camera->setTarget(wanted_target.toIrrVector());

        assert(!isnan(m_camera->getPosition().X));
        assert(!isnan(m_camera->getPosition().Y));
        assert(!isnan(m_camera->getPosition().Z));

    }
    if(mode==CM_FINAL)
    {
        m_next_end_camera    = m_end_cameras.size()>1 ? 1 : 0;
        m_current_end_camera = 0;
        if(m_end_cameras.size()>0 && 
            m_end_cameras[0].m_type==EndCameraInformation::EC_STATIC_FOLLOW_KART)
        {
            m_camera->setPosition(m_end_cameras[0].m_position.toIrrVector());
            m_camera->setTarget(m_kart->getXYZ().toIrrVector());
        }
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
    setMode(CM_NORMAL);
    setInitialTransform();
}   // reset

//-----------------------------------------------------------------------------
/** Saves the current kart position as initial starting position for the
 *  camera.
 */
void Camera::setInitialTransform()
{
    m_camera->setPosition(  m_kart->getXYZ().toIrrVector() 
                          + core::vector3df(0, 25, -50)   );
    // Reset the target from the previous target (in case of a restart
    // of a race) - otherwise the camera will initially point in the wrong
    // direction till smoothMoveCamera has corrected this. Setting target
    // to position doesn't make sense, but smoothMoves will adjust the
    // value before the first frame is rendered
    m_camera->setTarget(m_camera->getPosition());
    m_camera->setRotation(core::vector3df(0, 0, 0));
    m_camera->setRotation( core::vector3df( 0.0f, 0.0f, 0.0f ) );
    m_camera->setFOV(m_fov);

    assert(!isnan(m_camera->getPosition().X));
    assert(!isnan(m_camera->getPosition().Y));
    assert(!isnan(m_camera->getPosition().Z));
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
    if(m_mode!=CM_FALLING)
        m_camera->setPosition(current_position);
    m_camera->setTarget(current_target);

    assert(!isnan(m_camera->getPosition().X));
    assert(!isnan(m_camera->getPosition().Y));
    assert(!isnan(m_camera->getPosition().Z));
    
    if (race_manager->getNumLocalPlayers() < 2)
    {
        sfx_manager->positionListener(current_position,  current_target - current_position);
    }
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
    wanted_target->setY(wanted_target->getY()+ 0.75f);
    
    // This first line moves the camera around behind the kart, pointing it 
    // towards where the kart is turning (and turning even more while skidding).
    // The skidding effect is dampened.
    float steering = m_kart->getSteerPercent() 
                   * (1.0f + (m_kart->getSkidding() - 1.0f)/2.3f );
    // quadratically to dampen small variations (but keep sign)
    float dampened_steer =  fabsf(steering) * steering; 

    float tan_up = tan(m_kart->getKartProperties()->getCameraForwardUpAngle());
    Vec3 relative_position(-m_distance*m_rotation_range*dampened_steer*0.5f,
                            m_distance*tan_up+0.75f,
                           -m_distance);
    *wanted_position = m_kart->getTrans()(relative_position);

}   // computeNormalCameraPosition

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
        if(m_camera_style==CS_CLASSIC)
        {
            *above_kart = 0.3f;
            *cam_angle  = kp->getCameraBackwardUpAngle();
            *sideway    = 0.0f;
            *distance   = -1.5f*m_distance;
            *smoothing  = true;
            break;
        }
        // Fall through to falling mode.
    case CM_FALLING:
        {
            *above_kart    = 0.75f;
            float steering = m_kart->getSteerPercent() 
                           * (1.0f + (m_kart->getSkidding() - 1.0f)/2.3f );
            // quadratically to dampen small variations (but keep sign)
            float dampened_steer = fabsf(steering) * steering; 
            *cam_angle           = kp->getCameraForwardUpAngle();
            *sideway             = -m_rotation_range*dampened_steer*0.5f;
            *distance            = -m_distance;
            *smoothing           = true;
            break;
        }   // CM_FALLING
    case CM_REVERSE: // Same as CM_NORMAL except it looks backwards
        {
            *above_kart = 0.75f;
            *cam_angle  = kp->getCameraBackwardUpAngle();
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
                        * m_kart->getSkidding();
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

}   // get CameraPosition

//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 *  \param dt Time step.
 */
void Camera::update(float dt)
{
    // The following settings give a debug camera which shows the track from
    // high above the kart straight down.
    if(UserConfigParams::m_camera_debug)
    {
        core::vector3df xyz = m_kart->getXYZ().toIrrVector();
        m_camera->setTarget(xyz);
        xyz.Y = xyz.Y+30;
        m_camera->setPosition(xyz);
        m_camera->setNearValue(52.0); // To view inside tunnels (FIXME 52>30 why??? makes no sense)
        return;
    }

    if(m_mode==CM_FINAL)
    {
        handleEndCamera(dt);
        return;
    }

    float above_kart, cam_angle, side_way, distance;
    bool  smoothing;
    getCameraSettings(&above_kart, &cam_angle, &side_way, &distance,
                      &smoothing);

    // If an explosion is happening, stop moving the camera,
    // but keep it target on the kart.
    if(m_kart->playingExplosionAnimation())
    {
        // The camera target needs to be 'smooth moved', otherwise
        // there will be a noticable jump in the first frame

        // Aim at the usual same position of the kart (i.e. slightly
        // above the kart).
        core::vector3df wanted_target(m_kart->getXYZ().toIrrVector()
                                      +core::vector3df(0, above_kart, 0) );
        core::vector3df current_target   = m_camera->getTarget();
        // Note: this code is replicated from smoothMoveCamera so that
        // the camera keeps on pointing to the same spot.
        current_target += ((wanted_target-current_target)*m_target_speed)*dt;

        m_camera->setTarget(current_target);
        return;
    }

    positionCamera(dt, above_kart, cam_angle, side_way, distance, smoothing);
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

    wanted_target.setY(wanted_target.getY()+above_kart);

    float tan_up = tan(cam_angle);
    Vec3 relative_position(side_way,
                           fabsf(distance)*tan_up+above_kart,
                           distance);
    const btTransform &trans=m_kart->getTrans();
    wanted_position = trans(relative_position);
    if (smoothing)
    {
        smoothMoveCamera(dt, wanted_position, wanted_target);
    }
    else
    {
        if (m_mode!=CM_FALLING)
            m_camera->setPosition(wanted_position.toIrrVector());
        m_camera->setTarget(wanted_target.toIrrVector());
        
        if (race_manager->getNumLocalPlayers() < 2)
        {
            sfx_manager->positionListener(m_camera->getPosition(),  wanted_target - m_camera->getPosition());
        }
    }

}   // positionCamera

// ----------------------------------------------------------------------------
/** This function handles the end camera. It adjusts the camera position
 *  according to the current camera type, and checks if a switch to the 
 *  next camera should be made.
 *  \param dt Time step size.
*/
void Camera::handleEndCamera(float dt)
{
    EndCameraInformation::EndCameraType info 
        = m_end_cameras.size()==0 ? EndCameraInformation::EC_AHEAD_OF_KART
                                  : m_end_cameras[m_current_end_camera].m_type;

    switch(info)
    {
    case EndCameraInformation::EC_STATIC_FOLLOW_KART:
        {
            const core::vector3df &cp = m_camera->getAbsolutePosition();
            const Vec3            &kp = m_kart->getXYZ();
            // Estimate the fov, assuming that the vector from the camera to
            // the kart and the kart length are orthogonal to each other
            // --> tan (fov) = kart_length / camera_kart_distance
            // In order to show a little bit of the surrounding of the kart
            // the kart length is multiplied by 3 (experimentally found, but
            // this way we have approx one kart length on the left and right
            // side of the screen for the surroundings)
            float fov = 3*atan2(m_kart->getKartLength(),
                                (cp-kp.toIrrVector()).getLength());
            m_camera->setFOV(fov);
            m_camera->setTarget(m_kart->getXYZ().toIrrVector());
            break;
        }
    case EndCameraInformation::EC_AHEAD_OF_KART:
        {
            const KartProperties *kp=m_kart->getKartProperties();
            float cam_angle  = kp->getCameraBackwardUpAngle();

            positionCamera(dt, /*above_kart*/0.75f, 
                           cam_angle, /*side_way*/0, 
                           2.0f*m_distance, /*smoothing*/false);
            break;
        }
    default: break;
    }   // switch

    // Now test if the kart is close enough to the next end camera, and
    // if so activate it.
    if( m_end_cameras.size()>0 && 
        m_end_cameras[m_next_end_camera].isReached(m_kart->getXYZ()))
    {
        m_current_end_camera = m_next_end_camera;
        if(m_end_cameras[m_current_end_camera].m_type
            ==EndCameraInformation::EC_STATIC_FOLLOW_KART)
            m_camera->setPosition(
                m_end_cameras[m_current_end_camera].m_position.toIrrVector()
                );
        m_camera->setFOV(m_fov);
        m_next_end_camera++;
        if(m_next_end_camera>=(unsigned)m_end_cameras.size()) m_next_end_camera = 0;
    }
}   // checkForNextEndCamera

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

// ----------------------------------------------------------------------------
