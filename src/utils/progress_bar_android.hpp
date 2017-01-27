//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_PROGRESS_BAR_ANDROID_HPP
#define HEADER_PROGRESS_BAR_ANDROID_HPP

#ifdef ANDROID

#include "IrrlichtDevice.h"
#include "graphics/gl_headers.hpp"

class ProgressBarAndroid
{
private:
    GLuint m_program;
    GLuint m_vertex_shader;
    GLuint m_fragment_shader;
    GLint m_position;
    GLint m_progress;
    GLuint m_vbo;

    irr::IrrlichtDevice* m_device;
    bool m_initialized;
    bool m_close_event_received;

    bool compileShaders();
    void deleteShaders();
    void init();
    void close();

public:
    ProgressBarAndroid();
    ~ProgressBarAndroid();

    void draw(float value);
    bool closeEventReceived() {return m_close_event_received;}
};

#endif

#endif
