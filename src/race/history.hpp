//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_HISTORY_HPP
#define HEADER_HISTORY_HPP


#include "input/input.hpp"
#include "karts/controller/kart_control.hpp"

#include <string>
#include <vector>

class Kart;

/**
  * \ingroup race
  */
class History
{
private:
    /** True if a history should be replayed, */
    bool m_replay_history;

    /** Points to the last used input event index. */
    unsigned int m_event_index;

    /** The identities of the karts to use. */
    std::vector<std::string> m_kart_ident;

    // ------------------------------------------------------------------------
    struct InputEvent
    {
        /* Time at which this event occurred. */
        int m_world_ticks;
        /** For which kart the event was. */
        int m_kart_index;
        /** Which action it was. */
        PlayerAction m_action;
        /** The value to use. */
        int m_value;
    };   // InputEvent
    // ------------------------------------------------------------------------

    /** All input events. */
    std::vector<InputEvent> m_all_input_events;

    void  allocateMemory(int size=-1);
public:
    static bool m_online_history_replay;
          History        ();
    void  initRecording  ();
    void  Save           ();
    void  Load           ();
    void  updateReplay(int world_ticks);
    void  addEvent(int kart_id, PlayerAction pa, int value);

    // -------------------I-----------------------------------------------------
    /** Returns the identifier of the n-th kart. */
    const std::string& getKartIdent(unsigned int n)
    {
        return m_kart_ident[n];
    }   // getKartIdent
    // ------------------------------------------------------------------------
    /** Returns if a history is replayed, i.e. the history mode is not none. */
    bool  replayHistory() const { return m_replay_history; }
    // ------------------------------------------------------------------------
    /** Set if replay is enabled or not. */
    void  setReplayHistory(bool b) { m_replay_history=b;  }
};

extern History* history;

#endif
