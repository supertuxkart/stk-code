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

#ifndef HEADER_GHOST_KART_HPP
#define HEADER_GHOST_KART_HPP

#include "karts/kart.hpp"
#include "replay/replay_base.hpp"

#include "LinearMath/btTransform.h"

#include <vector>

/** \defgroup karts */

/** A ghost kart. It does not have a phsyics representation. It gets two
 *  transforms from the replay objects at two consecutive time steps,
 *  and will interpolate between those positions depending on the current
 *  time
 */
class GhostKart : public Kart
{
private:
    /** The transforms to assume at the corresponding time in m_all_times. */
    std::vector<btTransform>                 m_all_transform;

    std::vector<ReplayBase::PhysicInfo>      m_all_physic_info;

    std::vector<ReplayBase::KartReplayEvent> m_all_replay_events;

public:
                  GhostKart(const std::string& ident,
                            unsigned int world_kart_id, int position);
    virtual void  update (float dt);
    virtual void  reset();
    // ------------------------------------------------------------------------
    /** No physics body for ghost kart, so nothing to adjust. */
    virtual void  updateWeight() {};
    // ------------------------------------------------------------------------
    /** No physics for ghost kart. */
    virtual void  applyEngineForce (float force) {};
    // ------------------------------------------------------------------------
    // Not needed to create any physics for a ghost kart.
    virtual void  createPhysics() {};
    // ------------------------------------------------------------------------
    const float   getSuspensionLength(int index, int wheel) const
               { return m_all_physic_info[index].m_suspension_length[wheel]; }
    // ------------------------------------------------------------------------
    void          addReplayEvent(float time,
                                 const btTransform &trans,
                                 const ReplayBase::PhysicInfo &pi,
                                 const ReplayBase::KartReplayEvent &kre);
    // ------------------------------------------------------------------------
    /** Returns whether this kart is a ghost (replay) kart. */
    virtual bool  isGhostKart() const                         { return true; }
    // ------------------------------------------------------------------------
    /** Ghost can't be hunted. */
    virtual bool  isInvulnerable() const                      { return true; }
    // ------------------------------------------------------------------------
    /** Returns the speed of the kart in meters/second. */
    virtual float getSpeed() const;
    // ------------------------------------------------------------------------
    virtual void  kartIsInRestNow() {};
    // ------------------------------------------------------------------------

};   // GhostKart
#endif
