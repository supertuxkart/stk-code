

#include "tuxkart.h"


inline float sgnsq ( float x ) { return ( x < 0 ) ? -(x * x) : (x * x) ; }


void TrafficDriver::update ()
{
  /* Steering algorithm */

  /* If moving left-to-right and on the left - or right to left
     and on the right - do nothing. */

  sgVec2 track_velocity ;
  sgSubVec2 ( track_velocity, curr_track_coords, last_track_coords ) ;

  if ( ( track_velocity [ 0 ] < 0.0f && curr_track_coords [ 0 ] > 0.0f ) ||
       ( track_velocity [ 0 ] > 0.0f && curr_track_coords [ 0 ] < 0.0f ) )
    velocity.hpr[0] = sgnsq(curr_track_coords[0])*3.0f ;
  else
    velocity.hpr[0] = sgnsq(curr_track_coords[0])*12.0f ;

  velocity.xyz[1]  = TRAFFIC_VELOCITY ;
  velocity.xyz[2] -= GRAVITY * delta_t ;

  if ( wheelie_angle != 0.0f )
    wheelie_angle = 0.0f ;

  KartDriver::update () ;
}

void TrafficDriver::doObjectInteractions () {}
void TrafficDriver::doLapCounting        () {}
void TrafficDriver::doZipperProcessing   () {}

