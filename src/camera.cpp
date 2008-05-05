//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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
#include "world.hpp"
#include "player_kart.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "camera.hpp"
#include "user_config.hpp"
#include "constants.hpp"

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode              = CM_NORMAL;
    m_context           = new ssgContext ;
    m_distance          = kart->getKartProperties()->getCameraDistance();
    m_kart              = kart;

    btVector3 start_pos = m_kart->getPos();
    sgSetVec3(m_current_pos.xyz, start_pos.getX(), start_pos.getY(), start_pos.getZ());
    sgSetVec3(m_current_pos.hpr, 0, 0, 0);

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
    m_mode       = mode;
    m_last_pitch = 0.0f;
    if(m_mode==CM_CLOSEUP)
        m_distance = 2.5f;
    else
        m_distance = m_kart->getKartProperties()->getCameraDistance();
}   // setMode

//-----------------------------------------------------------------------------
/** Reset is called when a new race starts. Make sure that the camera
    is aligned neutral, and not like in the previous race
*/
void Camera::reset()
{
    m_last_pitch = 0.0f;
}   // reset

//-----------------------------------------------------------------------------
void Camera::update (float dt)
{
    sgCoord kartcoord;
    const Kart *kart;
    
    // First define the position of the kart
    if(m_mode==CM_LEADER_MODE)
    {
        kart=world->getKart(0);
        sgCopyCoord(&kartcoord, kart->getCoord());
    }
    else
    {
        kart = m_kart;
        sgCopyCoord(&kartcoord, kart->getCoord());

        // Use the terrain pitch to avoid the camera following a wheelie the kart is doing
        kartcoord.hpr[1]=RAD_TO_DEGREE(m_kart->getTerrainPitch(DEGREE_TO_RAD(kartcoord.hpr[0])) );
        kartcoord.hpr[2] = 0;
        // Only adjust the pitch if it's not the first frame (which is indicated by having
        // dt=0). Otherwise the camera will change pitch during ready-set-go.
        if(dt>0)
        {
            // If the terrain pitch is 'significantly' different from the camera angle,
            // start adjusting the camera. This helps with steep declines, where
            // otherwise the track is not visible anymore.
            if(fabsf(kartcoord.hpr[1]-m_last_pitch)>1.0f) {
                kartcoord.hpr[1] = m_last_pitch + (kartcoord.hpr[1]-m_last_pitch)*2.0f*dt;
            }
            else
            {
                kartcoord.hpr[1]=m_last_pitch;
            }
        }   //  dt>0.0
        m_last_pitch = kartcoord.hpr[1];
    }   // m_mode!=CM_LEADER_MODE
    if(m_mode==CM_SIMPLE_REPLAY) kartcoord.hpr[0] = 0;

    // Set the camera position relative to the kart
    // --------------------------------------------
    sgMat4 cam_pos;

    // The reverse mode and the cam used in follow the leader mode (when a
    // kart has been eliminated) are facing backwards:
    bool reverse= m_mode==CM_REVERSE || m_mode==CM_LEADER_MODE;
    sgMakeTransMat4(cam_pos, 0.f, -m_distance, reverse ? 0.75f : 1.5f);
    
    // Set the camera rotation
    // -----------------------
    sgMat4 cam_rot;
    sgMakeRotMat4(cam_rot, reverse            ? 180.0f : 0.0f,
                           m_mode==CM_CLOSEUP ? -15.0f : -5.0f,
                           0);
    
    // Matrix that transforms stuff to kart-space
    sgMat4 tokart;
    sgMakeCoordMat4 (tokart, &kartcoord);

    sgMat4 relative;
    sgMultMat4(relative, cam_pos, cam_rot);
    sgMat4 result;
    sgMultMat4(result, tokart, relative);

    sgSetCoord(&m_current_pos, result);

    m_context -> setCamera (&m_current_pos) ;
}   // update

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

