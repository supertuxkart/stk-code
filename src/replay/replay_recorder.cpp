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

#include "replay/replay_recorder.hpp"

#include <cassert>

#include "modes/world.hpp"

const float ReplayRecorder::REPLAY_TIME_STEP_MIN = 1.0f / (float)ReplayRecorder::REPLAY_FREQUENCY_MAX;

ReplayRecorder::ReplayRecorder()
: ReplayBase()
{
}

ReplayRecorder::~ReplayRecorder()
{
    destroy();
}

void ReplayRecorder::destroy()
{
    ReplayBase::destroy();
}

bool ReplayRecorder::initRecorder( unsigned int number_karts, size_t number_preallocated_frames )
{
    assert( number_karts );

    destroy();

    if( !m_ReplayBuffers.init( number_karts, number_preallocated_frames ) ) return false;

    return true;
}

bool ReplayRecorder::pushFrame()
{
    // we dont record the startphase ..
    assert( RaceManager::getWorld()->getPhase() != World::START_PHASE );
    assert( RaceManager::getWorld()->getNumKarts() == m_ReplayBuffers.getNumberKarts() );

    // make sure we're not under time-step-min 
    if( m_ReplayBuffers.getNumberFrames() )
    {
        ReplayFrame const *last_Frame = m_ReplayBuffers.getFrameAt( m_ReplayBuffers.getNumberFrames() - 1 );
        if( (RaceManager::getWorld()->getTime() - last_Frame->time) < REPLAY_TIME_STEP_MIN ) return true;
    }

    ReplayFrame *pFrame = getNewFrame();
    if( !pFrame ) return false;
    pFrame->time = RaceManager::getWorld()->getClock();

    Kart const *kart;
    int number_karts = RaceManager::getWorld()->getNumKarts();
    for( int kart_index = 0; kart_index < number_karts; ++kart_index )
    {
        kart = RaceManager::getKart( kart_index );
        sgCopyCoord( &( pFrame->p_kart_states[ kart_index ].position ), 
                     kart->getCoord() );
    }

    return true;
}


#endif // HAVE_GHOST_REPLAY
