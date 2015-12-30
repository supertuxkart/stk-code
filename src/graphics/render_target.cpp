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

#include "graphics/central_settings.hpp"
#include "graphics/render_target.hpp"

//-----------------------------------------------------------------------------
GL1RenderTarget::GL1RenderTarget(const irr::core::dimension2du &dimension,
                                 const std::string &name)
{
    
}

//-----------------------------------------------------------------------------
GLuint GL1RenderTarget::getTextureId() const
{
    
}

//-----------------------------------------------------------------------------
irr::core::dimension2du GL1RenderTarget::getTextureSize() const
{
    
}

//-----------------------------------------------------------------------------
GL3RenderTarget::GL3RenderTarget(const irr::core::dimension2du &dimension)
{
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    if (CVS->isARBTextureStorageUsable())
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, dimension.Width, dimension.Height);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, dimension.Width, dimension.Height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);    
    
    std::vector<GLuint> somevector;
    somevector.push_back(m_texture_id);
    m_frame_buffer = new FrameBuffer(somevector, dimension.Width, dimension.Height);
}

//-----------------------------------------------------------------------------
GL3RenderTarget::~GL3RenderTarget()
{
    glDeleteTextures(1, &m_texture_id);
    delete m_frame_buffer;
}

//-----------------------------------------------------------------------------
irr::core::dimension2du GL3RenderTarget::getTextureSize() const
{
    return irr::core::dimension2du(m_frame_buffer->getWidth(),
                                   m_frame_buffer->getHeight());
}