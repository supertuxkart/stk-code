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

#ifndef HEADER_GHOST_CONTROLLER_HPP
#define HEADER_GHOST_CONTROLLER_HPP

#include "karts/controller/controller.hpp"
#include "states_screens/state_manager.hpp"

#include <vector>

/** A class for Ghost controller.
 * \ingroup controller
 */
class GhostController : public Controller
{
private:
    /** Pointer to the last index in m_all_times that is smaller than
     *  the current world time. */
    unsigned int m_current_index;

    /** The current world time. */
    float m_current_time;

    /** Player name of the ghost kart. */
    core::stringw m_display_name;

    /** The list of the times at which the events of kart were reached. */
    std::vector<float> m_all_times;

public:
             GhostController(AbstractKart *kart, core::stringw display_name);
    virtual ~GhostController() {};
    virtual void reset() OVERRIDE;
    virtual void update (float dt) OVERRIDE;
    virtual bool disableSlipstreamBonus() const OVERRIDE { return true; }
    virtual void crashed(const Material *m) OVERRIDE {}
    virtual void crashed(const AbstractKart *k) OVERRIDE {}
    virtual void handleZipper(bool play_sound) OVERRIDE {}
    virtual void finishedRace(float time) OVERRIDE {}
    virtual void collectedItem(const Item &item, int add_info=-1,
                               float previous_energy=0) OVERRIDE {}
    virtual void setPosition(int p) OVERRIDE {}
    virtual bool isPlayerController() const OVERRIDE { return false; }
    virtual bool isLocalPlayerController() const OVERRIDE { return false; }
    virtual void action(PlayerAction action, int value) OVERRIDE;
    virtual void skidBonusTriggered() OVERRIDE {}
    virtual void newLap(int lap) OVERRIDE {}
    void         addReplayTime(float time);
    // ------------------------------------------------------------------------
    bool         isReplayEnd() const
                         { return m_current_index + 1 >= m_all_times.size(); }
    // ------------------------------------------------------------------------
    float        getReplayDelta() const
    {
        assert(m_current_index < m_all_times.size());
        return ((m_current_time - m_all_times[m_current_index]) /
            (m_all_times[m_current_index + 1] - m_all_times[m_current_index]));
    }
    // ------------------------------------------------------------------------
    unsigned int getCurrentReplayIndex() const
                                                   { return m_current_index; }
    // ------------------------------------------------------------------------
    /** Return the display name; if not set, use default display name (kart name) */
    core::stringw getName() const OVERRIDE
    {
        return m_display_name.empty() ? Controller::getName() : m_display_name;
    }
};   // GhostController

#endif
