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
    /** The list of the times at which the transform were reached. */
    std::vector<float>       m_all_times;

    /** The transforms to assume at the corresponding time in m_all_times. */
    std::vector<btTransform> m_all_transform;

    std::vector<ReplayBase::KartReplayEvent> m_replay_events;

    /** Pointer to the last index in m_all_times that is smaller than
     *  the current world time. */
    unsigned int m_current_transform;

    /** Index of the next kart replay event. */
    unsigned int m_next_event;

    void         updateTransform(float t, float dt);
public:
                 GhostKart(const std::string& ident);
    virtual void update (float dt);
    virtual void addTransform(float time, const btTransform &trans);
    virtual void addReplayEvent(const ReplayBase::KartReplayEvent &kre);
    virtual void reset();
    // ------------------------------------------------------------------------
    /** No physics body for ghost kart, so nothing to adjust. */
    virtual void updateWeight() {};
    // ------------------------------------------------------------------------
    /** No physics for ghost kart. */
    virtual void applyEngineForce (float force) {}
    // ------------------------------------------------------------------------
    // Not needed to create any physics for a ghost kart.
    virtual void createPhysics() {}
    // ------------------------------------------------------------------------

};   // GhostKart
#endif
