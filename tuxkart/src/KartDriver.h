//  $Id: KartDriver.h,v 1.14 2004/09/24 15:45:02 matzebraun Exp $
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

#ifndef HEADER_KARTDRIVER_H
#define HEADER_KARTDRIVER_H

#include <plib/ssg.h>
#include "Driver.h"
#include "joystick.h"
#include "ParticleSystem.h"
#include "World.h"

class Controller;  
class SkidMark;

class KartParticleSystem : public ParticleSystem
{
public:
  KartDriver* kart;

  KartParticleSystem ( KartDriver* kart, int num, float _create_rate,
      int _turn_to_face, float sz, float bsphere_size);

  virtual void update ( float t ) ;
  virtual void particle_create( int index, Particle* p );
  virtual void particle_update( float deltaTime, int index, Particle *p );
  virtual void particle_delete( int index, Particle* p );
};

class KartDriver : public Driver
{
protected:
public:
  int num_collectables ;
  int grid_position ;

  int collectable ;
  
  bool powersliding;

  /** The position of the karts controlls */
  JoyInfo controlls;
private:
  float attachment_time_left ;
  int   attachment_type ;
  int num_herring_gobbled ;
  ssgSelector *attachment ;
  
  ssgSimpleState     *smokepuff ;
  // don't delete the following 2 vars (they're kids in the hirarchy)
  KartParticleSystem *smoke_system ;
  ssgTransform       *exhaust_pipe ;

  float wheel_position;

  ssgTransform *wheel_front_l;
  ssgTransform *wheel_front_r;
  ssgTransform *wheel_rear_l;
  ssgTransform *wheel_rear_r;

  SkidMark* skidmark_left;
  SkidMark* skidmark_right;
  
  /** Search the given branch of objects that match the wheel names
      and if so assign them to wheel_* variables */
  void load_wheels(ssgBranch* obj);
    
  /** The Driver for this kart, ie. the object that controlls the
      steering */
  Controller* driver;
public:
  KartDriver (World* world, const KartProperties* kart_properties_,
      int position_, Controller* driver_ = 0 ) ;
  virtual ~KartDriver();

  void load_data();

  virtual void placeModel ();

  Controller* getDriver() const { return driver; }

  void addAttachment ( ssgEntity *e )
  {
    if ( attachment == NULL )
    {
      attachment = new ssgSelector () ;
      getModel() -> addKid ( attachment ) ;
    }

    attachment -> addKid ( e ) ;
    attachment -> select ( 0 ) ;
  }

  void attach ( int which, float time )
  {
    attachment -> selectStep ( which ) ;
    attachment_time_left = time ;
    attachment_type = which ;
  }

  int getAttachment      () { return  attachment_type ; }
  int getCollectable     () { return     collectable  ; }
  int getNumCollectables () { return num_collectables ; }
  int getNumHerring      () { return num_herring_gobbled ; }

  virtual void useAttachment        () ;
  virtual void forceCrash           () ;
  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   ( float delta ) ;
  virtual void doCollisionAnalysis  ( float delta, float hot ) ;
  virtual void update               ( float delta ) ;
  virtual void processInput         ( float delta ) ; 
  
  void beginPowerslide ();
  void endPowerslide ();
  
  void processAttachments(float delta);
  void processSkidMarks();
} ;

class TrafficDriver : public KartDriver
{
public:
  TrafficDriver (World* world, const KartProperties* kart_properties_,
                sgVec3 _pos )
    : KartDriver (world, kart_properties_, 0 )
  {
    sgCopyVec3 ( reset_pos.xyz, _pos ) ;
    reset () ;
  }

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void update (float delta) ;
} ;

#endif

/* EOF */
