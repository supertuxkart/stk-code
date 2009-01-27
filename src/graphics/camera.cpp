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

#define _WINSOCKAPI_
#include <plib/ssg.h>
#include "user_config.hpp"
#include "audio/sound_manager.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"
#include "race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/coord.hpp"

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode     = CM_NORMAL;
    m_index    = camera_index;
    m_context  = new ssgContext ;
    m_distance = kart->getKartProperties()->getCameraDistance();
    m_kart     = kart;
    m_xyz      = kart->getXYZ();
    m_hpr      = Vec3(0,0,0);

    // FIXME: clipping should be configurable for slower machines
    const Track* track  = RaceManager::getTrack();
    if (track->useFog())
        m_context -> setNearFar ( 0.05f, track->getFogEnd() ) ;
    else
        m_context -> setNearFar ( 0.05f, 1000.0f ) ;

    setScreenPosition(camera_index);
}   // Camera

// ----------------------------------------------------------------------------
Camera::~Camera()
{
    reset();
    if(m_context) delete m_context;
}

// ----------------------------------------------------------------------------
void Camera::setScreenPosition(int camera_index)
{
    const int num_players = race_manager->getNumLocalPlayers();
    assert(camera_index >= 0 && camera_index <= 3);

    if (num_players == 1)
    {
        m_context -> setFOV ( 75.0f, 0.0f ) ;
        m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 1.0f ;
    }
    else if (num_players == 2)
    {
        m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 1.0f; m_h = 0.5f; break;
        case 1 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f; break;
        }
    }
    else if (num_players == 3)
    {
        m_context -> setFOV ( 50.0f, 0.0f );
        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f;
                 m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ); break;
        }
    }
    else if (num_players == 4)
    {
        m_context -> setFOV ( 50.0f, 0.0f );
        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f; break;
        case 3 : m_x = 0.5f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f; break;
        }
    }
    m_last_pitch = 0.0f;
}  // setScreenPosition

//-----------------------------------------------------------------------------
void Camera::setMode(Mode mode)
{
    if(mode==CM_FINAL)
    {
        const Track* track=RaceManager::getTrack();
        // If the track doesn't have a final position, ignore this mode
        if(!track->hasFinalCamera()) return;
        m_velocity           = (track->getCameraPosition()-m_xyz)
                             / stk_config->m_final_camera_time;
        m_angular_velocity   = (track->getCameraHPR()-m_hpr)
                             / stk_config->m_final_camera_time;
        m_final_time         = 0.0f;
    }
    m_mode       = mode;
    m_last_pitch = 0.0f;
    if(m_mode==CM_CLOSEUP)
        m_distance = 2.5f;
    else
    {
        m_distance = m_kart->getKartProperties()->getCameraDistance();

        // In splitscreen mode we have a different FOVs and rotations so we use
        // 1.333 or 1.5 times the normal distance to compensate and make the 
        // kart visible
        const int num_players = race_manager->getNumPlayers();
        if(num_players==2 || (num_players==3 && m_index==3) )
            m_distance *= 1.5f;
        else if(num_players>=3)
            m_distance *= 1.3333333f;
    }
}   // setMode

// ----------------------------------------------------------------------------
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
    m_last_pitch = 0.0f;
    m_xyz        = m_kart->getXYZ();
    m_hpr        = Vec3(0,0,0);
    sound_manager->positionListener(m_xyz, m_xyz);
}   // reset

//-----------------------------------------------------------------------------
void Camera::update (float dt)
{
    if(m_mode==CM_FINAL) return finalCamera(dt);

    Vec3        kart_xyz, kart_hpr;
    const Kart *kart;
    
    // First define the position of the kart
    if(m_mode==CM_LEADER_MODE)
    {
        kart     = RaceManager::getKart(0);
        kart_hpr = kart->getHPR();
    }
    else
    {
        kart     = m_kart;
        kart_hpr = kart->getHPR();
        // Use the terrain pitch to avoid the camera following a wheelie the kart is doing
        kart_hpr.setPitch( m_kart->getTerrainPitch(kart_hpr.getHeading()) );
        kart_hpr.setRoll(0.0f);
        // Only adjust the pitch if it's not the race start, otherwise 
        // the camera will change pitch during ready-set-go.
        if(RaceManager::getWorld()->isRacePhase())
        {
            // If the terrain pitch is 'significantly' different from the camera angle,
            // start adjusting the camera. This helps with steep declines, where
            // otherwise the track is not visible anymore.
            if(fabsf(kart_hpr.getPitch()-m_last_pitch)>M_PI/180.0f) {
                m_last_pitch = m_last_pitch + (kart_hpr.getPitch()-m_last_pitch)*2.0f*dt;
            }
            kart_hpr.setPitch(m_last_pitch);
        }   //  dt>0.0
    }   // m_mode!=CM_LEADER_MODE
    kart_xyz = kart->getXYZ();
    if(m_mode==CM_SIMPLE_REPLAY) kart_hpr.setHeading(0.0f);

    // Set the camera position relative to the kart
    // --------------------------------------------
    // The reverse mode and the cam used in follow the leader mode (when a
    // kart has been eliminated) are facing backwards:
    bool reverse = m_kart->getControls().m_look_back || m_mode==CM_LEADER_MODE;
    Vec3 cam_rel_pos(0.f, reverse ? m_distance : -m_distance, 1.5f);

    // Set the camera rotation
    // -----------------------
    float sign = reverse ? 1.0f : -1.0f;
    const int num_players = race_manager->getNumLocalPlayers();
    float pitch;
    if(m_mode!=CM_CLOSEUP)
        pitch = race_manager->getNumLocalPlayers()>1 ? sign * DEGREE_TO_RAD(10.0f)
                                                     : sign * DEGREE_TO_RAD(15.0f);
    else
        pitch = sign * DEGREE_TO_RAD(25.0f);
      
    btQuaternion cam_rot(0.0f, pitch, reverse ? M_PI : 0.0f);
    // Camera position relative to the kart
    btTransform relative_to_kart(cam_rot, cam_rel_pos);

    btMatrix3x3 rotation;
    rotation.setEulerZYX(kart_hpr.getPitch(), kart_hpr.getRoll(), kart_hpr.getHeading());
    btTransform result = btTransform(rotation, kart_xyz) * relative_to_kart;
    
    // Convert transform to coordinate and pass on to plib
    Coord c(result);
    m_xyz = c.getXYZ();
    m_hpr = c.getHPR();
    m_context -> setCamera(&c.toSgCoord());
    if(race_manager->getNumLocalPlayers() < 2)
        sound_manager->positionListener(m_xyz, kart_xyz - m_xyz);
}   // update

//-----------------------------------------------------------------------------
void Camera::finalCamera(float dt)
{
    // Turn/move the camera for 1 second only
    m_final_time += dt;    
    if( m_final_time<stk_config->m_final_camera_time )
    {
        m_xyz += m_velocity*dt;
        m_hpr += m_angular_velocity*dt;
        Coord coord(m_xyz, m_hpr);
        m_context->setCamera(&coord.toSgCoord());
    }
#undef TEST_END_CAMERA_POSITION
#ifdef TEST_END_CAMERA_POSITION
    else
    {
        // This code is helpful when tweaking the final camera position:
        // Just set a breakpoint here, change the values for x,y,z,h,p,r,
        // and then keep on running, and you can see what the final position
        // looks like. When happy, just put these value as 
        // camera-final-position and camera-final-hpr in the .track file.
        static float x=5,y=20,z=3,h=180,p=-10,r=0.0f;
        Vec3 xyz(x,y,z);
        Vec3 hpr(DEGREE_TO_RAD(h),DEGREE_TO_RAD(p),DEGREE_TO_RAD(r));
        Coord coord(xyz, hpr);
        m_context->setCamera(&coord.toSgCoord());
    }
#endif

}   // finalCamera

//-----------------------------------------------------------------------------

void Camera::apply ()
{
    int width  = user_config->m_width ;
    int height = user_config->m_height;

    glViewport ( (int)((float)width  * m_x),
                 (int)((float)height * m_y),
                 (int)((float)width  * m_w),
                 (int)((float)height * m_h) ) ;

    m_context -> makeCurrent () ;
}   // apply

