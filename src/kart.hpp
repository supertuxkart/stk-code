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
#include "particle_system.hpp"
#include "kart_properties.hpp"
#include "attachment.hpp"
#include "collectable.hpp"
#ifdef BULLET
#include "btBulletDynamicsCommon.h"
#endif

struct KartControl
{
    float lr;
    float accel;
    bool  brake;
    bool  wheelie;
    bool  jump;
    bool  rescue;
    bool  fire;

    KartControl() : lr(0.0f), accel(0.0f), brake(false),
    wheelie(false), jump(false),  rescue(false), fire(false){}
};

class SkidMark;
class Kart;
class Herring;

class KartParticleSystem : public ParticleSystem
{
private:
    Kart* m_kart;

public:
    KartParticleSystem          (Kart* kart, int num, float _create_rate,
                                 int _turn_to_face, float sz, float bsphere_size);
    virtual void update         (float t                                        );
    virtual void particle_create(int index, Particle* p                         );
    virtual void particle_update(float deltaTime, int index, Particle *p        );
    virtual void particle_delete(int index, Particle* p                         );
};

// =============================================================================
class Kart : public Moveable
{
protected:
    bool         m_on_road;            //true if the kart is on top of the
                                       //road path drawn by the drivelines

    Attachment   m_attachment;
    Collectable  m_collectable;

    int          m_grid_position;
    int          m_race_position;
    KartControl  m_controls;           // The position of the karts controls
    int          m_track_sector;       // index in driveline, special values
                                       // e.g. UNKNOWN_SECTOR can be negative!
    float        m_zipper_time_left;
    sgVec2       m_last_track_coords;
    sgVec3       m_curr_track_coords;
    sgVec3       m_velocity_wc;        // velocity in world coordinates
    float        m_prev_accel;         // acceleration at previous time step
    bool         m_skid_front;         // true if front tires are skidding
    bool         m_skid_rear;          // true if rear tires are skidding
    float        m_max_speed;          // maximum speed of the kart, computed from
    float        m_max_speed_reverse_ratio;
    float        m_wheelie_angle ;
    float        m_current_friction;   // current friction
    float        m_lap_start_time;     // Time at start of a new lap
    float        m_kart_length;        // length of kart
    char         m_fastest_lap_message[255];
    int          m_shortcut_count;     // counts number of times a shortcut is used
    int          m_shortcut_sector;    // segment on which the shortcut was started
    enum        {SC_NONE,              // no shortcut
                 SC_SKIPPED_SECTOR,    // skipped too many sectors
                 SC_OUTSIDE_TRACK}     // too far away from tracj
                 m_shortcut_type  ;    // what kind of shortcut was detected
    // physics parameters, storing it saves time
#ifdef BULLET
    btRaycastVehicle::btVehicleTuning  *m_tuning;
    btCompoundShape                    *m_kart_chassis;
    btVehicleRaycaster                 *m_vehicle_raycaster;
    btRaycastVehicle                   *m_vehicle;
    float                               m_kart_height;
    float                               m_time_since_stuck;
#endif

private:
    int                 m_num_herrings_gobbled;
    ssgSimpleState*     m_smokepuff;
    // don't delete the following 2 vars (they're kids in the hirarchy)
    KartParticleSystem* m_smoke_system;
    ssgTransform*       m_exhaust_pipe;

    float               m_wheel_position;
    ssgTransform*       m_wheel_front_l;
    ssgTransform*       m_wheel_front_r;
    ssgTransform*       m_wheel_rear_l;
    ssgTransform*       m_wheel_rear_r;

    SkidMark*           m_skidmark_left;
    SkidMark*           m_skidmark_right;

    int                 m_race_lap;             // number of finished(!) laps
    float               m_time_at_last_lap;       // time at finishing last lap
    float               m_finish_time;
    bool                m_finished_race;

#ifdef BULLET
    float               m_speed;
#endif

protected:
    int                 m_rescue;
#ifdef BULLET
    float               m_rescue_pitch, m_rescue_roll;
#endif
    const KartProperties *m_kart_properties;

    /** Search the given branch of objects that match the wheel names
        and if so assign them to wheel_* variables */
    void  load_wheels          (ssgBranch* obj);

public:
    Kart(const KartProperties* kartProperties_, int position_, sgCoord init_pos);
    virtual ~Kart();

    void loadData();

    void placeModel ();
    const KartProperties* getKartProperties() const
        { return m_kart_properties; }
    void           setKartProperties   (const KartProperties *kp)
    { m_kart_properties=kp;}
    void           attach              (attachmentType attachment_, float time)
    { m_attachment.set(attachment_, time);}
    void           gotZipper           (float angle, float time)
    {
        m_wheelie_angle=angle;
        m_zipper_time_left=time;
    }
    void           setCollectable      (collectableType t, int n)
    { m_collectable.set(t, n);            }
    void           setPosition         (int p)    {m_race_position = p;          }
    int            getSector           () { return m_track_sector;               }
    float          getDistanceDownTrack() { return m_curr_track_coords[1];       }
    float          getDistanceToCenter () { return m_curr_track_coords[0];       }
    Attachment    *getAttachment       () { return &m_attachment;                }
    void           setAttachmentType   (attachmentType t, float time_left=0.0f,
                                        Kart*k=NULL)
                                          { m_attachment.set(t, time_left, k);   }
    Collectable   *getCollectable      () { return &m_collectable;               }
    int            getNumCollectables  () const { return  m_collectable.getNum();}
    int            getNumHerring       () const { return  m_num_herrings_gobbled;}
    int            getLap              () const { return  m_race_lap;            }
    int            getPosition         () const { return  m_race_position ;      }
    void           setFinishingState(float time);
    float          getFinishTime       () const  { return m_finish_time;         }
    bool           isTakingShortcut    () const  { return m_shortcut_type!=SC_NONE;}
    bool           raceIsFinished      () const  { return m_finished_race;       }
    void           endRescue           ();
    void           processSkidMarks    ();
    void           getClosestKart      (float *cdist, int *closest);
    void           handleMagnet        (float cdist, int closest);
    void           doZipperProcessing  (float dt);
    void           updatePhysics       (float dt);
    float          NormalizedLateralForce(float alpha, float corner) const;

    // Functions to access the current kart properties (which might get changed,
    // e.g. mass increase or air_friction increase depending on attachment etc.)
    // -------------------------------------------------------------------------
    const sgVec3*  getColor         () const {return m_kart_properties->getColor();}
    float          getMass          () const
    {
        return m_kart_properties->getMass()
               + m_attachment.WeightAdjust();
    }
    float          getRollResistance() const {return m_kart_properties->getRollResistance();}
    float          getMaxPower      () const {return m_kart_properties->getMaxPower();}
    float          getTimeFullSteer () const {return m_kart_properties->getTimeFullSteer();}
    float          getBrakeFactor   () const {return m_kart_properties->getBrakeFactor();}
    float          getBrakeForce    () const {return m_kart_properties->getBrakeForce();}
    float          getWheelBase     () const {return m_kart_properties->getWheelBase();}
    float          getHeightCOG     () const {return m_kart_properties->getHeightCOG();}
    float          getTireGrip      () const {return m_kart_properties->getTireGrip();}
    float          getMaxSteerAngle () const {return m_kart_properties->getMaxSteerAngle();}
    float          getCornerStiffF  () const {return m_kart_properties->getCornerStiffF();}
    float          getCornerStiffR  () const {return m_kart_properties->getCornerStiffR();}
    float          getInertia       () const {return m_kart_properties->getInertia();     }
    float          getGravityCenterShift   () const
        {return m_kart_properties->getGravityCenterShift();                    }
    float          getWheelieMaxSpeedRatio () const
        {return m_kart_properties->getWheelieMaxSpeedRatio();                  }
    float          getWheelieMaxPitch  () const
        {return m_kart_properties->getWheelieMaxPitch();                       }
    float          getWheeliePitchRate () const
        {return m_kart_properties->getWheeliePitchRate();                      }
    float          getWheelieRestoreRate() const
        {return m_kart_properties->getWheelieRestoreRate();                    }
    float          getWheelieSpeedBoost() const
        {return m_kart_properties->getWheelieSpeedBoost();                     }
    float          getSteerAngle    () const
        {return m_controls.lr*m_kart_properties->getMaxSteerAngle();           }
    float          getAirResistance () const;
    float          getSteerPercent  () const {return m_controls.lr;            }
    float          getMaxSpeed      () const {return m_max_speed;              }
    void           setTimeAtLap     (float t){m_time_at_last_lap=t;            }
    float          getTimeAtLap     () const {return m_time_at_last_lap;       }
    float          getKartLength    () const {return m_kart_length;            }
    void           createPhysics    (ssgEntity *obj);
#ifdef BULLET
    float             getKartHeight () const {return m_kart_height;            }
    btRaycastVehicle *getVehicle    () const {return m_vehicle;                }
    void              updateBulletPhysics(float dt);
    void              draw          ();
    bool              isInRest      ();
    //have to use this instead of moveable getVelocity to get velocity for bullet rigid body
    float             getSpeed      () const {return m_speed;                 }
    float             handleWheelie(float dt);
    float             getActualWheelForce();
#endif
    void           adjustSpeedWeight(float f);
    void           forceRescue      ();
    void           handleExplosion  (const sgVec3& pos, bool direct_hit);
    const std::string& getName      () const {return m_kart_properties->getName();}
    virtual int    isPlayerKart     () const {return 0;                        }
    // addMessages gets called by world to add messages to the gui
    virtual void   addMessages      () {};
    virtual void   collectedHerring (Herring* herring);
    virtual void   reset            ();
    virtual void   handleZipper     ();
    virtual void   forceCrash       ();
    virtual void   doLapCounting    ();
    virtual void   update           (float dt               );
#ifndef BULLET
    virtual void   doCollisionAnalysis(float dt, float hot    );
#endif
    virtual void   doObjectInteractions();
    virtual void   OutsideTrack     (int isReset) {m_rescue=true;}
};

class TrafficDriver : public Kart
{
public:
    TrafficDriver (const KartProperties* kartProperties_, sgVec3 _pos,
                   sgCoord init_pos)
        : Kart (kartProperties_, 0, init_pos )
    {
        sgCopyVec3 ( m_reset_pos.xyz, _pos ) ;
        reset () ;
    }
    virtual void doObjectInteractions () ;
    virtual void doLapCounting        () ;
    virtual void doZipperProcessing   () ;
    virtual void update (float delta) ;
} ;


#endif

/* EOF */

