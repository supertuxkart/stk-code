//  $Id: Camera.cxx,v 1.4 2004/08/01 00:13:27 grumbel Exp $
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

  sgSetVec3 ( location.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( location.hpr, 0, 0, 0 ) ;
  whichKart  = 0 ;
  setScreenPosition ( 0 ) ;
  cam_delay  = 10.0f ;
}


void Camera::update ()
{
  /* Update the camera */

  if ( whichKart >= num_karts || whichKart < 0 ) whichKart = 0 ;

  sgCoord cam, target, diff ;

  sgCopyCoord ( &target, kart[whichKart]->getCoord   () ) ;
  sgCopyCoord ( &cam   , kart[whichKart]->getHistory ( (int)cam_delay ) ) ;

  float dist = 5.0f + sgDistanceVec3 ( target.xyz, cam.xyz ) ;

  if ( dist < MIN_CAM_DISTANCE && cam_delay < 50 ) cam_delay++ ; 
  if ( dist > MAX_CAM_DISTANCE && cam_delay >  1 ) cam_delay-- ;

  sgVec3 offset ;
  sgMat4 cam_mat ;

  sgSetVec3 ( offset, -0.5f, -5.0f, 1.5f ) ;
  sgMakeCoordMat4 ( cam_mat, &cam ) ;

  sgXformPnt3 ( offset, cam_mat ) ;

  sgCopyVec3 ( cam.xyz, offset ) ;

  cam.hpr[1] = -5.0f ;
  cam.hpr[2] = 0.0f;

  sgSubVec3 ( diff.xyz, cam.xyz, location.xyz ) ;
  sgSubVec3 ( diff.hpr, cam.hpr, location.hpr ) ;

  while ( diff.hpr[0] >  180.0f ) diff.hpr[0] -= 360.0f ;
  while ( diff.hpr[0] < -180.0f ) diff.hpr[0] += 360.0f ;
  while ( diff.hpr[1] >  180.0f ) diff.hpr[1] -= 360.0f ;
  while ( diff.hpr[1] < -180.0f ) diff.hpr[1] += 360.0f ;
  while ( diff.hpr[2] >  180.0f ) diff.hpr[2] -= 360.0f ;
  while ( diff.hpr[2] < -180.0f ) diff.hpr[2] += 360.0f ;

  location.xyz[0] += 0.2f * diff.xyz[0] ;
  location.xyz[1] += 0.2f * diff.xyz[1] ;
  location.xyz[2] += 0.2f * diff.xyz[2] ;
  location.hpr[0] += 0.1f * diff.hpr[0] ;
  location.hpr[1] += 0.1f * diff.hpr[1] ;
  location.hpr[2] += 0.1f * diff.hpr[2] ;

  final_camera = location ;

  // sgVec3 interfovealOffset ;
  // sgMat4 mat ;
  // sgSetVec3 ( interfovealOffset, 0.2 * (float)stereoShift(), 0, 0 ) ;
  // sgMakeCoordMat4 ( mat, &final_camera ) ;
  // sgXformPnt3 ( final_camera.xyz, interfovealOffset, mat ) ;

  context -> setCamera ( & final_camera ) ;
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



