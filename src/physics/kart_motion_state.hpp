//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty ofati
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_KART_MOTION_STATE_HPP
#define HEADER_KART_MOTION_STATE_HPP

#include <cmath>
#include "utils/vs.hpp"

#include "LinearMath/btMotionState.h"

/** This is a very simple motion state implementation for bullet, which does
 *  not support any transformation from physics transform to graphics
 *  transform.
 * \ingroup physics
 */

class KartMotionState : public btMotionState
{
private:
    btTransform m_center_of_mass;

public:
    /** Constructor.
     *  \param start_trans An optional start transformation. Defaults to
     *         identity.
     */
    KartMotionState(const btTransform& start_trans = btTransform::getIdentity())
        : m_center_of_mass(start_trans)

    {
    }   // KartMotionState

    // ------------------------------------------------------------------------
    /** Returns the current world transform.
     *  \param center_of_mass The btTransform object that stores the current
     *         transformation.
     */
    virtual void getWorldTransform(btTransform& center_of_mass) const
    {
        center_of_mass = m_center_of_mass;
    }   // getWorldTransform

    // ------------------------------------------------------------------------
    /** Synchronizes world transform from physics to user.
     *  Bullet calls the update of worldtransform for active objects.
     *  \param new_trans The new transformation for the object.
     */
    virtual void setWorldTransform(const btTransform &new_trans)
    {
        assert(!std::isnan(new_trans.getOrigin().getX()));
        assert(!std::isnan(new_trans.getOrigin().getY()));
        assert(!std::isnan(new_trans.getOrigin().getZ()));
        assert(!std::isnan(new_trans.getRotation().getX()));
        assert(!std::isnan(new_trans.getRotation().getY()));
        assert(!std::isnan(new_trans.getRotation().getZ()));
        assert(!std::isnan(new_trans.getRotation().getW()));
        m_center_of_mass = new_trans;
    }   // setWorldTransform

};   // KartMotionState

#endif // HEADER_KART_MOTION_STATE_HPP
