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

#include <vector>
#include <string>

#include "LinearMath/btQuaternion.h"

#include "karts/controller/kart_control.hpp"
#include "utils/aligned_array.hpp"
#include "utils/vec3.hpp"

class Kart;

/**
  * \ingroup race
  */
class History
{
public:
    /** Determines which replay mode is selected:
     *  HISTORY_NONE: no history replay.
     *  HISTORY_POSITION: replay the positions and orientations of the karts,
     *                    but don't simulate the physics.
     *  HISTORY_PHYSICS:  Simulate the phyics based on the recorded actions.
     *  These values can be used together, e.g. HISTORY_POSITION|HISTORY_CONTROL
     */
    enum HistoryReplayMode { HISTORY_NONE     = 0,
                             HISTORY_POSITION = 1,
                             HISTORY_PHYSICS  = 2 };
private:
    /** maximum number of history events to store. */
    HistoryReplayMode          m_replay_mode;

    /** Points to the last used entry, and will wrap around. */
    int                        m_current;

    /** True if the buffer has wrapped around. */
    bool                       m_wrapped;

    /** Counts how many entries in the arrays are used.  So if
     *  the buffer hasn't wrapped around, this will indicate
     *  how many entries to save. */
    int                        m_size;

    /** Stores all time step sizes. */
    std::vector<float>         m_all_deltas;

    /** Stores the kart controls being used (for physics replay). */
    std::vector<KartControl>   m_all_controls;

    /** Stores the coordinates (for simple replay). */
    AlignedArray<Vec3>         m_all_xyz;

    /** Stores the rotations of the karts. */
    AlignedArray<btQuaternion> m_all_rotations;

    /** The identities of the karts to use. */
    std::vector<std::string>  m_kart_ident;

    void  allocateMemory(int number_of_frames);
public:
          History        ();
    void  startReplay    ();
    void  initRecording  ();
    void  Save           ();
    void  Load           ();
    void  updateSaving(float dt);
    float updateReplayAndGetDT();

    // -------------------I-----------------------------------------------------
    /** Returns the identifier of the n-th kart. */
    const std::string& getKartIdent(unsigned int n)
    {
        return m_kart_ident[n];
    }   // getKartIdent
    // ------------------------------------------------------------------------
    /** Returns if a history is replayed, i.e. the history mode is not none. */
    bool  replayHistory  () const { return m_replay_mode != HISTORY_NONE;    }
    // ------------------------------------------------------------------------
    /** Enable replaying a history, enabled from the command line. */
    void  doReplayHistory(HistoryReplayMode m) {m_replay_mode = m;           }
    // ------------------------------------------------------------------------
    /** Returns true if the physics should not be simulated in replay mode.
     *  I.e. either no replay mode, or physics replay mode. */
    bool dontDoPhysics   () const { return m_replay_mode == HISTORY_POSITION;}
};

extern History* history;

#endif
