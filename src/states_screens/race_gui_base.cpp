//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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


#include "states_screens/race_gui_base.hpp"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  define _WINSOCKAPI_
#  ifdef WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif

#include "karts/kart.hpp"

RaceGUIBase::RaceGUIBase()
{
    m_lightning             = 0.0f;

}   // RaceGUIBase

// ----------------------------------------------------------------------------
/** Updates lightning related information.
*/
void RaceGUIBase::renderGlobal(float dt)
{
    if (m_lightning > 0.0f) m_lightning -= dt;

}   // renderGlobal

// ----------------------------------------------------------------------------
void RaceGUIBase::renderPlayerView(const Kart *kart)
{
    const core::recti &viewport    = kart->getCamera()->getViewport();

    if (m_lightning > 0.0f)
    {
        GLint glviewport[4];
        glviewport[0] = viewport.UpperLeftCorner.X;
        glviewport[1] = viewport.UpperLeftCorner.Y;
        glviewport[2] = viewport.LowerRightCorner.X;
        glviewport[3] = viewport.LowerRightCorner.Y;
        //glGetIntegerv(GL_VIEWPORT, glviewport);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColor4f(0.7f*m_lightning, 0.7f*m_lightning, 0.7f*std::min(1.0f, m_lightning*1.5f), 1.0f);
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_CULL_FACE);
        glBegin(GL_QUADS);
        
        glVertex3d(glviewport[0],glviewport[1],0);
        glVertex3d(glviewport[0],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[1],0);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
#if 0 // Rainy look, off, TODO: needs to be settable per track
    else
    {
        GLint glviewport[4];
        glGetIntegerv(GL_VIEWPORT, glviewport);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_CULL_FACE);
        glBegin(GL_QUADS);
        
        glVertex3d(glviewport[0],glviewport[1],0);
        glVertex3d(glviewport[0],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[3],0);
        glVertex3d(glviewport[2],glviewport[1],0);
        glEnd();
        glEnable(GL_BLEND);
    }
#endif

}   // renderPlayerView
