//  $Id: KartDriver.cxx,v 1.48 2004/09/05 20:09:58 matzebraun Exp $
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

#include <iostream>
#include "tuxkart.h"
#include "constants.h"
#include "utils.h"
#include "Herring.h"
#include "sound.h"
#include "Loader.h"
#include "Shadow.h"
#include "KartDriver.h"
#include "Projectile.h"
#include "PlayerDriver.h"
#include "SkidMark.h"
#include "World.h"
#include "Track.h"
#include "material.h"
#include "Config.h"

KartParticleSystem::KartParticleSystem(KartDriver* kart_, 
                                       int num, float _create_rate, int _ttf,
                                       float sz, float bsphere_size)                                       
  : ParticleSystem (num, _create_rate, _ttf, sz, bsphere_size),
    kart(kart_)
{
  getBSphere () -> setCenter ( 0, 0, 0 ) ;
  getBSphere () -> setRadius ( 1000.0f ) ;
  dirtyBSphere();
}

void
KartParticleSystem::update ( float t ) 
{
#if 0
      std::cout << "BSphere: r:" << getBSphere()->radius 
                << " ("  << getBSphere()->center[0]
                << ", "  << getBSphere()->center[1]
                << ", "  << getBSphere()->center[2]
                << ")"
                << std::endl;
#endif
  getBSphere () -> setRadius ( 1000.0f ) ;
  ParticleSystem::update(t);
}

void
KartParticleSystem::particle_create(int, Particle *p)
{
  sgSetVec4 ( p -> col, 1, 1, 1, 1 ) ; /* initially white */
  sgSetVec3 ( p -> pos, 0, 0, 0 ) ;    /* start off on the ground */
  sgSetVec3 ( p -> vel, 0, 0, 0 ) ;
  sgSetVec3 ( p -> acc, 0, 0, 2.0f ) ; /* Gravity */
  p -> size = .5f;
  p -> time_to_live = 0.5 ;            /* Droplets evaporate after 5 seconds */
  
  const sgCoord* pos = kart->getVisiCoord ();
  const sgCoord* vel = kart->getVelocity ();
   
  float xDirection = sgCos (pos->hpr[0] - 90.0f); // Point at the rear 
  float yDirection = sgSin (pos->hpr[0] - 90.0f); // Point at the rear
  
  sgCopyVec3 (p->pos, pos->xyz);
  
  p->pos[0] += xDirection * 0.7;
  p->pos[1] += yDirection * 0.7;
  
  float abs_vel = sqrt((vel->xyz[0] * vel->xyz[0]) + (vel->xyz[1] * vel->xyz[1]));
  
  p->vel[0] = xDirection * -abs_vel/2;
  p->vel[1] = yDirection * -abs_vel/2;
  
  p->vel[0] += sgCos (rand()%180) * 1;
  p->vel[1] += sgSin (rand()%180) * 1;
  p->vel[2] += sgSin (rand()%100) * 1;

  getBSphere () -> setCenter ( pos->xyz[0], pos->xyz[1], pos->xyz[2] ) ;
}

void
KartParticleSystem::particle_update (float delta, int, Particle * particle)
{
  particle->size    += delta*2.0f;
  particle->col[3]  -= delta * 2.0f;
  
  particle->pos[0] += particle->vel[0] * delta;
  particle->pos[1] += particle->vel[1] * delta;
  particle->pos[2] += particle->vel[2] * delta;
}

void
KartParticleSystem::particle_delete (int , Particle* )
{
}

KartDriver::KartDriver (const KartProperties* kart_properties_,
    int position_ , Controller* driver_ ) 
  : Driver(kart_properties_)
{
  grid_position        = position_ ;
  num_collectables     = 0 ;
  num_herring_gobbled  = 0 ;
  collectable          = COLLECT_NOTHING ;
  powersliding	       = false;
  attachment           = NULL ;
  attachment_time_left = 0.0f ;
  attachment_type      = ATTACH_NOTHING ;
  smokepuff	       = NULL;
  smoke_system	       = NULL;
  exhaust_pipe         = NULL;
  skidmark_left        = NULL;
  skidmark_right       = NULL;

  driver               = driver_;
  if (driver)
    driver->setKart(this);

  wheel_position = 0;

  wheel_front_l = NULL;
  wheel_front_r = NULL;
  wheel_rear_l  = NULL;
  wheel_rear_r  = NULL;
}

KartDriver::~KartDriver()
{
  delete driver;
  delete smokepuff;

  ssgDeRefDelete(wheel_front_l);
  ssgDeRefDelete(wheel_front_r);
  ssgDeRefDelete(wheel_rear_l);
  ssgDeRefDelete(wheel_rear_r);

  ssgDeRefDelete(skidmark_left);
  ssgDeRefDelete(skidmark_right);
}

void
KartDriver::useAttachment ()
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

	World::current()->projectile[m]->fire ( this, collectable ) ;
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
    race_lap++ ;
  else
  if ( curr_track_coords[1] > 100.0f &&
       last_track_coords[1] <  20.0f )
    race_lap-- ;
}


void KartDriver::doObjectInteractions ()
{
  int i;
  for ( i = 0 ; i < grid_position ; i++ )
  {
    if ( i == grid_position )
      continue ;

    sgVec3 xyz ;

    sgSubVec3 ( xyz, getCoord()->xyz, World::current()->getKart(i) -> getCoord () -> xyz ) ;

    if ( sgLengthSquaredVec2 ( xyz ) < 1.0f )
    {
      if ( this == World::current()->getPlayerKart(0) || i == 0 )
        sound->playSfx ( SOUND_OW ) ;

      sgNormalizeVec2 ( xyz ) ;

      if ( velocity.xyz[1] > World::current()->getKart(i)->getVelocity()->xyz[1] )
      {
        forceCrash () ;
        sgSubVec2 ( World::current()->getKart(i)->getCoord()->xyz, xyz ) ;
      }
      else
      {
        World::current()->getKart(i)->forceCrash () ;
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
      herring [ i ] . time_to_return = World::current()->clock + 2.0f  ;

      if ( this == World::current()->getPlayerKart(0) )
        sound->playSfx ( ( herring[i].type == HE_GREEN ) ?
                          SOUND_UGH : SOUND_BURP ) ;

      switch ( herring [ i ] . type )
      {
        case HE_GREEN  : 
          switch ( rand () % 2 )
          {
            case 0 : attach ( ATTACH_PARACHUTE, 4.0f ) ;

                     // if ( this == kart[0] )
                     //   sound -> playSfx ( SOUND_SHOOMF ) ;
                     break ;

            case 1 : attach ( ATTACH_ANVIL, 2.0f ) ;

                     // if ( this == kart[0] )
                     //   sound -> playSfx ( SOUND_SHOOMF ) ;
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



void KartDriver::doZipperProcessing ( float delta)
{
  if ( zipper_time_left > delta )
  {
    zipper_time_left -= delta ;  

    if ( velocity.xyz[1] < ZIPPER_VELOCITY )
      velocity.xyz[1] = ZIPPER_VELOCITY ;
  }
  else
    zipper_time_left = 0.0f ;                                                   
}



void KartDriver::forceCrash ()
{
  if ( this == World::current()->getPlayerKart(0) )
    sound->playSfx ( SOUND_BONK ) ;

  wheelie_angle = CRASH_PITCH ;

  velocity.xyz[0] = velocity.xyz[1] = velocity.xyz[2] =
    velocity.hpr[0] = velocity.hpr[1] = velocity.hpr[2] = 0.0f ;
}

void KartDriver::beginPowerslide ()
{
  // deactivated for now... kart_properties shouldn't be changed, rather add
  // more values to Driver or so...
#if 0
  if (!powersliding) {
    kart_properties.corn_f *= 20;
    kart_properties.corn_r /= 5;
    kart_properties.max_grip /= 1.5;
    //kart_properties.max_wheel_turn = M_PI;
    kart_properties.inertia *= 1;
		
    powersliding = true;
  }
#endif
}

void KartDriver::endPowerslide ()
{	
#if 0
  if (powersliding) {
    kart_properties.corn_f /= 20;
    kart_properties.corn_r *= 5;
    kart_properties.max_grip *= 1.5;
    //kart_properties.max_wheel_turn = M_PI/2;
    kart_properties.inertia /= 1;
    
    powersliding = false;
  }
#endif
}


void KartDriver::doCollisionAnalysis ( float delta, float hot )
{
  if ( collided )
    {
      if ( velocity.xyz[1] > MIN_COLLIDE_VELOCITY )
        {
          velocity.xyz[1] -= COLLIDE_BRAKING_RATE * delta ;
        }
      else if ( velocity.xyz[1] < -MIN_COLLIDE_VELOCITY )
        {
          velocity.xyz[1] += COLLIDE_BRAKING_RATE * delta ;
        }
    }

  if ( crashed && velocity.xyz[1] > MIN_CRASH_VELOCITY )
    {
      forceCrash () ;
    }
  else if ( wheelie_angle < 0.0f )
    {
      wheelie_angle += PITCH_RESTORE_RATE * delta ;

      if ( wheelie_angle >= 0.0f )
        wheelie_angle = 0.0f ;
    }

  if ( is_on_ground() )
    {
      position.xyz[2] = hot ;
      velocity.xyz[2] = 0.0f ;

      pr_from_normal( position.hpr, curr_normal ) ;
    }
}

void
KartDriver::update (float delta)
{
  sgCoord temp;
  sgCopyCoord(&temp, &position);
    
  if (driver)
    driver->update();
  
  //std::cout << on_ground << " " 
  //<< velocity.xyz[0] << ", " << velocity.xyz[1] << ", " << velocity.xyz[2] << std::endl;
  wheel_position += sgLengthVec3(velocity.xyz) * delta;

  if ( rescue )
  {
    rescue = FALSE ;
    attach ( ATTACH_TINYTUX, 4.0f ) ;
  }

  attachment_time_left -= World::current()->clock ;

  if ( attachment_time_left <= 0.0f && attachment != NULL )
  {
    if ( getAttachment () == ATTACH_TINYTUX )
    {
      if ( track_hint > 0 )
        track_hint-- ;

      float d = position.xyz[2] ;

      World::current() ->track -> trackToSpatial ( position.xyz, track_hint ) ;

      position.xyz[2] = d ;
    }

    attachment -> select ( 0 ) ;
    attachment_type = ATTACH_NOTHING ;
  }

  processAttachments(delta);

  /*smoke drawing control point*/
  if ( config.smoke )
  {
    if (smoke_system != NULL)
      smoke_system->update (delta);
  }

  Driver::update (delta) ;
  processInput(delta);
  processSkidMarks();


  // Lock the vehicle in its start position until the race has
  // really started
  if (World::current()->getPhase() == World::START_PHASE)
    {
      sgCopyCoord(&position, &temp);
      sgCopyCoord(&last_pos, &temp);
      sgCopyCoord(&last_relax_pos, &temp);
    }

  
}

void
KartDriver::processSkidMarks()
{
  if (skidmark_left && (velocity.hpr[0] > 20.0f || velocity.hpr[0] < -20))
    {
      float angle  = -43;
      float length = 0.57;

      sgCoord wheelpos;
      sgCopyCoord(&wheelpos, getVisiCoord());

      wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
      wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);

      if (skidmark_left->newSkidmark)
        {
	  skidmark_left->addBreak (&wheelpos);
	  skidmark_left->newSkidmark--;
	} else
          skidmark_left->add(&wheelpos);
    } 
  else if (skidmark_left)
    {
      skidmark_left->newSkidmark = 2;
    }

  if (skidmark_right && (velocity.hpr[0] > 20.0f || velocity.hpr[0] < -20))
    {
      float angle  = 43;
      float length = 0.57;

      sgCoord wheelpos;
      sgCopyCoord(&wheelpos, getVisiCoord());

      wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
      wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
      
      if (skidmark_right->newSkidmark)
        {
	  skidmark_right->addBreak (&wheelpos);
	  skidmark_right->newSkidmark--;
	} else
          skidmark_right->add(&wheelpos);
    }
  else if (skidmark_right)
    { 
      skidmark_right->newSkidmark = 2;
    }
}

void
KartDriver::processInput(float delta)
{
  if ( controlls.fire ) 
    {
      if ( getCollectable() == COLLECT_NOTHING ) 
         beginPowerslide ();  
        //sound -> playSfx ( SOUND_BEEP ) ;

      useAttachment () ;      
    } 
  else
      endPowerslide ();
      
  if ( ( controlls.wheelie ) &&
       getVelocity()->xyz[1] >= MIN_WHEELIE_VELOCITY ) 
    {
      if ( wheelie_angle < WHEELIE_PITCH )
        wheelie_angle += WHEELIE_PITCH_RATE * delta ;
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

  if ( controlls.jump )
    getVelocity()->xyz[2] += JUMP_IMPULSE ;

  if ( controlls.rescue )
    {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
    }

  if ( controlls.accel ) {
    throttle = kart_properties->max_throttle;
  } else if (throttle > 0) {
    throttle -= kart_properties->max_throttle * delta;
  } else
    throttle = 0.0f;

  if ( controlls.brake ) {  
    if (getVelocity()->xyz[1] > 0) {
      brake = kart_properties->max_throttle/2;
      throttle = 0.0f;
    } else {
      brake = 0.0f;
      throttle = -kart_properties->max_throttle/2;
    }
  } else {
    brake = 0.0f;
  }
  
  if ( wheelie_angle <= 0.0f ) {      
    steer_angle = -kart_properties->turn_speed * controlls.lr;
  
    if ( steer_angle > kart_properties->max_wheel_turn)
      steer_angle = kart_properties->max_wheel_turn;
    if ( steer_angle < -kart_properties->max_wheel_turn)
      steer_angle = -kart_properties->max_wheel_turn;	
  }
  else
    getVelocity()->hpr[0] = 0.0f ;

  // XXX this should be handled in physicsUpdate or when doing CD...
  //force[2] = -GRAVITY * kart_properties.mass;
}

void print_model(ssgEntity* entity, int indent)
{
  if (entity)
    {
      for(int i = 0; i < indent; ++i)
        std::cout << "  ";

      std::cout << entity->getTypeName() << " " << entity->getType() << " '" 
                << entity->getPrintableName() 
                << "' '" 
                << (entity->getName() ? entity->getName() : "null")
                << "' " << int(entity) << std::endl;
  
      ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);
  
      if (branch)
        {
          for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid())
            {
              print_model(i, indent + 1);
            }
        }
    }
}

static ssgTransform* add_transform(ssgBranch* branch)
{
  if (!branch)
    return 0;
  
  //std::cout << "1 BEGIN: " << std::endl;
  //print_model(branch, 0);
  //std::cout << "1 END: " << std::endl;

  ssgTransform* transform = new ssgTransform;
  transform->ref();
  for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid())
    {
      transform->addKid(i);
    }

  branch->removeAllKids();
  branch->addKid(transform);

  // Set some user data, so that the wheel isn't ssgFlatten()'ed
  branch->setUserData(new ssgBase());
  transform->setUserData(new ssgBase());

  //std::cout << "2 BEGIN: " << std::endl;
  //print_model(branch, 0);
  //std::cout << "2 END: " << std::endl;

  return transform;
}

void
KartDriver::load_wheels(ssgBranch* branch)
{
  if (branch)
    {
      for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid())
        {
          if (i->getName())
            { // We found something that might be a wheel
              if (strcmp(i->getName(), "WheelFront.L") == 0)
                {
                  wheel_front_l = add_transform(dynamic_cast<ssgTransform*>(i));
                }
              else if (strcmp(i->getName(), "WheelFront.R") == 0)
                {
                  wheel_front_r = add_transform(dynamic_cast<ssgTransform*>(i));
                }
              else if (strcmp(i->getName(), "WheelRear.L") == 0) 
                {
                  wheel_rear_l = add_transform(dynamic_cast<ssgTransform*>(i));
                }
              else if (strcmp(i->getName(), "WheelRear.R") == 0)
                {
                  wheel_rear_r = add_transform(dynamic_cast<ssgTransform*>(i));
                }
              else
                {
                  // Wasn't a wheel, continue searching
                  load_wheels(dynamic_cast<ssgBranch*>(i));
                }
            }
          else
            { // Can't be a wheel,continue searching
              load_wheels(dynamic_cast<ssgBranch*>(i));
            }
        }
    }
}

void
KartDriver::load_data()
{
  const char *tinytux_file   = "tinytux_magnet.ac" ;
  const char *parachute_file = "parachute.ac"  ;
  const char *magnet_file    = "magnet.ac"     ;
  const char *magnet2_file   = "magnetbzzt.ac" ;
  const char *anvil_file     = "anvil.ac"      ;

  float r [ 2 ] = { -10.0f, 100.0f } ;
  
  smokepuff = new ssgSimpleState ();
  smokepuff -> setTexture        (loader->createTexture ("smoke.png", true, true, true)) ;
  smokepuff -> setTranslucent    () ;
  smokepuff -> enable            ( GL_TEXTURE_2D ) ;
  smokepuff -> setShadeModel     ( GL_SMOOTH ) ;
  smokepuff -> enable            ( GL_CULL_FACE ) ;
  smokepuff -> enable            ( GL_BLEND ) ;
  smokepuff -> enable            ( GL_LIGHTING ) ;
  smokepuff -> setColourMaterial ( GL_EMISSION ) ;
  smokepuff -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  smokepuff -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  smokepuff -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  smokepuff -> setShininess      (  0 ) ;

  ssgEntity *obj = kart_properties->getModel();

  //if (kart_properties.model_file == "mrcube.ac")
  //print_model(obj, 0);

  load_wheels(dynamic_cast<ssgBranch*>(obj));

  /*if (kart_properties.model_file == "mrcube.ac")
    {
    std::cout << "WHEELS: " << std::endl;
    print_model(wheel_front_l, 0);
    print_model(wheel_front_r, 0);

    print_model(wheel_rear_l, 0);
    print_model(wheel_rear_r, 0);
    }
  */

  // Optimize the model
  ssgBranch *newobj = new ssgBranch () ;
  newobj -> addKid ( obj ) ;
  ssgFlatten    ( obj ) ;
  ssgStripify   ( newobj ) ;
  obj = newobj;

  /*if (kart_properties.model_file == "mrcube.ac")
    print_model(obj, 0);*/
  
  ssgRangeSelector *lod = new ssgRangeSelector ;

  lod -> addKid ( obj ) ;
  lod -> setRanges ( r, 2 ) ;

  this-> getModel() -> addKid ( lod ) ;
  
  // Attach Particle System
  sgCoord pipe_pos = {{0, 0, .3}, {0, 0, 0}} ;
  smoke_system = new KartParticleSystem(this, 50, 100.0f, TRUE, 0.35f, 1000);
  smoke_system -> init(5);
  smoke_system -> setState (getMaterial ("smoke.png")-> getState() );
  //smoke_system -> setState ( smokepuff ) ;
  exhaust_pipe = new ssgTransform (&pipe_pos);
  exhaust_pipe -> addKid (smoke_system) ;
  comp_model-> addKid (exhaust_pipe) ;
  //World::current()->scene -> addKid (exhaust_pipe);

  // Load data for this kart (bonus objects, the kart model, etc)
  ssgEntity *pobj1 = ssgLoad ( parachute_file, loader ) ;
  ssgEntity *pobj2 = ssgLoad ( magnet_file   , loader ) ;
  ssgEntity *pobj3 = ssgLoad ( magnet2_file  , loader ) ;
  ssgEntity *pobj4 = ssgLoad ( anvil_file    , loader ) ;

  sgCoord cc ;
  sgSetCoord ( &cc, 0, 0, 2, 0, 0, 0 ) ;
  ssgTransform *ttt = new ssgTransform ( & cc ) ;
  ttt -> addKid ( ssgLoad ( tinytux_file  , loader ) ) ;

  sgSetCoord ( &cc, 0, 0, 0, 0, 0, 0 ) ;

#if 0
  // matze: does this have any effect?
  ssgTransform *xxx = new ssgTransform ( & cc ) ;
  xxx -> addKid ( obj ) ;
  obj = xxx ;
#endif

  ssgEntity *pobj5 = ttt ;

  this-> addAttachment ( pobj1 ) ;
  this-> addAttachment ( pobj2 ) ;
  this-> addAttachment ( pobj3 ) ;
  this-> addAttachment ( pobj4 ) ;
  this-> addAttachment ( pobj5 ) ;

  if (1)
    {
      skidmark_left  = new SkidMark();
      skidmark_left->ref();
      World::current()->scene->addKid(skidmark_left);
      
      skidmark_right = new SkidMark();
      skidmark_right->ref();
      World::current()->scene->addKid(skidmark_right);
    }

  shadow = createShadow(kart_properties->shadow_file, -1, 1, -1, 1);
  shadow->ref();
  comp_model->addKid ( shadow );
}

void
KartDriver::placeModel ()
{
  sgMat4 wheel_front;
  sgMat4 wheel_steer;
  sgMat4 wheel_rot;
  
  sgMakeRotMat4( wheel_rot, 0, -wheel_position, 0);
  sgMakeRotMat4( wheel_steer, getSteerAngle()/M_PI * 180.0f * 0.25f, 0, 0);

  sgMultMat4(wheel_front, wheel_steer, wheel_rot);

  if (wheel_front_l) wheel_front_l->setTransform(wheel_front);
  if (wheel_front_r) wheel_front_r->setTransform(wheel_front);
  
  if (wheel_rear_l) wheel_rear_l->setTransform(wheel_rot);
  if (wheel_rear_r) wheel_rear_r->setTransform(wheel_rot);
  
  //printf ("meep: %f\n", position.xyz[0]);
  //exhaust_pipe -> setTransform (position.xyz);

  Driver::placeModel();
}

void 
KartDriver::processAttachments(float delta)
{
  if ( getAttachment () == ATTACH_TINYTUX )
    {
      sgZeroVec3 ( velocity.xyz ) ;
      sgZeroVec3 ( velocity.hpr ) ;
      velocity.xyz[2] = 1.1 * GRAVITY * delta ;
    }
  else if ( getAttachment () == ATTACH_PARACHUTE &&
            velocity.xyz[1] > MAX_PARACHUTE_VELOCITY )
    {
      velocity.xyz[1] = MAX_PARACHUTE_VELOCITY ;
    }
  else if ( getAttachment () == ATTACH_ANVIL &&
            velocity.xyz[1] > MAX_ANVIL_VELOCITY )
    {
      velocity.xyz[1] = MAX_ANVIL_VELOCITY ;
    }

  if ( getAttachment () == ATTACH_MAGNET ||
       getAttachment () == ATTACH_MAGNET_BZZT )
    {
      float cdist = SG_MAX ;
      int   closest = -1 ;

      for ( int i = 0; i < World::current()->getNumKarts() ; ++i )
        {
          if ( World::current()->getKart(i) == this ) continue ;

          if ( World::current()->getKart(i)->getDistanceDownTrack() < getDistanceDownTrack() )
            continue ;

          float d = sgDistanceSquaredVec2 ( getCoord()->xyz,
                                            World::current()->getKart(i)->getCoord()->xyz ) ;

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
              if ( this == World::current()->getKart(0) || closest == 0 )
                sound -> playSfx ( SOUND_BZZT ) ;

              attach ( ATTACH_MAGNET_BZZT,
                       attachment_time_left < 4.0f ? 4.0f : attachment_time_left ) ;
            }

          sgVec3 vec ;
          sgSubVec2 ( vec, World::current()->getKart(closest)->getCoord()->xyz, getCoord()->xyz ) ;
          vec [ 2 ] = 0.0f ;
          sgNormalizeVec3 ( vec ) ;

          sgHPRfromVec3 ( getCoord()->hpr, vec ) ;

          float tgt_velocity = World::current()->getKart(closest)->getVelocity()->xyz[1] ;

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
}

/* EOF */
