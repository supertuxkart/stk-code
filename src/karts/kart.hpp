//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef HEADER_KART_HPP
#define HEADER_KART_HPP

#include "btBulletDynamicsCommon.h"

#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "karts/moveable.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_control.hpp"
#include "karts/kart_model.hpp"
#include "tracks/terrain_info.hpp"

class SkidMarks;
class Item;
class Smoke;
class ssgTransform;
class Nitro;
class SFXBase;
class btUprightConstraint;
class btKart;
class btRaycastVehicle::btVehicleTuning;

class Kart : public TerrainInfo, public Moveable
{
private:
    btTransform  m_reset_transform;    // reset position
    unsigned int m_world_kart_id;      // index of kart in world
    float        m_skidding;           ///< Accumulated skidding factor.

protected:
    Attachment   m_attachment;
    Powerup      m_powerup;
    int          m_race_position;      // current race position (1-numKarts)
    int          m_initial_position;   // initial position of kart

    KartControl  m_controls;           // The position of the karts controls

    float        m_max_speed;          // maximum speed of the kart, computed from
    /** Depending on terrain a certain reduction to the maximum speed applies.
     *  This reduction is accumulated in m_max_speed_reduction. */
    float        m_max_speed_reduction;
    float        m_power_reduction;
    float        m_max_gear_rpm;       /**<Maximum engine rpm's for the current gear*/
    float        m_max_speed_reverse_ratio;
    float        m_zipper_time_left;   /**<Zipper time left. */
    float        m_bounce_back_time;   /**<A short time after a collision acceleration
                                        *  is disabled to allow the karts to bounce back*/

    // physics parameters, storing it saves time
    btRaycastVehicle::btVehicleTuning 
                            *m_tuning;
    btCompoundShape          m_kart_chassis;
    btVehicleRaycaster      *m_vehicle_raycaster;
    btKart                  *m_vehicle;
    btUprightConstraint     *m_uprightConstraint;

private:
                       /** The amount of energy collected by hitting coins. */
    float         m_collected_energy;
    ssgTransform *m_shadow;  /**<The shadow of the kart. */
    /** If a kart is flying, the shadow is disabled (since it is
     *  stuck to the kart, i.e. the shadow would be flying, too). */
    bool          m_shadow_enabled;
    Smoke        *m_smoke_system;    /**<Smoke from skidding. */

    /** Fire when using a nitro. */
    Nitro        *m_nitro;

    float         m_wheel_rotation;
    /** For each wheel it stores the suspension length after the karts are at 
     *  the start position, i.e. the suspension will be somewhat compressed.
     *  The bullet suspensionRestLength is the value when the suspension is not
     *  at all compressed. */
    float         m_default_suspension_length[4];

    /** The skidmarks object for this kart. */
    SkidMarks    *m_skidmarks;

    float         m_finish_time;
    bool          m_finished_race;

    /** When a kart has its view blocked by the plunger, this variable will be 
     *  > 0 the number it contains is the time left before removing plunger. */
    float         m_view_blocked_by_plunger;
    float         m_speed;
    float         m_current_gear_ratio;
    bool          m_rescue;
    bool          m_eliminated;

    SFXBase      *m_engine_sound;
    SFXBase      *m_beep_sound;
    SFXBase      *m_crash_sound;
    SFXBase      *m_skid_sound;
    SFXBase      *m_goo_sound;
    float         m_time_last_crash;

protected:
    float                 m_rescue_pitch, m_rescue_roll;
    const KartProperties *m_kart_properties;

    
public:
                   Kart(const std::string& kart_name, int position, 
                        const btTransform& init_transform);
    virtual       ~Kart();
    unsigned int   getWorldKartId() const            { return m_world_kart_id;   }
    void           setWorldKartId(unsigned int n)    { m_world_kart_id=n;        }
    void           loadData();
    virtual void   updateGraphics(const Vec3& off_xyz,  const Vec3& off_hpr);
    const KartProperties* 
                   getKartProperties() const      { return m_kart_properties; }
    // ------------------------------------------------------------------------
    /** Sets the kart properties. */
    void setKartProperties(const KartProperties *kp)
    {
        m_kart_properties=kp;                
    }
    // ------------------------------------------------------------------------
    /** Sets the attachment and time it stays attached. */
    void attach(attachmentType attachment_, float time)
    { 
        m_attachment.set(attachment_, time); 
    }
    // ------------------------------------------------------------------------
    /** Sets a new powerup. */
    void setPowerup (PowerupType t, int n)
    { 
        m_powerup.set(t, n);
    }
    // ------------------------------------------------------------------------
    /** Sets the position in race this kart has (1<=p<=n). */
    virtual void setPosition(int p)    
    {
        m_race_position = p;
    }
    // ------------------------------------------------------------------------
    Attachment *getAttachment() 
    {
        return &m_attachment;          
    }
    // ------------------------------------------------------------------------
    void setAttachmentType(attachmentType t, float time_left=0.0f, Kart*k=NULL)
    {
        m_attachment.set(t, time_left, k);   
    }
    // ------------------------------------------------------------------------

    Powerup       *getPowerup          ()       { return &m_powerup;         }
    int            getNumPowerup       () const { return  m_powerup.getNum();}
    float          getEnergy           () const { return  m_collected_energy;}
    int            getPosition         () const { return  m_race_position;       }
    int            getInitialPosition  () const { return  m_initial_position;    }
    float          getFinishTime       () const { return  m_finish_time;         }
    bool           hasFinishedRace     () const { return  m_finished_race;       }
    void           endRescue           ();
    void           getClosestKart      (float *cdist, int *closest);
    void           updatePhysics       (float dt);

    bool           hasViewBlockedByPlunger() const
                                                { return m_view_blocked_by_plunger > 0; }
    void           blockViewWithPlunger()       { m_view_blocked_by_plunger = 10; }
    
   /**
       returns a bullet transform object located at the kart's position
       and oriented in the direction the kart is going. Can be useful
       e.g. to calculate the starting point and direction of projectiles
    */
   btTransform    getKartHeading      (const float customPitch=-1);

    
    // Functions to access the current kart properties (which might get changed,
    // e.g. mass increase or air_friction increase depending on attachment etc.)
    // -------------------------------------------------------------------------
    const Vec3    &getColor         () const {return m_kart_properties->getColor();}
    float          getMass          () const
    {
        return m_kart_properties->getMass()
               + m_attachment.weightAdjust();
    }
    float          getMaxPower      () const {return m_kart_properties->getMaxPower();}
    float          getTimeFullSteer () const {return m_kart_properties->getTimeFullSteer();}
    float          getBrakeFactor   () const {return m_kart_properties->getBrakeFactor();}
    float          getFrictionSlip  () const {return m_kart_properties->getFrictionSlip();}
    float          getSkidding      () const {return m_skidding;}
    float          getMaxSteerAngle () const
                       {return m_kart_properties->getMaxSteerAngle(getSpeed());}
    const Vec3&    getGravityCenterShift   () const
        {return m_kart_properties->getGravityCenterShift();                    }
    float          getSteerPercent  () const {return m_controls.m_steer;       }
    const KartControl&
                   getControls      () const {return m_controls;               }
    /** Sets the kart controls. Used e.g. by replaying history. */
    void           setControls(const KartControl &c) { m_controls = c;         }
    /** Returns the maximum speed of the kart independent of the 
     *  terrain it is on. */
    float          getMaxSpeed      () const {return m_max_speed;              }    
    /** Returns the maximum speed of the kart but includes the effect of 
     *  the terrain it is on. */    
    float          getMaxSpeedOnTerrain() const {return m_max_speed-
                                                     m_max_speed_reduction;    }
    /** Returns the length of the kart. */
    float          getKartLength    () const
                   {return m_kart_properties->getKartModel()->getLength();     }
    /** Returns the height of the kart. */
    float          getKartHeight    () const 
                   {return m_kart_properties->getKartModel()->getHeight();     }
    btKart        *getVehicle       () const {return m_vehicle;                }
    btUprightConstraint *getUprightConstraint() const {return m_uprightConstraint;}
    void           createPhysics    ();
    void           draw             ();
    bool           isInRest         () const;
    //have to use this instead of moveable getVelocity to get velocity for bullet rigid body
    float          getSpeed         () const {return m_speed;                 }
    /** This is used on the client side only to set the speed of the kart
     *  from the server information.                                       */
    void           setSpeed         (float s) {m_speed = s;                   }
    void           setSuspensionLength();
    float          handleNitro      (float dt);
    float          getActualWheelForce();
    bool           isOnGround       () const;
    bool           isNearGround     () const;
    bool           isEliminated     () const {return m_eliminated;}
    void           eliminate        ();
    bool           isRescue         () const {return m_rescue;}
    void           resetBrakes      ();
    void           adjustSpeed      (float f);
    void           updatedWeight    ();
    void           forceRescue      ();
    void           handleExplosion  (const Vec3& pos, bool direct_hit);
    const std::string& getName      () const {return m_kart_properties->getName();}
    const std::string& getIdent     () const {return m_kart_properties->getIdent();}
    virtual bool    isPlayerKart    () const {return false;                        }
    /** Called by world in case of the kart taking a shortcut. The player kart
     *  will display a message in this case, default behaviour is to do nothing.
     */
    virtual void    doingShortcut() {};
    // addMessages gets called by world to add messages to the gui
    virtual void   addMessages      () {};
    virtual void   collectedItem    (const Item &item, int random_attachment);
    virtual void   reset            ();
    virtual void   handleZipper     ();
    virtual void   crashed          (Kart *k);
    
    virtual void   update           (float dt);
    virtual void   raceFinished     (float time);
    void           beep             ();
};


#endif

/* EOF */

