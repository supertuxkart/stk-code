
#include "tuxkart.h"

#define MIN_CAM_DISTANCE      5.0f
#define MAX_CAM_DISTANCE     10.0f  // Was 15
#define MAX_FIXED_CAMERA      9

int   cam_follow =  0    ;

static sgCoord fixedpos [ MAX_FIXED_CAMERA ] =
{
  { {    0,    0, 500 }, {    0, -90, 0 } },

  { {    0,  180,  30 }, {  180, -15, 0 } },
  { {    0, -180,  40 }, {    0, -15, 0 } },

  { {  300,    0,  60 }, {   90, -15, 0 } },
  { { -300,    0,  60 }, {  -90, -15, 0 } },

  { {  200,  100,  30 }, {  120, -15, 0 } },
  { {  200, -100,  40 }, {   60, -15, 0 } },
  { { -200, -100,  30 }, {  -60, -15, 0 } },
  { { -200,  100,  40 }, { -120, -15, 0 } }
} ;


static sgCoord steady_cam = {{ 0, 0, 0 }, { 0, 0, 0 }} ;


void updateCamera ()
{
  static float cam_delay  = 10.0f ;

  /* Update the camera */

  if ( cam_follow < 0 )
    cam_follow = 18 + MAX_FIXED_CAMERA - 1 ;
  else
  if ( cam_follow >= 18 + MAX_FIXED_CAMERA )
    cam_follow = 0 ;

  sgCoord final_camera ;

  if ( cam_follow < num_karts )
  {
    sgCoord cam, target, diff ;

    sgCopyCoord ( &target, kart[cam_follow]->getCoord   () ) ;
    sgCopyCoord ( &cam   , kart[cam_follow]->getHistory ( (int)cam_delay ) ) ;

    float dist = 5.0f + sgDistanceVec3 ( target.xyz, cam.xyz ) ;

    if ( dist < MIN_CAM_DISTANCE && cam_delay < 50 )
      cam_delay++ ;

    if ( dist > MAX_CAM_DISTANCE && cam_delay > 1 )
      cam_delay-- ;

    sgVec3 offset ;
    sgMat4 cam_mat ;

    sgSetVec3 ( offset, -0.5f, -5.0f, 1.5f ) ;
    sgMakeCoordMat4 ( cam_mat, &cam ) ;

    sgXformPnt3 ( offset, cam_mat ) ;

    sgCopyVec3 ( cam.xyz, offset ) ;

    cam.hpr[1] = -5.0f ;
    cam.hpr[2] = 0.0f;

    sgSubVec3 ( diff.xyz, cam.xyz, steady_cam.xyz ) ;
    sgSubVec3 ( diff.hpr, cam.hpr, steady_cam.hpr ) ;

    while ( diff.hpr[0] >  180.0f ) diff.hpr[0] -= 360.0f ;
    while ( diff.hpr[0] < -180.0f ) diff.hpr[0] += 360.0f ;
    while ( diff.hpr[1] >  180.0f ) diff.hpr[1] -= 360.0f ;
    while ( diff.hpr[1] < -180.0f ) diff.hpr[1] += 360.0f ;
    while ( diff.hpr[2] >  180.0f ) diff.hpr[2] -= 360.0f ;
    while ( diff.hpr[2] < -180.0f ) diff.hpr[2] += 360.0f ;

    steady_cam.xyz[0] += 0.2f * diff.xyz[0] ;
    steady_cam.xyz[1] += 0.2f * diff.xyz[1] ;
    steady_cam.xyz[2] += 0.2f * diff.xyz[2] ;
    steady_cam.hpr[0] += 0.1f * diff.hpr[0] ;
    steady_cam.hpr[1] += 0.1f * diff.hpr[1] ;
    steady_cam.hpr[2] += 0.1f * diff.hpr[2] ;

    final_camera = steady_cam ;
  }
  else
  if ( cam_follow < num_karts + MAX_FIXED_CAMERA )
    final_camera = fixedpos[cam_follow-num_karts] ;
  else
    final_camera = steady_cam ;

  sgVec3 interfovealOffset ;
  sgMat4 mat ;

  sgSetVec3 ( interfovealOffset, 0.2 * (float)stereoShift(), 0, 0 ) ;
  sgMakeCoordMat4 ( mat, &final_camera ) ;
  sgXformPnt3 ( final_camera.xyz, interfovealOffset, mat ) ;

  ssgSetCamera ( &final_camera ) ;
}


