#ifdef HAVE_GHOST_REPLAY

#include "replay_base.hpp"


const std::string ReplayBase::REPLAY_FOLDER = "replay";
const std::string ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE = "rph";
const std::string ReplayBase::REPLAY_FILE_EXTENSION_BINARY = "rpb";


ReplayBase::ReplayBase()
: m_ReplayBuffers()
{
}

ReplayBase::~ReplayBase()
{
    destroy();
}

void ReplayBase::destroy()
{
    m_ReplayBuffers.destroy();
}




#endif // HAVE_GHOST_REPLAY
