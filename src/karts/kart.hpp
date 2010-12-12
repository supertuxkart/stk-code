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

/** \defgroup karts */

#include "btBulletDynamicsCommon.h"

#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/emergency_animation.hpp"
#include "karts/max_speed.hpp"
#include "karts/moveable.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "tracks/terrain_info.hpp"
#include "utils/no_copy.hpp"

class btKart;
class btUprightConstraint;
class btVehicleTuning;
class Camera;
class Item;
class Nitro;
class Quad;
class Shadow;
class SFXBase;
class SkidMarks;
class SlipStream;
class Smoke;
class WaterSplash;

/** The main kart class. All type of karts are of this object, but with 
 *  different controllers. The controllers are what turn a kart into a 
 *  player kart (i.e. the controller handle input), or an AI kart (the
 *  controller runs the AI code to set steering etc).
 *  Kart has two base classes: the most important one is moveable (which
 *  is an object that is moved on the track, and has position and rotations)
 *  and TerrainInfo, which manages the terrain the kart is on.
 * \ingroup karts
 */
class Kart : public TerrainInfo, public Moveable, public EmergencyAnimation,
             public MaxSpeed
{
private:
    
    bool m_flying;
    
    /** Reset position. */
    btTransform  m_reset_transform;
    /** Index of kart in world. */
    unsigned int m_world_kart_id;
    /** Accumulated skidding factor. */
    float        m_skidding;

    /** The main controller of this object, used for driving. This 
     *  controller is used to run the kart. It will be replaced
     *  with an end kart controller when the kart finishes the race. */
    Controller  *m_controller;
    /** This saves the original controller when the end controller is
     *  used. This is an easy solution for restarting the race, since
     *  the controller do not need to be reinitialised. */
    Controller  *m_saved_controller;

    /** Initial rank of the kart. */
    int          m_initial_position;

    /** Current race position (1-num_karts). */
    int          m_race_position;

    /** The camera for each kart. Not all karts have cameras (e.g. AI karts
     *  usually don't), but there are exceptions: e.g. after the end of a
     *  race an AI kart is replacing the kart for a player.
     */

protected:       // Used by the AI atm
    
    KartControl  m_controls;           // The kart controls (e.g. steering, fire, ...)
    Powerup      m_powerup;
    Attachment  *m_attachment;
    
    Camera      *m_camera;
    
private:
    /** True if the kart hasn't moved since 'ready-set-go' - used to 
     *  determine startup boost. */
    bool         m_has_started;

    float        m_max_gear_rpm;       /**<Maximum engine rpm's for the current gear*/
    float        m_bounce_back_time;   /**<A short time after a collision acceleration
                                        *  is disabled to allow the karts to bounce back*/

    // Bullet physics parameters
    // -------------------------
    btRaycastVehicle::btVehicleTuning 
                            *m_tuning;
    btCompoundShape          m_kart_chassis;
    btVehicleRaycaster      *m_vehicle_raycaster;
    btKart                  *m_vehicle;
    btUprightConstraint     *m_uprightConstraint;

     /** The amount of energy collected by hitting coins. Note that it
      *  must be float, since dt is subtraced in each timestep. */
    float         m_collected_energy;

    // Graphical effects
    // -----------------
    /** The shadow of a kart. */
    Shadow       *m_shadow;
    
    /** If a kart is flying, the shadow is disabled (since it is
     *  stuck to the kart, i.e. the shadow would be flying, too). */
    bool          m_shadow_enabled;
    
    /** Smoke from skidding. */
    Smoke        *m_smoke_system;
    
    /** Water splash when driving in water. */
    WaterSplash  *m_water_splash_system;

    /** Graphical effect when using a nitro. */
    Nitro        *m_nitro;

    /** Handles all slipstreaming. */
    SlipStream   *m_slipstream;

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

    std::vector<SFXBase*> m_custom_sounds;
    SFXBase      *m_beep_sound;
    SFXBase      *m_engine_sound;
    SFXBase      *m_crash_sound;
    SFXBase      *m_terrain_sound;
    /** A pointer to the previous terrain sound needs to be saved so that an
     *  'older' sfx can be finished and an abrupt end of the sfx is avoided. */
    SFXBase      *m_previous_terrain_sound;
    SFXBase      *m_skid_sound;
    SFXBase      *m_goo_sound;
    float         m_time_last_crash;

    void          updatePhysics(float dt);

protected:
    const KartProperties *m_kart_properties;
    /** This stores a copy of the kart model. It has to be a copy
     *  since otherwise incosistencies can happen if the same kart
     *  is used more than once. */
    KartModel*            m_kart_model;
    
public:
                   Kart(const std::string& ident, int position, 
                        const btTransform& init_transform);
    virtual       ~Kart();
    unsigned int   getWorldKartId() const            { return m_world_kart_id;   }
    void           setWorldKartId(unsigned int n)    { m_world_kart_id=n;        }
    void           loadData();
    virtual void   updateGraphics(const Vec3& off_xyz,  
                                  const btQuaternion& off_rotation);
    void           createPhysics    ();
    bool           isInRest         () const;
    void           setSuspensionLength();
    void           applyEngineForce (float force);
    float          handleNitro      (float dt);
    float          getActualWheelForce();

    virtual void flyUp();
    virtual void flyDown();
    
    void           resetBrakes      ();
    void           startEngineSFX   ();
    void           adjustSpeed      (float f);
    void           capSpeed         (float max_speed);
    void           updatedWeight    ();
    virtual void   collectedItem    (Item *item, int random_attachment);
    virtual void   reset            ();
    virtual void   handleZipper     (const Material *m=NULL, bool play_sound=false);
    virtual void   crashed          (Kart *k);
    
    virtual void   update           (float dt);
    virtual void   finishedRace     (float time);
    void           beep             ();
    bool           playCustomSFX    (unsigned int type);
    void           setController(Controller *controller);
    // ------------------------------------------------------------------------
    /** Returns this kart's kart model. */
    KartModel*     getKartModel()                 { return m_kart_model;      }
    // ------------------------------------------------------------------------
    /** Returns the kart properties of this kart. */
    const KartProperties*
                   getKartProperties() const      { return m_kart_properties; }
    // ------------------------------------------------------------------------
    /** Sets the kart properties. */
    void setKartProperties(const KartProperties *kp) { m_kart_properties=kp; }
    // ------------------------------------------------------------------------
    /** Sets the attachment and time it stays attached. */
    void attach(attachmentType attachment_, float time) 
                                     { m_attachment->set(attachment_, time); }
    // ------------------------------------------------------------------------
    /** Sets a new powerup. */
    void setPowerup (PowerupManager::PowerupType t, int n)
                                     { m_powerup.set(t, n); }
    // ------------------------------------------------------------------------
    /** Sets the position in race this kart has (1<=p<=n). */
    virtual void setPosition(int p)    
    {
        m_race_position = p;
        m_controller->setPosition(p);
    }   // setPosition
    // ------------------------------------------------------------------------
    /** Returns the current attachment. */
    const Attachment* getAttachment() const {return m_attachment; }
    // ------------------------------------------------------------------------
    /** Returns the current attachment, non-const version. */
    Attachment*    getAttachment() {return m_attachment; }
    // ------------------------------------------------------------------------
    /** Returns the camera of this kart (or NULL if no camera is attached
     *  to this kart). */
    Camera*        getCamera         ()       {return m_camera;}
    // ------------------------------------------------------------------------
    /** Returns the camera of this kart (or NULL if no camera is attached
     *  to this kart) - const version. */
    const Camera*  getCamera         () const {return m_camera;}
    // ------------------------------------------------------------------------
    /** Sets the camera for this kart. Takes ownership of the camera and will delete it. */
    void           setCamera(Camera *camera) {m_camera=camera; }
    // ------------------------------------------------------------------------
    /** Returns the current powerup. */
    const Powerup *getPowerup          () const { return &m_powerup;         }
    // ------------------------------------------------------------------------
    /** Returns the current powerup. */
    Powerup       *getPowerup          ()       { return &m_powerup;         }
    // ------------------------------------------------------------------------
    /** Returns the number of powerups. */
    int            getNumPowerup       () const { return m_powerup.getNum(); }
    // ------------------------------------------------------------------------
    /** Returns the remaining collected energy. */
    float          getEnergy           () const { return m_collected_energy; }
    // ------------------------------------------------------------------------
    /** Returns the current position of this kart in the race. */
    int            getPosition         () const { return m_race_position;    }
    // ------------------------------------------------------------------------
    /** Returns the initial position of this kart. */
    int            getInitialPosition  () const { return m_initial_position; }
    // ------------------------------------------------------------------------
    /** Returns the finished time for a kart. */
    float          getFinishTime       () const { return m_finish_time;      }
    // ------------------------------------------------------------------------
    /** Returns true if this kart has finished the race. */
    bool           hasFinishedRace     () const { return m_finished_race;    }
    // ------------------------------------------------------------------------
    /** Returns true if the kart has a plunger attached to its face. */
    bool           hasViewBlockedByPlunger() const
                                     { return m_view_blocked_by_plunger > 0; }
    // ------------------------------------------------------------------------
    /** Sets that the view is blocked by a plunger. The duration depends on
     *  the difficulty, see KartPorperties getPlungerInFaceTime. */
    void           blockViewWithPlunger()   
                             { m_view_blocked_by_plunger = 
                                   m_kart_properties->getPlungerInFaceTime();}
    // -------------------------------------------------------------------------
    /** Returns a bullet transform object located at the kart's position
        and oriented in the direction the kart is going. Can be useful
        e.g. to calculate the starting point and direction of projectiles. */
    btTransform     getKartTransform(const float customPitch=-1);
    // -------------------------------------------------------------------------
    /** Returns the color used for this kart. */
    const video::SColor &getColor() const 
                                        {return m_kart_properties->getColor();}
    // ------------------------------------------------------------------------
    /** Returns the current mass of this kart, including any attachment this
     *  kart might have. */
    float getMass() const { return m_kart_properties->getMass()
                                 + m_attachment->weightAdjust();}
    // ------------------------------------------------------------------------
    /** Returns the maximum engine power for this kart. */
    float getMaxPower     () const {return m_kart_properties->getMaxPower(); }
    // ------------------------------------------------------------------------
    /** Returns the strenght of the brakes for this kart. */
    float getBrakeFactor() const {return m_kart_properties->getBrakeFactor();}
    // ------------------------------------------------------------------------
    /** Returns the time till full steering is reached for this kart. */
    float getTimeFullSteer() const 
                              { return m_kart_properties->getTimeFullSteer(); }
    // ------------------------------------------------------------------------
    /** Returns the maximum steering angle for this kart, which depends on the
     *  speed. */
    float getMaxSteerAngle () const
                    { return m_kart_properties->getMaxSteerAngle(getSpeed()); }
    // ------------------------------------------------------------------------
    /** Returns the amount of skidding for this kart. */
    float getSkidding() const { return m_skidding; }
    // ------------------------------------------------------------------------
    /** Returns the current steering value for this kart. */
    float getSteerPercent() const { return m_controls.m_steer;  }
    // ------------------------------------------------------------------------
    /** Returns all controls of this kart. */
    KartControl&  getControls() { return m_controls; }
    // ------------------------------------------------------------------------
    /** Returns all controls of this kart - const version. */
    const KartControl& getControls() const { return m_controls; }
    // ------------------------------------------------------------------------
    /** Sets the kart controls. Used e.g. by replaying history. */
    void           setControls(const KartControl &c) { m_controls = c;        }
    // ------------------------------------------------------------------------
    /** Returns the length of the kart. */
    float          getKartLength   () const {return m_kart_model->getLength();}
    // ------------------------------------------------------------------------
    /** Returns the height of the kart. */
    float          getKartHeight   () const {return m_kart_model->getHeight();}
    // ------------------------------------------------------------------------
    /** Returns the width of the kart. */
    float          getKartWidth    () const {return m_kart_model->getWidth(); }
    // ------------------------------------------------------------------------
    /** Returns the bullet vehicle which represents this kart. */
    btKart        *getVehicle      () const {return m_vehicle;                }
    // ------------------------------------------------------------------------
    /** Returns the upright constraint for this kart. */
    btUprightConstraint *getUprightConstraint() const 
                                                  {return m_uprightConstraint;}
    // ------------------------------------------------------------------------
    /** Returns the speed of the kart. */
    float          getSpeed         () const {return m_speed;                 }
    // ------------------------------------------------------------------------
    /** This is used on the client side only to set the speed of the kart
     *  from the server information.                                       */
    void           setSpeed         (float s) {m_speed = s;                   }
    // ------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    const SlipStream* getSlipstream() const {return m_slipstream; }
    // ------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    SlipStream* getSlipstream() {return m_slipstream; }
    // ------------------------------------------------------------------------
    /** Activates a slipstream effect, atm that is display some nitro. */
    void           setSlipstreamEffect(float f);
    // ------------------------------------------------------------------------
    /** Returns a name to be displayed for this kart. */
    virtual const irr::core::stringw& getName() const 
                                       { return m_kart_properties->getName(); }
    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this kart (name of the directory the
     *  kart was loaded from). */
    const std::string& getIdent() const {return m_kart_properties->getIdent();}
    // ------------------------------------------------------------------------
    /** Returns the start transform, i.e. position and rotation. */
    const btTransform getResetTransform() const {return m_reset_transform;}
    // ------------------------------------------------------------------------
    /** Returns the controller of this kart. */
    Controller*    getController() { return m_controller; }
    // ------------------------------------------------------------------------
    /** Returns the controller of this kart (const version). */
    const Controller* getController() const { return m_controller; }
    // ------------------------------------------------------------------------
    /** True if the wheels are touching the ground. */
    bool           isOnGround       () const;
    // ------------------------------------------------------------------------
    /** Returns true if the kart is close to the ground, used to dis/enable
     *  the upright constraint to allow for more realistic explosions. */
    bool           isNearGround     () const;
    // ------------------------------------------------------------------------
};


#endif

/* EOF */

