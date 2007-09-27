#ifdef HAVE_GHOST_REPLAY

#include <cassert>

#include "replay_recorder.hpp"
#include "world.hpp"

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
    assert( world->getPhase() != World::START_PHASE );
    assert( world->getNumKarts() == m_ReplayBuffers.getNumberKarts() );

    // make sure we're not under time-step-min 
    if( m_ReplayBuffers.getNumberFrames() )
    {
        ReplayFrame const *last_Frame = m_ReplayBuffers.getFrameAt( m_ReplayBuffers.getNumberFrames() - 1 );
        if( (world->getClock() - last_Frame->time) < REPLAY_TIME_STEP_MIN ) return true;
    }

    ReplayFrame *pFrame = getNewFrame();
    if( !pFrame ) return false;
    pFrame->time = world->getClock();

    Kart const *kart;
    int number_karts = world->getNumKarts();
    for( int kart_index = 0; kart_index < number_karts; ++kart_index )
    {
        kart = world->getKart( kart_index );
        sgCopyCoord( &( pFrame->p_kart_states[ kart_index ].position ), 
                     kart->getCoord() );
    }

    return true;
}


#endif // HAVE_GHOST_REPLAY
