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

void Camera::setScreenPosition ( int numPlayers, int pos )
{
    assert(pos >= 0 && pos <= 3);

    if (numPlayers == 1)
    {
        m_context -> setFOV ( 75.0f, 0.0f ) ;
        m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 1.0f ;
    }
    else if (numPlayers == 2)
    {
        m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
        switch ( pos )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 1.0f; m_h = 0.5f;
            break;
        case 1 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f;
            break;
        }
    }
    else if (numPlayers == 3)
    {
        m_context -> setFOV ( 50.0f, 0.0f );
        switch ( pos )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f;
            break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f;
            break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f;
            m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
            break;
        }
    }
    else if (numPlayers == 4)
    {
        m_context -> setFOV ( 50.0f, 0.0f );
        switch ( pos )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f;
            break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f;
            break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f;
            break;
        case 3 : m_x = 0.5f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f;
            break;
        }
    }
    m_LastPitch = 0.0f;
}  // setScreenPosition

//-----------------------------------------------------------------------------
Camera::Camera(int numPlayers, int which)
  : m_reverse(false)
{
    m_which_kart = which;   // Just for now
    m_mode = CM_NORMAL;
    m_context = new ssgContext ;

    // FIXME: clipping should be configurable for slower machines
    const Track* track = world->getTrack();
    if (track->useFog())
        m_context -> setNearFar ( 0.05f, track->getFogEnd() ) ;
    else
        m_context -> setNearFar ( 0.05f, 1000.0f ) ;

    setScreenPosition ( numPlayers, m_which_kart ) ;
    m_last_steer_offset = 0;
}   // Camera

//-----------------------------------------------------------------------------
void Camera::setMode(Mode mode)
{
    m_mode = mode;
}   // setMode

//-----------------------------------------------------------------------------
void Camera::setReverseHeading(bool b)
{
  m_reverse = b;
}   // setReverseHeading

//-----------------------------------------------------------------------------
/** Reset is called when a new race starts. Make sure that the camera
    is aligned neutral, and not like in the previous race
*/
void Camera::reset()
{
    m_LastPitch = 0.0f;
}   // reset

//-----------------------------------------------------------------------------
void Camera::update (float dt)
{
    // Update the camera
    if ( m_which_kart >= int(world->getNumKarts()) || m_which_kart < 0 ) m_which_kart = 0 ;

    sgCoord kartcoord;
    const Kart *kart=world->getPlayerKart(m_which_kart);
    sgCopyCoord(&kartcoord, world->getPlayerKart(m_which_kart)->getCoord());

    // Use the terrain pitch to avoid the camera following a wheelie the kart is doing
    kartcoord.hpr[1]=RAD_TO_DEGREE( kart->getTerrainPitch(DEGREE_TO_RAD(kartcoord.hpr[0])) );
    kartcoord.hpr[2] = 0;
    // Only adjust the pitch if it's not the first frame (which is indicated by having
    // dt=0). Otherwise the camera will change pitch during ready-set-go.
    if(dt>0)
    {
        // If the terrain pitch is 'significantly' different from the camera angle,
        // start adjusting the camera. This helps with steep declines, where
        // otherwise the track is not visible anymore.
        if(fabsf(kartcoord.hpr[1]-m_LastPitch)>1.0f) {
            kartcoord.hpr[1] = m_LastPitch + (kartcoord.hpr[1]-m_LastPitch)*2.0f*dt;
        }
        else
        {
            kartcoord.hpr[1]=m_LastPitch;
        }
    }
    m_LastPitch = kartcoord.hpr[1];

    if (m_mode == CM_SIMPLE_REPLAY)
        kartcoord.hpr[0] = 0;

    // Uncomment this for a simple MarioKart-like replay-camera
    // kartcoord.hpr[0] = 0;

    // Matrix that transforms stuff to kart-space
    sgMat4 tokart;
    sgMakeCoordMat4 (tokart, &kartcoord);

    // Relative position from the middle of the kart
    sgMat4 relative;
    sgMat4 cam_pos;

    if (m_mode == CM_CLOSEUP)
        sgMakeTransMat4(cam_pos, 0.f, -2.5f, 1.5f);
    else
        sgMakeTransMat4(cam_pos, 0.f, -3.5f, 1.5f);

    if (m_reverse)
    {
      // If player is looking back all other camera options are ignored.
      sgMat4 cam_lb;
      sgMakeTransMat4(cam_pos, 0.f, -2.5f, 0.75f);

      // Applies 'look back' rotation.
      sgMakeRotMat4(cam_lb, 180, 0, 0);
      sgMultMat4(relative, cam_pos, cam_lb);
    }
    else if (m_mode == CM_NO_FAKE_DRIFT)
    {
        const float STEER_OFFSET = world->getPlayerKart(m_which_kart)->getSteerAngle()*-10.0f;

        sgMat4 cam_rot;
        sgMat4 tmp;
        sgMakeRotMat4(cam_rot, 0, -5, 0);
        sgMultMat4(tmp, cam_pos, cam_rot);
        sgMakeRotMat4(cam_rot, STEER_OFFSET, 0, 0);
        sgMultMat4(relative, cam_rot, tmp);
    }
    else
    {
      sgMat4 cam_rot;

      if (m_mode == CM_CLOSEUP)
        sgMakeRotMat4(cam_rot, 0, -15, 0);
      else
        sgMakeRotMat4(cam_rot, 0, -5, 0);

      sgMultMat4(relative, cam_pos, cam_rot);
    }

    sgMat4 result;
    sgMultMat4(result, tokart, relative);

    sgCoord cam;
    sgSetCoord(&cam, result);

    m_context -> setCamera (&cam) ;
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

