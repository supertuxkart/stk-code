
#include "tuxkart.h"


void Projectile::update ()
{
  wheelie_angle = 0 ;
  zipper_time_left = 0.0f ;

  if ( type == COLLECT_HOMING_MISSILE )
    velocity.xyz[1] = MAX_HOMING_PROJECTILE_VELOCITY ;
  else
  if ( type == COLLECT_MISSILE )
    velocity.xyz[1] = MAX_PROJECTILE_VELOCITY ;
  else
    velocity.xyz[1] = MAX_PROJECTILE_VELOCITY / 5.0f ;

  Driver::update () ;
  wheelie_angle = 0 ;
  zipper_time_left = 0.0f ;
}

void Projectile::doObjectInteractions ()
{
  float ndist = SG_MAX ;
  int nearest = -1 ;

  for ( int i = 0 ; i < num_karts ; i++ )
  {
    sgCoord *pos ;
 
    pos = kart [ i ] -> getCoord () ;
 
    if ( type != COLLECT_NOTHING && kart[i] != owner )
    {
      float d = sgDistanceSquaredVec3 ( pos->xyz, getCoord()->xyz ) ;

      if ( d < 2.0f )
      {
        kart [ i ] -> forceCrash () ;
        curr_pos.xyz[2] += 1.2f ;
        explosion[0]->start(curr_pos.xyz);
        off () ;
      }
      else
      if ( d < ndist )
      {
        ndist = d ;
        nearest = i ;
      }
    }
  }

  if ( type == COLLECT_HOMING_MISSILE && nearest != -1 &&
        ndist < MAX_HOME_DIST_SQD )
  {
    sgVec3 delta ;
    sgVec3 hpr ;
    sgCoord *k = kart[nearest]->getCoord() ;

    sgSubVec3 ( delta, k->xyz, curr_pos.xyz ) ;

delta[2] = 0.0f ;

    sgHPRfromVec3 ( hpr, delta ) ;

    sgSubVec3 ( hpr, curr_pos.hpr ) ;

    if ( hpr[0] >  180.0f ) hpr[0] -= 360.0f ;
    if ( hpr[0] < -180.0f ) hpr[0] += 360.0f ;
    if ( hpr[1] >  180.0f ) hpr[1] -= 360.0f ;
    if ( hpr[1] < -180.0f ) hpr[1] += 360.0f ;

    if ( hpr[0] > 80.0f || hpr[0] < -80.0f )
      velocity.hpr[0] = 0.0f ;
    else
    {
      if ( hpr[0] > 3.0f )
        velocity.hpr[0] = HOMING_MISSILE_TURN_RATE ;
      else
      if ( hpr[0] < -3.0f )
        velocity.hpr[0] = -HOMING_MISSILE_TURN_RATE ;
      else
        velocity.hpr[0] = 0.0f ;

      if ( hpr[1] > 1.0f )
        velocity.hpr[1] = HOMING_MISSILE_PITCH_RATE ;
      else
      if ( hpr[1] < -1.0f )
        velocity.hpr[1] = -HOMING_MISSILE_PITCH_RATE ;
      else
        velocity.hpr[1] = 0.0f ;
    }
  }
  else
    velocity.hpr[0] = velocity.hpr[1] = 0.0f ;

}

void Projectile::doLapCounting        () {}
void Projectile::doZipperProcessing   () {}

void Projectile::doCollisionAnalysis  ( float /* hot */ )
{
  if ( collided || crashed )
  {
    if ( type == COLLECT_SPARK )
    {
      sgVec3 bouncevec ;
      sgVec3 direction ;

      sgNormalizeVec3 ( bouncevec, surface_avoidance_vector ) ;
      sgSubVec3 ( direction, curr_pos.xyz, last_pos.xyz ) ;
      sgReflectInPlaneVec3 ( direction, bouncevec ) ;

      sgHPRfromVec3 ( curr_pos.hpr, direction ) ;
    }
    else
    if ( type != COLLECT_NOTHING )
    {
      curr_pos.xyz[2] += 1.2f ;
      explosion[0]->start(curr_pos.xyz);
      off () ;
    }
  }
}


