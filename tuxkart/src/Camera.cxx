//  $Id: Camera.cxx,v 1.6 2004/08/05 12:53:08 grumbel Exp $
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

#include "tuxkart.h"
#include "Driver.h"
#include "Camera.h"

#define MIN_CAM_DISTANCE      5.0f
#define MAX_CAM_DISTANCE     10.0f  // Was 15
#define MAX_FIXED_CAMERA      9

Camera *camera [ 4 ] = { NULL, NULL, NULL, NULL } ;

int Camera::numSplits = 4 ;


void Camera::setScreenPosition ( int pos )
{
  switch ( numSplits )
  {
    case 1 : x = 0.0f ; y = 0.0f ; w = 1.0f ; h = 1.0f ;
             context -> setFOV ( 75.0f, 0.0f ) ;
             return ;

    case 2 :
      context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
      switch ( pos )
      {
        case 0 : x = 0.0f ; y = 0.0f ; w = 1.0f ; h = 0.5f ; return ;
        case 1 : x = 0.0f ; y = 0.5f ; w = 1.0f ; h = 0.5f ; return ;
        default: break ;
      }
      break ;

    case 4 :
      context -> setFOV ( 50.0f, 0.0f ) ;
      switch ( pos )
      {
        case 0 : x = 0.0f ; y = 0.0f ; w = 0.5f ; h = 0.5f ; return ;
        case 1 : x = 0.0f ; y = 0.5f ; w = 0.5f ; h = 0.5f ; return ;
        case 2 : x = 0.5f ; y = 0.0f ; w = 0.5f ; h = 0.5f ; return ;
        case 3 : x = 0.5f ; y = 0.5f ; w = 0.5f ; h = 0.5f ; return ;
        default: break ;
      }
      break ;

    default:
      break ;
  }

  x = 0.0f ; y = 0.0f ; w = 0.0f ; h = 0.0f ;
}


Camera::Camera ( int which )
{
  context = NULL ;
  init () ;
  whichKart = which ;   // Just for now
  setScreenPosition ( which ) ;
}


void Camera::init ()
{
  delete context ;
  context = new ssgContext ;
  context -> setNearFar ( 0.05f, 1000.0f ) ;

  whichKart  = 0 ;
  setScreenPosition ( 0 ) ;
  cam_delay  = 10.0f ;
}


void Camera::update ()
{
  // Update the camera
  if ( whichKart >= num_karts || whichKart < 0 ) whichKart = 0 ;

  sgCoord* kartcoord = kart[whichKart]->getCoord();

  // Matrix that transforms stuff to kart-space
  sgMat4 tokart;
  sgMakeCoordMat4 (tokart, kartcoord);

  // Relative position from the middle of the kart
  sgMat4 relative;
  sgMat4 cam_pos;
  sgMakeTransMat4(cam_pos, 0.f, -3.5f, 1.5f);
  if (0)
    {
    sgMat4 cam_rot;
    sgMakeRotMat4(cam_rot, kart[whichKart]->getSteerAngle()*-5.0f, -5, 0);
    sgMultMat4(relative, cam_rot, cam_pos);
    }
  else
    {
      sgCopyMat4(relative, cam_pos);
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

  assert ( scene != NULL ) ;

  glViewport ( (int)((float)width  * x),
               (int)((float)height * y),
               (int)((float)width  * w),
               (int)((float)height * h) ) ;

  context -> makeCurrent () ;
}


void updateCameras ()
{
  for ( int i = 0 ; i < Camera::getNumSplits() ; i++ )
    camera [ i ] -> update () ;
}


void initCameras ()
{
  for ( int i = 0 ; i < 4 ; i++ )
  {
    delete camera [ i ] ;
    camera [ i ] = new Camera ( i ) ;
  }
}



