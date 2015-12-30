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

#ifndef HEADER_RENDER_TARGET_HPP
#define HEADER_RENDER_TARGET_HPP

#include "graphics/glwrap.hpp"
#include <irrlicht.h>
#include <string>

class RenderTarget
{
public:
    virtual GLuint                   getTextureId()   const = 0;
    virtual irr::core::dimension2du  getTextureSize() const = 0;
};

class GL1RenderTarget: public RenderTarget
{
private:
    irr::video::ITexture *m_render_target_texture;
    
    
public:
    GL1RenderTarget(const irr::core::dimension2du &dimension,
                    const std::string &name);
    
    GLuint getTextureId() const;
    irr::core::dimension2du getTextureSize() const;   
    
    
};

class GL3RenderTarget: public RenderTarget
{
private:
    FrameBuffer *m_frame_buffer;
    GLuint m_texture_id;
    
public:
    GL3RenderTarget(const irr::core::dimension2du &dimension);
    ~GL3RenderTarget();
    
    GLuint getTextureId() const                       { return m_texture_id;    }
    irr::core::dimension2du getTextureSize() const;
    FrameBuffer* getFrameBuffer()                     { return m_frame_buffer; }
    
};


#endif
