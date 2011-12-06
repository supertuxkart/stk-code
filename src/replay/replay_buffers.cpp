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

#ifdef HAVE_GHOST_REPLAY

#include "replay/replay_buffers.hpp"

#include <cassert>

#include "replay/replay_base.hpp"

#define REPLAY_SAVE_STATISTIC

ReplayBuffers::ReplayBuffers()
: m_number_karts(0),
  m_BufferFrame(),
  m_BufferKartState()
{
}

ReplayBuffers::~ReplayBuffers()
{
    destroy();
}

void ReplayBuffers::destroy()
{
    m_BufferFrame.destroy();
    m_BufferKartState.destroy();
    m_number_karts = 0;
}

bool ReplayBuffers::init( unsigned int number_karts, 
                          size_t number_preallocated_frames )
{
    m_number_karts = number_karts;

    if( !m_BufferFrame.init( number_preallocated_frames ) ) return false;
    if( !m_BufferKartState.init( number_preallocated_frames, number_karts ) ) return false;

    return true;
}

ReplayFrame* 
ReplayBuffers::getNewFrame()
{
    // make sure initialization was called properly
    assert( m_BufferFrame.getNumberObjectsUsed() == m_BufferKartState.getNumberArraysUsed() );

    if( !isHealthy() ) return NULL;

    ReplayFrame* frame = m_BufferFrame.getNewObject();
    if( !frame ) return NULL;

    // get current karts-array
    ReplayKartState* karts_array = m_BufferKartState.getNewArray();
    if( !karts_array ) return NULL;
    
    frame->p_kart_states = karts_array;

    return frame;
}

bool ReplayBuffers::saveReplayHumanReadable( FILE *fd ) const
{
    if( !isHealthy() ) return false;

    if( fprintf( fd, "frames: %u\n", getNumberFrames() ) < 1 ) return false;
    
#ifdef REPLAY_SAVE_STATISTIC
    float time_step_min = 9999999.0f, time_step_max = 0.0f, time_last;
#endif

    unsigned int frame_idx, kart_idx;
    ReplayFrame const *frame;
    ReplayKartState const *kart;
    for( frame_idx = 0; frame_idx < getNumberFrames(); ++frame_idx )
    {
        frame = getFrameAt( frame_idx );
        if( fprintf( fd, "frame %u  time %f\n", frame_idx, frame->time ) < 1 ) return false;

        for( kart_idx = 0; kart_idx < m_number_karts; ++kart_idx )
        {
            kart = frame->p_kart_states + kart_idx;

            if( fprintf( fd, "\tkart %u: %f,%f,%f,%f,%f,%f\n", kart_idx, 
                     kart->position.xyz[0], kart->position.xyz[1], kart->position.xyz[2],
                     kart->position.hpr[0], kart->position.hpr[1], kart->position.hpr[2] ) < 1 ) return false;   
        }

#ifdef REPLAY_SAVE_STATISTIC
        if( frame_idx )
        {
            float diff = frame->time - time_last;
            if( diff < time_step_min ) time_step_min = diff;
            if( diff > time_step_max ) time_step_max = diff;
        }
        time_last = frame->time;
#endif
    }

#ifdef REPLAY_SAVE_STATISTIC
    float time_step_avg;
    if( getNumberFrames() > 1 )
    {
        time_step_avg = time_last / ( getNumberFrames() - 1 );
    }
    else
    {
        time_step_avg = -1.0f;
    }
    fprintf( fd, "\n# statistic time-steps:\n# \tmin: %f\n# \tmax: %f\n# \tavg: %f\n", time_step_min, time_step_max, time_step_avg );
#endif

    return true;
}

bool ReplayBuffers::loadReplayHumanReadable( FILE *fd, unsigned int number_karts )
{
    size_t frames;
    if( fscanf( fd, "frames: %u\n", &frames ) != 1 ) return false;

    if( !init( number_karts, frames ) ) return false;

    assert( m_number_karts );
    unsigned int frame_idx, kart_idx, tmp;
    ReplayFrame *frame;
    ReplayKartState *kart;
    for( frame_idx = 0; frame_idx < frames; ++frame_idx )
    {
        // if we are here, it cant fail, since enough objects have to be allocated above
        frame = getNewFrame();
        assert( frame );

        if( fscanf( fd, "frame %u  time %f\n", &tmp, &frame->time ) != 2 ) return false;

        for( kart_idx = 0; kart_idx < m_number_karts; ++kart_idx )
        {
            kart = frame->p_kart_states + kart_idx;

            if( fscanf( fd, "\tkart %u: %f,%f,%f,%f,%f,%f\n", &tmp, 
                     &kart->position.xyz[0], &kart->position.xyz[1], &kart->position.xyz[2],
                     &kart->position.hpr[0], &kart->position.hpr[1], &kart->position.hpr[2] ) != 7 ) return false;   
        }
    }

    assert( frames == getNumberFrames() );
    assert( m_BufferFrame.getNumberObjectsUsed() == getNumberFrames() );
    assert( m_BufferKartState.getNumberArraysUsed() == getNumberFrames() );

    // there should be no reallocation ..
    assert( m_BufferFrame.getNumberBlocks() == 1 );
    assert( m_BufferKartState.getNumberBlocks() == 1 );

    return true;
}



#endif // HAVE_GHOST_REPLAY
