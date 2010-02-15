//  $Id: flyable.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  Linear item-kart intersection function written by
//  by David Mikos. Copyright (C) 2009.
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

#include "irrlicht.h"
using namespace irr;

#include "audio/sfx_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/moveable.hpp"
#include "tracks/terrain_info.hpp"

class FlyableInfo;
class Kart;
class PhysicalObject;
class XMLNode;

class Flyable : public Moveable, public TerrainInfo
{
public:
private:
    bool              m_has_hit_something;
    /** This flag is used to avoid that a rocket explodes mode than once.
     *  It can happen that more than one collision between a rocket and
     *  a track or kart is reported by the physics.                        */
    bool              m_exploded;
    /** If this flag is set, the Z velocity of the kart will not be
     *  adjusted in case that the objects is too high or too low above the
     *  terrain. Otherwise gravity will not work correctly on this object. */
    bool              m_adjust_z_velocity;

protected:
    Kart*             m_owner;              // the kart which released this flyable
    btCollisionShape *m_shape;
    float             m_max_height;
    float             m_min_height;
    float             m_average_height;     // average of m_{min,ax}_height
    float             m_force_updown;
    float             m_speed;
    float             m_mass;
    btVector3         m_extend;
    // The flyable class stores the values for each flyable type, e.g.
    // speed, min_height, max_height. These variables must be static,
    // so we need arrays of these variables to have different values
    // for bowling balls, missiles, ...
    static float      m_st_speed[POWERUP_MAX];         // Speed of the projectile
    static scene::IMesh *m_st_model[POWERUP_MAX];         // 3d model
    static float      m_st_min_height[POWERUP_MAX];    // min height above track
    static float      m_st_max_height[POWERUP_MAX];    // max height above track
    static float      m_st_force_updown[POWERUP_MAX];  // force pushing up/down
    static btVector3  m_st_extend[POWERUP_MAX];        // size of the model

    /** time since thrown. used so a kart can't hit himself when trying something,
        and also to put some time limit to some collectibles */
    float             m_time_since_thrown;

    /** set to something > -1 if this flyable should auto-destrcut after a while */
    float             m_max_lifespan;

    /** if set to true, the kart that throwns this flyable can't collide with it
        for a short time */
    bool              m_owner_has_temporary_immunity;

    /** Returns information on what is the closest kart and at what
        distance it is. All 3 parameters first are of type 'out'.
        'inFrontOf' can be set if you wish to know the closest
        kart in front of some karts (will ignore those behind).
        Useful e.g. for throwing projectiles in front only.
     */
    void              getClosestKart(const Kart **minKart, float *minDistSquared,
                                     btVector3 *minDelta, const Kart* inFrontOf=NULL,
                                     const bool backwards=false) const;

    /** Returns information on the parameters needed to hit a target kart
        moving at constant velocity and direction for a given speed in the
        XY-plane.
     */
    void getLinearKartItemIntersection(const btVector3 origin, const Kart *target_kart,
                                       float item_XY_velocity, float gravity, float y_offset,
                                       float *fire_angle, float *up_velocity, float *time);


    /** init bullet for moving objects like projectiles */
    void              createPhysics(float y_offset,
                                    const btVector3 &velocity,
                                    btCollisionShape *shape, const float gravity=0.0f,
                                    const bool rotates=false, const bool turn_around=false,
                                    const btTransform* customDirection=NULL);
public:

                 Flyable     (Kart* kart, PowerupType type, float mass=1.0f);
    virtual     ~Flyable     ();
    /** Enables/disables adjusting ov velocity depending on height above
     *  terrain. Missiles can 'follow the terrain' with this adjustment,
     *  but gravity will basically be disabled.                          */
    void         setAdjustZVelocity(bool f) { m_adjust_z_velocity = f; }
    static void  init        (const XMLNode &node, scene::IMesh *model,
                              PowerupType type);
    virtual void update      (float);
    void         updateFromServer(const FlyableInfo &f, float dt);

    virtual void hitTrack    () {};
    virtual void hit         (Kart* kart, PhysicalObject* obj=NULL);
    bool         hasHit      () { return m_has_hit_something; }
    /** Indicates that something was hit and that this object must
     *  be removed. */
    void         setHasHit   () { m_has_hit_something = true; }
    void         reset       () { Moveable::reset();          }
    bool         isOwnerImmunity(const Kart *kart_hit) const;
    virtual const char*  getExplosionSound() const { return "explosion"; }
    /** Indicates if an explosion needs to be added if this flyable
      * is removed. */
    virtual bool needsExplosion() const {return true;}
};   // Flyable

#endif
