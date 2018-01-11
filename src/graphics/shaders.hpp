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

#ifndef HEADER_SHADERS_HPP
#define HEADER_SHADERS_HPP

#include "graphics/shader.hpp"

#include <SColor.h>

namespace Shaders
{
// ========================================================================
/** Shader to draw a colored line.
 */
class ColoredLine : public Shader<ColoredLine, irr::video::SColor>
{
private:
    GLuint m_vao, m_vbo;
public:
    ColoredLine();
    // --------------------------------------------------------------------
    ~ColoredLine();
    // --------------------------------------------------------------------
    /** Bind the vertex array of this shader. */
    void bindVertexArray()
    {
        glBindVertexArray(m_vao);
        glVertexAttrib4f(8, 0.0f, 0.0f, 0.0f, 0.0f);
        glVertexAttrib4f(9, 0.0f, 0.0f, 0.0f, 0.0f);
        glVertexAttrib4f(10, 1.0f, 1.0f, 1.0f, 1.0f);
    }   // bindVertexArray
    // --------------------------------------------------------------------
    /** Binds the vbo of this shader. */
    void bindBuffer()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    }   // bindBuffer
};   // class ColoredLine

};   // class Shaders

#endif

#endif   // SHADER_ONLY
