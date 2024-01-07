//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#ifndef HEADER_ABSTRACT_KART_HPP
#define HEADER_ABSTRACT_KART_HPP

#include <memory>

#include "items/powerup_manager.hpp"
#include "karts/moveable.hpp"
#include "karts/controller/kart_control.hpp"
#include "race/race_manager.hpp"

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
class Controller;
class Item;
class ItemState;
class KartGFX;
class KartModel;
class KartProperties;
class Material;
class Powerup;
namespace GE { class GERenderInfo; }
class SFXBuffer;
class Skidding;
class SlipStream;
class Stars;
class TerrainInfo;


/** An abstract interface for the actual karts. Some functions are actually
 *  implemented here in order to allow inlining.
 * \ingroup karts
 */
class AbstractKart : public Moveable
{
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

    // ------------------------------------------------------------------------
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

    /** A kart animation object to handle rescue, explosion etc. */
    AbstractKartAnimation *m_kart_animation;

    /** Node between wheels and kart. Allows kart to be scaled independent of wheels, when being squashed.*/
    irr::scene::IDummyTransformationSceneNode    *m_wheel_box;
public:
                   AbstractKart(const std::string& ident,
                                int world_kart_id,
                                int position, const btTransform& init_transform,
                                HandicapLevel handicap,
                                std::shared_ptr<GE::GERenderInfo> ri);
    virtual       ~AbstractKart();
    // ------------------------------------------------------------------------
    /** Returns a name to be displayed for this kart. */
    const core::stringw& getName() const                     { return m_name; }
    // ------------------------------------------------------------------------
    virtual void   reset();
    virtual void   init(RaceManager::KartType type) = 0;
    // ========================================================================
    // Functions related to controlling the kart
    // ------------------------------------------------------------------------
    /** Returns the current steering value for this kart. */
    virtual float getSteerPercent() const { return m_controls.getSteer(); }
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
    const KartProperties* getKartProperties() const
                            { return m_kart_properties.get(); }
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
    /** Returns the maximum steering angle for this kart, which depends on the
     *  speed. */
    virtual float getMaxSteerAngle () const = 0;
    // ------------------------------------------------------------------------
    /** Returns the (maximum) speed for a given turn radius.
     *  \param radius The radius for which the speed needs to be computed. */
    virtual float  getSpeedForTurnRadius(float radius) const = 0;
    // ------------------------------------------------------------------------
    /** Returns the time till full steering is reached for this kart.
     *  This can depend on the current steering value, which must be >= 0.
     */
    virtual float getTimeFullSteer(float steer) const = 0;

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

    // ------------------------------------------------------------------------
    /** Returns the time at which the kart was at a given distance.
      * Returns -1.0f if none */
    virtual float getTimeForDistance(float distance);
    // ------------------------------------------------------------------------
    /** Returns true if this kart has no wheels. */
    bool isWheeless() const;
    // ------------------------------------------------------------------------
    /** Returns the coordinates of the front of the kart. This is used for
     *  determining when the lap line is crossed. */
    virtual const Vec3& getFrontXYZ() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the position of a wheel relative to the kart.
     *  \param i Index of the wheel: 0=front right, 1 = front left, 2 = rear
     *           right, 3 = rear left.  */
    const Vec3& getWheelGraphicsPosition(int i) const
                {assert(i>=0 && i<4); return m_wheel_graphics_position[i];}

    // ========================================================================
    // Emergency animation related functions.
    // ------------------------------------------------------------------------
    /** Returns a kart animation (if any), or NULL if currently no kart
     *  animation is being shown. */
    AbstractKartAnimation *getKartAnimation() { return m_kart_animation; }
    // ------------------------------------------------------------------------
    const AbstractKartAnimation *getKartAnimation() const
                                                   { return m_kart_animation; }
    // ------------------------------------------------------------------------
    /** Sets a new kart animation. */
    virtual void setKartAnimation(AbstractKartAnimation *ka);
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    /** Returns the index of this kart in world. */
    unsigned int   getWorldKartId() const         { return m_world_kart_id;   }
    // ------------------------------------------------------------------------
    /** Saves the old controller in m_saved_controller and stores a new
     *  controller. The save controller is needed in case of a reset.
     *  \param controller The new controller to use (atm it's always an
     *         end controller). */
    virtual void setController(Controller *controller) = 0;
    // ------------------------------------------------------------------------
    /** Returns the controller of this kart. */
    virtual Controller* getController() = 0;
    // ------------------------------------------------------------------------
    /** Returns the controller of this kart (const version). */
    virtual const Controller* getController() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values). */
    virtual const Skidding *getSkidding() const = 0;
    // ------------------------------------------------------------------------
    virtual RaceManager::KartType getType() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the skidding object for this kart (which can be used to query
     *  skidding related values), non-const. */
    virtual Skidding *getSkidding() = 0;
    // ------------------------------------------------------------------------
    /** Returns true if the kart is eliminated. */
    virtual bool isEliminated() const = 0;
    // ------------------------------------------------------------------------
    /** Marks this kart to be eliminated. */
    virtual void eliminate() = 0;
    // ------------------------------------------------------------------------
    virtual void finishedRace(float time, bool from_server=false) = 0;
    // ------------------------------------------------------------------------
    /** Returns the finished time for a kart. */
    virtual float getFinishTime() const = 0;
    // ------------------------------------------------------------------------
    /** Returns true if the kart has a plunger attached to its face. */
    virtual int getBlockedByPlungerTicks() const = 0;
    // ------------------------------------------------------------------------
    /** Sets that the view is blocked by a plunger. The duration depends on
     *  the difficulty, see KartPorperties getPlungerInFaceTime. */
    virtual void blockViewWithPlunger() = 0;
    // ------------------------------------------------------------------------
    /** Returns if the kart is currently being squashed. */
    virtual bool isSquashed() const = 0;
    // ------------------------------------------------------------------------
    /** Squashes this kart: it will scale the kart in up direction, and causes
     *  a slowdown while this kart is squashed.
     *  Returns true if the squash is successful, false otherwise.
     *  \param time How long the kart will be squashed.
     *  \param slowdown Reduction of max speed.    */
    virtual bool setSquash(float time, float slowdown) = 0;
    // ------------------------------------------------------------------------
    /** Makes the kart unsquashed again. */
    virtual void unsetSquash() = 0;
    // ------------------------------------------------------------------------
    /** Returns the speed of the kart in meters/second. This is not declared
     *  pure abstract, since this function is not needed for certain classes,
     *  like Ghost. */
    virtual float getSpeed() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the current maximum speed for this kart, this includes all
     *  bonus and maluses that are currently applied. */
    virtual float getCurrentMaxSpeed() const = 0;
    // ------------------------------------------------------------------------
    /** Returns how much increased speed time is left over in the given
     *  category.
     *  \param category Which category to report on. */
    virtual int getSpeedIncreaseTicksLeft(unsigned int category) const = 0;
    
    // ------------------------------------------------------------------------
    /** Sets the kart AI boost state.
     *  Not pure abstract, since there is no need to implement this e.g. in Ghost.
     *  \param boosted True if a boost should be applied. */
    virtual void setBoostAI(bool boosted) = 0;
    
    // ------------------------------------------------------------------------
    /** Returns the kart AI boost state.
     *  Not pure abstract, since there is no need to implement this e.g. in Ghost. */
    virtual bool getBoostAI() const = 0;
    
    // ------------------------------------------------------------------------
    
    /** Sets an increased maximum speed for a category.
     *  \param category The category for which to set the higher maximum speed.
     *  \param add_speed How much speed (in m/s) is added to the maximum speed.
     *  \param engine_force Additional engine force to affect the kart.
     *  \param duration How long the speed increase will last.
     *  \param fade_out_time How long the maximum speed will fade out linearly.
     */
    virtual void increaseMaxSpeed(unsigned int category, float add_speed,
                                  float engine_force, int duration,
                                  int fade_out_time) = 0;

    // ----------------------------------------------------------------------------
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
    virtual void instantSpeedIncrease(unsigned int category, float add_max_speed,
                                    float speed_boost, float engine_force,
                                    int duration, int fade_out_time) = 0;

    // ------------------------------------------------------------------------
    /** Defines a slowdown, which is in fraction of top speed.
     *  \param category The category for which the speed is increased.
     *  \param max_speed_fraction Fraction of top speed to allow only.
     *  \param fade_in_time How long till maximum speed is capped. */
    virtual void setSlowdown(unsigned int category, float max_speed_fraction,
                             int fade_in_time) = 0;
    // ------------------------------------------------------------------------
    /** Returns the remaining collected energy. */
    virtual float getEnergy() const = 0;
    // ------------------------------------------------------------------------
    /** Called when an item is collected. It will either adjust the collected
     *  energy, or update the attachment or powerup for this kart.
     *  \param item The item that was hit. */
    virtual void  collectedItem(ItemState *item_state) = 0;
    // ------------------------------------------------------------------------
    /** Returns the current position of this kart in the race. */
    virtual int getPosition() const = 0;
    // ------------------------------------------------------------------------
    /** Set the current position of this kart in the race. */
    virtual void setPosition(int p) = 0;
    // ------------------------------------------------------------------------
    /** Returns the initial position of this kart. */
    virtual int getInitialPosition() const = 0;
    // ------------------------------------------------------------------------
    /** True if the wheels are touching the ground. */
    virtual bool isOnGround() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual const SlipStream* getSlipstream() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the slipstream object of this kart. */
    virtual SlipStream* getSlipstream() = 0;
    // ------------------------------------------------------------------------
    /** Activates a slipstream effect, atm that is display some nitro. */
    virtual void setSlipstreamEffect(float f) = 0;
    // ------------------------------------------------------------------------
    /** Plays a beep sfx. */
    virtual void beep() = 0;
    // ------------------------------------------------------------------------
    /** This function will play a particular character voice for this kart.
     *  It returns whether or not a character voice sample exists for the
     *  particular event.  If there is no voice sample, a default can be
     *  played instead. */
    virtual bool playCustomSFX(unsigned int type) = 0;
    // ------------------------------------------------------------------------
    /** Show fire to go with a zipper. */
    virtual void showZipperFire() = 0;
    // ------------------------------------------------------------------------
    /** Sets zipper time, and apply one time additional speed boost. It can be
     *  used with a specific material, in which case the zipper parmaters are
     *  taken from this material (parameters that are <0 will be using the
     *  kart-specific values from kart-properties. */
    virtual void handleZipper(const Material *m=NULL,
                              bool play_sound=false) = 0;
    // ------------------------------------------------------------------------
    /** Returns true if this kart has finished the race. */
    virtual bool hasFinishedRace() const = 0;
    // ------------------------------------------------------------------------
    virtual void setEnergy(float val) = 0;
    // ------------------------------------------------------------------------
    /** Return whether nitro is being used despite the nitro button not being
     *  pressed due to minimal use time requirements
     */
    virtual bool isOnMinNitroTime() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the current material the kart is on. */
    virtual const Material *getMaterial() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the previous material the kart was one (which might be
     *  the same as getMaterial() ). */
    virtual const Material *getLastMaterial() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual const Powerup *getPowerup() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the current powerup. */
    virtual Powerup *getPowerup() = 0;
    // ------------------------------------------------------------------------
    /** Returns the last used powerup type. */
    virtual PowerupManager::PowerupType getLastUsedPowerup() = 0;
    // ------------------------------------------------------------------------
    /** Returns the number of powerups. */
    virtual int getNumPowerup() const = 0;
    // ------------------------------------------------------------------------
    /** Returns a points to this kart's graphical effects. */
    virtual KartGFX* getKartGFX() = 0;
    // ------------------------------------------------------------------------
    virtual void setPowerup (PowerupManager::PowerupType t, int n) = 0;
    // ------------------------------------------------------------------------
    /** Returns the bullet vehicle which represents this kart. */
    virtual btKart* getVehicle() const = 0;
    // ------------------------------------------------------------------------
    virtual btQuaternion getVisualRotation() const = 0;
    // ------------------------------------------------------------------------
    /** Returns true if the kart is 'resting', i.e. (nearly) not moving. */
    virtual bool isInRest() const = 0;
    // ------------------------------------------------------------------------
    /** Starts the engine sound effect. Called once the track intro phase is
     *  over. */
    virtual void startEngineSFX() = 0;
    // ------------------------------------------------------------------------
    /** Multiplies the velocity of the kart by a factor f (both linear
     *  and angular). This is used by anvils, which suddenly slow down the kart
     *  when they are attached. */
    virtual void adjustSpeed(float f) = 0;
    // ------------------------------------------------------------------------
    /** This is used on the client side only to set the speed of the kart
     *  from the server information.                                       */
    virtual void setSpeed(float s) = 0;
    // ------------------------------------------------------------------------
    /** Returns if the kart is invulnerable. */
    virtual bool isInvulnerable() const = 0;
    // ------------------------------------------------------------------------
    virtual void setInvulnerableTicks(int ticks) = 0;
    // ------------------------------------------------------------------------
    /** Returns if the kart is protected by a shield. */
    virtual bool isShielded() const = 0;
    // ------------------------------------------------------------------------
    virtual void setShieldTime(float t) = 0;
    // ------------------------------------------------------------------------
    virtual float getShieldTime() const = 0;
    // ------------------------------------------------------------------------
    /** Decreases the kart's shield time. */
    virtual void decreaseShieldTime() = 0;
    // ------------------------------------------------------------------------

    /** Shows the star effect for a certain time. */
    virtual void showStarEffect(float t) = 0;
    // ------------------------------------------------------------------------
    /** Returns the terrain info oject. */
    virtual const TerrainInfo *getTerrainInfo() const = 0;
    // ------------------------------------------------------------------------
    /** Called when the kart crashes against another kart.
     *  \param k The kart that was hit.
     *  \param update_attachments If true the attachment of this kart and the
     *          other kart hit will be updated (e.g. bombs will be moved). */
    virtual void crashed(AbstractKart *k, bool update_attachments) = 0;
    // ------------------------------------------------------------------------
    virtual void crashed(const Material *m, const Vec3 &normal) = 0;
    // ------------------------------------------------------------------------
    /** Returns the normal of the terrain the kart is over atm. This is
     *  defined even if the kart is flying. */
    virtual const Vec3& getNormal() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the position 0.25s before */
    virtual const Vec3& getPreviousXYZ() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the most recent different previous position */
    virtual const Vec3& getRecentPreviousXYZ() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the time at which the recent previous position occured */
    virtual const float getRecentPreviousXYZTime() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the height of the terrain. we're currently above */
    virtual float getHoT() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the pitch of the terrain depending on the heading. */
    virtual float getTerrainPitch(float heading) const = 0;
    // -------------------------------------------------------------------------
    /** Returns a bullet transform object located at the kart's position
        and oriented in the direction the kart is going. Can be useful
        e.g. to calculate the starting point and direction of projectiles. */
    virtual btTransform getAlignedTransform(const float customPitch=-1) = 0;
    // -------------------------------------------------------------------------
    /** Set a text that is displayed on top of a kart.
     */
    virtual void setOnScreenText(const core::stringw& text) = 0;
    // ------------------------------------------------------------------------
    /** Returns whether this kart wins or loses. */
    virtual bool getRaceResult() const = 0;
    // ------------------------------------------------------------------------
    /** Returns whether this kart is a ghost (replay) kart. */
    virtual bool isGhostKart() const = 0;
    // ------------------------------------------------------------------------
    /** Returns whether this kart is jumping. */
    virtual bool isJumping() const = 0;
    // ------------------------------------------------------------------------
    virtual void playSound(SFXBuffer* buffer) = 0;
    // ------------------------------------------------------------------------
    virtual bool isVisible() const = 0;
    // ------------------------------------------------------------------------
    virtual void makeKartRest();
    // ------------------------------------------------------------------------
    virtual void setStartupBoost(float val) = 0;
    // ------------------------------------------------------------------------
    virtual float getStartupBoost() const = 0;
    // ------------------------------------------------------------------------
    virtual float getStartupBoostFromStartTicks(int ticks) const = 0;
    // ------------------------------------------------------------------------
    virtual Stars* getStarsEffect() const = 0;
    // ------------------------------------------------------------------------
    int getLiveJoinUntilTicks() const              { return m_live_join_util; }
    // ------------------------------------------------------------------------
    void setLiveJoinKart(int util_ticks)     { m_live_join_util = util_ticks; }
    // ------------------------------------------------------------------------
    /** Return the confirmed finish ticks (sent by the server)
     *  indicating that this kart has really finished the race. */
    virtual int getNetworkConfirmedFinishTicks() const = 0;
};   // AbstractKart


#endif

/* EOF */
