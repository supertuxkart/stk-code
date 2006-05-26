//  $Id: Kart.cxx,v 1.5 2005/09/30 16:46:09 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
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
#include <plib/ssg.h>

#include "HerringManager.h"
#include "sound.h"
#include "Loader.h"
#include "SkidMark.h"
#include "Config.h"
#include "constants.h"
#include "Shadow.h"
#include "Track.h"
#include "World.h"
#include "Kart.h"

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
  finishingPosition    = 9;
  finishingMins        = 0;
  finishingSecs        = 0;
  finishingTenths      = 0;
  prevAccel            = 0.0;
  powersliding	       = false;
  smokepuff	           = NULL;
  smoke_system	       = NULL;
  exhaust_pipe         = NULL;
  skidmark_left        = NULL;
  skidmark_right       = NULL;

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

  raceLap        = -1;
  racePosition   = 9;
  finishedRace         = false;
  finishingPosition    = 9;
  finishingMins        = 0;
  finishingSecs        = 0;
  finishingTenths      = 0;

  ZipperTimeLeft = 0.0f ;
  rescue         = FALSE;
  attachment.clear();
  num_herring_gobbled = 0;
  wheel_position = 0;
  trackHint = world -> track -> absSpatialToTrack(curr_track_coords,
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
    wheelie_angle += PITCH_RESTORE_RATE * delta ;
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
  wheel_position += sgLengthVec3(velocity.xyz);

  if ( rescue ) {
    rescue = FALSE ;
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
  skidding = false;
  sgVec3 force, AirResistance, SysResistance;
  // Get some values once, to avoid calling them more than once.
  float  gravity     = world->getGravity();
  float  wheelBase   = getWheelBase();
  float  mass        = getMass();          // includes attachment.WeightAdjust
  float  airFriction = getAirFriction();   // includes attachmetn.AirFrictAdjust
  float  rollResist  = getRollResistance();

  //  if(materialHOT) rollResist +=materialHOT->getFriction();
  float throttle=0.0f;
  if(on_ground) {
    throttle =  controls.brake ? -getBrakeFactor() : controls.accel;
  }  // if on_ground

  force[1]           = throttle * getMaxPower();
  force[0]           = 0;

  // apply air friction and system friction
  AirResistance[0]   = 0.0f;
  AirResistance[1]   = airFriction*velocity.xyz[1]*fabs(velocity.xyz[1]);
  AirResistance[2]   = 0.0f;
  SysResistance[0]   = rollResist*velocity.xyz[0];;
  SysResistance[1]   = rollResist*velocity.xyz[1];
  SysResistance[2]   = 0.0f;
  
  // 
  if(on_ground) {
    if(normalHOT) {
      //      printf("Normal: %f,%f,%f, %f,angl:%f\n", (*normalHOT)[0],(*normalHOT)[1],(*normalHOT)[2], (*normalHOT)[3], this->curr_pos.hpr[0]);
    }
    if(controls.jump) {
      // ignore gravity down when jumping
      force[2] = physicsParameters->jumpImpulse*gravity;
    } else {
      force[2] = -gravity * mass;
    }
  } else {  // kart is not on ground, gravity applies all to z axis.
    force[2]    = -gravity * mass;
  }
  
  // Compute longitudinal acceleration for slipping
  // ----------------------------------------------
  float effForce          = (force[1]-AirResistance[1]-SysResistance[1]);
  float ForceOnRearTire   = 0.5f*mass*gravity + prevAccel*mass*getHeightCOG()/wheelBase;
  float ForceOnFrontTire  =      mass*gravity - ForceOnRearTire;
  float maxGrip           = max(ForceOnRearTire,ForceOnFrontTire)*getTireGrip();

  // If the kart is on ground, modify the grip by the friction 
  // modifier for the texture/terrain.
  if(on_ground && materialHOT) maxGrip *= materialHOT->getFriction();

#undef PHYSICS_DEBUG

  // Slipping: more force than what can be supported by the back wheels
  // --> reduce the effective force acting on the kart - currently
  //     by an arbitrary value.
  while(fabs(effForce)>maxGrip) {
    effForce *= 0.6f;
    skidding  = true;
  }   // while effForce>maxGrip

  float accel       = effForce / mass;


#undef  ORIGINAL_STEERING
#define NEW_STEERING

#ifdef ORIGINAL_STEERING
  if(wheelie_angle <= 0.0f && on_ground) {
    float msa = getMaxSteerAngle();
    velocity.hpr[0] = msa*controls.lr;
    if(velocity.hpr[0]>msa ) velocity.hpr[0] =  msa;
    if(velocity.hpr[0]<-msa) velocity.hpr[0] = -msa;
  } else {   // wheelie or not on ground
    velocity.hpr[0]=0.0;
  }   // wheelie or not on ground
#endif


#ifdef NEW_STEERING
  float steer_angle    = controls.lr*getMaxSteerAngle()*M_PI/180.0;
  float AngVelocity    = velocity.hpr[0]*M_PI/180.0f;
  float slipAngleFront, slipAngleRear;
  if(fabsf(velocity.xyz[1])<0.01) {
    slipAngleFront = 0.0f;
    slipAngleRear  = 0.0f;
  } else {
#define XX
#ifdef XX
    float turnSpeed = velocity.xyz[0]+AngVelocity*wheelBase/2;
    slipAngleFront  = atan(turnSpeed/velocity.xyz[1])
                    - sgn(velocity.xyz[1])*steer_angle;
    turnSpeed       = velocity.xyz[0]-AngVelocity*wheelBase/2;
    slipAngleRear   = atan(turnSpeed/velocity.xyz[1]);
#else
    float turnSpeed = velocity.xyz[0];
    slipAngleFront  = atan(velocity.xyz[0]/velocity.xyz[1])+AngVelocity*wheelBase/2*dt
                    - sgn(velocity.xyz[1])*steer_angle;
		    //    turnSpeed       = velocity.xyz[0]
    slipAngleRear   = atan(velocity.xyz[0]/velocity.xyz[1])-AngVelocity*wheelBase/2*dt;
#endif
  }
#undef   PHYSICS_DEBUG
#  ifdef PHYSICS_DEBUG    
  printf("%f<>%f<>%fv[0]= %f v[1]= %f sa= %f saf= %f sar= %f ", 
	 atan(velocity.xyz[0]/velocity.xyz[1])+AngVelocity*wheelBase/2*dt
	 - sgn(velocity.xyz[1])*steer_angle,
	 atan(velocity.xyz[0]/velocity.xyz[1])+AngVelocity/2.0f
	 - sgn(velocity.xyz[1])*steer_angle,
	 atan((velocity.xyz[0]+AngVelocity*wheelBase/2)/velocity.xyz[1])
	 - sgn(velocity.xyz[1])*steer_angle,
	 velocity.xyz[0],
	 velocity.xyz[1], steer_angle, slipAngleFront, slipAngleRear);
#  endif

  float ForceLatFront  = NormalizedLateralForce(slipAngleFront, getCornerStiffF())
    * ForceOnFrontTire - SysResistance[0]*0.5;
  float ForceLatRear   = NormalizedLateralForce(slipAngleRear,  getCornerStiffR())
    * ForceOnRearTire  - SysResistance[0]*0.5;
  float cornerForce    = ForceLatRear + cos(steer_angle)*ForceLatFront;
  velocity.xyz[0]      = cornerForce/mass*dt;
  float torque         =                  ForceLatRear *wheelBase/2
                       - cos(steer_angle)*ForceLatFront*wheelBase/2; 
  float angAcc         = torque/getInertia();
  // A certain percentage (which should probably depend on tire grip, road 
  // surface etc) gets lost. This is necessary to stop the kart from 
  // rotating when going in a straight line after turning.
  float grip = getTireGrip();
  if(on_ground&&materialHOT) grip *= materialHOT->getFriction();
   
#  ifdef PHYSICS_DEBUG    
  printf("fF %f rF %f t %f\n",ForceLatFront, ForceLatRear, torque);
#  endif

  velocity.hpr[0] += angAcc*dt*180.0f/M_PI;
#endif

  // 'Integrate' accelleration to get speed
  //  velocity.xyz[0] = 0.0;
  velocity.xyz[1] += accel           * dt;
  velocity.xyz[2] += force[2] / mass * dt;
  prevAccel         = accel;

}   // updatePhysics

// -----------------------------------------------------------------------------
float Kart::NormalizedLateralForce(float alpha, float corner) const {
  float const maxAlpha=3.14/4;
  if(fabsf(alpha)<maxAlpha) {
    return corner*alpha;
  } else {
    return alpha>0.0f ? corner*maxAlpha : -corner*maxAlpha;
  }
}   // NormalizedLateralForce
// -----------------------------------------------------------------------------
void Kart::handleRescue() {
  if ( trackHint > 0 ) trackHint-- ;
  float d = curr_pos.xyz[2] ;
  world ->track -> trackToSpatial ( curr_pos.xyz, trackHint ) ;
  curr_pos.xyz[2] = d ;
}   // handleRescue

// -----------------------------------------------------------------------------
void Kart::processSkidMarks() {

  assert(skidmark_left);
  assert(skidmark_right);
  
  if(skidding) {
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
  } else {   // !skidding
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

  // Optimize the model
  ssgBranch *newobj = new ssgBranch () ;
  newobj -> addKid ( obj ) ;
  ssgFlatten    ( obj ) ;
  ssgStripify   ( newobj ) ;
  obj = newobj;

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

  shadow = createShadow(kartProperties->shadow_file, -1, 1, -1, 1);
  shadow->ref();
  model->addKid ( shadow );
}   // load_data

// -----------------------------------------------------------------------------
void Kart::placeModel () {
  sgMat4 wheel_front;
  sgMat4 wheel_steer;
  sgMat4 wheel_rot;

  sgMakeRotMat4( wheel_rot, 0, -wheel_position, 0);
  sgMakeRotMat4( wheel_steer, getSteerAngle()/M_PI , 0, 0);

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
void Kart::setFinishingState (int pos, float time)
{
    finishedRace = true;

    finishingPosition = pos;

    finishingMins = (int) floor ( time / 60.0 ) ;
    finishingSecs = (int) floor ( time - (double) ( 60 * finishingMins ) ) ;
    finishingTenths = (int) floor ( 10.0f * (time - (double)(finishingSecs +
        60*finishingMins)));
}

/* EOF */
