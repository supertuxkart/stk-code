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


#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/progress_bar_android.hpp"

#ifdef ANDROID

#include <irrlicht.h>

ProgressBarAndroid::ProgressBarAndroid()
{
#ifndef SERVER_ONLY
    m_program = 0;
    m_vertex_shader = 0;
    m_fragment_shader = 0;
    m_position = 0;
    m_progress = 0;
    m_vbo = 0;
#endif
    m_device = NULL;
    m_initialized = false;
    m_close_event_received = false;

    init();
}

ProgressBarAndroid::~ProgressBarAndroid()
{
    close();
}

bool ProgressBarAndroid::compileShaders()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return false;
        
    const GLchar* vsh =
        "precision mediump float;"
        "attribute vec2 position;"
        "uniform float progress;"
        "void main(void)"
        "{"
        "    float pos_x = (position.x + 1.0) * progress - 1.0;"
        "    float pos_y = position.y * 0.1 - 0.6;"
        "    gl_Position = vec4(pos_x, pos_y, 0.0, 1.0);"
        "}";

    const GLchar* fsh =
        "precision mediump float;"
        "void main(void)"
        "{"
        "    gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);"
        "}";

    m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertex_shader, 1, &vsh, NULL);
    glCompileShader(m_vertex_shader);

    GLint success;
    glGetShaderiv(m_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        Log::error("ProgressBarAndroid", "Failed to compile vertex shader.");
        return false;
    }

    m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragment_shader, 1, &fsh, NULL);
    glCompileShader(m_fragment_shader);

    glGetShaderiv(m_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        Log::error("ProgressBarAndroid", "Failed to compile fragment shader.");
        return false;
    }

    m_program = glCreateProgram();

    glAttachShader(m_program, m_vertex_shader);
    glAttachShader(m_program, m_fragment_shader);
    glLinkProgram(m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        Log::error("ProgressBarAndroid", "Failed to link program.");
        return false;
    }

    m_position = glGetAttribLocation(m_program, "position");
    if (m_position == -1)
    {
        Log::error("ProgressBarAndroid", "Failed to get attrib location.");
        return false;
    }

    m_progress = glGetUniformLocation(m_program, "progress");
    if (m_progress == -1)
    {
        Log::error("ProgressBarAndroid", "Failed to get uniform location.");
        return false;
    }
#endif

    return true;
}

void ProgressBarAndroid::deleteShaders()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
        
    glDeleteShader(m_vertex_shader);
    glDeleteShader(m_fragment_shader);
    glDeleteProgram(m_program);
#endif
}

void ProgressBarAndroid::init()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
        
    SIrrlichtCreationParameters params;
    params.DriverType    = video::EDT_OGLES2;
    params.Bits          = 32;
    params.Fullscreen    = UserConfigParams::m_fullscreen;
    params.WindowSize    = core::dimension2du(640, 480);
    params.PrivateData   = NULL;


    m_device = createDeviceEx(params);

    if (!m_device)
        return;

    bool success = compileShaders();

    if (!success)
        return;

    const GLfloat vertices[] =
    {
        -1,  1,  1, -1, -1, -1,
         1, -1, -1,  1,  1,  1
    };

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(m_program);
    glEnableVertexAttribArray(m_position);
    glVertexAttribPointer(m_position, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_initialized = true;
#endif
}

void ProgressBarAndroid::close()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
        
    glDisableVertexAttribArray(m_position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    deleteShaders();

    if (m_device != NULL)
    {
        m_device->closeDevice();
        m_device->clearSystemMessages();
        m_device->run();
        m_device->drop();
        m_device = NULL;
    }

    m_initialized = false;
#endif
}

void ProgressBarAndroid::draw(float value)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
        
    if (!m_initialized || m_close_event_received)
        return;

    value = value > 1.0f ? 1.0f : value;

    m_close_event_received = !m_device->run();
    
    if (!m_close_event_received)
    {
        m_device->getVideoDriver()->beginScene(true, true);
    
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1f(m_progress, value);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    
        m_device->getVideoDriver()->endScene();
    }
#endif
}

#endif
