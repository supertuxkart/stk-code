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
#include "replay/replay_play.hpp"
#include "utils/cpp2011.hpp"

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

    std::vector<ReplayBase::BonusInfo>       m_all_bonus_info;

    std::vector<ReplayBase::KartReplayEvent> m_all_replay_events;

    ReplayPlay::ReplayData m_replay_data;

    unsigned int                             m_last_egg_idx;

    bool m_finish_computed;

    // ----------------------------------------------------------------------------
    /** Compute the time at which the ghost finished the race */
    void          computeFinishTime();
public:
                  GhostKart(const std::string& ident, unsigned int world_kart_id,
                            int position, float color_hue,
                            const ReplayPlay::ReplayData& rd);
    virtual void  update(int ticks) OVERRIDE;
    virtual void  updateGraphics(float dt) OVERRIDE;
    virtual void  reset() OVERRIDE;
    // ------------------------------------------------------------------------
    /** No physics for ghost kart. */
    virtual void  applyEngineForce (float force) OVERRIDE {};
    // ------------------------------------------------------------------------
    // Not needed to create any physics for a ghost kart.
    virtual void  createPhysics() OVERRIDE {};
    // ------------------------------------------------------------------------
    const float   getSuspensionLength(int index, int wheel) const
               { return m_all_physic_info[index].m_suspension_length[wheel]; }
    // ------------------------------------------------------------------------
    void          addReplayEvent(float time,
                                 const btTransform &trans,
                                 const ReplayBase::PhysicInfo &pi,
                                 const ReplayBase::BonusInfo &bi,
                                 const ReplayBase::KartReplayEvent &kre);
    // ------------------------------------------------------------------------
    /** Returns whether this kart is a ghost (replay) kart. */
    virtual bool  isGhostKart() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Ghost can't be hunted. */
    virtual bool  isInvulnerable() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Returns the speed of the kart in meters/second. */
    virtual float getSpeed() const OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns the finished time for a ghost kart. */
    float  getGhostFinishTime();
    // ------------------------------------------------------------------------
    /** Returns the time at which the kart was at a given distance.
      * Returns -1.0f if none */
    virtual float getTimeForDistance(float distance) OVERRIDE;

    // ----------------------------------------------------------------------------
    /** Returns the smallest time at which the kart had the required number of eggs
      * Returns -1.0f if none */
    float getTimeForEggs(int egg_number);

    // ------------------------------------------------------------------------
    virtual void kartIsInRestNow() OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual void makeKartRest() OVERRIDE {}
    // ------------------------------------------------------------------------
    const ReplayPlay::ReplayData& getReplayData() const
                                                     { return m_replay_data; }
};   // GhostKart
#endif

