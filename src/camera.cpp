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

#include <plib/ssg.h>
#include "coord.hpp"
#include "world.hpp"
#include "player_kart.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "camera.hpp"
#include "user_config.hpp"
#include "constants.hpp"

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode     = CM_NORMAL;
    m_context  = new ssgContext ;
    m_distance = kart->getKartProperties()->getCameraDistance();
    m_kart     = kart;
    m_xyz      = kart->getXYZ();
    m_hpr      = Vec3(0,0,0);

    // FIXME: clipping should be configurable for slower machines
    const Track* track  = world->getTrack();
    if (track->useFog())
        m_context -> setNearFar ( 0.05f, track->getFogEnd() ) ;
    else
        m_context -> setNearFar ( 0.05f, 1000.0f ) ;

    setScreenPosition(camera_index);
}   // Camera

// ----------------------------------------------------------------------------
void Camera::setScreenPosition(int camera_index)
{
    const int num_players = race_manager->getNumPlayers();
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
        const Track* track=world->getTrack();
        // If the track doesn't have a final position, ignore this mode
        if(!track->hasFinalCamera()) return;
        const float duration = 1.0f;
        m_velocity           = (track->getCameraPosition()-m_xyz)/duration;
        m_angular_velocity   = (track->getCameraHPR()-m_hpr)/duration;
        m_final_time         = 0.0f;
    }
    m_mode       = mode;
    m_last_pitch = 0.0f;
    if(m_mode==CM_CLOSEUP)
        m_distance = 2.5f;
    else
        m_distance = m_kart->getKartProperties()->getCameraDistance();
}   // setMode
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
        kart     = world->getKart(0);
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
	if(world->isRacePhase())
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
    bool reverse = m_mode==CM_REVERSE || m_mode==CM_LEADER_MODE;
    Vec3 cam_rel_pos(0.f, -m_distance, reverse ? 0.75f : 1.5f) ;

    // Set the camera rotation
    // -----------------------
    btQuaternion cam_rot(0.0f,
                         m_mode==CM_CLOSEUP ? -0.2618f : -0.0873f,  // -15 or -5 degrees
                         reverse            ? M_PI     : 0.0f);
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
}   // update

//-----------------------------------------------------------------------------
void Camera::finalCamera(float dt)
{
    // Turn/move the camera for 1 second only
    m_final_time += dt;    
    if( m_final_time<1.0f )
    {
        m_xyz += m_velocity*dt;
        m_hpr += m_angular_velocity*dt;
        Coord coord(m_xyz, m_hpr);
        m_context->setCamera(&coord.toSgCoord());
    }
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

