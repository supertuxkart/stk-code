//  $Id: flyable.hpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#ifndef HEADER_FLYABLE_H
#define HEADER_FLYABLE_H

#include "moveable.hpp"
#include "moving_physics.hpp"
#include "kart.hpp"
#include "terrain_info.hpp"

class Flyable : public Moveable, public TerrainInfo
{
public:
    /** FlyableInfo stores information for updating flyables on the clients.
     *  It contains only the coordinates, rotation, and explosion state.    */
    // -----------------------------------------------------------------------
    class FlyableInfo
    {
    public:
        Vec3         m_xyz;
        btQuaternion m_rotation;
        bool         m_exploded;
        FlyableInfo(const Vec3& xyz, const btQuaternion &rotation, bool exploded) :
                    m_xyz(xyz), m_rotation(rotation), m_exploded(exploded)
                    {};
        FlyableInfo() {};
    };
private:
    bool              m_has_hit_something;
    bool              m_exploded;

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
    static float      m_st_speed[COLLECT_MAX];         // Speed of the projectile
    static ssgEntity* m_st_model[COLLECT_MAX];         // 3d model
    static float      m_st_min_height[COLLECT_MAX];    // min height above track
    static float      m_st_max_height[COLLECT_MAX];    // max height above track
    static float      m_st_force_updown[COLLECT_MAX];  // force pushing up/down 
    static btVector3  m_st_extend[COLLECT_MAX];        // size of the model

    /** Returns information on what is the closest kart and at what
        distance it is. All 3 parameters first are of type 'out'.
        'inFrontOf' can be set if you wish to know the closest
        kart in front of some karts (will ignore those behind).
        Useful e.g. for throwing projectiles in front only.
     */
    void              getClosestKart(const Kart **minKart, float *minDistSquared, 
                                     btVector3 *minDelta, const Kart* inFrontOf=NULL) const;
    void              createPhysics(float y_offset, 
                                    const btVector3 velocity,
                                    btCollisionShape *shape, const bool gravity=false,
                                    const bool rotates=false, const btTransform* customDirection=NULL);
public:

                 Flyable     (Kart* kart, CollectableType type, float mass=1.0f);
    virtual     ~Flyable     ();
    static void  init        (const lisp::Lisp* lisp, ssgEntity *model, 
                              CollectableType type);
    virtual void update      (float);
    void         updateFromServer(const FlyableInfo &f);

    virtual void hitTrack    () {};
    void         explode     (Kart* kart, MovingPhysics* moving_physics=NULL);
    bool         hasHit      ()               { return m_has_hit_something; }
    void         reset       () { Moveable::reset(); }
};   // Flyable

#endif
