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

#include <memory>
#include <SColor.h>

#include "items/powerup_manager.hpp"    // For PowerupType
#include "karts/controller/kart_control.hpp"
#include "karts/moveable.hpp"
#include "LinearMath/btTransform.h"
#include "race/race_manager.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"

namespace irr
{
    namespace scene
    {
        class IDummyTransformationSceneNode;
    }
}

class AbstractKartAnimation;
class Attachment;
class btKart;
class btQuaternion;
class btUprightConstraint;
class Controller;
class HitEffect;
class Item;
class ItemState;
class KartGFX;
class KartModel;
class KartProperties;
class KartRewinder;
class Material;
class MaxSpeed;
class ParticleEmitter;
class ParticleKind;
class Powerup;
namespace GE { class GERenderInfo; }
class SFXBase;
class SFXBuffer;
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
class Kart : public Moveable
{
    friend class Skidding;
private:
    /** Length of the kart, copy of the data from KartModel. */
    float m_kart_length;
    /** Width of the kart, copy of the data from KartModel. */
    float m_kart_width;
    /** Height of the kart, copy of the data from KartModel. */
    float m_kart_height;
    /** Coordinate on up axis */
    float m_kart_highest_point;
    /** The position of all four wheels in the 3d model */
    const Vec3* m_wheel_graphics_position;

    /** Index of kart in world. */
    unsigned int m_world_kart_id;

    /** Name of the kart with translation. */
    core::stringw m_name;

    int m_network_finish_check_ticks;
    int m_network_confirmed_finish_ticks;

    void loadKartProperties(const std::string& new_ident,
                            HandicapLevel handicap,
                            std::shared_ptr<GE::GERenderInfo> ri,
                            const KartData& kart_data = KartData());
protected:
    btTransform m_starting_transform;

    int m_live_join_util;

    /** The kart properties. */
    std::unique_ptr<KartProperties> m_kart_properties;

    /** The handicap level of this kart. */
    HandicapLevel m_handicap;

    /** This stores a copy of the kart model. It has to be a copy
     *  since otherwise incosistencies can happen if the same kart
     *  is used more than once. */
    std::unique_ptr<KartModel> m_kart_model;

    /** Handles the attachment the kart might have. */
    std::unique_ptr<Attachment> m_attachment;

    /** The kart controls (e.g. steering, fire, ...). */
    KartControl  m_controls;

    /** Used to make the steering angle conform with TimeToFullSteer
     *  It should always be between -1 and 1
     *  We put it in Kart because of the constraints from
     *  getSteerPercent, but this should be re-architectured*/
    float m_effective_steer;

    /** A kart animation object to handle rescue, explosion etc. */
    AbstractKartAnimation *m_kart_animation;

    /** Node between wheels and kart. Allows kart to be scaled independent of wheels, when being squashed.*/
    irr::scene::IDummyTransformationSceneNode    *m_wheel_box;

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

    /** Time a kart is invulnerable to basket-ball squashing only. */
    int16_t      m_basket_squash_invulnerable_ticks;

    /** If > 0 then bubble gum effect is on. This is the sliding when hitting a gum on the floor, not the shield. */
    int16_t      m_bubblegum_ticks;

    /** When a kart has its view blocked by the plunger, this variable will be
     *  > 0 the number it contains is the time left before removing plunger. */
    int16_t       m_view_blocked_by_plunger;

    /** Current leaning of the kart. */
    float        m_current_lean;

    /** To prevent using nitro in too short bursts */
    int16_t        m_min_nitro_ticks;

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

    /** If > 0 then nitro-hack effect is on. */
    int16_t       m_nitro_hack_ticks;

    /** The bonus factor for nitro use **/
    float         m_nitro_hack_factor;

    /** Used to display stolen nitro in the UI **/
    int16_t       m_stolen_nitro_ticks;

    float         m_stolen_nitro_amount;

    float           m_startup_boost;

    float           m_startup_engine_force;

    // boost-level 0 corresponds to a start penalty
    // boost-level 1 corresponds to a start without boost or penalty
    // boost-level 2 or more corresponds to a start with boost
    // This is only used for networking
    uint8_t         m_startup_boost_level;

    float           m_falling_time;

    float           m_weight;

    /** A bit mask containing the buckets a random powerup 
     * was already picked from */
    uint32_t      m_powerup_mask;

    /** Contains the list of powerup buckets in the order
     * they were picked, so they can be removed*/
    std::vector <uint8_t> m_powerup_buckets;

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
    void          updateSteering(int ticks);
    void          updateEngineSFX(float dt);
    void          updateSpeed();
    void          updateNitro(int ticks);
    float         compensateLinearSlowdown (float engine_power);
    float         applyAirFriction (float engine_power);
    float         getActualWheelForce();
    void          handleRescue(bool auto_rescue=false);
    void          playCrashSFX(const Material* m, Kart *k);
    void          loadData(RaceManager::KartType type, bool animatedModel);
    void          updateWeight();
    void          initSound();
public:
                   Kart(const std::string& ident, unsigned int world_kart_id,
                        int position, const btTransform& init_transform,
                        HandicapLevel handicap,
                        std::shared_ptr<GE::GERenderInfo> ri);
    virtual       ~Kart();
    /** Returns a name to be displayed for this kart. */
    const core::stringw& getName() const { return m_name; }
    /** Returns the index of this kart in world. */
    unsigned int   getWorldKartId() const         { return m_world_kart_id;   }
    virtual void   init(RaceManager::KartType type);
    // ========================================================================
    // Functions related to controlling the kart
    // ------------------------------------------------------------------------
    /** Returns the current steering value for this kart.
     *  This function exists only to be overriden with steering smoothing
     *  in the rewinder.*/
    virtual float getSteerPercent() const { return m_effective_steer; }
    // ------------------------------------------------------------------------
    virtual void  setEffectiveSteer(float effective_steer)
                            { m_effective_steer = effective_steer; }
    // ------------------------------------------------------------------------
    /** Returns all controls of this kart. */
    KartControl&  getControls() { return m_controls; }
    // ------------------------------------------------------------------------
    /** Returns all controls of this kart - const version. */
    const KartControl& getControls() const { return m_controls; }
    // ========================================================================
    // Access to the kart properties.
    // ------------------------------------------------------------------------
    /** Returns the kart properties of this kart. */
    const KartProperties* getKartProperties() const { return m_kart_properties.get(); }
    // ========================================================================
    /** Change to new kart instancely (used in network live join). */
    virtual void changeKart(const std::string& new_ident,
                            HandicapLevel handicap,
                            std::shared_ptr<GE::GERenderInfo> ri,
                            const KartData& kart_data = KartData());
    // ========================================================================
    // Access to the handicap.
    // ------------------------------------------------------------------------
    /** Returns the handicap of this kart. */
    const HandicapLevel getHandicap() const { return m_handicap; }
    // ------------------------------------------------------------------------
    /** Sets the handicap. */
    void setHandicap(const HandicapLevel h) { m_handicap=h; }
    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this kart (name of the directory the
     *  kart was loaded from). */
    virtual const std::string& getIdent() const;
    // ------------------------------------------------------------------------
    virtual void   updateGraphics(float dt) OVERRIDE;
    virtual void   createPhysics    ();
    // ------------------------------------------------------------------------
    /** Returns true if the kart is 'resting', i.e. (nearly) not moving. */
    virtual bool isInRest () const;
    virtual void   applyEngineForce (float force);

    virtual void   flyUp() OVERRIDE;
    virtual void   flyDown() OVERRIDE;
    // ------------------------------------------------------------------------
    /** Starts the engine sound effect. Called once the track intro phase is
     *  over. */
    virtual void startEngineSFX();
    // ------------------------------------------------------------------------
    /** Called when an item is collected. It will either adjust the collected
     *  energy, or update the attachment or powerup for this kart.
     *  \param item The item that was hit. */
    virtual void  collectedItem(ItemState *item);
    // ------------------------------------------------------------------------
    /** Called when the NitroHack powerup is used. **/
    virtual void  activateNitroHack();
    // ------------------------------------------------------------------------
    /** Returns the nitro hack status of this kart. */
    virtual bool  isNitroHackActive() const { return m_nitro_hack_ticks > 0; }
    // ------------------------------------------------------------------------
    /** Sets the stolen nitro info of this kart. */
    virtual void  setStolenNitro(float amount, float duration);
    virtual bool  hasStolenNitro() const { return m_stolen_nitro_ticks > 0; }
    virtual float getStolenNitro() const  { return m_stolen_nitro_amount; }
    virtual float getEffectiveSteer() const { return m_effective_steer; }
    virtual bool  hasHeldMini() const;
    virtual void setStartupBoostFromStartTicks(int ticks);
    virtual float getStartupBoost() const { return m_startup_boost; }
    virtual uint8_t getStartupBoostLevel() const { return m_startup_boost_level; }
    virtual void setStartupBoost(uint8_t boost_level);
    // ------------------------------------------------------------------------
    /** Returns the current material the kart is on. */
    virtual const Material *getMaterial() const;
    // ------------------------------------------------------------------------
    /** Returns the previous material the kart was one (which might be
     *  the same as getMaterial() ). */
    virtual const Material *getLastMaterial() const;
    // ------------------------------------------------------------------------
    /** Returns the pitch of the terrain depending on the heading. */
    virtual float getTerrainPitch(float heading) const;

    virtual void   reset            ();
    // ------------------------------------------------------------------------
    /** Sets zipper time, and apply one time additional speed boost. It can be
     *  used with a specific material, in which case the zipper parmaters are
     *  taken from this material (parameters that are <0 will be using the
     *  kart-specific values from kart-properties. */
    virtual void   handleZipper     (const Material *m=NULL,
                                     bool play_sound=false,
                                     bool mini_zipper=false);
    /** Squashes this kart: it will scale the kart in up direction, and causes
     *  a slowdown while this kart is squashed.
     *  Returns true if the squash is successful, false otherwise.
     *  \param time How long the kart will be squashed.
     *  \param slowdown Reduction of max speed.    */
    virtual bool   setSquash        (float time, float slowdown);
            void   setSquashGraphics();
    /** Makes the kart unsquashed again. */
    virtual void   unsetSquash      ();
    /** This activates the kart's electro-shield. */
    virtual void   setElectroShield();
    /** This disables the kart's electro-shield */
    virtual void   unsetElectroShield();

    // ------------------------------------------------------------------------
    /** Called when the kart crashes against another kart.
     *  \param k The kart that was hit.
     *  \param update_attachments If true the attachment of this kart and the
     *          other kart hit will be updated (e.g. bombs will be moved). */
    virtual void   crashed          (Kart *k, bool update_attachments);
    virtual void   crashed          (const Material *m, const Vec3 &normal);
    // ------------------------------------------------------------------------
    /** Returns the height of the terrain. we're currently above */
    virtual float  getHoT           () const;
    virtual void   update           (int ticks) OVERRIDE;
    virtual void finishedRace (float time, bool from_server=false);
    // ------------------------------------------------------------------------
    /** Sets the position of this kart in the race. */
    virtual void   setPosition      (int p);
    // ------------------------------------------------------------------------
    /** Plays a beep sfx. */
    virtual void   beep             ();
    // ------------------------------------------------------------------------
    /** Show fire to go with a zipper. */
    virtual void   showZipperFire   ();
    // ------------------------------------------------------------------------
    /** Get the value of the powerup bitmask. */
    virtual uint32_t getPowerupMask () { return m_powerup_mask; }
    // ------------------------------------------------------------------------
    /** Set a new powerup mask */
    virtual void   updatePowerupMask   (int bucket);
    // ------------------------------------------------------------------------
    /** This function will play a particular character voice for this kart.
     *  It returns whether or not a character voice sample exists for the
     *  particular event.  If there is no voice sample, a default can be
     *  played instead. */
    virtual bool   playCustomSFX    (unsigned int type);
    /** Saves the old controller in m_saved_controller and stores a new
     *  controller. The save controller is needed in case of a reset.
     *  \param controller The new controller to use (atm it's always an
     *         end controller). */
    virtual void   setController(Controller *controller);
    virtual void   setXYZ(const Vec3& a) OVERRIDE;
    // ========================================================================================
    // SPEED and speed-boost related functions
    // ----------------------------------------------------------------------------------------
    /** Multiplies the velocity of the kart by a factor f (both linear
     *  and angular). This is used by anvils, which suddenly slow down the kart
     *  when they are attached. */
    virtual void adjustSpeed(float f);
    // ----------------------------------------------------------------------------------------
    /** Sets an increased maximum speed for a category.
     *  \param category The category for which to set the higher maximum speed.
     *  \param add_speed How much speed (in m/s) is added to the maximum speed.
     *  \param engine_force Additional engine force to affect the kart.
     *  \param duration How long the speed increase will last.
     *  \param fade_out_time How long the maximum speed will fade out linearly.
     */
    virtual void   increaseMaxSpeed(unsigned int category, float add_speed,
                                    float engine_force, int duration,
                                    int fade_out_time);
    // ----------------------------------------------------------------------------------------
    /** This adjusts the top speed using increaseMaxSpeed, but additionally
     *  causes an instant speed boost, which can be smaller than add-max-speed.
     *  (e.g. a zipper can give an instant boost of 5 m/s, but over time would
     *  allow the speed to go up by 10 m/s).
     *  \param category The category for which the speed is increased.
     *  \param add_max_speed Increase of the maximum allowed speed.
     *  \param speed_boost An instant speed increase for this kart.
     *  \param engine_force Additional engine force.
     *  \param duration Duration of the increased speed.
     *  \param fade_out_time How long the maximum speed will fade out linearly.
     */
    virtual void   instantSpeedIncrease(unsigned int category, float add_max_speed,
                                    float speed_boost, float engine_force, 
                                    int duration, int fade_out_time);
    // ----------------------------------------------------------------------------------------
    /** Defines a slowdown, which is in fraction of top speed.
     *  \param category The category for which the speed is increased.
     *  \param max_speed_fraction Fraction of top speed to allow only.
     *  \param fade_in_time How long till maximum speed is capped. */
    virtual void   setSlowdown(unsigned int category, float max_speed_fraction,
                               int fade_in_time);
    // ----------------------------------------------------------------------------------------
    /** Returns how much increased speed time is left over in the given
     *  category.
     *  \param category Which category to report on. */
    virtual int   getSpeedIncreaseTicksLeft(unsigned int category) const;
    // ----------------------------------------------------------------------------------------
    /** Returns the speed of the kart in meters/second. */
    virtual float  getSpeed() const { return m_speed; }
    // ----------------------------------------------------------------------------------------
    /** Returns the current maximum speed for this kart, this includes all
     *  bonus and maluses that are currently applied. */
    virtual float  getCurrentMaxSpeed() const;
    // ----------------------------------------------------------------------------------------
    /** This is used on the client side only to set the speed of the kart
     *  from the server information. */
    virtual void setSpeed(float s) { m_speed = s; }

    // ========================================================================================
    // STEERING and skidding related functions
    // ----------------------------------------------------------------------------------------
    /** Returns the maximum steering angle for this kart, which depends on the
     *  speed. */
    virtual float getMaxSteerAngle () const { return getMaxSteerAngle(getSpeed()); }
    // ----------------------------------------------------------------------------------------
    /** Returns the time till full steering is reached for this kart.
     *  \param steer Current steer value (must be >=0), on which the time till
     *         full steer depends. */
    virtual float getTimeFullSteer(float steer) const;
    // ----------------------------------------------------------------------------------------
    /** Returns the (maximum) speed for a given turn radius.
     *  \param radius The radius for which the speed needs to be computed. */
    virtual float  getSpeedForTurnRadius(float radius) const;
    // ----------------------------------------------------------------------------------------
    virtual float  getMaxSteerAngle(float speed) const;
    // ----------------------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values). */
    virtual const Skidding *getSkidding() const { return m_skidding.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values) - non-const. */
    virtual Skidding *getSkidding() { return m_skidding.get(); }

    // ========================================================================
    // Attachment related functions.
    // ------------------------------------------------------------------------
    /** Returns the current attachment. */
    const Attachment* getAttachment() const {return m_attachment.get(); }
    // ------------------------------------------------------------------------
    /** Returns the current attachment, non-const version. */
    Attachment*    getAttachment() {return m_attachment.get(); }

    // ========================================================================
    // Access to the graphical kart model.
    // ------------------------------------------------------------------------
    /** Returns this kart's kart model. */
    KartModel* getKartModel() const { return m_kart_model.get();      }
    // ------------------------------------------------------------------------
    /** Returns the length of the kart. */
    float getKartLength() const { return m_kart_length; }
    // ------------------------------------------------------------------------
    /** Returns the height of the kart. */
    float getKartHeight() const { return m_kart_height; }
    // ------------------------------------------------------------------------
    /** Returns the width of the kart. */
    float getKartWidth() const {return m_kart_width; }
    // ------------------------------------------------------------------------
    /** Returns the highest point of the kart (coordinate on up axis) */
    float getHighestPoint() const { return m_kart_highest_point;  }
    // ------------------------------------------------------------------------
    /** Called after the kart comes to rest. It can be used to e.g. compute
     *  differences between graphical and physical chassis. Note that
     *  overwriting this function is possible, but this implementation must
     *  be called. */
    virtual void kartIsInRestNow();

    // ========================================================================
    // Emergency animation related functions.
    // ------------------------------------------------------------------------
    /** Returns a kart animation (if any), or NULL if currently no kart
     *  animation is being shown. */
    AbstractKartAnimation *getKartAnimation() { return m_kart_animation; }
    // ------------------------------------------------------------------------
    const AbstractKartAnimation *getKartAnimation() const { return m_kart_animation; }
    // ------------------------------------------------------------------------
    /** Sets a new kart animation. */
    virtual void setKartAnimation(AbstractKartAnimation *ka);
    // ------------------------------------------------------------------------

    // ========================================================================================
    // NITRO related functions.
    // ----------------------------------------------------------------------------------------
    /** Returns the remaining collected energy. */
    virtual float getEnergy() const { return m_collected_energy; }
    // ----------------------------------------------------------------------------------------
    /** Allows to add nitro while enforcing max nitro storage. */
    virtual void addEnergy(float val, bool allow_negative);
    // ----------------------------------------------------------------------------------------
    /** Sets the energy the kart has collected. */
    virtual void setEnergy(float val) { m_collected_energy = val; }
    // ----------------------------------------------------------------------------------------
    /** Return whether nitro is being used despite the nitro button not being
     *  pressed due to minimal use time requirements
     */
    virtual bool isOnMinNitroTime() const { return m_min_nitro_ticks > 0; }

    // ========================================================================================
    // POWERUP related functions.
    // ----------------------------------------------------------------------------------------
    /** Sets a new powerup. */
    virtual void setPowerup (PowerupManager::PowerupType t, int n);
    // ----------------------------------------------------------------------------------------
    /** Sets the last used powerup. */
    virtual void setLastUsedPowerup (PowerupManager::PowerupType t);
    // ----------------------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual const Powerup* getPowerup() const { return m_powerup; }
    // ----------------------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual Powerup* getPowerup() { return m_powerup; }
    // ----------------------------------------------------------------------------------------
    /** Returns the last used powerup. */
    virtual PowerupManager::PowerupType getLastUsedPowerup() { return m_last_used_powerup; }
    // ----------------------------------------------------------------------------------------
    /** Returns the number of powerups. */
    virtual int getNumPowerup() const;

    // ========================================================================================
    // SPECIAL-STATUS related functions (plunger, squash, shield, immunity).
    // ----------------------------------------------------------------------------------------
    /** Makes a kart invulnerable for a certain amount of time. */
    virtual void setInvulnerableTicks(int ticks)
    {
        // int16_t max
        if (ticks > 32767)
            ticks = 32767;
        m_invulnerable_ticks = ticks;
    }   // setInvulnerableTicks
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is invulnerable. */
    virtual bool isInvulnerable() const { return m_invulnerable_ticks > 0; }
    // ----------------------------------------------------------------------------------------
    /** Makes a kart invulnerable to basketball squashingfor a certain amount of time. */
    virtual void setBasketSquashImmunityTicks(int ticks)
    {
        // int16_t max
        if (ticks > 32767)
            ticks = 32767;
        m_basket_squash_invulnerable_ticks = ticks;
    }   // setBasketSquashImmunityTicks
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is invulnerable to basket ball squashing. */
    virtual bool hasBasketSquashImmunity() const { return m_basket_squash_invulnerable_ticks > 0; }
    // ----------------------------------------------------------------------------------------
    /** Returns true if the kart has a plunger attached to its face. */
    virtual int getBlockedByPlungerTicks() const { return m_view_blocked_by_plunger; }
    // ----------------------------------------------------------------------------------------
    /** Sets the view to blocked by a plunger. The duration depends on
     *  the difficulty, see KartProperties getPlungerInFaceTime. */
    virtual void blockViewWithPlunger();
    // ----------------------------------------------------------------------------------------
    /** Enables a kart shield protection for a certain amount of time. */
    virtual void setShieldTime(float t);
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is protected by a shield. */
    virtual bool isShielded() const;
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is protected by a gum shield. */
    virtual bool isGumShielded() const;
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is protected by a weak shield. */
    virtual bool isWeakShielded() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the remaining time the kart is protected by a shield. */
    virtual float getShieldTime() const;
    // ----------------------------------------------------------------------------------------
    /** Decreases the kart's shield time. */
    virtual void decreaseShieldTime();
    // ----------------------------------------------------------------------------------------
    /** Returns if the kart is currently being squashed. */
    virtual bool isSquashed() const;

    // ========================================================================================
    // CONTROLLER related functions
    // ----------------------------------------------------------------------------------------
    /** Sets the kart AI boost state.
     *  Not pure abstract, since there is no need to implement this e.g. in Ghost.
     *  \param boosted True if a boost should be applied. */
    virtual void  setBoostAI     (bool boosted);
    // ----------------------------------------------------------------------------------------
    /** Returns the kart AI boost state.
     *  Not pure abstract, since there is no need to implement this e.g. in Ghost. */
    virtual bool  getBoostAI     () const;
    // ----------------------------------------------------------------------------------------
    /** Returns the controller of this kart. */
    virtual Controller* getController() { return m_controller; }
    // ----------------------------------------------------------------------------------------
    /** Returns the controller of this kart (const version). */
    const Controller* getController() const { return m_controller; }

    // ========================================================================================
    // LOCATION ON-TRACK related functions
    // ----------------------------------------------------------------------------------------
    /** Returns the time at which the kart was at a given distance.
      * Returns -1.0f if none */
    virtual float getTimeForDistance(float distance) { return -1.0f; }
    // ------------------------------------------------------------------------
    /** Returns true if this kart has no wheels. */
    bool isWheeless() const;
    // ------------------------------------------------------------------------
    /** Returns the coordinates of the front of the kart. This is used for
     *  determining when the lap line is crossed. */
    virtual const Vec3& getFrontXYZ() const { return m_xyz_front; }
    // ------------------------------------------------------------------------
    /** Returns the position of a wheel relative to the kart.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear
     *           right, 3 = rear left.  */
    const Vec3& getWheelGraphicsPosition(int i) const { assert(i>=0 && i<4); return m_wheel_graphics_position[i]; }
    // -----------------------------------------------------------------------------------------
    /** Returns a bullet transform object located at the kart's position
        and oriented in the direction the kart is going. Can be useful
        e.g. to calculate the starting point and direction of projectiles. */
    virtual btTransform getAlignedTransform(const float customPitch=-1);
    // ----------------------------------------------------------------------------------------
    /** Returns the start transform, i.e. position and rotation. */
    const btTransform& getResetTransform() const {return m_reset_transform;}
    // ----------------------------------------------------------------------------------------
    /** True if the wheels are touching the ground. */
    virtual bool isOnGround() const;
    // ----------------------------------------------------------------------------------------
    /** Returns true if the kart is close to the ground, used to dis/enable
     *  the upright constraint to allow for more realistic explosions. */
    bool isNearGround() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the normal of the terrain the kart is over atm. This is
     *  defined even if the kart is flying. */
    virtual const Vec3& getNormal() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the position 0.25s before */
    virtual const Vec3& getPreviousXYZ() const
            { return m_previous_xyz[m_xyz_history_size-1]; }
    // ----------------------------------------------------------------------------------------
    /** Returns a more recent different previous position */
    virtual const Vec3& getRecentPreviousXYZ() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the time at which the recent previous position occured */
    virtual const float getRecentPreviousXYZTime() const
            { return m_previous_xyz_times[m_xyz_history_size/5]; }
    // ----------------------------------------------------------------------------------------
    /** For debugging only: check if a kart is flying. */
    bool isFlying() const { return m_flying;  }
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart is jumping. */
    virtual bool isJumping() const { return m_is_jumping; }
    // ----------------------------------------------------------------------------------------
    /** Returns the terrain info oject. */
    virtual const TerrainInfo *getTerrainInfo() const { return m_terrain_info; }

    // ========================================================================================
    // ----------------------------------------------------------------------------------------
    /** Returns a pointer to this kart's graphical effects. */
    virtual KartGFX* getKartGFX() { return m_kart_gfx.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the current position of this kart in the race. */
    virtual int getPosition() const { return m_race_position; }
    // ----------------------------------------------------------------------------------------
    /** Returns the initial position of this kart. */
    virtual int getInitialPosition () const { return m_initial_position; }
    // ----------------------------------------------------------------------------------------
    /** Returns the finished time for a kart. */
    virtual float getFinishTime () const { return m_finish_time; }
    // ----------------------------------------------------------------------------------------
    /** Returns true if this kart has finished the race. */
    virtual bool hasFinishedRace () const { return m_finished_race; }
    // -----------------------------------------------------------------------------------------
    /** Returns the color used for this kart. */
    const irr::video::SColor &getColor() const;
    // ----------------------------------------------------------------------------------------
    virtual RaceManager::KartType getType() const { return m_type; }
    // ----------------------------------------------------------------------------------------
    /** Returns the bullet vehicle which represents this kart. */
    virtual btKart *getVehicle() const { return m_vehicle.get(); }
    // ----------------------------------------------------------------------------------------
    virtual btQuaternion getVisualRotation() const;
    // ----------------------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual const SlipStream* getSlipstream() const { return m_slipstream.get(); }
    // ----------------------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual SlipStream* getSlipstream()  {return m_slipstream.get(); }
    // ----------------------------------------------------------------------------------------
    /** Activates a slipstream effect, atm that is display some nitro. */
    virtual void setSlipstreamEffect(float f);
    // ----------------------------------------------------------------------------------------
    /** Returns true if the kart is eliminated. */
    virtual bool isEliminated() const { return m_eliminated; }
    // ----------------------------------------------------------------------------------------
    /** Marks this kart to be eliminated. */
    virtual void eliminate();
    // ----------------------------------------------------------------------------------------
    /** Set a text that is displayed on top of a kart. */
    virtual void setOnScreenText(const core::stringw& text);
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart wins or loses. */
    virtual bool getRaceResult() const { return m_race_result;  }
    // ----------------------------------------------------------------------------------------
    /** Set this kart race result. */
    void setRaceResult();
    // ----------------------------------------------------------------------------------------
    /** Returns whether this kart is a ghost (replay) kart. */
    virtual bool isGhostKart() const { return false;  }
    // ----------------------------------------------------------------------------------------
    SFXBase* getNextEmitter();
    // ----------------------------------------------------------------------------------------
    virtual void playSound(SFXBuffer* buffer);
    // ----------------------------------------------------------------------------------------
    virtual bool isVisible() const;
    // ------------------------------------------------------------------------
    virtual void makeKartRest();
    // ----------------------------------------------------------------------------------------
    /** Shows the star effect for a certain time. */
    virtual void showStarEffect(float t);
    // ----------------------------------------------------------------------------------------
    virtual Stars* getStarsEffect() const { return m_stars_effect.get(); }
    // ------------------------------------------------------------------------
    int getLiveJoinUntilTicks() const              { return m_live_join_util; }
    // ------------------------------------------------------------------------
    void setLiveJoinKart(int util_ticks)     { m_live_join_util = util_ticks; }
    // ------------------------------------------------------------------------
    /** Return the confirmed finish ticks (sent by the server)
     *  indicating that this kart has really finished the race. */
    int getNetworkConfirmedFinishTicks() const { return m_network_confirmed_finish_ticks; }

};   // Kart


#endif

/* EOF */
