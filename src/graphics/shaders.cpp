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


#ifndef SERVER_ONLY
#include "graphics/shaders.hpp"

// ============================================================================
Shaders::ColoredLine::ColoredLine()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER,   "sp_pass.vert",
                        GL_FRAGMENT_SHADER, "coloredquad.frag");

    assignUniforms("color");

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 1024 * sizeof(float), 0,
        GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    for (int i = 1; i < 16; i++)
    {
        glDisableVertexAttribArray(i);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}   // Shaders::ColoredLine

// ----------------------------------------------------------------------------
Shaders::ColoredLine::~ColoredLine()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}   // ~Shaders::ColoredLine

#endif   // !SERVER_ONLY
