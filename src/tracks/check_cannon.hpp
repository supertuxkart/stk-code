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

#ifndef HEADER_CHECK_CANNON_HPP
#define HEADER_CHECK_CANNON_HPP

#include "animations/animation_base.hpp"
#include "tracks/check_line.hpp"
#include "utils/cpp2011.hpp"

class CheckManager;
class Flyable;
class Ipo;
class ShowCurve;
class XMLNode;

/**
 *  \brief Implements a simple checkline that will cause a kart to be
 *         shot to a specified point.
 *
 * \ingroup tracks
 */
class CheckCannon : public CheckLine
{
private:
    /** The target point the kart will fly to. */
    Vec3 m_target_left;
    Vec3 m_target_right;

    /** Stores the cannon curve data. */
    Ipo *m_curve;

#ifdef DEBUG
    /** If track debugging is enabled, this will show the the curve of
     *  the cannon in the race. */
    ShowCurve * m_show_curve;

    /** Used to display debug information about checklines. */
    scene::IMeshSceneNode *m_debug_target_node;
#endif
    std::vector<Flyable*> m_all_flyables;
    std::vector<Vec3>     m_flyable_previous_position;

public:
             CheckCannon(const XMLNode &node, unsigned int index);
    virtual ~CheckCannon();
    virtual void trigger(unsigned int kart_index) OVERRIDE;
    virtual void changeDebugColor(bool is_active) OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual bool triggeringCheckline() const { return false; }
    void addFlyable(Flyable *flyable);
    void removeFlyable(Flyable *flyable);
};   // CheckLine

#endif

