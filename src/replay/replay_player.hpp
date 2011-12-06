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

#ifndef HEADER_REPLAYPLAYER_HPP
#define HEADER_REPLAYPLAYER_HPP

#ifdef HAVE_GHOST_REPLAY

#include <vector>
#include "replay/replay_base.hpp"


class KartProperties;
class ReplayKart
{
public:
    ReplayKart();
    ~ReplayKart();

    ReplayKart( ReplayKart const &kart ) { *this = kart; }
    ReplayKart& operator=( ReplayKart const &kart ) 
    {
        assert( this != &kart );
        m_kart_properties = kart.m_kart_properties;
        m_model = kart.m_model;
        sgCopyCoord ( &m_position, &kart.m_position );
        return *this;
    }
    bool                init( const std::string &strKartIdent );
    void                destroy();

    ssgTransform*       getModel() { return m_model; }

    void                setPosition( const sgCoord &pos ) { sgCopyCoord ( &m_position, &pos ); m_model->setTransform(&m_position); }

private:
    const KartProperties    *m_kart_properties;
    sgCoord                 m_position;
    ssgTransform            *m_model;
};

// class managing:
//      - the loading of replay-file
//      - the rendering of the replay (interpolation if needed)
class ReplayPlayer : public ReplayBase
{
public:
    ReplayPlayer();
    virtual ~ReplayPlayer();

    void            destroy();

    bool            loadReplayHumanReadable( FILE *fd );

    // calc state of replay-objects at given time
    void            showReplayAt( float abs_time );

    void            reset()                         { m_current_frame_index = 0; }

private:
    typedef std::vector<ReplayKart>    ReplayKarts;

    int                    m_current_frame_index;
    ReplayKarts            m_Karts;
};


#endif // HAVE_GHOST_REPLAY

#endif // HEADER_REPLAYPLAYER_HPP

