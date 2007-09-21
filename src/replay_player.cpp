#ifdef HAVE_GHOST_REPLAY

#include "kart_properties_manager.hpp"
#include "kart_properties.hpp"

#include "replay_player.hpp"


ReplayKart::ReplayKart()
: m_kart_properties(NULL), m_model(NULL)
{
}

ReplayKart::~ReplayKart()
{
    destroy();
}

void ReplayKart::destroy()
{
    m_kart_properties = NULL;
}

bool ReplayKart::init( const std::string &strKartIdent )
{
    assert( !m_kart_properties );

    m_model = new ssgTransform();
    m_model->ref();

    m_kart_properties = kart_properties_manager->getKart( strKartIdent );
    if( NULL == m_kart_properties ) return false;

    ssgEntity *obj = m_kart_properties->getModel();
    assert( obj );
    // Optimize the model, this can't be done while loading the model
    // because it seems that it removes the name of the wheels or something
    // else needed to load the wheels as a separate object.
    ssgFlatten(obj);

    ssgRangeSelector *lod = new ssgRangeSelector;

    float r [ 2 ] = { -10.0f, 100.0f } ;
    lod -> addKid ( obj ) ;
    lod -> setRanges ( r, 2 ) ;

    m_model -> addKid ( lod ) ;

    return true;
}
    
#include "scene.hpp"

ReplayPlayer::ReplayPlayer() 
: ReplayBase(), m_current_frame_index(-1)
{
}

ReplayPlayer::~ReplayPlayer() 
{ 
    destroy(); 
}

void ReplayPlayer::destroy() 
{ 
    m_current_frame_index = -1;
    m_Karts.clear();
    ReplayBase::destroy();
}

bool ReplayPlayer::loadReplayHumanReadable( FILE *fd ) 
{ 
    destroy();

    bool blnRet = false;
    int intTemp;
    char buff[1000];
    size_t number_karts;
    if( fscanf( fd, "Version: %s\n", buff ) != 1 ) return false;
    if( fscanf( fd, "numkarts: %u\n", &number_karts ) != 1 ) return false;
    if( fscanf( fd, "numplayers: %s\n", buff ) != 1 ) return false;
    if( fscanf( fd, "difficulty: %s\n", buff ) != 1 ) return false;
    if( fscanf( fd, "track: %s\n", buff ) != 1 ) return false;
    for( size_t k = 0; k < number_karts; ++k )
    {
        if( fscanf( fd, "model %d: %s\n", &intTemp, buff ) != 2 ) return false;

        m_Karts.resize( m_Karts.size() + 1 );
        ReplayKart &kart = m_Karts[ m_Karts.size() - 1 ];
        if( !kart.init( buff ) ) return false;

        scene->add ( kart.getModel() );
    }

    if( !m_ReplayBuffers.loadReplayHumanReadable( fd, number_karts ) ) return false;

    m_current_frame_index = 0;
    updateObjects();

    return true;
}

void ReplayPlayer::showReplayAt( float abs_time )
{
    assert( m_current_frame_index > -1 );
    assert( (size_t)m_current_frame_index < m_ReplayBuffers.getNumberFrames() );

    ReplayFrame const* frame;

    // find the current frame, we only scroll forward ..
    while(1)
    {
        // end reached?
        if( (m_current_frame_index + 1) == m_ReplayBuffers.getNumberFrames() ) break;

        // check time of next frame
        frame = m_ReplayBuffers.getFrameAt( m_current_frame_index+1 );
        if( frame->time > abs_time ) break;

        ++m_current_frame_index;
    }

    updateObjects();
}

void ReplayPlayer::updateObjects()
{
    ReplayFrame const* frame = m_ReplayBuffers.getFrameAt( m_current_frame_index );

    for( size_t k = 0; k < m_Karts.size(); ++k )
    {
        m_Karts[ k ].setPosition( frame->p_kart_states[ k ].position );
    }
}


#endif // HAVE_GHOST_REPLAY
