
#include "tuxkart.h"


void KartDriver::useAttachment ()
{
  switch ( collectable )
  {
    case COLLECT_MAGNET  :
      attach ( ATTACH_MAGNET, 10.0f ) ;
      break ;

    case COLLECT_ZIPPER  :
      wheelie_angle = 45.0f ;
      zipper_time_left = ZIPPER_TIME ;
      break ;

    case COLLECT_SPARK :
    case COLLECT_MISSILE :
    case COLLECT_HOMING_MISSILE :
      {
	static int m = 0 ;

        if ( ++m >= NUM_PROJECTILES ) m = 0 ;

	projectile[m]->fire ( this, collectable ) ;
      }
      break ;

    case COLLECT_NOTHING :
    default :
	      break ;
  }
 
  if ( --num_collectables <= 0 )
  {
    collectable = COLLECT_NOTHING ;
    num_collectables = 0 ;
  }                                                                           
}

void KartDriver::doLapCounting ()
{
  if ( last_track_coords[1] > 100.0f &&
       curr_track_coords[1] <  20.0f )
    lap++ ;
  else
  if ( curr_track_coords[1] > 100.0f &&
       last_track_coords[1] <  20.0f )
    lap-- ;
}


void KartDriver::doObjectInteractions ()
{
  int i;
  for ( i = 0 ; i < grid_position ; i++ )
  {
    if ( i == grid_position )
      continue ;

    sgVec3 xyz ;

    sgSubVec3 ( xyz, getCoord()->xyz, kart [ i ] -> getCoord () -> xyz ) ;

    if ( sgLengthSquaredVec2 ( xyz ) < 1.0f )
    {
      if ( this == kart[0] || i == 0 )
        sound->playSfx ( SOUND_OW ) ;

      sgNormalizeVec2 ( xyz ) ;

      if ( velocity.xyz[1] > kart[i]->getVelocity()->xyz[1] )
      {
        forceCrash () ;
        sgSubVec2 ( kart[i]->getCoord()->xyz, xyz ) ;
      }
      else
      {
        kart[i]->forceCrash () ;
        sgAddVec2 ( getCoord()->xyz, xyz ) ;
      }
    } 
  }


  for ( i = 0 ; i < MAX_HERRING ; i++ )
  {
    if ( herring[i].her == NULL || herring[i].eaten )
      continue ;

    sgVec3 hpos ;

    herring [ i ] . getPos ( hpos ) ;

    if ( sgDistanceSquaredVec2 ( hpos, getCoord()->xyz ) < 0.8f )
    {
      herring [ i ] . eaten = TRUE ;
      herring [ i ] . time_to_return = fclock->getAbsTime() + 2.0f  ;

      if ( this == kart[0] )
        sound->playSfx ( ( herring[i].type == HE_GREEN ) ?
                          SOUND_UGH : SOUND_BURP ) ;

      switch ( herring [ i ] . type )
      {
        case HE_GREEN  : 
          switch ( rand () % 2 )
          {
            case 0 : attach ( ATTACH_PARACHUTE, 4.0f ) ;

                     if ( this == kart[0] )
                       sound -> playSfx ( SOUND_SHOOMF ) ;
                     break ;

            case 1 : attach ( ATTACH_ANVIL, 2.0f ) ;

                     if ( this == kart[0] )
                       sound -> playSfx ( SOUND_SHOOMF ) ;
                     break ;
          }
          break ;

        case HE_SILVER : num_herring_gobbled++ ; break ;
        case HE_GOLD   : num_herring_gobbled += 3 ; break ;

        case HE_RED   :
	  if ( collectable == COLLECT_NOTHING )
	  {
	    int n = 0 ;

	    switch ( rand () % 5 )
	    {
	      case 0 : n = 1 ; collectable = COLLECT_SPARK          ; break ; 
	      case 1 : n = 1 ; collectable = COLLECT_MISSILE        ; break ;
	      case 2 : n = 1 ; collectable = COLLECT_HOMING_MISSILE ; break ;
	      case 3 : n = 1 ; collectable = COLLECT_ZIPPER         ; break ;
	      case 4 : n = 1 ; collectable = COLLECT_MAGNET         ; break ;
	    }

	    if ( num_collectables < 1 + getNumHerring() /
                                            (MAX_HERRING_EATEN/4) )
	      num_collectables = 1 + getNumHerring() / (MAX_HERRING_EATEN/4) ;
	  }
          break ;
      }

      if ( num_herring_gobbled > MAX_HERRING_EATEN )
        num_herring_gobbled = MAX_HERRING_EATEN ;
    } 
  }
}



void KartDriver::doZipperProcessing ()
{
  if ( zipper_time_left > delta_t )
  {
    zipper_time_left -= delta_t ;  

    if ( velocity.xyz[1] < ZIPPER_VELOCITY )
      velocity.xyz[1] = ZIPPER_VELOCITY ;
  }
  else
    zipper_time_left = 0.0f ;                                                   
}



void KartDriver::forceCrash ()
{
  if ( this == kart[0] )
    sound->playSfx ( SOUND_BONK ) ;

  wheelie_angle = CRASH_PITCH ;

  velocity.xyz[0] = velocity.xyz[1] = velocity.xyz[2] =
    velocity.hpr[0] = velocity.hpr[1] = velocity.hpr[2] = 0.0f ;
}


void KartDriver::doCollisionAnalysis ( float hot )
{
  if ( collided )
  {
    if ( velocity.xyz[1] > MIN_COLLIDE_VELOCITY )
      velocity.xyz[1] -= COLLIDE_BRAKING_RATE * delta_t ;
    else
    if ( velocity.xyz[1] < -MIN_COLLIDE_VELOCITY )
      velocity.xyz[1] += COLLIDE_BRAKING_RATE * delta_t ;
  }

  if ( crashed && velocity.xyz[1] > MIN_CRASH_VELOCITY )
  {
    forceCrash () ;
  }
  else
  if ( wheelie_angle < 0.0f )
  {
    wheelie_angle += PITCH_RESTORE_RATE * delta_t ;

    if ( wheelie_angle >= 0.0f )
      wheelie_angle = 0.0f ;
  }

  if ( on_ground )
  {
    curr_pos.xyz[2] = hot ;
    velocity.xyz[2] = 0.0f ;

    pr_from_normal( curr_pos.hpr, curr_normal ) ;
  }
}

void KartDriver::update ()
{
  if ( rescue )
  {
    rescue = FALSE ;
    attach ( ATTACH_TINYTUX, 4.0f ) ;
  }

  attachment_time_left -= fclock->getDeltaTime () ;

  if ( attachment_time_left <= 0.0f && attachment != NULL )
  {
    if ( getAttachment () == ATTACH_TINYTUX )
    {
      if ( track_hint > 0 )
        track_hint-- ;

      float d = curr_pos.xyz[2] ;

      curr_track -> trackToSpatial ( curr_pos.xyz, track_hint ) ;

      curr_pos.xyz[2] = d ;
    }

    attachment -> select ( 0 ) ;
    attachment_type = ATTACH_NOTHING ;
  }

  if ( getAttachment () == ATTACH_TINYTUX )
  {
    sgZeroVec3 ( velocity.xyz ) ;
    sgZeroVec3 ( velocity.hpr ) ;
    velocity.xyz[2] = 1.1 * GRAVITY * delta_t ;
  }
  else
  if ( getAttachment () == ATTACH_PARACHUTE &&
       velocity.xyz[1] > MAX_PARACHUTE_VELOCITY )
    velocity.xyz[1] = MAX_PARACHUTE_VELOCITY ;
  else
  if ( getAttachment () == ATTACH_ANVIL &&
       velocity.xyz[1] > MAX_ANVIL_VELOCITY )
    velocity.xyz[1] = MAX_ANVIL_VELOCITY ;

  if ( getAttachment () == ATTACH_MAGNET ||
       getAttachment () == ATTACH_MAGNET_BZZT )
  {
    float cdist = SG_MAX ;
    int   closest = -1 ;

    for ( int i = 0 ; i < num_karts ; i++ )
    {
      if ( kart[i] == this ) continue ;

      if ( kart[i]->getDistanceDownTrack() < getDistanceDownTrack() )
        continue ;

      float d = sgDistanceSquaredVec2 ( getCoord()->xyz,
                                     kart[i]->getCoord()->xyz ) ;

      if ( d < cdist && d < MAGNET_RANGE_SQD )
      {
        cdist = d ;
        closest = i ;
      }
    }

    if ( closest != -1 )
    {
      if ( getAttachment () == ATTACH_MAGNET )
      {
        if ( this == kart[0] || closest == 0 )
          sound -> playSfx ( SOUND_BZZT ) ;

        attach ( ATTACH_MAGNET_BZZT,
             attachment_time_left < 4.0f ? 4.0f : attachment_time_left ) ;
      }

      sgVec3 vec ;
      sgSubVec2 ( vec, kart[closest]->getCoord()->xyz, getCoord()->xyz ) ;
      vec [ 2 ] = 0.0f ;
      sgNormalizeVec3 ( vec ) ;

      sgHPRfromVec3 ( getCoord()->hpr, vec ) ;

      float tgt_velocity = kart[closest]->getVelocity()->xyz[1] ;

      if ( cdist > MAGNET_MIN_RANGE_SQD )
      {
        if ( velocity.xyz[1] < tgt_velocity )
          velocity.xyz[1] = tgt_velocity * 1.4 ;
      }
      else
        velocity.xyz[1] = tgt_velocity ;
    }
    else
    if ( getAttachment () == ATTACH_MAGNET_BZZT )
      attach ( ATTACH_MAGNET, attachment_time_left ) ;
  }

  Driver::update () ;
}


KartDriver::KartDriver ( int _position, ssgTransform *m ) : Driver ( m )
{
  grid_position = _position ;
  num_collectables = 0 ;
  num_herring_gobbled = 0 ;
  collectable = COLLECT_NOTHING ;
  attachment = NULL ;
  attachment_time_left = 0.0f ;
  attachment_type = ATTACH_NOTHING ;

  reset_pos.xyz[0] += (float) (grid_position-2) * 2.0f ;
  reset_pos.xyz[1] += 2.0f ;

  reset () ;
}

