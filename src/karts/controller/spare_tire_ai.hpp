//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_SPARE_TIRE_AI_HPP
#define HEADER_SPARE_TIRE_AI_HPP

#include "karts/controller/battle_ai.hpp"

/** The AI for spare tire karts in battle mode, allowing kart to gain life.
 * \ingroup controller
 */
class SpareTireAI : public BattleAI
{
private:
    /** The 4 bounding boxes \ref ArenaNode to follow. */
    int m_fixed_target_nodes[4];

    /** The current index of \ref ArenaNode in \ref m_fixed_target_nodes to
     *  follow, if it's -1, \ref update is not needed to be called. */
    int m_idx;

    /** Store the time before calling \ref unspawn. */
    float m_timer;

    // ------------------------------------------------------------------------
    virtual void  findTarget() OVERRIDE;
    // ------------------------------------------------------------------------
    void          findDefaultPath();

public:
                 SpareTireAI(AbstractKart *kart);
    // ------------------------------------------------------------------------
    virtual void crashed(const AbstractKart *k) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void update(float delta) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset() OVERRIDE;
    // ------------------------------------------------------------------------
    void         spawn(float time_to_last);
    // ------------------------------------------------------------------------
    void         unspawn();
    // ------------------------------------------------------------------------
    /** Return true if this AI needed to be called \ref update by \ref World,
     *  ie it is spawned. */
    bool         isMoving() const                       { return m_idx != -1; }

};

#endif
