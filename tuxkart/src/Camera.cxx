//  $Id: Camera.cxx,v 1.15 2004/08/16 00:17:22 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <plib/sg.h>
#include "tuxkart.h"
#include "KartDriver.h"
#include "World.h"
#include "Camera.h"

static inline void relaxation(float& target, float& prev, float rate)
{
  target = (prev) + (rate) * ((target) - (prev));
  prev = (target);
}

void
Camera::setScreenPosition ( int pos )
{
  switch ( mode )
    {
    case ONE_SPLIT : x = 0.0f ; y = 0.0f ; w = 1.0f ; h = 1.0f ;
      context -> setFOV ( 75.0f, 0.0f ) ;
      break;

    case TWO_SPLIT :
      context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
      switch ( pos )
        {
        case 0 :
          x = 0.0f ; y = 0.0f ; w = 1.0f ; h = 0.5f ;
          break;
        case 1 : 
          x = 0.0f ; y = 0.5f ; w = 1.0f ; h = 0.5f ;
          break;
        }
      break ;

    case FOUR_SPLIT :
      context -> setFOV ( 50.0f, 0.0f ) ;
      switch ( pos )
        {
        case 0 :
          x = 0.0f ; y = 0.0f ; w = 0.5f ; h = 0.5f ;
          break;
        case 1 :
          x = 0.0f ; y = 0.5f ; w = 0.5f ; h = 0.5f ;
          break;
        case 2 :
          x = 0.5f ; y = 0.0f ; w = 0.5f ; h = 0.5f ;
          break;
        case 3 : 
          x = 0.5f ; y = 0.5f ; w = 0.5f ; h = 0.5f ;
          break;
        }
      break ;
    }
}

Camera::Camera ( Mode mode_, int which )
{
  context = NULL ;
  init () ;
  whichKart = which ;   // Just for now
  mode = mode_;
  setScreenPosition ( which ) ;
  last_steer_offset = 0;
}


void Camera::init ()
{
  delete context ;
  context = new ssgContext ;

  // FIXME: clipping should be configurable for slower machines
  context -> setNearFar ( 0.05f, 1000.0f ) ;

  whichKart  = 0 ;
  setScreenPosition ( 0 ) ;
}


void Camera::update ()
{
  // Update the camera
  if ( whichKart >= int(World::current()->kart.size()) || whichKart < 0 ) whichKart = 0 ;

  sgCoord kartcoord;
  sgCopyCoord(&kartcoord, World::current()->kart[whichKart]->getCoord());

  kartcoord.hpr[2] = 0;
  kartcoord.hpr[1] = 0;

  // Uncomment this for a simple MarioKart-like replay-camera
  // kartcoord.hpr[0] = 0;

  // Matrix that transforms stuff to kart-space
  sgMat4 tokart;
  sgMakeCoordMat4 (tokart, &kartcoord);

  // Relative position from the middle of the kart
  sgMat4 relative;
  sgMat4 cam_pos;
  sgMakeTransMat4(cam_pos, 0.f, -3.5f, 1.5f);

  if (!use_fake_drift)
    {
      float steer_offset = World::current()->kart[whichKart]->getSteerAngle()*-10.0f;
      relaxation(steer_offset, last_steer_offset, .25);
                 
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
  int width  = getScreenWidth  () ;
  int height = getScreenHeight () ;

  assert ( World::current()->scene != NULL ) ;

  glViewport ( (int)((float)width  * x),
               (int)((float)height * y),
               (int)((float)width  * w),
               (int)((float)height * h) ) ;

  context -> makeCurrent () ;
}

/* EOF */
