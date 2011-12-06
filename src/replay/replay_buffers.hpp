//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Maik Semder <ikework@gmx.de>
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

#ifndef HEADER_REPLAYBUFFERS_HPP
#define HEADER_REPLAYBUFFERS_HPP

#ifdef HAVE_GHOST_REPLAY


#include <cstdio>

#include "replay/replay_buffer_tpl.hpp"

struct ReplayKartState;
struct ReplayFrame;


// the worker-class .. administrates the memory-usage with no copy of 
// previous replay-items when reallocation needed if buffers are full
class ReplayBuffers
{
public:

    ReplayBuffers();
    ~ReplayBuffers();

    bool                init( unsigned int number_karts, 
                              size_t number_preallocated_frames );
    void                destroy();

    // returns a new *free* frame to be used to store the current frame-data into it
    // used to *record* the replay
    ReplayFrame*        getNewFrame();

    // returs frame at given position from replay data
    // used to *show* the replay
    // if frame_index >= num_frames -> returns NULL
    ReplayFrame const*  getFrameAt( size_t frame_index ) const  { return m_BufferFrame.getObjectAt( frame_index ); }

    size_t              getNumberFrames() const                 { return m_BufferFrame.getNumberObjectsUsed(); }
    unsigned int        getNumberKarts() const                  { return m_number_karts; }

    bool                saveReplayHumanReadable( FILE *fd ) const;
    bool                loadReplayHumanReadable( FILE *fd, unsigned int number_karts );

private:
    bool                isHealthy() const                       { return m_BufferFrame.isHealthy() && m_BufferKartState.isHealthy(); }

private:
    typedef ReplayBuffer<ReplayFrame>           BufferFrame;
    typedef ReplayBufferArray<ReplayKartState>  BufferKartState;

    unsigned int        m_number_karts;
    BufferFrame         m_BufferFrame;
    BufferKartState     m_BufferKartState;
};


#endif // HAVE_GHOST_REPLAY

#endif // HEADER_REPLAYBUFFERS_HPP

