//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Damien Morel <divdams@free.fr>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_REPLAYBASE_H
#define HEADER_REPLAYBASE_H

#ifdef HAVE_GHOST_REPLAY


#include <string>
#include <plib/sg.h>

#include "replay_buffers.hpp"


// the data stored for each kart in each frame
struct ReplayKartState
{
    sgCoord position;
};
// the data stored for each frame
struct ReplayFrame
{
    // absolute time of frame
    float           time;
    // for each kart in frame, points to continious block 
    // in Buffers::m_pp_blocks_kart_states with m_number_cars items
    ReplayKartState *p_kart_states;
};


class ReplayBase
{
public:
    static const std::string REPLAY_FOLDER;
    static const std::string REPLAY_FILE_EXTENSION_HUMAN_READABLE;
    static const std::string REPLAY_FILE_EXTENSION_BINARY;

public:

    ReplayBase();
    virtual ~ReplayBase();

    virtual void    destroy();

    bool            saveReplayHumanReadable( FILE *fd ) const { return m_ReplayBuffers.saveReplayHumanReadable( fd ); }

private:

protected:
    ReplayBuffers   m_ReplayBuffers;
};





#endif // HAVE_GHOST_REPLAY

#endif // HEADER_REPLAYBASE_H

