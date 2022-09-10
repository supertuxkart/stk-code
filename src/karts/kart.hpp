//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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

/**
  * \defgroup karts
  * Contains classes that deal with the properties, models and physics
  * of karts.
  */

#include "LinearMath/btTransform.h"

#include "items/powerup_manager.hpp"    // For PowerupType
#include "karts/abstract_kart.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"

#include <SColor.h>

class AbstractKartAnimation;
class Attachment;
class btKart;
class btUprightConstraint;
class Controller;
class HitEffect;
class Item;
class ItemState;
class KartGFX;
class KartRewinder;
class MaxSpeed;
class ParticleEmitter;
class ParticleKind;
class SFXBase;
class Shadow;
class Skidding;
class SkidMarks;
class SlipStream;
class Stars;
class TerrainInfo;

/** The main kart class. All type of karts are of this object, but with
 *  different controllers. The controllers are what turn a kart into a
 *  player kart (i.e. the controller handle input), or an AI kart (the
 *  controller runs the AI code to set steering etc).
 *  Kart has two base classes: the most important one is moveable (which
 *  is an object that is moved on the track, and has position and rotations)
 *  and TerrainInfo, which manages the terrain the kart is on.
 * \ingroup karts
 */
class Kart : public AbstractKart
{
    friend class Skidding;
private:
    int m_network_finish_check_ticks;
    int m_network_confirmed_finish_ticks;
protected:
    /** Offset of the graphical kart chassis from the physical chassis. */
    float m_graphical_y_offset;

    /** The coordinates of the front of the kart, used to determine when a
     *  new lap is triggered. */
    Vec3 m_xyz_front;

    /* Determines the time covered by the history size, in seconds */
    const float XYZ_HISTORY_TIME = 0.25f;

    /* Determines the number of previous XYZ positions of the kart to remember
       Initialized in the constructor and unchanged from then on */
    int m_xyz_history_size;

    /** The coordinates of the XYZ_HISTORY_SIZE previous positions */
    std::vector<Vec3> m_previous_xyz;

    /** The times at which the previous positions occured.
        Currently used for finish time computation */
    std::vector<float> m_previous_xyz_times;

    float m_time_previous_counter;

    /** Is time flying activated */
    bool m_is_jumping;

    /** The sign of torque to apply after hitting a bubble gum. */
    bool        m_bubblegum_torque_sign;

    /** A short time after a collision acceleration is disabled to allow
     *  the karts to bounce back*/
    uint8_t      m_bounce_back_ticks;

protected:
    /** Handles speed increase and capping due to powerup, terrain, ... */
    MaxSpeed *m_max_speed;

    /** Stores information about the terrain the kart is on. */
    TerrainInfo *m_terrain_info;

    /** Handles the powerup of a kart. */
    Powerup *m_powerup;

    std::unique_ptr<btVehicleRaycaster> m_vehicle_raycaster;

    std::unique_ptr<btKart> m_vehicle;

    /** This object handles all skidding. */
    std::unique_ptr<Skidding> m_skidding;

    /** For stars rotating around head effect */
    std::unique_ptr<Stars> m_stars_effect;

    // Graphical effects
    // -----------------

#ifndef SERVER_ONLY
    /** The shadow of a kart. */
    std::unique_ptr<Shadow> m_shadow;

    /** The skidmarks object for this kart. */
    std::unique_ptr<SkidMarks> m_skidmarks;
#endif

    /** All particle effects. */
    std::unique_ptr<KartGFX> m_kart_gfx;

    /** Handles all slipstreaming. */
    std::unique_ptr<SlipStream> m_slipstream;

    // Bullet physics parameters
    // -------------------------
    struct btCompoundShapeDeleter
    {
        void operator()(btCompoundShape* p) const
        {
            for(int i = 0; i< p->getNumChildShapes(); i++)
                delete p->getChildShape(i);
            delete p;
        }
    };
    std::unique_ptr<btCompoundShape, btCompoundShapeDeleter> m_kart_chassis;

    /** For collisions */
    ParticleEmitter *m_collision_particles;

    /** The main controller of this object, used for driving. This
     *  controller is used to run the kart. It will be replaced
     *  with an end kart controller when the kart finishes the race. */
    Controller  *m_controller;

    /** This saves the original controller when the end controller is
     *  used. This is an easy solution for restarting the race, since
     *  the controller do not need to be reinitialised. */
    Controller  *m_saved_controller;

    /** Remember the last **used** powerup type of a kart for AI purposes. */
    PowerupManager::PowerupType m_last_used_powerup;

    /** True if kart is flying (for debug purposes only). */
    bool m_flying;

    /** Set when hitting bubblegum */
    bool m_has_caught_nolok_bubblegum;

    /** True if the kart wins, false otherwise. */
    bool m_race_result;

    /** True if the kart is eliminated. */
    bool m_eliminated;

    /** Initial rank of the kart. */
    int m_initial_position;

    /** Current race position (1-num_karts). */
    int m_race_position;

    /** Maximum engine rpm's for the current gear. */
    float        m_max_gear_rpm;

    /** How long the brake key has been pressed - the longer the harder
     *  the kart will brake. */
    int          m_brake_ticks;

    /** Time a kart is invulnerable. */
    int16_t      m_invulnerable_ticks;

    /** If > 0 then bubble gum effect is on. This is the sliding when hitting a gum on the floor, not the shield. */
    int16_t      m_bubblegum_ticks;

    /** When a kart has its view blocked by the plunger, this variable will be
     *  > 0 the number it contains is the time left before removing plunger. */
    int16_t       m_view_blocked_by_plunger;

    /** Current leaning of the kart. */
    float        m_current_lean;

    /** To prevent using nitro in too short bursts */
    int8_t        m_min_nitro_ticks;

    /** True if fire button was pushed and not released */
    bool         m_fire_clicked;

    /** True if the kart has been selected to have a boosted ai */
    bool         m_boosted_ai;

    bool            m_finished_race;

    float           m_finish_time;

     /** The amount of energy collected with nitro cans. Note that it
      *  must be float, since dt is subtraced in each timestep. */
    float         m_collected_energy;

    float         m_consumption_per_tick;

    float         m_energy_to_min_ratio;

    float           m_startup_boost;

    float           m_falling_time;

    float           m_weight;

    /** The current speed (i.e. length of velocity vector) of this kart. */
    float         m_speed;

    /** For smoothing engine sound**/
    float         m_last_factor_engine_sound;

    /** For changeKart**/
    float         m_default_suspension_force;

    /** Reset position. */
    btTransform  m_reset_transform;

    std::vector<SFXBase*> m_custom_sounds;
    int m_emitter_id = 0;
    static const int EMITTER_COUNT = 3;
    SFXBase      *m_emitters[EMITTER_COUNT];
    SFXBase      *m_engine_sound;
    /** Sound to be played depending on terrain. */
    SFXBase      *m_terrain_sound;

    /** The material for which the last sound effect was played. */
    const Material *m_last_sound_material;

    SFXBase      *m_nitro_sound;
    /** A pointer to the previous terrain sound needs to be saved so that an
     *  'older' sfx can be finished and an abrupt end of the sfx is avoided. */
    SFXBase      *m_previous_terrain_sound;
    SFXBase      *m_skid_sound;
    SFXBuffer    *m_horn_sound;
    static const int CRASH_SOUND_COUNT = 3;
    SFXBuffer    *m_crash_sounds[CRASH_SOUND_COUNT];
    SFXBuffer    *m_goo_sound;
    SFXBuffer    *m_boing_sound;
    /* Used to avoid re-play the sound during rewinding, if it's happening at
     * the same ticks. */
    int          m_ticks_last_crash;
    int          m_ticks_last_zipper;
    RaceManager::KartType m_type;

    void          updatePhysics(int ticks);
    void          handleMaterialSFX();
    void          handleMaterialGFX(float dt);
    void          updateFlying();
    void          updateSliding();
    void          updateEnginePowerAndBrakes(int ticks);
    void          updateEngineSFX(float dt);
    void          updateSpeed();
    void          updateNitro(int ticks);
    float         applyAirFriction (float engine_power);
    float         getActualWheelForce();
    void          playCrashSFX(const Material* m, AbstractKart *k);
    void          loadData(RaceManager::KartType type, bool animatedModel);
    void          updateWeight();
    void          initSound();
public:
                   Kart(const std::string& ident, unsigned int world_kart_id,
                        int position, const btTransform& init_transform,
                        HandicapLevel handicap,
                        std::shared_ptr<GE::GERenderInfo> ri);
    virtual       ~Kart();
    virtual void   init(RaceManager::KartType type) OVERRIDE;
    virtual void   kartIsInRestNow() OVERRIDE;
    virtual void   updateGraphics(float dt) OVERRIDE;
    virtual void   createPhysics    ();
    virtual bool   isInRest         () const OVERRIDE;
    virtual void   applyEngineForce (float force);

    virtual void   flyUp() OVERRIDE;
    virtual void   flyDown() OVERRIDE;

    virtual void   startEngineSFX   () OVERRIDE;
    virtual void  collectedItem(ItemState *item) OVERRIDE;
    virtual float getStartupBoostFromStartTicks(int ticks) const OVERRIDE;
    virtual float getStartupBoost() const OVERRIDE  { return m_startup_boost; }
    virtual void setStartupBoost(float val) OVERRIDE { m_startup_boost = val; }
    virtual const Material *getMaterial() const OVERRIDE;
    virtual const Material *getLastMaterial() const OVERRIDE;
    /** Returns the pitch of the terrain depending on the heading. */
    virtual float getTerrainPitch(float heading) const OVERRIDE;

    virtual void   reset            () OVERRIDE;
    virtual void   handleZipper     (const Material *m=NULL,
                                     bool play_sound=false) OVERRIDE;
    virtual bool   setSquash        (float time, float slowdown) OVERRIDE;
            void   setSquashGraphics();
    virtual void   unsetSquash      () OVERRIDE;

    virtual void   crashed          (AbstractKart *k, bool update_attachments) OVERRIDE;
    virtual void   crashed          (const Material *m, const Vec3 &normal) OVERRIDE;
    virtual float  getHoT           () const OVERRIDE;
    virtual void   update           (int ticks) OVERRIDE;
    virtual void   finishedRace     (float time, bool from_server=false) OVERRIDE;
    virtual void   setPosition      (int p) OVERRIDE;
    virtual void   beep             () OVERRIDE;
    virtual void   showZipperFire   () OVERRIDE;


    virtual bool   playCustomSFX    (unsigned int type) OVERRIDE;
    virtual void   setController(Controller *controller) OVERRIDE;
    virtual void   setXYZ(const Vec3& a) OVERRIDE;
    virtual void changeKart(const std::string& new_ident,
                            HandicapLevel handicap,
                            std::shared_ptr<GE::GERenderInfo> ri,
                            const KartData& kart_data = KartData()) OVERRIDE;

    // ========================================================================================
    // SPEED and speed-boost related functions
    // ----------------------------------------------------------------------------------------
    virtual void   adjustSpeed      (float f) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual void   increaseMaxSpeed(unsigned int category, float add_speed,
                                    float engine_force, int duration,
                                    int fade_out_time) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual void   instantSpeedIncrease(unsigned int category, float add_max_speed,
                                    float speed_boost, float engine_force, 
                                    int duration, int fade_out_time) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual void   setSlowdown(unsigned int category, float max_speed_fraction,
                               int fade_in_time) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual int   getSpeedIncreaseTicksLeft(unsigned int category) const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual float  getSpeed() const OVERRIDE { return m_speed; }
    // ----------------------------------------------------------------------------------------
    virtual float  getCurrentMaxSpeed() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** This is used on the client side only to set the speed of the kart
     *  from the server information. */
    virtual void setSpeed(float s) OVERRIDE { m_speed = s; }

    // ========================================================================================
    // STEERING and skidding related functions
    // ----------------------------------------------------------------------------------------
    /** Returns the maximum steering angle for this kart, which depends on the
     *  speed. */
    virtual float getMaxSteerAngle () const OVERRIDE
                    { return getMaxSteerAngle(getSpeed()); }
    // ----------------------------------------------------------------------------------------
    /** Returns the time till full steering is reached for this kart.
     *  \param steer Current steer value (must be >=0), on which the time till
     *         full steer depends. */
    virtual float getTimeFullSteer(float steer) const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual float  getSpeedForTurnRadius(float radius) const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual float  getMaxSteerAngle(float speed) const;
    // ----------------------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values). */
    virtual const Skidding *getSkidding() const OVERRIDE { return m_skidding.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values) - non-const. */
    virtual Skidding *getSkidding() OVERRIDE { return m_skidding.get(); }

    // ========================================================================================
    // NITRO related functions.
    // ----------------------------------------------------------------------------------------
    /** Returns the remaining collected energy. */
    virtual float getEnergy() const OVERRIDE { return m_collected_energy; }
    // ----------------------------------------------------------------------------------------
    /** Sets the energy the kart has collected. */
    virtual void setEnergy(float val) OVERRIDE { m_collected_energy = val; }
    // ----------------------------------------------------------------------------------------
    /** Return whether nitro is being used despite the nitro button not being
     *  pressed due to minimal use time requirements
     */
    virtual bool isOnMinNitroTime() const OVERRIDE { return m_min_nitro_ticks > 0; }

    // ========================================================================================
    // POWERUP related functions.
    // ----------------------------------------------------------------------------------------
    /** Sets a new powerup. */
    virtual void setPowerup (PowerupManager::PowerupType t, int n) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Sets the last used powerup. */
    virtual void setLastUsedPowerup (PowerupManager::PowerupType t);
    // ----------------------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual const Powerup* getPowerup() const OVERRIDE { return m_powerup; }
    // ----------------------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual Powerup* getPowerup() OVERRIDE  { return m_powerup; }
    // ----------------------------------------------------------------------------------------
    /** Returns the last used powerup. */
    virtual PowerupManager::PowerupType getLastUsedPowerup() OVERRIDE
    {
        return m_last_used_powerup;
    }
    // ----------------------------------------------------------------------------------------
    /** Returns the number of powerups. */
    virtual int getNumPowerup() const OVERRIDE;

    // ========================================================================================
    // SPECIAL-STATUS related functions (plunger, squash, shield, immunity).
    // ----------------------------------------------------------------------------------------
    /** Makes a kart invulnerable for a certain amount of time. */
    virtual void setInvulnerableTicks(int ticks) OVERRIDE
    {
        // int16_t max
        if (ticks > 32767)
            ticks = 32767;
        m_invulnerable_ticks = ticks;
    }   // setInvulnerableTicks
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is invulnerable. */
    virtual bool isInvulnerable() const OVERRIDE { return m_invulnerable_ticks > 0; }
    // ----------------------------------------------------------------------------------------
    /** Returns true if the kart has a plunger attached to its face. */
    virtual int getBlockedByPlungerTicks() const OVERRIDE
                                         { return m_view_blocked_by_plunger; }
    // ----------------------------------------------------------------------------------------
    /** Sets the view to blocked by a plunger. The duration depends on
     *  the difficulty, see KartProperties getPlungerInFaceTime. */
    virtual void blockViewWithPlunger() OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Enables a kart shield protection for a certain amount of time. */
    virtual void setShieldTime(float t) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is protected by a shield. */
    virtual bool isShielded() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the remaining time the kart is protected by a shield. */
    virtual float getShieldTime() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Decreases the kart's shield time. */
    virtual void decreaseShieldTime() OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is currently being squashed. */
    virtual bool isSquashed() const OVERRIDE;

    // ========================================================================================
    // CONTROLLER related functions
    // ----------------------------------------------------------------------------------------
    virtual void  setBoostAI     (bool boosted) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual bool  getBoostAI     () const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the controller of this kart. */
    virtual Controller* getController() OVERRIDE { return m_controller; }
    // ----------------------------------------------------------------------------------------
    /** Returns the controller of this kart (const version). */
    const Controller* getController() const OVERRIDE { return m_controller; }

    // ========================================================================================
    // LOCATION ON-TRACK related functions
    // ----------------------------------------------------------------------------------------
    /** Returns the coordinates of the front of the kart. This is used for
     *  determining when the lap line is crossed. */
    virtual const Vec3& getFrontXYZ() const OVERRIDE { return m_xyz_front; }
    // -----------------------------------------------------------------------------------------
    /** Returns a bullet transform object located at the kart's position
        and oriented in the direction the kart is going. Can be useful
        e.g. to calculate the starting point and direction of projectiles. */
    virtual btTransform getAlignedTransform(const float customPitch=-1) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the start transform, i.e. position and rotation. */
    const btTransform& getResetTransform() const {return m_reset_transform;}
    // ----------------------------------------------------------------------------------------
    /** True if the wheels are touching the ground. */
    virtual bool isOnGround() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns true if the kart is close to the ground, used to dis/enable
     *  the upright constraint to allow for more realistic explosions. */
    bool isNearGround() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the normal of the terrain the kart is over atm. This is
     *  defined even if the kart is flying. */
    virtual const Vec3& getNormal() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the position 0.25s before */
    virtual const Vec3& getPreviousXYZ() const OVERRIDE
            { return m_previous_xyz[m_xyz_history_size-1]; }
    // ----------------------------------------------------------------------------------------
    /** Returns a more recent different previous position */
    virtual const Vec3& getRecentPreviousXYZ() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the time at which the recent previous position occured */
    virtual const float getRecentPreviousXYZTime() const OVERRIDE
            { return m_previous_xyz_times[m_xyz_history_size/5]; }
    // ----------------------------------------------------------------------------------------
    /** For debugging only: check if a kart is flying. */
    bool isFlying() const { return m_flying;  }
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart is jumping. */
    virtual bool isJumping() const OVERRIDE { return m_is_jumping; }
    // ----------------------------------------------------------------------------------------
    /** Returns the terrain info oject. */
    virtual const TerrainInfo *getTerrainInfo() const OVERRIDE { return m_terrain_info; }

    // ========================================================================================
    // ----------------------------------------------------------------------------------------
    /** Returns a pointer to this kart's graphical effects. */
    virtual KartGFX* getKartGFX() OVERRIDE         { return m_kart_gfx.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the current position of this kart in the race. */
    virtual int getPosition() const OVERRIDE { return m_race_position; }
    // ----------------------------------------------------------------------------------------
    /** Returns the initial position of this kart. */
    virtual int getInitialPosition () const OVERRIDE { return m_initial_position; }
    // ----------------------------------------------------------------------------------------
    /** Returns the finished time for a kart. */
    virtual float getFinishTime () const OVERRIDE { return m_finish_time; }
    // ----------------------------------------------------------------------------------------
    /** Returns true if this kart has finished the race. */
    virtual bool hasFinishedRace () const OVERRIDE { return m_finished_race; }
    // -----------------------------------------------------------------------------------------
    /** Returns the color used for this kart. */
    const irr::video::SColor &getColor() const;
    // ----------------------------------------------------------------------------------------
    virtual RaceManager::KartType getType() const OVERRIDE { return m_type; }
    // ----------------------------------------------------------------------------------------
    /** Returns the bullet vehicle which represents this kart. */
    virtual btKart *getVehicle() const OVERRIDE { return m_vehicle.get(); }
    // ----------------------------------------------------------------------------------------
    virtual btQuaternion getVisualRotation() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual const SlipStream* getSlipstream() const OVERRIDE { return m_slipstream.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual SlipStream* getSlipstream() OVERRIDE  {return m_slipstream.get(); }
    // ----------------------------------------------------------------------------------------
    /** Activates a slipstream effect, atm that is display some nitro. */
    virtual void setSlipstreamEffect(float f) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual bool isEliminated() const OVERRIDE { return m_eliminated; }
    // ----------------------------------------------------------------------------------------
    virtual void eliminate() OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual void setOnScreenText(const core::stringw& text) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart wins or loses. */
    virtual bool getRaceResult() const OVERRIDE { return m_race_result;  }
    // ----------------------------------------------------------------------------------------
    /** Set this kart race result. */
    void setRaceResult();
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart is a ghost (replay) kart. */
    virtual bool isGhostKart() const OVERRIDE { return false;  }
    // ----------------------------------------------------------------------------------------
    SFXBase* getNextEmitter();
    // ----------------------------------------------------------------------------------------
    virtual void playSound(SFXBuffer* buffer) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual bool isVisible() const OVERRIDE;
    // ----------------------------------------------------------------------------------------
    /** Shows the star effect for a certain time. */
    virtual void showStarEffect(float t) OVERRIDE;
    // ----------------------------------------------------------------------------------------
    virtual Stars* getStarsEffect() const OVERRIDE
                                               { return m_stars_effect.get(); }
    // ------------------------------------------------------------------------
    /** Return the confirmed finish ticks (sent by the server)
     *  indicating that this kart has really finished the race. */
    int getNetworkConfirmedFinishTicks() const OVERRIDE
                                   { return m_network_confirmed_finish_ticks; }

};   // Kart


#endif

/* EOF */
