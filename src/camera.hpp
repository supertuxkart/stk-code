//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_CAMERA_H
#define HEADER_CAMERA_H

class ssgContext;

class Camera
{
public:
    enum Mode {
        CM_NORMAL,
        CM_CLOSEUP,
        CM_LEADER_MODE,   // for deleted player karts in follow the leader
        //FIXME: NO_FAKE_DRIFT is broken
        CM_NO_FAKE_DRIFT,
        CM_SIMPLE_REPLAY
    };
protected:
    ssgContext *m_context ;

    int    m_which_kart;
    Mode m_mode;
    float m_last_steer_offset;
    float m_x, m_y, m_w, m_h ;
    float m_LastPitch;

    bool m_reverse;

public:
    Camera ( int numPlayers, int id ) ;

    /** Set the camera to the given mode */
    void setMode(Mode mode_);

    void setReverseHeading(bool);

    void setScreenPosition ( int numPlayers, int pos ) ;

    void reset();
    void update (float dt) ;
    void apply  () ;

    int getKartId() const { return m_which_kart; }
} ;

#endif

/* EOF */
