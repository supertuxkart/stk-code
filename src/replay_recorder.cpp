#ifdef HAVE_GHOST_REPLAY

#include "replay_recorder.hpp"

#include <cassert>


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

bool ReplayRecorder::initRecorder( unsigned int number_cars, size_t number_preallocated_frames )
{
    assert( number_cars );

    destroy();

    if( !m_ReplayBuffers.init( number_cars, number_preallocated_frames ) ) return false;

    return true;
}


#endif // HAVE_GHOST_REPLAY
