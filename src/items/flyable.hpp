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

namespace irr
{
    namespace scene { class IMesh; }
}
#include <irrString.h>
using namespace irr;

#include "items/powerup_manager.hpp"
#include "karts/moveable.hpp"
#include "tracks/terrain_info.hpp"

class AbstractKart;
class AbstractKartAnimation;
class HitEffect;
class PhysicalObject;
class XMLNode;

/**
  * \ingroup items
  */
class Flyable : public Moveable, public TerrainInfo
{
public:
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
    float             m_mass;

    /** Size of this flyable. */
    Vec3              m_extend;

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

    /** Time since thrown. used so a kart can't hit himself when trying
     *  something, and also to put some time limit to some collectibles */
    float             m_time_since_thrown;

    /** Set to something > -1 if this flyable should auto-destrcut after
     *  a while. */
    float             m_max_lifespan;

    /** If set to true, the kart that throwns this flyable can't collide
     *  with it for a short time. */
    bool              m_owner_has_temporary_immunity;

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
public:

                 Flyable     (AbstractKart* kart,
                              PowerupManager::PowerupType type,
                              float mass=1.0f);
    virtual     ~Flyable     ();
    static void  init        (const XMLNode &node, scene::IMesh *model,
                              PowerupManager::PowerupType type);
    virtual bool              updateAndDelete(float);
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
    void         reset       () { Moveable::reset();          }
    // ------------------------------------------------------------------------
    /** Returns the type of flyable. */
    PowerupManager::PowerupType getType() const {return m_type;}
    // ------------------------------------------------------------------------
    /** Sets wether Flyable should update TerrainInfo as part of its update
     *  call, or if the inheriting object will update TerrainInfo itself
     *  (or perhaps not at all if it is not needed). */
    void setDoTerrainInfo(bool d) { m_do_terrain_info = d; }
    // ------------------------------------------------------------------------
    /** Returns the size (extend) of the mesh. */
    const Vec3 &getExtend() const { return m_extend;  }
    // ------------------------------------------------------------------------
};   // Flyable

#endif
