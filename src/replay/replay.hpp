//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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

#ifndef HEADER_REPLAY_HPP
#define HEADER_REPLAY_HPP

#include "LinearMath/btTransform.h"
#include "utils/no_copy.hpp"

#include <string>
#include <vector>

class GhostKart;

/**
  * \ingroup race
  */
class Replay : public NoCopy
{
private:
    struct ReplayEvent
    {
        /** The id of the kart for which triggers this event. */
        unsigned int m_kart_id;
        /** Time at which this event happens. */
        float        m_time;
        enum {EV_TRANSFORM, EV_NONE} m_type;
        struct {
                btTransform m_t;
        } m_event;   // union
    };   // ReplayEvent
    // ------------------------------------------------------------------------

    /** The array storing all events. */
    std::vector<ReplayEvent> m_events;

    /** Static pointer to the one instance of the replay object. */
    static Replay *m_replay;

    /** True if a replay is done. */
    static bool m_do_replay;

    /** Points to the next free entry. */
    unsigned int m_next;
    
    /** Number of (ghost) karts contained in the replay file. */
    unsigned int m_num_ghost_karts;

    /** The identities of the karts to use. */
    std::vector<std::string>  m_kart_ident;

    /** All ghost karts. */
    std::vector<GhostKart*>        m_ghost_karts;

    void  updateRecording (float dt);
    void  updateReplay(float dt);
          Replay();
         ~Replay();
public:
    void  initReplay();
    void  initRecording();
    void  update(float dt);
    void  reset();
    void  Save();
    void  Load();

    // ------------------------------------------------------------------------
    /** Creates a new instance of the replay object. */
    static void create() { m_replay = new Replay(); }
    // ------------------------------------------------------------------------
    /** Returns the instance of the replay object. */
    static Replay *get() { assert(m_replay); return m_replay; }
    // ------------------------------------------------------------------------
    /** Delete the instance of the replay object. */
    static void destroy() { delete m_replay; m_replay=NULL; }
    // ------------------------------------------------------------------------
    /** Sets that a replay is to be done. */
    static void doReplay() { m_do_replay = true; }
    // ------------------------------------------------------------------------
    /** Returns if a replay is to be done. */
    static bool isReplay() { return m_do_replay; }
    // ------------------------------------------------------------------------
    /** Returns the identifier of the n-th kart. */
    const std::string& getKartIdent(unsigned int n) const
    {
        return m_kart_ident[n];
    }
    // ------------------------------------------------------------------------
    /** Returns the number of karts contained in the replay file. */
    unsigned int getNumberGhostKarts() const { return m_num_ghost_karts;}
};   // Replay

#endif
