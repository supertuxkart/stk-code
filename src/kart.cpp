//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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
#include <plib/ssg.h>

#include "herring_manager.hpp"
#include "sound.hpp"
#include "loader.hpp"
#include "skid_mark.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "world.hpp"
#include "kart.hpp"

static ssgTransform* add_transform(ssgBranch* branch);

// =============================================================================
KartParticleSystem::KartParticleSystem(Kart* kart_,
                                       int num, float _create_rate, int _ttf,
                                       float sz, float bsphere_size)
  : ParticleSystem (num, _create_rate, _ttf, sz, bsphere_size),
    kart(kart_) {
  getBSphere () -> setCenter ( 0, 0, 0 ) ;
  getBSphere () -> setRadius ( 1000.0f ) ;
  dirtyBSphere();
}   // KartParticleSystem
 
// -----------------------------------------------------------------------------
void KartParticleSystem::update ( float t ) {
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
}   // update

// -----------------------------------------------------------------------------
void KartParticleSystem::particle_create(int, Particle *p) {
  sgSetVec4 ( p -> col, 1, 1, 1, 1 ) ; /* initially white */
  sgSetVec3 ( p -> pos, 0, 0, 0 ) ;    /* start off on the ground */
  sgSetVec3 ( p -> vel, 0, 0, 0 ) ;
  sgSetVec3 ( p -> acc, 0, 0, 2.0f ) ; /* Gravity */
  p -> size = .5f;
  p -> time_to_live = 0.5 ;            /* Droplets evaporate after 5 seconds */
  
  const sgCoord* pos = kart->getCoord();
  const sgCoord* vel = kart->getVelocity();
  
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
}   // particle_create

// -----------------------------------------------------------------------------
void KartParticleSystem::particle_update (float delta, int, 
                                          Particle * particle) {
  particle->size    += delta*2.0f;
  particle->col[3]  -= delta * 2.0f;
  
  particle->pos[0] += particle->vel[0] * delta;
  particle->pos[1] += particle->vel[1] * delta;
  particle->pos[2] += particle->vel[2] * delta;
}  // particle_update

// -----------------------------------------------------------------------------
void KartParticleSystem::particle_delete (int , Particle* ) {
}   // particle_delete

// =============================================================================
Kart::Kart (const KartProperties* kartProperties_, int position_ ) 
  : Moveable(true), attachment(this), collectable(this) {
  kartProperties       = kartProperties_;
  grid_position        = position_ ;
  num_herring_gobbled  = 0;
  finishedRace         = false;
  finishTime           = 0.0f;
  prevAccel            = 0.0f;
  powersliding         = false;
  smokepuff                = NULL;
  smoke_system         = NULL;
  exhaust_pipe         = NULL;
  skidmark_left        = NULL;
  skidmark_right       = NULL;
  // Neglecting the roll resistance (which is small for high speeds compared
  // to the air resistance), maximum speed is reached when the engine
  // power equals the air resistance force, resulting in this formula:
  maxSpeed             = sqrt(getMaxPower()/getAirResistance());

  wheel_position = 0;

  wheel_front_l = NULL;
  wheel_front_r = NULL;
  wheel_rear_l  = NULL;
  wheel_rear_r  = NULL;
}   // Kart

// -----------------------------------------------------------------------------
Kart::~Kart() {
  delete smokepuff;

  ssgDeRefDelete(wheel_front_l);
  ssgDeRefDelete(wheel_front_r);
  ssgDeRefDelete(wheel_rear_l);
  ssgDeRefDelete(wheel_rear_r);

  delete skidmark_left;
  delete skidmark_right;
}   // ~Kart

// -----------------------------------------------------------------------------
void Kart::reset() {
  Moveable::reset();

  raceLap             = -1;
  racePosition        = 9;
  finishedRace        = false;
  finishTime          = 0.0f;
  ZipperTimeLeft      = 0.0f ;
  rescue              = FALSE;
  attachment.clear();
  collectable.clear();
  num_herring_gobbled = 0;
  wheel_position      = 0;
  trackHint = 0;
  world -> track -> absSpatialToTrack(curr_track_coords,
                                                  curr_pos.xyz);
}   // reset

// -----------------------------------------------------------------------------
void Kart::handleZipper() {
  wheelie_angle  = ZIPPER_ANGLE;
  ZipperTimeLeft = ZIPPER_TIME;
}   // handleZipper

// -----------------------------------------------------------------------------
void Kart::doLapCounting () {
  if (      last_track_coords[1] > 100.0f && curr_track_coords[1] <  20.0f )
    raceLap++ ;
  else if ( curr_track_coords[1] > 100.0f && last_track_coords[1] <  20.0f )
      raceLap-- ;
}   // doLapCounting

// -----------------------------------------------------------------------------
void Kart::doObjectInteractions () {
  int i;
  for ( i = 0 ; i < grid_position ; i++ ) {

    sgVec3 xyz ;

    sgSubVec3(xyz, getCoord()->xyz, world->getKart(i)->getCoord()->xyz );

    if ( sgLengthSquaredVec2 ( xyz ) < 1.0f ) {
      sgNormalizeVec2 ( xyz ) ;

      if ( velocity.xyz[1] > world->getKart(i)->getVelocity()->xyz[1] ) {
        forceCrash () ;
        sgSubVec2 ( world->getKart(i)->getCoord()->xyz, xyz ) ;
      } else {
        world->getKart(i)->forceCrash () ;
        sgAddVec2 ( getCoord()->xyz, xyz ) ;
      }
    }   // if sgLengthSquaredVec2(xy)<1.0
  }   // for i

  // Check if any herring was hit.
  herring_manager->hitHerring(this);
}   // doObjectInteractions

// -----------------------------------------------------------------------------
void Kart::collectedHerring(Herring* herring) {
  herringType type = herring->getType();

  switch (type) {
    case HE_GREEN  : attachment.hitGreenHerring(); break;
    case HE_SILVER : num_herring_gobbled++ ;       break;
    case HE_GOLD   : num_herring_gobbled += 3 ;    break;
    case HE_RED    : int n=1 + 4*getNumHerring() / MAX_HERRING_EATEN;
                     collectable.hitRedHerring(n); break;
  }   // switch type
  if ( num_herring_gobbled > MAX_HERRING_EATEN )
    num_herring_gobbled = MAX_HERRING_EATEN ;
}   // hitHerring
// -----------------------------------------------------------------------------
void Kart::doZipperProcessing (float delta) {
  if ( ZipperTimeLeft > delta ) {
    ZipperTimeLeft -= delta ;
    if ( velocity.xyz[1] < ZIPPER_VELOCITY )
      velocity.xyz[1] = ZIPPER_VELOCITY ;
  } else ZipperTimeLeft = 0.0f ;
}   // doZipperProcessing

// -----------------------------------------------------------------------------
void Kart::forceCrash () {

  wheelie_angle = CRASH_PITCH ;

  velocity.xyz[0] = velocity.xyz[1] = velocity.xyz[2] =
    velocity.hpr[0] = velocity.hpr[1] = velocity.hpr[2] = 0.0f ;
}  // forceCrash

// -----------------------------------------------------------------------------
void Kart::beginPowerslide () {
  // deactivated for now... kartProperties shouldn't be changed, rather add
  // more values to Driver or so...
#if 0
  if (!powersliding) {
    kartProperties.corn_f *= 20;
    kartProperties.corn_r /= 5;
    kartProperties.max_grip /= 1.5;
    //kartProperties.max_wheel_turn = M_PI;
    kartProperties.inertia *= 1;
    
    powersliding = true;
  }
#endif
}   // beginPowerslide

// -----------------------------------------------------------------------------
void Kart::endPowerslide () {   
#if 0
  if (powersliding) {
    kartProperties.corn_f /= 20;
    kartProperties.corn_r *= 5;
    kartProperties.max_grip *= 1.5;
    //kart_properties.max_wheel_turn = M_PI/2;
    kartProperties.inertia /= 1;
    
    powersliding = false;
  }
#endif
}   // endPowerslide

// -----------------------------------------------------------------------------
void Kart::doCollisionAnalysis ( float delta, float hot ) {
  if ( collided ) {
    if ( velocity.xyz[1] > MIN_COLLIDE_VELOCITY ) {
      velocity.xyz[1] -= COLLIDE_BRAKING_RATE * delta ;
    } else if ( velocity.xyz[1] < -MIN_COLLIDE_VELOCITY ) {
      velocity.xyz[1] += COLLIDE_BRAKING_RATE * delta ;
    }
  }   // if collided

  if ( crashed && velocity.xyz[1] > MIN_CRASH_VELOCITY ) {
    forceCrash () ;
  } else if ( wheelie_angle < 0.0f ) {
    wheelie_angle += getWheelieRestoreRate() * delta;
    if ( wheelie_angle >= 0.0f ) wheelie_angle = 0.0f ;
  }
   
  /* Make sure that the car doesn't go through the floor */
  if ( isOnGround() ) {
    velocity.xyz[2] = 0.0f ;
  }   // isOnGround
}   // doCollisionAnalysis

// -----------------------------------------------------------------------------
void Kart::update (float dt) {
  //wheel_position gives the rotation around the X-axis, and since velocity's
  //timeframe is the delta time, we don't have to multiply it with dt.
  wheel_position += velocity.xyz[1];

  if ( rescue ) {
    if(attachment.getType() != ATTACH_TINYTUX)
        attachment.set( ATTACH_TINYTUX, 4.0f ) ;
  }
  attachment.update(dt, &velocity);
   
  /*smoke drawing control point*/
  if ( config->smoke ) {
    if (smoke_system != NULL)
      smoke_system->update (dt);
  }  // config->smoke
  doZipperProcessing (dt) ;
  updatePhysics(dt);

  sgCopyVec2  ( last_track_coords, curr_track_coords );
  Moveable::update (dt) ;
  doObjectInteractions();

  trackHint = world->track->spatialToTrack(
                        curr_track_coords, curr_pos.xyz, trackHint );

  doLapCounting () ;
  processSkidMarks();
  
}   // update

// -----------------------------------------------------------------------------
#define sgn(x) ((x<0)?-1.0f:((x>0)?1.0f:0.0f))
#define max(m,n) ((m)>(n) ? (m) : (n))
#define min(m,n) ((m)<(n) ? (m) : (n))

// -----------------------------------------------------------------------------
void Kart::updatePhysics (float dt) {
  skidFront = skidRear = false;
  sgVec3 AirResistance, SysResistance;
  // Get some values once, to avoid calling them more than once.
  float  gravity     = world->getGravity();
  float  wheelBase   = getWheelBase();
  float  mass        = getMass();         // includes attachment.WeightAdjust
  float  airFriction = getAirResistance(); // includes attachmetn.AirFrictAdjust
  float  rollResist  = getRollResistance();

  //  if(materialHOT) rollResist +=materialHOT->getFriction();
  float throttle;

  if(wheelie_angle>0.0f) {
    velocity.xyz[1]-=getWheelieSpeedBoost()*wheelie_angle/getWheelieMaxPitch();
  }


  // Handle throttle and brakes
  // ==========================
  if(on_ground) {
    if(controls.brake) {
      throttle = velocity.xyz[1]<0.0 ? -1.0f : -getBrakeFactor();
    } else {   // not breaking
	throttle =  controls.accel;
    }
    // Handle wheelies
    // ===============
    if ( controls.wheelie && velocity.xyz[1] >= 
	 getMaxSpeed()*getWheelieMaxSpeedRatio()) {
      velocity.hpr[0]=0.0;
      if ( wheelie_angle < getWheelieMaxPitch() )
        wheelie_angle += getWheeliePitchRate() * dt;
      else
        wheelie_angle = getWheelieMaxPitch();
    } else if ( wheelie_angle > 0.0f ) {
      wheelie_angle -= getWheelieRestoreRate() * dt;
      if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
    }
  } else {   // not on ground
    throttle = 0.0;
  }   // if !on_ground




#ifndef NEW_PHYSICS

  float ForceLong = throttle * getMaxPower();
  // apply air friction and system friction
  AirResistance[0]   = 0.0f;
  AirResistance[1]   = airFriction*velocity.xyz[1]*fabs(velocity.xyz[1]);
  AirResistance[2]   = 0.0f;
  SysResistance[0]   = rollResist*velocity.xyz[0];;
  SysResistance[1]   = rollResist*velocity.xyz[1];
  SysResistance[2]   = 0.0f;
  
  // 
  // Compute longitudinal acceleration for slipping
  // ----------------------------------------------
  float ForceOnRearTire   = 0.5f*mass*gravity + prevAccel*mass*getHeightCOG()/wheelBase;
  float ForceOnFrontTire  =      mass*gravity - ForceOnRearTire;
  float maxGrip           = max(ForceOnRearTire,ForceOnFrontTire)*getTireGrip();

  // If the kart is on ground, modify the grip by the friction 
  // modifier for the texture/terrain.
  if(on_ground && materialHOT) maxGrip *= materialHOT->getFriction();

  // Gravity handling
  // ================
  float ForceGravity;
  if(on_ground) {
    if(normalHOT) {
      // Adjust pitch and roll according to the current terrain. To compute
      // the correspondant angles, we consider first a normalised line 
      // pointing into the direction the kart is facing (pointing from (0,0,0) 
      // to (x,y,0)). The angle between this line and the triangle the kart is 
      // on determines the pitch. Similartly the roll is computed, using a 
      // normalised line pointing to the right of the kart, for which we simply
      // use (-y,x,0).
      float kartAngle = curr_pos.hpr[0]*M_PI/180.0f;
      float x = -sin(kartAngle);
      float y =  cos(kartAngle);
      // Compute the angle between the normal of the plane and the line to 
      // (x,y,0).  (x,y,0) is normalised, so are the coordinates of the plane, 
      // simplifying the computation of the scalar product.
      float pitch = ( (*normalHOT)[0]*x + (*normalHOT)[1]*y );  // use ( x,y,0)
      float roll  = (-(*normalHOT)[0]*y + (*normalHOT)[1]*x );  // use (-y,x,0)

      // The actual angle computed above is between the normal and the (x,y,0) 
      // line, so to compute the actual angles 90 degrees must be subtracted.
      pitch = acosf(pitch)/M_PI*180.0f-90.0f;
      roll  = acosf(roll )/M_PI*180.0f-90.0f;
      // if dt is too big, the relaxation will overshoot, and the
      // karts will either be hopping, or even turn upside down etc.
      if(dt<=0.05) {
#define RELAX(oldVal, newVal) (oldVal + (newVal-oldVal)*dt*20.0f)
	curr_pos.hpr[1] = RELAX(curr_pos.hpr[1],pitch);
	curr_pos.hpr[2] = RELAX(curr_pos.hpr[2],roll );
      } else {
	curr_pos.hpr[1] = pitch;
	curr_pos.hpr[2] = roll ;
      }
      if(fabsf(curr_pos.hpr[1])>fabsf(pitch)) curr_pos.hpr[1]=pitch;
      if(fabsf(curr_pos.hpr[2])>fabsf(roll )) curr_pos.hpr[2]=roll;
    }
    if(controls.jump) { // ignore gravity down when jumping
      ForceGravity = physicsParameters->jumpImpulse*gravity;
    } else {   // kart is on groud and not jumping
      if(config->improvedPhysics) {
	// FIXME:
	// While these physics computation is correct, it is very difficult
	// to drive, esp. the sandtrack: the grades (with the current
	// physics parameters) are too steep, so the kart needs a very high
	// initial velocity to be able to reach the top. Especially the
	// AI gets stuck very easily! Perhaps reduce the forces somewhat?
	float pitch_in_rad = curr_pos.hpr[1]*M_PI/180.0f;
	ForceGravity   = -gravity * mass * cos(pitch_in_rad);
	ForceLong     -=  gravity * mass * sin(pitch_in_rad);
      } else {
	ForceGravity   = -gravity * mass;
      }
    }
  } else {  // kart is not on ground, gravity applies all to z axis.
    ForceGravity = -gravity * mass;
  }
  velocity.xyz[2] += ForceGravity / mass * dt;


  if(wheelie_angle <= 0.0f && on_ground) {
    // At low speed, the advanced turning mode can result in 'flickering', i.e.
    // very quick left/right movements. The reason might be:
    // 1) integration timestep to big
    // 2) the kart turning too much, thus 'oversteering', which then gets
    //    corrected (which might be caused by wrongly tuned physics parameters,
    //    or the too big timestep mentioned above
    // Since at lower speed the simple turning algorithm is good enough, 
    // the advanced sliding turn algorithm is only used at higher speeds.

    // FIXME: for now, only use the simple steering algorithm.
    //        so the test for 'lower speed' is basically disabled for now,
    //        since the velocity will always be lower than 1.5*100000.0f
    if(fabsf(velocity.xyz[1])<150000.0f) {
      float msa       = getMaxSteerAngle();
      velocity.hpr[0] = controls.lr * ((velocity.xyz[1]>=0.0f) ? msa : -msa);
      if(velocity.hpr[0]> msa) velocity.hpr[0] =  msa;  // In case the AI sets
      if(velocity.hpr[0]<-msa) velocity.hpr[0] = -msa;  // controls.lr >1 or <-1
    } else {
      float steer_angle    = controls.lr*getMaxSteerAngle()*M_PI/180.0;
      float TurnDistance   = velocity.hpr[0]*M_PI/180.0f * wheelBase/2.0f;
      float slipAngleFront = atan((velocity.xyz[0]+TurnDistance)
				  /fabsf(velocity.xyz[1]))
                           - sgn(velocity.xyz[1])*steer_angle;
      float slipAngleRear  = atan((velocity.xyz[0]-TurnDistance)
				  /fabsf(velocity.xyz[1]));
      float ForceLatFront  = NormalizedLateralForce(slipAngleFront, getCornerStiffF())
                           * ForceOnFrontTire - SysResistance[0]*0.5;
      float ForceLatRear   = NormalizedLateralForce(slipAngleRear,  getCornerStiffR())
                           * ForceOnRearTire  - SysResistance[0]*0.5;
      float cornerForce    = ForceLatRear + cos(steer_angle)*ForceLatFront;
      velocity.xyz[0]      = cornerForce/mass*dt;
      float torque         =                  ForceLatRear *wheelBase/2
                           - cos(steer_angle)*ForceLatFront*wheelBase/2; 
      float angAcc         = torque/getInertia();

      velocity.hpr[0] += angAcc*dt*180.0f/M_PI;
    }   // fabsf(velocity.xyz[1])<0.5
  }   // wheelie_angle <=0.0f && on_ground

  // Longitudinal accelleration 
  // ==========================
  float effForce  = (ForceLong-AirResistance[1]-SysResistance[1]);
  // Slipping: more force than what can be supported by the back wheels
  // --> reduce the effective force acting on the kart - currently
  //     by an arbitrary value.
  if(fabs(effForce)>maxGrip) {
    //    effForce *= 0.4f;
    //    skidRear  = true;
  }   // while effForce>maxGrip
  float accel       = effForce / mass;

  velocity.xyz[1] += accel           * dt;
  prevAccel        = accel;

#else     // new physics

  // apply air friction and system friction
  SysResistance[0]   = rollResist*velocity_wc[0]*10;
  SysResistance[1]   = rollResist*velocity_wc[1];
  SysResistance[2]   = 0.0f;
  
  // 
  // Compute longitudinal acceleration for slipping
  // ----------------------------------------------
  float ForceOnRearTire   = 0.5f*mass*gravity;// + prevAccel*mass*getHeightCOG()/wheelBase;
  float ForceOnFrontTire  =      mass*gravity - ForceOnRearTire;
  float maxGrip           = max(ForceOnRearTire,ForceOnFrontTire)*getTireGrip();

  // If the kart is on ground, modify the grip by the friction 
  // modifier for the texture/terrain.
  if(on_ground && materialHOT) maxGrip *= materialHOT->getFriction();

  // Longitudinal accelleration 
  // ==========================
  float ForceLong = throttle * getMaxPower();

  // Turning forces
  // ==============
  float ForceLatFront;
  float ForceLatRear;
  float cos_s_a;
  if(wheelie_angle <= 0.0f && on_ground) {
    // At low speed, the advanced turning mode can result in 'flickering', i.e.
    // very quick left/right movements. The reason might be:
    // 1) integration timestep to big
    // 2) the kart turning too much, thus 'oversteering', which then gets
    //    corrected (which might be caused by wrongly tuned physics parameters,
    //    or the too big timestep mentioned above)
    // Since at lower speed the simple turning algorithm is good enough, 
    // the advanced sliding turn algorithm is only used at higher speeds.
    float speed=sqrt(velocity.xyz[0]*velocity.xyz[0]+velocity.xyz[1]*velocity.xyz[1]);
    if(speed<1.5/100000.0f) {
      float steerAngleDegrees = controls.lr * getMaxSteerAngle();
      curr_pos.hpr[0] += steerAngleDegrees*dt;
      if(ForceLong>maxGrip) {
	skidRear=true;
	ForceLong *= 0.4;
      }
      cos_s_a = cos(steerAngleDegrees*M_PI/180.0f);
      ForceLatRear  = 0.0f;
      ForceLatFront = 0.0f;
    } else {   // speed high enough to justify advanced sliding steer algorithm
      float steer_angle    = controls.lr*getMaxSteerAngle()*M_PI/180.0f;
      float TurnDistance   = velocity.hpr[0]*M_PI/180.0f * wheelBase/2.0f;
      float slipAngleFront = atan((velocity.xyz[0]+TurnDistance)
				  /fabsf(velocity.xyz[1]))
                           - sgn(velocity.xyz[1])*steer_angle;
      float slipAngleRear  = atan((velocity.xyz[0]-TurnDistance)
				  /fabsf(velocity.xyz[1]));

#define   PHYSICS_DEBUG

      cos_s_a       = cos(steer_angle);
      ForceLatFront = NormalizedLateralForce(slipAngleFront, getCornerStiffF())
                    * ForceOnFrontTire - SysResistance[0]*0.5;
      ForceLatRear  = NormalizedLateralForce(slipAngleRear,  getCornerStiffR())
                    * ForceOnRearTire  - SysResistance[0]*0.5;
#  ifdef PHYSICS_DEBUG    
      printf("steera % 5.3f saf % 5.3f sar % 5.3f v: % 7.3f , % 7.3f sqsum % 7.3f ", 
	     steer_angle, slipAngleFront, slipAngleRear,
	     velocity.xyz[0],velocity.xyz[1],speed	     );
#  endif
      float torque      = ForceLatRear  * wheelBase/2
	                - ForceLatFront * wheelBase/2 * cos_s_a; 

      float angAcc      = torque/getInertia();
      float rotResist   = 0.0f;
      if(on_ground && materialHOT) rotResist = velocity.hpr[0]*materialHOT->getFriction()*10.0f;
      velocity.hpr[0] += (angAcc*180.0f/M_PI-rotResist)*dt;
      curr_pos.hpr[0] += velocity.hpr[0]*dt;
      
#  ifdef PHYSICS_DEBUG    
      printf(" flo % 6.2f flaf % 8.2f flar % 8.2f skid %1d%1d angAcc % 5.3f v_h[0] % 5.3f p_h[0] % 5.3f td % 5.3f dt %f",ForceLong, 
	     ForceLatFront, ForceLatRear,
	     skidFront, skidRear, angAcc, velocity.hpr[0]*dt, curr_pos.hpr[0],
	     TurnDistance,dt);
#  endif
    }   // fabsf(velocity.xyz[1])<1.5
  } else {   // wheelie_angle <=0.0f && on_ground
    ForceLatRear  = 0.0f;
    ForceLatFront = 0.0f;
    cos_s_a       = 1.0f;
  }

  // Longitudinal accelleration 
  // ==========================
  AirResistance[0]   = airFriction*velocity.xyz[0]*fabs(velocity.xyz[0]);;
  AirResistance[1]   = airFriction*velocity.xyz[1]*fabs(velocity.xyz[1]);
  AirResistance[2]   = 0.0f;
  float accelLong     = (ForceLong-AirResistance[1]) / mass;

  float cornerForce   = ForceLatRear + cos_s_a*ForceLatFront-AirResistance[0]-SysResistance[0];
  float accelLat      = (cornerForce-AirResistance[0])/mass;

  float kartAngle    = curr_pos.hpr[0]*M_PI/180.0f;
  float sinKartAngle = sin(kartAngle);
  float cosKartAngle = cos(kartAngle);
  float aX =  cosKartAngle * accelLat + sinKartAngle * accelLong;
  float aY = -sinKartAngle * accelLat + cosKartAngle * accelLong;

  velocity_wc[0]    += aX * dt;
  velocity_wc[1]    += aY * dt;
  prevAccel          = accelLong;
  printf(" ax % 5.3f ay % 5.3f %d\n", accelLat, accelLong,on_ground);

  // Gravity handling
  // ================
  float GravityForce;
  if(on_ground) {
    if(normalHOT) {
      //      printf("Normal: %f,%f,%f, %f,angl:%f\n", (*normalHOT)[0],
      //      (*normalHOT)[1],(*normalHOT)[2], (*normalHOT)[3], 
      //      this->curr_pos.hpr[0]);
    }
    if(controls.jump) { // ignore gravity down when jumping
      GravityForce = physicsParameters->jumpImpulse*gravity;
    } else {
      GravityForce = -gravity * mass;
    }
  } else {  // kart is not on ground, gravity applies all to z axis.
    GravityForce   = -gravity * mass;
  }
  velocity.xyz[2] += GravityForce / mass * dt;



  // Now transpose the velocity from world coordinates into kart coordinates
  // =====================================================================
  velocity.xyz[0] =  cosKartAngle*velocity_wc[0] - sinKartAngle*velocity_wc[1];
  velocity.xyz[1] =  sinKartAngle*velocity_wc[0] + cosKartAngle*velocity_wc[1];

// -----------------------------------------------------------------------------

#ifdef CAN_BE_DELETED
      // different ways of calculating the slip angles, but they 
      // tend to be less stable
      float AngVelocity = velocity.hpr[0]*M_PI/180.0f;
      float turnSpeed = velocity.xyz[0];
      slipAngleFront  = atan(velocity.xyz[0]/velocity.xyz[1])+AngVelocity*wheelBase/2*dt
                      - sgn(velocity.xyz[1])*steer_angle;
      //    turnSpeed       = velocity.xyz[0]
      slipAngleRear   = atan(velocity.xyz[0]/velocity.xyz[1])-AngVelocity*wheelBase/2*dt;
#endif


#ifdef SLIPPING
      float flo2  = ForceLong        * ForceLong;
      float flf2  = ForceLatFront    * ForceLatFront;
      float foft2 = ForceOnFrontTire * ForceOnFrontTire;
      if( flf2 + flo2 > foft2*10000.0f ) {
	ForceLatFront *= foft2 / (flf2 + flo2);
	ForceLong     *= flo2  / (flf2 + flo2);
	skidFront      = true;
      }
      float flr2  = ForceLatRear    * ForceLatRear;
      float fort2 = ForceOnRearTire * ForceOnRearTire;
      if( flr2   + flo2 > fort2*10000.0f  ) {
	ForceLatRear *= flr2 /(flr2+flo2);
	ForceLong    *= flo2 /(flr2+flo2);
	skidRear      = true;
      }
#endif

#endif

      if(wheelie_angle>0.0f) {
	velocity.xyz[1]+=
	  getWheelieSpeedBoost()*wheelie_angle/getWheelieMaxPitch();
      }
}   // updatePhysics

// -----------------------------------------------------------------------------
// PHORS recommends: f=B*alpha/(1+fabs(A*alpha)^p), where A, B, and p
//                   are appropriately chosen constants.
float Kart::NormalizedLateralForce(float alpha, float corner) const {
  float const maxAlpha=3.14/4;
  if(fabsf(alpha)<maxAlpha) {
    return corner*alpha;
  } else {
    return alpha>0.0f ? corner*maxAlpha : -corner*maxAlpha;
  }
}   // NormalizedLateralForce
// -----------------------------------------------------------------------------
#ifdef NEW_PHYSICS
void Kart::updatePosition(float dt, sgMat4 result) {
  // what should happen here:
  // new_position = old_position + dt*velocity.xyz
  // new_angle    = old_angle    + dt*velocity.angle (????)
  // currently:
  // new_position = old_position + dt*velocity.xyz*curr_pos.alpha
  // new_angle    = old_angle    + dt*velocity.angle

  sgCoord scaled_velocity ;
  /* Scale velocities to current time step. */
  sgScaleVec3(scaled_velocity.xyz, velocity_wc, dt );
  sgCoord result1; sgAddVec3(result1.xyz, curr_pos.xyz, scaled_velocity.xyz);
  sgCopyVec3(result1.hpr,curr_pos.hpr);
  sgMakeCoordMat4(result, &result1);
  return;
  /*
  sgZeroVec3 (scaled_velocity.hpr);
  sgMat4 delta;  sgMakeCoordMat4 (delta, & scaled_velocity );
  sgCoord posit;  sgZeroCoord(&posit);sgCopyVec3(posit.xyz,curr_pos.xyz);
  sgMat4 mat;     sgMakeCoordMat4 (mat  , & posit        );
  sgMultMat4      (result, mat, delta       );
  */

}   // updatePosition
#endif
// -----------------------------------------------------------------------------
void Kart::handleRescue() {
  if ( trackHint > 0 ) trackHint-- ;
  world ->track -> trackToSpatial ( curr_pos.xyz, trackHint ) ;
  curr_pos.hpr[0] = world->track->angle[trackHint] ;
  rescue = FALSE ;
}   // handleRescue

// -----------------------------------------------------------------------------
void Kart::processSkidMarks() {

  assert(skidmark_left);
  assert(skidmark_right);
  
  if(skidRear || skidFront) {
    if(on_ground) {
      const float length = 0.57f;
      if(skidmark_left) {
        const float angle  = -43.0f;
        
        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());
        
        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
        
        if(skidmark_left->wasSkidMarking())
          skidmark_left->add(&wheelpos);
        else
          skidmark_left->addBreak(&wheelpos);
      }   // if skidmark_left

      if(skidmark_right) {
        const float angle  = 43.0f;
        
        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());
        
        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);

        if(skidmark_right->wasSkidMarking())
          skidmark_right->add(&wheelpos);
        else
          skidmark_right->addBreak(&wheelpos);
      }   // if skidmark_right
    } else {   // not on ground
      if(skidmark_left) {
        const float length = 0.57f;
        const float angle  = -43.0f;
        
        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());
        
        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
        
        skidmark_left->addBreak(&wheelpos);
      }   // if skidmark_left

      if(skidmark_right) {
        const float length = 0.57f;
        const float angle  = 43.0f;
        
        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());
        
        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
        
        skidmark_right->addBreak(&wheelpos);
      }   // if skidmark_right
    }   // on ground
  } else {   // !skidRear && !skidFront
    if(skidmark_left)
      if(skidmark_left->wasSkidMarking()) {
        const float angle  = -43.0f;
        const float length = 0.57f;
        
        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());
        
        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
        
        skidmark_left->addBreak(&wheelpos);
      }   // skidmark_left->wasSkidMarking

    if(skidmark_right)
      if(skidmark_right->wasSkidMarking()) {
        const float angle  = 43.0f;
        const float length = 0.57f;

        sgCoord wheelpos;
        sgCopyCoord(&wheelpos, getCoord());

        wheelpos.xyz[0] += length * sgSin(wheelpos.hpr[0] + angle);
        wheelpos.xyz[1] += length * -sgCos(wheelpos.hpr[0] + angle);
        
        skidmark_right->addBreak(&wheelpos);
      }   // skidmark_right->wasSkidMarking
  }   // velocity < 20
}   // processSkidMarks

// -----------------------------------------------------------------------------
void Kart::load_wheels(ssgBranch* branch) {
  if (!branch) return;

  for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid()) {
    if (i->getName()) { // We found something that might be a wheel
      if (strcmp(i->getName(), "WheelFront.L") == 0) {
        wheel_front_l = add_transform(dynamic_cast<ssgTransform*>(i));
      } else if (strcmp(i->getName(), "WheelFront.R") == 0) {
        wheel_front_r = add_transform(dynamic_cast<ssgTransform*>(i));
      } else if (strcmp(i->getName(), "WheelRear.L") == 0) {
        wheel_rear_l = add_transform(dynamic_cast<ssgTransform*>(i));
      } else if (strcmp(i->getName(), "WheelRear.R") == 0) {
        wheel_rear_r = add_transform(dynamic_cast<ssgTransform*>(i));
      }
      else {
        // Wasn't a wheel, continue searching
        load_wheels(dynamic_cast<ssgBranch*>(i));
      }
    } else { // Can't be a wheel,continue searching
      load_wheels(dynamic_cast<ssgBranch*>(i));
    }
  }   // for i
}   // load_wheels

// -----------------------------------------------------------------------------
void Kart::load_data() {
  float r [ 2 ] = { -10.0f, 100.0f } ;

  smokepuff = new ssgSimpleState ();
  smokepuff -> setTexture        (loader->createTexture ("smoke.rgb", true, true, true)) ;
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

  ssgEntity *obj = kartProperties->getModel();

  load_wheels(dynamic_cast<ssgBranch*>(obj));

  // Optimize the model, this can't be done while loading the model
  // because it seems that it removes the name of the wheels or something
  // else needed to load the wheels as a separate object.
  ssgFlatten(obj);

  ssgRangeSelector *lod = new ssgRangeSelector ;

  lod -> addKid ( obj ) ;
  lod -> setRanges ( r, 2 ) ;

  this-> getModel() -> addKid ( lod ) ;

  // Attach Particle System
  //JH  sgCoord pipe_pos = {{0, 0, .3}, {0, 0, 0}} ;
  smoke_system = new KartParticleSystem(this, 50, 100.0f, TRUE, 0.35f, 1000);
  smoke_system -> init(5);
  //JH      smoke_system -> setState (getMaterial ("smoke.png")-> getState() );
  //smoke_system -> setState ( smokepuff ) ;
  //      exhaust_pipe = new ssgTransform (&pipe_pos);
  //      exhaust_pipe -> addKid (smoke_system) ;
  //      comp_model-> addKid (exhaust_pipe) ;

  skidmark_left  = new SkidMark();
  skidmark_right = new SkidMark();

  shadow = createShadow(kartProperties->getShadowFile(), -1, 1, -1, 1);
  shadow->ref();
  model->addKid ( shadow );
}   // load_data

// -----------------------------------------------------------------------------
void Kart::placeModel () {
  sgMat4 wheel_front;
  sgMat4 wheel_steer;
  sgMat4 wheel_rot;

  sgMakeRotMat4( wheel_rot, 0, -wheel_position, 0);
  sgMakeRotMat4( wheel_steer, getSteerAngle()/getMaxSteerAngle() * 30.0f , 0, 0);

  sgMultMat4(wheel_front, wheel_steer, wheel_rot);
 
  if (wheel_front_l) wheel_front_l->setTransform(wheel_front);
  if (wheel_front_r) wheel_front_r->setTransform(wheel_front);
  
  if (wheel_rear_l) wheel_rear_l->setTransform(wheel_rot);
  if (wheel_rear_r) wheel_rear_r->setTransform(wheel_rot);
  // We don't have to call Moveable::placeModel, since it does only setTransform
  sgCoord c ;
  sgCopyCoord ( &c, &curr_pos ) ;
  c.hpr[1] += wheelie_angle ;
  c.xyz[2] += 0.3*fabs(sin(wheelie_angle 
                           *SG_DEGREES_TO_RADIANS));
  model -> setTransform ( & c ) ;

}   // placeModel

// -----------------------------------------------------------------------------
void Kart::getClosestKart(float *cdist, int *closest) {
  *cdist   = SG_MAX ;
  *closest = -1 ;
      
  for ( int i = 0; i < world->getNumKarts() ; ++i ) {
    if ( world->getKart(i) == this ) continue ;
    if ( world->getKart(i)->getDistanceDownTrack() < getDistanceDownTrack() )
      continue ;
         
    float d = sgDistanceSquaredVec2 ( getCoord()->xyz,
                                      world->getKart(i)->getCoord()->xyz ) ;
    if ( d < *cdist && d < physicsParameters->magnetRangeSQ) {
      *cdist = d ;
      *closest = i ;
    }
  }   // for i
}   // getClosestKart

// -----------------------------------------------------------------------------
void Kart::handleMagnet(float cdist, int closest) {

  sgVec3 vec ;
  sgSubVec2 ( vec, world->getKart(closest)->getCoord()->xyz, getCoord()->xyz );
  vec [ 2 ] = 0.0f ;
  sgNormalizeVec3 ( vec ) ;
         
  sgHPRfromVec3 ( getCoord()->hpr, vec ) ;
         
  float tgt_velocity = world->getKart(closest)->getVelocity()->xyz[1] ;
      
  //JH FIXME: that should probably be changes, e.g. by increasing the throttle
  //          to something larger than 1???
  if (cdist > physicsParameters->magnetMinRangeSQ) {
    if ( velocity.xyz[1] < tgt_velocity )
      velocity.xyz[1] = tgt_velocity * 1.4 ;
  } else
    velocity.xyz[1] = tgt_velocity ;
}   // handleMagnet

// =============================================================================
static ssgTransform* add_transform(ssgBranch* branch) {
  if (!branch) return 0;
   
  //std::cout << "1 BEGIN: " << std::endl;
  //print_model(branch, 0);
  //std::cout << "1 END: " << std::endl;
   
  ssgTransform* transform = new ssgTransform;
  transform->ref();
  for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid()) {
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
}   // add_transform

// =============================================================================
void print_model(ssgEntity* entity, int indent, int maxLevel) {
  if(maxLevel <0) return;
  if (entity) {
    for(int i = 0; i < indent; ++i)
      std::cout << "  ";
      
    std::cout << entity->getTypeName() << " " << entity->getType() << " '" 
              << entity->getPrintableName() 
              << "' '" 
              << (entity->getName() ? entity->getName() : "null")
              << "' " << entity << std::endl;
    
    ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);
      
    if (branch) {
      for(ssgEntity* i = branch->getKid(0); i != NULL; 
                     i = branch->getNextKid()) {
        print_model(i, indent + 1, maxLevel-1);
      }
    }   // if branch
  }   // if entity
}   // print_model

// =============================================================================
void Kart::setFinishingState(float time) {
  finishedRace = true;
  finishTime   = time;
}

/* EOF */
