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

   void
   Camera::setScreenPosition ( int numPlayers, int pos )
   {
      assert(pos >= 0 && pos <= 3);
   
      if (numPlayers == 1)
      {
         context -> setFOV ( 75.0f, 0.0f ) ;
         x = 0.0f; y = 0.0f; w = 1.0f; h = 1.0f ;
      }
      else if (numPlayers == 2)
      {
         context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
         switch ( pos )
         {
            case 0 : x = 0.0f; y = 0.5f; w = 1.0f; h = 0.5f; 
               break;
            case 1 : x = 0.0f; y = 0.0f; w = 1.0f; h = 0.5f; 
               break;
         }
      }
      else if (numPlayers == 3 || numPlayers == 4)
      {
         context -> setFOV ( 50.0f, 0.0f );
         switch ( pos )
         {
            case 0 : x = 0.0f; y = 0.5f; w = 0.5f; h = 0.5f; 
               break;
            case 1 : x = 0.5f; y = 0.5f; w = 0.5f; h = 0.5f; 
               break;
            case 2 : x = 0.0f; y = 0.0f; w = 0.5f; h = 0.5f; 
               break;
            case 3 : x = 0.5f; y = 0.0f; w = 0.5f; h = 0.5f; 
               break;
         }
      }
   }

   Camera::Camera ( int numPlayers, int which_ )
   {
      whichKart = which_ ;   // Just for now
      mode = CM_NORMAL;
      context = new ssgContext ;
   
   // FIXME: clipping should be configurable for slower machines
      const Track* track = track_manager->getTrack(world->raceSetup.track);
      if (track->useFog())
         context -> setNearFar ( 0.05f, track->getFogEnd() ) ;
      else
         context -> setNearFar ( 0.05f, 1000.0f ) ;
   
      setScreenPosition ( numPlayers, whichKart ) ;
      last_steer_offset = 0;
   }

   void
   Camera::setMode(Mode mode_)
   {
      mode = mode_;
   }

   void Camera::update ()
   {
   // Update the camera
      if ( whichKart >= int(world->getNumKarts()) || whichKart < 0 ) whichKart = 0 ;
   
      sgCoord kartcoord;
      sgCopyCoord(&kartcoord, world->getPlayerKart(whichKart)->getCoord());
   
      kartcoord.hpr[2] = 0;
      kartcoord.hpr[1] = 0;
   
      if (mode == CM_SIMPLE_REPLAY)
         kartcoord.hpr[0] = 0;
   
   // Uncomment this for a simple MarioKart-like replay-camera
   // kartcoord.hpr[0] = 0;
   
   // Matrix that transforms stuff to kart-space
      sgMat4 tokart;
      sgMakeCoordMat4 (tokart, &kartcoord);
   
   // Relative position from the middle of the kart
      sgMat4 relative;
      sgMat4 cam_pos;
   
      if (mode == CM_CLOSEUP)
         sgMakeTransMat4(cam_pos, 0.f, -2.5f, 1.5f);
      else
         sgMakeTransMat4(cam_pos, 0.f, -3.5f, 1.5f);
   
      if (mode == CM_NO_FAKE_DRIFT)
      {
         float steer_offset = world->getPlayerKart(whichKart)->getSteerAngle()*-10.0f;

         sgMat4 cam_rot;
         sgMat4 tmp;
         sgMakeRotMat4(cam_rot, 0, -5, 0);
         sgMultMat4(tmp, cam_pos, cam_rot);
         sgMakeRotMat4(cam_rot, steer_offset, 0, 0);
         sgMultMat4(relative, cam_rot, tmp);
      }
      else
      {
         sgMat4 cam_rot;       
         if (mode == CM_CLOSEUP)
            sgMakeRotMat4(cam_rot, 0, -15, 0);
         else
            sgMakeRotMat4(cam_rot, 0, -5, 0);
         sgMultMat4(relative, cam_pos, cam_rot);
      }
   
      sgMat4 result;
      sgMultMat4(result, tokart, relative);
   
      sgCoord cam;
      sgSetCoord(&cam, result);
   
      context -> setCamera (&cam) ;
   }

   void Camera::apply ()
   {
     //JH      int width  = getScreenWidth  () ;
      int width  = 800 ;
      //JH      int height = getScreenHeight () ;
      int height = 600;
   
      assert ( world->scene != NULL ) ;
   
      glViewport ( (int)((float)width  * x),
                 (int)((float)height * y),
                 (int)((float)width  * w),
                 (int)((float)height * h) ) ;
   
      context -> makeCurrent () ;
   }

/* EOF */
