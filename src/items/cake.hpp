//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
//
//  Physics improvements and linear intersection algorithm by
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

#ifndef HEADER_CAKE_HPP
#define HEADER_CAKE_HPP

namespace irr
{
    namespace scene { class IMesh; }
}
#include <irrString.h>

#include "items/flyable.hpp"

class XMLNode;

/**
  * \ingroup items
  */
class Cake : public Flyable
{
private:
    /** Maximum distance for a missile to be attracted. */
    static float m_st_max_distance_squared;
    static float m_gravity;

    btVector3    m_initial_velocity;

    /** Which kart is targeted by this projectile (NULL if none). */
    Moveable*    m_target;

public:
                 Cake (AbstractKart *kart);
    static  void init     (const XMLNode &node, scene::IMesh *cake_model);
    virtual bool hit(AbstractKart* kart, PhysicalObject* obj=NULL) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void hitTrack () OVERRIDE
    {
        if (!m_has_server_state)
            return;
        hit(NULL);
    }
    // ------------------------------------------------------------------------
    /** Kinematic objects are not allowed to have a velocity (assertion in
     *  bullet), so we have to do our own velocity handling here. This
     *  function returns the velocity of this object. */
    virtual const btVector3 &getVelocity() const OVERRIDE
                                                 { return m_initial_velocity; }
    // ------------------------------------------------------------------------
    /** Kinematic objects are not allowed to have a velocity (assertion in
     *  bullet), so we have to do our own velocity handling here. This
     *  function sets the velocity of this object.
     *  \param v Linear velocity of this object.
     */
    virtual void setVelocity(const btVector3& v) OVERRIDE
                                                    { m_initial_velocity = v; }
    // ------------------------------------------------------------------------
    virtual void onFireFlyable() OVERRIDE;
};   // Cake

#endif
