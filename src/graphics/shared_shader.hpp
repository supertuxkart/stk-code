//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_SHARED_SHADER_HPP
#define HEADER_SHARED_SHADER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"

class SharedShader : public NoCopy
{
private:
    GLuint m_shader_id;
public:
    // ------------------------------------------------------------------------
    SharedShader() { m_shader_id = 0; }
    // ------------------------------------------------------------------------
    virtual ~SharedShader() {}
    // ------------------------------------------------------------------------
    GLuint getShaderID() const { return m_shader_id; }
    // ------------------------------------------------------------------------
    virtual const char* getName() = 0;
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() = 0;
    // ------------------------------------------------------------------------
    void loadSharedShader();

};   // SharedShader

#endif

#endif   // !SERVER_ONLY

