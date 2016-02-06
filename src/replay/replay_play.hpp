//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_REPLAY__PLAY_HPP
#define HEADER_REPLAY__PLAY_HPP

#include "karts/ghost_kart.hpp"
#include "replay/replay_base.hpp"
#include "utils/ptr_vector.hpp"

#include <string>
#include <vector>

class GhostKart;

/**
  * \ingroup replay
  */
class ReplayPlay : public ReplayBase
{
private:
    static ReplayPlay *m_replay_play;

    /** Points to the next free entry. */
    unsigned int m_next;

    std::vector<std::string> m_ghost_karts_list;

    /** All ghost karts. */
    PtrVector<GhostKart>    m_ghost_karts;

          ReplayPlay();
         ~ReplayPlay();
    void  readKartData(FILE *fd, char *next_line);
public:
    void  update(float dt);
    void  reset();
    void  load();
    void  loadKartInfo();

    // ------------------------------------------------------------------------
    GhostKart*         getGhostKart(int n)    { return m_ghost_karts.get(n); }
    // ------------------------------------------------------------------------
    const unsigned int getNumGhostKart() const
                                         { return m_ghost_karts_list.size(); }
    // ------------------------------------------------------------------------
    const std::string& getGhostKartName(int n) const
                                          { return m_ghost_karts_list.at(n); }
    // ------------------------------------------------------------------------
    /** Creates a new instance of the replay object. */
    static void        create()          { m_replay_play = new ReplayPlay(); }
    // ------------------------------------------------------------------------
    /** Returns the instance of the replay object. */
    static ReplayPlay  *get()                        { return m_replay_play; }
    // ------------------------------------------------------------------------
    /** Delete the instance of the replay object. */
    static void        destroy()
                               { delete m_replay_play; m_replay_play = NULL; }
    // ------------------------------------------------------------------------
};   // Replay

#endif
