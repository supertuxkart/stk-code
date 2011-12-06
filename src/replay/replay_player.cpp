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

#include "replay/replay_player.hpp"

#include "karts/kart_properties_manager.hpp"
#include "karts/kart_properties.hpp"


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

    return true;
}

void ReplayPlayer::showReplayAt( float abs_time )
{
    assert( m_ReplayBuffers.getNumberFrames() );
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

    frame = m_ReplayBuffers.getFrameAt( m_current_frame_index );

    // interpolate, if we are not at the end
    if( (m_current_frame_index + 1) < (int)m_ReplayBuffers.getNumberFrames() )
    {
        ReplayFrame const* frame_next = m_ReplayBuffers.getFrameAt( m_current_frame_index+1 );

        // calc scale factor based on time between frames
        assert( frame_next->time > frame->time );
        assert( frame_next->time != frame->time );
        float scale = (abs_time - frame->time) / (frame_next->time - frame->time);

        sgVec3 tmp_v3;
        sgCoord pos;

        // calc interpolations for all objects
        for( size_t k = 0; k < m_Karts.size(); ++k )
        {
            // calc distance between next and current frame-position
            sgCopyVec3( pos.xyz, frame->p_kart_states[k].position.xyz ) ;
            sgCopyVec3( tmp_v3, frame_next->p_kart_states[k].position.xyz ) ;
            sgSubVec3( tmp_v3, pos.xyz );

            // scale it based on time between frames
            sgScaleVec3( tmp_v3, scale );

            // add interpolated vector
            sgAddVec3( pos.xyz, tmp_v3 );

            // no orientation-interpolation for starters
            sgCopyVec3( pos.hpr, frame->p_kart_states[k].position.hpr );

            m_Karts[ k ].setPosition( pos );
        }
    }
    else
    {
        // replay finished, leave them at last known position
        for( size_t k = 0; k < m_Karts.size(); ++k )
        {
            m_Karts[ k ].setPosition( frame->p_kart_states[ k ].position );
        }
    }

}


#endif // HAVE_GHOST_REPLAY
