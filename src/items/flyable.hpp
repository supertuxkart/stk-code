//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
//
//  Linear item-kart intersection function written by
//  Copyright (C) 2009-2015 David Mikos.
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

#ifndef HEADER_FLYABLE_HPP
#define HEADER_FLYABLE_HPP

#include "items/powerup_manager.hpp"
#include "karts/moveable.hpp"
#include "network/rewinder.hpp"
#include "tracks/terrain_info.hpp"
#include "utils/cpp2011.hpp"

#include <irrString.h>
namespace irr
{
    namespace scene { class IMesh; }
}
using namespace irr;

class AbstractKart;
class AbstractKartAnimation;
class HitEffect;
class PhysicalObject;
class SFXBase;
class XMLNode;

/**
  * \ingroup items
  */
class Flyable : public Moveable, public TerrainInfo,
                public Rewinder
{
private:
    bool              m_has_hit_something;

    /** If this flag is set, the up velocity of the kart will not be
     *  adjusted in case that the objects is too high or too low above the
     *  terrain. Otherwise gravity will not work correctly on this object. */
    bool              m_adjust_up_velocity;

    /** An offset that is added when doing the raycast for terrain. This
     *  is useful in case that the position of the object is just under
     *  the terrain (perhaps due to floating point errors), and would
     *  otherwise result in an invalid terrain. */
    Vec3              m_position_offset;

    /** If this variable is set to true (which is the default) flyable
     *  will update the height of terrain when its updateAndDelete
     *  function is called. If it's necessary to update the height of
     *  terrain yourself (e.g. order of operations is important)
     *  set this to false with a call do setDoTerrainInfo(). */
    bool              m_do_terrain_info;

    /* Used in network to restore previous gravity in compressed form. */
    uint32_t          m_compressed_gravity_vector;

    /** If the flyable is in a cannon, this is the pointer to the cannon
     *  animation. NULL otherwise. */
    AbstractKartAnimation *m_animation;

protected:
    /** Kart which shot this flyable. */
    AbstractKart*     m_owner;

    /** Type of the powerup. */
    PowerupManager::PowerupType
                      m_type;

    /** Collision shape of this Flyable. */
    btCollisionShape *m_shape;

    /** Maximum height above terrain. */
    float             m_max_height;

    /** Minimum height above terrain. */
    float             m_min_height;

    /** Average of average of m_{min,ax}_height. */
    float             m_average_height;

    /** Force pushing the Flyable up. */
    float             m_force_updown;

    /** Speed of this Flyable. */
    float             m_speed;

    /** Mass of this Flyable. */
    const float       m_mass;

    /** Size of this flyable. */
    Vec3              m_extend;

    /** Time since thrown. used so a kart can't hit himself when trying
     *  something, and also to put some time limit to some collectibles */
    uint16_t          m_ticks_since_thrown;

    /* True if this flyable exists in server, and will trigger a rewind.
     * For each local state it will reset it to false and call moveToInfinity,
     * and for each restoreState it will set it to true. Also when re-fire the
     * flyable during rewind it will set to true too. */
    bool              m_has_server_state;

    /** If set to true, the kart that throwns this flyable can't collide
     *  with it for a short time. */
    bool              m_owner_has_temporary_immunity;

    /* Set to true once when onDeleteFlyable, this is used to create HitEffect
     * only once. */
    bool              m_deleted_once;

    /* Save the locally detected deleted ticks, if the confirmed state world
     * ticks in computeError > this, the flyable can be deleted in client. */
    int               m_last_deleted_ticks;

    // The flyable class stores the values for each flyable type, e.g.
    // speed, min_height, max_height. These variables must be static,
    // so we need arrays of these variables to have different values
    // for bowling balls, missiles, ...

    /** Speed of the projectile. */
    static float      m_st_speed[PowerupManager::POWERUP_MAX];

    /** The mesh of this Flyable. */
    static scene::IMesh *m_st_model[PowerupManager::POWERUP_MAX];

    /** Minimum height above track. */
    static float      m_st_min_height[PowerupManager::POWERUP_MAX];

    /**Max height above track. */
    static float      m_st_max_height[PowerupManager::POWERUP_MAX];

    /** Force pushing up/down. */
    static float      m_st_force_updown[PowerupManager::POWERUP_MAX];

    /** Size of the model. */
    static Vec3       m_st_extend[PowerupManager::POWERUP_MAX];

    /** Set to something > -1 if this flyable should auto-destrcut after
     *  that may ticks. */
    int               m_max_lifespan;

    /* For debugging purpose */
    int               m_created_ticks;

    void              getClosestKart(const AbstractKart **minKart,
                                     float *minDistSquared,
                                     Vec3 *minDelta,
                                     const AbstractKart* inFrontOf=NULL,
                                     const bool backwards=false) const;

    void getLinearKartItemIntersection(const Vec3 &origin,
                                       const AbstractKart *target_kart,
                                       float item_XY_velocity, float gravity,
                                       float forw_offset,
                                       float *fire_angle, float *up_velocity);


    /** init bullet for moving objects like projectiles */
    void              createPhysics(float y_offset,
                                    const Vec3 &velocity,
                                    btCollisionShape *shape,
                                    float restitution,
                                    const btVector3& gravity=btVector3(0.0f,0.0f,0.0f),
                                    const bool rotates=false,
                                    const bool turn_around=false,
                                    const btTransform* customDirection=NULL);

    void              moveToInfinity(bool set_moveable_trans = true);
    void              removePhysics();
    void              fixSFXSplitscreen(SFXBase* sfx);
public:

                 Flyable     (AbstractKart* kart,
                              PowerupManager::PowerupType type,
                              float mass=1.0f);
    virtual     ~Flyable     ();
    static void  init        (const XMLNode &node, scene::IMesh *model,
                              PowerupManager::PowerupType type);
    void                      updateGraphics(float dt) OVERRIDE;
    virtual bool              updateAndDelete(int ticks);
    virtual void              setAnimation(AbstractKartAnimation *animation);
    virtual HitEffect*        getHitEffect() const;
    bool                      isOwnerImmunity(const AbstractKart *kart_hit) const;
    virtual bool              hit(AbstractKart* kart, PhysicalObject* obj=NULL);
    void                      explode(AbstractKart* kart, PhysicalObject* obj=NULL,
                                      bool secondary_hits=true);
    unsigned int              getOwnerId();
    // ------------------------------------------------------------------------
    /** Returns if this flyable has an animation playing (e.g. cannon). */
    bool hasAnimation() const { return m_animation != NULL;  }
    // ------------------------------------------------------------------------
    /** If true the up velocity of the flyable will be adjust so that the
     *  flyable stays at a height close to the average height.
     *  \param f True if the up velocity should be adjusted. */
    void         setAdjustUpVelocity(bool f) { m_adjust_up_velocity = f; }
    // ------------------------------------------------------------------------
    /** Sets the offset to be used when determining the terrain under the
     *  flyable. This needs to be used in case that an object might be just
     *  under the actual terrain (e.g. rubber ball on a steep uphill slope). */
    void         setPositionOffset(const Vec3 &o) {m_position_offset = o; }
    // ------------------------------------------------------------------------
    /** Called when this flyable hits the track. */
    virtual void hitTrack    () {};
    // ------------------------------------------------------------------------
    /** Enables/disables adjusting ov velocity depending on height above
     *  terrain. Missiles can 'follow the terrain' with this adjustment,
     *  but gravity will basically be disabled.                          */
    bool         hasHit      () { return m_has_hit_something; }
    // ------------------------------------------------------------------------
    /** Indicates that something was hit and that this object must
     *  be removed. */
    void         setHasHit   () { m_has_hit_something = true; }
    // ------------------------------------------------------------------------
    /** Resets this flyable. */
    void         reset() OVERRIDE { Moveable::reset();          }
    // ------------------------------------------------------------------------
    /** Returns the type of flyable. */
    PowerupManager::PowerupType getType() const {return m_type;}

    // ------------------------------------------------------------------------
    /** Returns the owner's kart */
    AbstractKart *getOwner() const { return m_owner;}  
    // ------------------------------------------------------------------------
    /** Sets wether Flyable should update TerrainInfo as part of its update
     *  call, or if the inheriting object will update TerrainInfo itself
     *  (or perhaps not at all if it is not needed). */
    void setDoTerrainInfo(bool d) { m_do_terrain_info = d; }
    // ------------------------------------------------------------------------
    /** Returns the size (extend) of the mesh. */
    const Vec3 &getExtend() const { return m_extend;  }
    // ------------------------------------------------------------------------
    void addForRewind(const std::string& uid);
    // ------------------------------------------------------------------------
    virtual void undoEvent(BareNetworkString *buffer) OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual void rewindToEvent(BareNetworkString *buffer) OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual void undoState(BareNetworkString *buffer) OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual void saveTransform() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void computeError() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual BareNetworkString* saveState(std::vector<std::string>* ru)
        OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString *buffer, int count) OVERRIDE;
    // ------------------------------------------------------------------------
    /* Return true if still in game state, or otherwise can be deleted. */
    bool hasServerState() const                  { return m_has_server_state; }
    // ------------------------------------------------------------------------
    /** Call when the item is (re-)fired (during rewind if needed) by
     *  projectile_manager. */
    virtual void onFireFlyable();
    // ------------------------------------------------------------------------
    virtual void onDeleteFlyable();
    // ------------------------------------------------------------------------
    void setCreatedTicks(int ticks)                { m_created_ticks = ticks; }
};   // Flyable

#endif
