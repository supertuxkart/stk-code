//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#ifndef HEADER_CHECK_TRIGGER_HPP
#define HEADER_CHECK_TRIGGER_HPP

#include "tracks/check_structure.hpp"
#include "utils/cpp2011.hpp"
#include "utils/types.hpp"

#include <functional>

/** This class implements a check point like item, but used only for scripting
 *  or sound trigger.
 * \ingroup tracks
 */
class CheckTrigger : public CheckStructure
{
private:
    /** Center of the trigger. */
    const Vec3 m_center;

    /** Squared of the triggering distance. */
    const float m_distance2;

    /** Function to call when triggered. */
    std::function<void(int)> m_triggering_function;

    /** Time since last trigger, if any triggering between 2 seconds ignored
     *  (like items). */
    uint64_t m_last_triggered_time;

public:
    CheckTrigger(const Vec3& center, float distance,
                 std::function<void(int)> triggering_function);
    // ------------------------------------------------------------------------
    virtual ~CheckTrigger() {}
    // ------------------------------------------------------------------------
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                             int kart_id) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void trigger(unsigned int kart_index) OVERRIDE
    {
        if (!m_triggering_function) return;
        m_triggering_function(kart_index);
        CheckStructure::trigger(kart_index);
    }
    // ------------------------------------------------------------------------
    virtual CheckStructure* clone() OVERRIDE
    {
        CheckTrigger* ct = new CheckTrigger(*this);
        // Drop unneeded stuff ( trigger function is not supported in server,
        // no scripting atm)
        ct->m_triggering_function = nullptr;
        return ct;
    }
};   // CheckTrigger

#endif

