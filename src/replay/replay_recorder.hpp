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

#ifndef HEADER_REPLAYRECORDER_HPP
#define HEADER_REPLAYRECORDER_HPP

#ifdef HAVE_GHOST_REPLAY

#include "replay/replay_base.hpp"


// class managing:
//      - the recording of the replay
//      - the serializing to file
class ReplayRecorder : public ReplayBase
{
private:
    // assuming 10 minutes with 50 frames per second
    enum { BUFFER_PREALLOCATE_FRAMES = 10 * 50 * 60, };
    enum { REPLAY_FREQUENCY_MAX = 30 };
    // calculated from REPLAY_FREQUENCY_MAX
    static const float  REPLAY_TIME_STEP_MIN;

public:
    ReplayRecorder();
    virtual ~ReplayRecorder();

    void            destroy();
    bool            initRecorder( unsigned int number_karts, 
                                  size_t number_preallocated_frames = BUFFER_PREALLOCATE_FRAMES );

    // something might go wrong, since a new buffer may be allocated, so false means
    // no memory available
    bool            pushFrame();

private:
    // returns a new *free* frame to be used to store the current frame-data into it
    // used to *record* the replay
    ReplayFrame*    getNewFrame() { return m_ReplayBuffers.getNewFrame(); }
};



#endif // HAVE_GHOST_REPLAY

#endif // HEADER_REPLAYRECORDER_HPP

