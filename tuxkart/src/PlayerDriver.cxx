
#include "tuxkart.h"


int check_hint = 0 ;

void PlayerKartDriver::update ()
{
  KartDriver::update () ;
}


void PlayerKartDriver::incomingJoystick  ( JoyInfo *j )
{
  /*
    Delta-t is faked on slow machines for most activities - but
    for the player's joystick, it has to be the true delta-t.
  */

  float true_delta_t = fclock -> getDeltaTime () ;

  if ( j -> hits & 0x04 )  /* C == Fire */
  {
    if ( collectable == COLLECT_NOTHING )
      sound -> playSfx ( SOUND_BEEP ) ;

    useAttachment () ;
  }

  if ( on_ground )
  {
    if ( ( j -> buttons & 0x20 ) &&
         velocity.xyz[1] >= MIN_WHEELIE_VELOCITY )  /* D == Wheelie */
    {
      if ( wheelie_angle < WHEELIE_PITCH )
        wheelie_angle += WHEELIE_PITCH_RATE * true_delta_t ;
      else
        wheelie_angle = WHEELIE_PITCH ;
    }
    else
    if ( wheelie_angle > 0.0f )
    {
      wheelie_angle -= PITCH_RESTORE_RATE ;

      if ( wheelie_angle <= 0.0f )
        wheelie_angle = 0.0f ;
    }
 
    if ( j -> hits & 0x10 )  /* R == Jump */
      velocity.xyz[2] += JUMP_IMPULSE ;

    if ( j -> hits & 0x08 )  /* D == Unused */
    {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
    }

    if ( j -> buttons & 2 )  /* B == Active Braking */
      velocity.xyz[1] -= MAX_BRAKING * true_delta_t ;
    else
    if ( ( j -> buttons & 1 ) &&
          velocity.xyz[1] < MAX_NATURAL_VELOCITY *
                       ( 1.0f + wheelie_angle/90.0f ) )  /* A == Accellerate */
      velocity.xyz[1] += MAX_ACCELLERATION * true_delta_t ;
    else
    if ( velocity.xyz[1] > MAX_DECELLERATION * true_delta_t )
      velocity.xyz[1] -= MAX_DECELLERATION * true_delta_t ;
    else
    if ( velocity.xyz[1] < -MAX_DECELLERATION * true_delta_t )
      velocity.xyz[1] += MAX_DECELLERATION * true_delta_t ;
    else
      velocity.xyz[1] = 0.0f ;

    if ( wheelie_angle <= 0.0f )
    {
      if ( velocity.xyz[1] >= 0.0f )
        velocity.hpr[0] = -MAX_TURN_RATE * sqrt( velocity.xyz[1])* j->data[0];
      else
        velocity.hpr[0] =  MAX_TURN_RATE * sqrt(-velocity.xyz[1])* j->data[0];
    }
    else
      velocity.hpr[0] = 0.0f ;

    float s = SKID_RATE * velocity.hpr[0] * velocity.xyz[1] ;
    velocity.xyz[0] = (s >= 0) ? (s*s) : -(s*s) ;
  }

  velocity.xyz[2] -= GRAVITY * true_delta_t ;
}


void PlayerKartDriver::incomingKeystroke ( int k )
{
  switch ( k )
  {
    /* CTRL-R ...infinite ammo cheat. */
    case  0x12: switch ( rand () % 5 )
                {
                  case 0 : collectable = COLLECT_SPARK ;
                           num_collectables = 1000000 ;
                           break ;
                  case 1 : collectable = COLLECT_MISSILE ;
                           num_collectables = 1000000 ;
                           break ;
                  case 2 : collectable = COLLECT_HOMING_MISSILE ;
                           num_collectables = 1000000 ;
                           break ;
                  case 3 : collectable = COLLECT_ZIPPER ;
                           num_collectables = 1000000 ;
                           break ;
                  case 4 : collectable = COLLECT_MAGNET ;
                           num_collectables = 1000000 ;
                           break ;
                }
                break ;
  }
}



