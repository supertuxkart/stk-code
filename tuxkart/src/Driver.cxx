#include "tuxkart.h"

void Driver::update ()
{
  float dt = fclock->getDeltaTime () ;

  while ( dt > 0.05f )
  {
    delta_t = 0.05f ;
    coreUpdate () ;
    dt -= 0.05f ;
  }

  if ( dt > 0.0f )
  {
    delta_t = dt ;
    coreUpdate () ;
  }

  doObjectInteractions () ;
}


void Driver::coreUpdate ()
{
  sgCoord scaled_velocity ;

  doZipperProcessing () ;

  sgCopyCoord ( &last_pos        , &curr_pos         ) ;
  sgCopyVec2  ( last_track_coords, curr_track_coords ) ;

  if ( velocity.xyz[1] > MAX_VELOCITY )
    velocity.xyz[1] = MAX_VELOCITY ;

  if ( velocity.xyz[1] < MAX_REVERSE_VELOCITY )
    velocity.xyz[1] = MAX_REVERSE_VELOCITY ;

  /* Scale velocities to current time step. */

  sgScaleVec3 ( scaled_velocity.xyz, velocity.xyz, delta_t ) ;
  sgScaleVec3 ( scaled_velocity.hpr, velocity.hpr, delta_t ) ;

  sgMat4 mat    ;
  sgMat4 result ;
  sgMat4 delta  ;

  /* Form new matrix */

  sgMakeCoordMat4 ( delta, & scaled_velocity ) ;
  sgMakeCoordMat4 ( mat  , & curr_pos        ) ;
  sgMultMat4      ( result, mat, delta ) ;
  sgVec3 start ; sgCopyVec3 ( start, curr_pos.xyz ) ;
  sgVec3 end   ; sgCopyVec3 ( end  , result[3]    ) ;

  float hot = collectIsectData ( start, end ) ;

  sgCopyVec3 ( result[3], end ) ;

  sgSetCoord ( &curr_pos, result  ) ;
  float hat = curr_pos.xyz[2]-hot ;
   
  on_ground = ( hat <= 0.01 ) ;

  doCollisionAnalysis ( hot ) ;

  track_hint = curr_track -> spatialToTrack ( curr_track_coords,
                                              curr_pos.xyz,
                                              track_hint ) ;

  sgCopyCoord ( & history[history_index], & curr_pos ) ;
  placeModel () ;

  if ( ++history_index >= HISTORY_FRAMES )
    history_index = 0 ;

  firsttime = FALSE ;
  doLapCounting () ;
}


void Driver::doObjectInteractions () { /* Empty by Default. */ } 
void Driver::doLapCounting        () { /* Empty by Default. */ } 
void Driver::doZipperProcessing   () { /* Empty by Default. */ } 
void Driver::doCollisionAnalysis  ( float ) { /* Empty by Default. */ }

#define ISECT_STEP_SIZE         0.4f
#define COLLISION_SPHERE_RADIUS 0.6f

float Driver::collectIsectData ( sgVec3 start, sgVec3 end )
{
  sgVec3 vel ;

  collided = crashed = FALSE ;  /* Initial assumption */

  sgSubVec3 ( vel, end, start ) ;

  float speed = sgLengthVec3 ( vel ) ;

  /*
    At higher speeds, we must test frequently so we can't
    pass through something thin by mistake.

    At very high speeds, this is getting costly...so beware!
  */

  int nsteps = (int) ceil ( speed / ISECT_STEP_SIZE ) ;

  if ( nsteps == 0 ) nsteps = 1 ;

  if ( nsteps > 100 )
  {
    fprintf ( stderr, "WARNING: Speed too high for collision detect!\n" ) ;
    fprintf ( stderr, "WARNING: Nsteps=%d, Speed=%f!\n", nsteps,speed ) ;
    nsteps = 100 ;
  }

  sgScaleVec3 ( vel, vel, 1.0f / (float) nsteps ) ;

  sgVec3 pos1, pos2 ;

  sgCopyVec3 ( pos1, start ) ;

  float hot = 0.0f ;

  for ( int i = 0 ; i < nsteps ; i++ )
  {
    sgAddVec3 ( pos2, pos1, vel ) ;    
    hot = getIsectData ( pos1, pos2 ) ;
    sgCopyVec3 ( pos1, pos2 ) ;    
  }

  sgCopyVec3 ( end, pos2 ) ;    
  return hot ;
}



float Driver::getIsectData ( sgVec3 start, sgVec3 end )
{
  ssgHit *results ;
  int num_hits ;

  sgSphere sphere ;
  sgMat4   invmat ;

  /*
    It's necessary to lift the center of the bounding sphere
    somewhat so that Player can stand on a slope.
  */

  sphere.setRadius ( COLLISION_SPHERE_RADIUS ) ;
  sphere.setCenter ( 0.0f, 0.0f, COLLISION_SPHERE_RADIUS + 0.3 ) ;

  /* Do a bounding-sphere test for Player. */

  sgMakeIdentMat4 ( invmat ) ;
  invmat[3][0] = -end[0] ;
  invmat[3][1] = -end[1] ;
  invmat[3][2] = -end[2] ;

  if ( firsttime )
    num_hits = 0 ;
  else
    num_hits = ssgIsect ( scene, &sphere, invmat, &results ) ;
 
  sgSetVec3 ( surface_avoidance_vector, 0.0f, 0.0f, 0.0f ) ;

  int i ;

  /* Look at all polygons near to Player */

  for ( i = 0 ; i < num_hits ; i++ )
  {
    ssgHit *h = &results [ i ] ;

    if ( getMaterial ( h->leaf ) -> isIgnore () )
      continue ;

    float dist = sgDistToPlaneVec3 ( h->plane, sphere.getCenter() ) ;

    /*
      This is a nasty kludge to stop a weird interaction
      between collision detection and height-of-terrain
      that causes Player to get 'stuck' on some polygons
      corners. This should be fixed more carefully.

      Surfaces that are this close to horizontal
      are handled by the height-of-terrain code anyway.
    */

    if ( h -> plane[2] > 0.4 )
      continue ;

    if ( dist > 0 && dist < sphere.getRadius() )
    {
      dist = sphere.getRadius() - dist ;
      sgVec3 nrm ;
      sgCopyVec3  ( nrm, h->plane ) ;
      sgScaleVec3 ( nrm, nrm, dist ) ;

      sgAddVec3 ( surface_avoidance_vector, nrm ) ;

      sgVec3 tmp ;
      sgCopyVec3 ( tmp, sphere.getCenter() ) ;
      sgAddVec3 ( tmp, nrm ) ;
      sphere.setCenter ( tmp ) ;

      collided = TRUE ;

      if ( getMaterial ( h->leaf ) -> isZipper    () ) collided = FALSE ;
      if ( getMaterial ( h->leaf ) -> isCrashable () ) crashed  = TRUE  ;
      if ( getMaterial ( h->leaf ) -> isReset     () ) rescue   = TRUE  ;
    }
  }

  /* Look for the nearest polygon *beneath* Player (assuming avoidance) */

  sgAddVec3 ( end, surface_avoidance_vector ) ;

  float hot ;        /* H.O.T == Height Of Terrain */
  sgVec3 HOTvec ;

  invmat[3][0] = - end [0] ;
  invmat[3][1] = - end [1] ;
  invmat[3][2] = 0.0 ;

  float top = COLLISION_SPHERE_RADIUS +
                            (( start[2] > end[2] ) ? start[2] : end[2] ) ;

  sgSetVec3 ( HOTvec, 0.0f, 0.0f, top ) ;

  num_hits = ssgHOT ( scene, HOTvec, invmat, &results ) ;
  
  hot = -1000000.0f ;

  int need_rescue = FALSE ;

  for ( i = 0 ; i < num_hits ; i++ )
  {
    ssgHit *h = &results [ i ] ;

    if ( getMaterial ( h->leaf ) -> isIgnore () )
      continue ;

    float hgt = - h->plane[3] / h->plane[2] ;

    if ( hgt >= hot )
    {
      hot = hgt ;
      sgCopyVec3 ( curr_normal, h->plane ) ;

      need_rescue = getMaterial ( h->leaf ) -> isReset  () ;

      if ( getMaterial ( h->leaf ) -> isZipper () )
      {
        if ( this == kart[0] )
          sound->playSfx ( SOUND_WEE ) ;

        wheelie_angle = 45.0f ;
        zipper_time_left = ZIPPER_TIME ;
      }
    }
  }

  if ( end [ 2 ] < hot )
  {
    end [ 2 ] = hot ;

    if ( need_rescue )
      rescue = TRUE ;
  }

  return hot ;
}

