//  $Id: KartDriver.h,v 1.3 2004/08/10 21:57:25 straver Exp $
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
#include <plib/ssgAux.h>
#include "Driver.h"
  
//static void create_smoke (ssgaParticleSystem *, int, ssgaParticle *p);
//static void update_smoke (float delta_t, ssgaParticleSystem *, int, ssgaParticle *p);

class ParticleSystem ;

typedef void (* ParticleCreateFunc) ( ParticleSystem *ps,
                                      int index,
                                      ssgaParticle *p ) ;

class ParticleSystem : public ssgaParticleSystem
{
	public:
	ParticleSystem ( int num, int initial_num,
                         float _create_rate, int _turn_to_face,
                         float sz, float bsphere_size,
                         ParticleCreateFunc _particle_create,
                         ssgaParticleUpdateFunc _particle_update = NULL,
                         ssgaParticleDeleteFunc _particle_delete = NULL );
	//virtual ~ParticleSystem ();
	
	void *userdata;
};

class KartDriver : public Driver
{
protected:
  int num_collectables ;
  int grid_position ;
  int collectable ;
  float attachment_time_left ;
  int   attachment_type ;
  int num_herring_gobbled ;
  ssgSelector *attachment ;
  
  ssgSimpleState     *smokepuff ;
  ParticleSystem     *smoke_system ;
  ssgTransform       *exhaust_pipe ;

  float wheel_position;

  ssgTransform *wheel_front_l;
  ssgTransform *wheel_front_r;
  ssgTransform *wheel_rear_l;
  ssgTransform *wheel_rear_r;
  
  /** Search the given branch of objects that match the wheel names
      and if so assign them to wheel_* variables */
  void load_wheels(ssgBranch* obj);
    
public:
  KartDriver ( const KartProperties& kart_properties_, int position_ ) ;
  virtual ~KartDriver() {}

  void load_data();

  virtual void placeModel ();

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
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float hot ) ;
  virtual void update               () ;
} ;


class NetworkKartDriver : public KartDriver
{
public:
  NetworkKartDriver ( const KartProperties& kart_properties_, int _pos )
    : KartDriver (kart_properties_, _pos)
  {
  }

  virtual ~NetworkKartDriver() {}

  virtual void update () ;
} ;



class TrafficDriver : public KartDriver
{
public:
  TrafficDriver ( const KartProperties& kart_properties_, sgVec3 _pos )
    : KartDriver ( kart_properties_, 0 )
  {
    sgCopyVec3 ( reset_pos.xyz, _pos ) ;
    reset () ;
  }

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void update () ;
} ;


class AutoKartDriver : public KartDriver
{
  float time_since_last_shoot ;
public:
  AutoKartDriver ( const KartProperties& kart_properties_, int _pos ) 
    : KartDriver ( kart_properties_, _pos )
  {
    time_since_last_shoot = 0.0f ;
  }

  virtual ~AutoKartDriver() {}

  virtual void update () ;
} ;


class PlayerKartDriver : public KartDriver
{  
protected:
  float    tscale ;
  float    rscale ;
  
  // physics debugging
  float *selected_property;

public:
  PlayerKartDriver ( const KartProperties& kart_properties_, int _pos ) 
    : KartDriver ( kart_properties_, _pos )
  {
    tscale = 10.0 ;
    rscale =  3.0 ;
  }

  virtual void update () ;

  void incomingKeystroke ( const SDL_keysym& ) ;
  void incomingJoystick  ( JoyInfo& ji ) ;
} ;

#endif

/* EOF */
