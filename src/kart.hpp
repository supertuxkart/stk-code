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

#ifndef HEADER_KART_H
#define HEADER_KART_H

#include <plib/sg.h>
#include "moveable.hpp"
#include "kart_control.hpp"
#include "particle_system.hpp"
#include "kart_properties.hpp"
#include "attachment.hpp"
#include "collectable.hpp"


class SkidMark;
class Kart;
class Herring;

class KartParticleSystem : public ParticleSystem {
private:
  Kart* kart;

public:
  KartParticleSystem          (Kart* kart, int num, float _create_rate,
			       int _turn_to_face, float sz, float bsphere_size);
  virtual void update         (float t                                        );
  virtual void particle_create(int index, Particle* p                         );
  virtual void particle_update(float deltaTime, int index, Particle *p        );
  virtual void particle_delete(int index, Particle* p                         );
};

class Kart : public Moveable {
protected:
  Attachment   attachment;
  Collectable  collectable;

  int          grid_position;
  int          racePosition;
  bool         powersliding;
  KartControl  controls;           // The position of the karts controlls 
  unsigned int trackHint;          // index in driveline                  
  float        ZipperTimeLeft;
  sgVec2       last_track_coords;
  sgVec2       curr_track_coords;
  sgVec3       velocity_wc;        // velocity in world coordinates
  float        prevAccel;          // acceleration at previous time step
  bool         skidFront;          // true if front tires are skidding
  bool         skidRear;           // true if rear tires are skidding
  float        maxSpeed;           // maximum speed of the kart, computed from
                                   // physics parameters, storing it saves time

private:
  int                 num_herring_gobbled;
  ssgSimpleState*     smokepuff;
  // don't delete the following 2 vars (they're kids in the hirarchy)
  KartParticleSystem* smoke_system;
  ssgTransform*       exhaust_pipe;

  float               wheel_position;
  ssgTransform*       wheel_front_l;
  ssgTransform*       wheel_front_r;
  ssgTransform*       wheel_rear_l;
  ssgTransform*       wheel_rear_r;

  SkidMark*           skidmark_left;
  SkidMark*           skidmark_right;

  int                 raceLap;             // number of finished(!) laps
  float               finishTime;
  bool                finishedRace;
 protected:
  int                 rescue;
  const KartProperties *kartProperties;  
  
  /** Search the given branch of objects that match the wheel names
      and if so assign them to wheel_* variables */
  void  load_wheels          (ssgBranch* obj);
    
public:
  Kart(const KartProperties* kartProperties_, int position_);
  virtual ~Kart();

  void load_data();

  virtual void placeModel ();
  const KartProperties* getKartProperties() const 
                                        { return kartProperties; }
  void           setKartProperties   (const KartProperties *kp) 
                                        { kartProperties=kp;}
  void           attach              (attachmentType attachment_, float time)
                                        { attachment.set(attachment_, time);}
  void           gotZipper           (float angle, float time)
                                        { wheelie_angle=angle; 
					  ZipperTimeLeft=time;              }
  void           setCollectable      (collectableType t, int n) 
                                        { collectable.set(t, n);            }
  void           setPosition         (int p)    {racePosition = p;          }
  int            getHint             () { return trackHint;                 }
  float          getDistanceDownTrack() { return curr_track_coords[1];      }
  float          getDistanceToCenter () { return curr_track_coords[0];      }
  attachmentType getAttachment       () { return  attachment.getType();     }
  void           setAttachmentType   (attachmentType t)
                                        { attachment.set(t);                }
  Collectable   *getCollectable      () { return &collectable;              }
  int            getNumCollectables  () { return  collectable.getNum();     }
  int            getNumHerring       () { return  num_herring_gobbled;      }
  int            getLap              () { return  raceLap;                  }
  int            getPosition         () { return  racePosition ;            }
  void           setFinishingState(float time);
  float          getFinishTime       () const  { return finishTime;         }
  bool           raceIsFinished      () const  { return finishedRace;       }
  void           handleRescue        ();
  void           beginPowerslide     ();
  void           endPowerslide       ();
  void           processSkidMarks    ();
  void           getClosestKart      (float *cdist, int *closest);
  void           handleMagnet        (float cdist, int closest);
  void           doZipperProcessing  (float dt);
  void           updatePhysics       (float dt);
  float          NormalizedLateralForce(float alpha, float corner) const;

  // Functions to access the current kart properties (which might get changed,
  // e.g. mass increase or air_friction increase depending on attachment etc.)
  // -------------------------------------------------------------------------
  const sgVec3*  getColor         () const {return kartProperties->getColor();}
  float          getMass          () const {return kartProperties->getMass()
                                                 + attachment.WeightAdjust();}
  float          getRollResistance() const {return kartProperties->getRollResistance();}
  float          getMaxPower      () const {return kartProperties->getMaxPower();}
  float          getTimeFullSteer () const {return kartProperties->getTimeFullSteer();}
  float          getBrakeFactor   () const {return kartProperties->getBrakeFactor();}
  float          getWheelBase     () const {return kartProperties->getWheelBase();}
  float          getHeightCOG     () const {return kartProperties->getHeightCOG();}
  float          getTireGrip      () const {return kartProperties->getTireGrip();}
  float          getMaxSteerAngle () const {return kartProperties->getMaxSteerAngle();}
  float          getCornerStiffF  () const {return kartProperties->getCornerStiffF();}
  float          getCornerStiffR  () const {return kartProperties->getCornerStiffR();}
  float          getInertia       () const {return kartProperties->getInertia();     }
  float          getWheelieMaxSpeedRatio () const
                                     {return kartProperties->getWheelieMaxSpeedRatio();}
  float          getWheelieMaxPitch  () const
                                     {return kartProperties->getWheelieMaxPitch();   }
  float          getWheeliePitchRate () const
                                     {return kartProperties->getWheeliePitchRate();  }
  float          getWheelieRestoreRate() const
                                     {return kartProperties->getWheelieRestoreRate();}
  float          getWheelieSpeedBoost() const
                                     {return kartProperties->getWheelieSpeedBoost(); }
  float          getSteerAngle    () const {return controls.lr*
                                               kartProperties->getMaxSteerAngle();}
  float          getAirResistance () const;
  float          getSteerPercent  () const {return controls.lr;}
  float          getMaxSpeed      () const {return maxSpeed;   }
  virtual int    isPlayerKart     () const {return 0;          }
  virtual void   collectedHerring (Herring* herring);
  virtual void   reset            ();
  virtual void   handleZipper     ();
  virtual void   forceCrash       ();
  virtual void   doLapCounting    ();
  virtual void   update           (float dt               );
  virtual void   doCollisionAnalysis(float dt, float hot    );
  virtual void   doObjectInteractions();
#ifdef NEW_PHYSICS
  virtual void   updatePosition   (float dt, sgMat4 result);
#endif
  virtual void   OutsideTrack     (int isReset) {rescue=true;} 
};

class TrafficDriver : public Kart {
public:
  TrafficDriver (const KartProperties* kartProperties_, sgVec3 _pos )
    : Kart (kartProperties_, 0 )
  {
    sgCopyVec3 ( reset_pos.xyz, _pos ) ;
    reset () ;
  }
  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void update (float delta) ;
  
} ;

#ifdef JH
class NetworkKartDriver : public Kart {
public:
  NetworkKartDriver ( int _pos, ssgTransform *m ) : Kart ( _pos, m )
  {
  }

  virtual void update () ;
} ;


#endif

void print_model(ssgEntity* entity, int indent, int maxLevel);
void optimise_model(ssgEntity* entity);

#endif

/* EOF */
  
