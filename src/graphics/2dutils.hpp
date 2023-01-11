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

#ifndef UTILS2D_HPP
#define UTILS2D_HPP

#include "gl_headers.hpp"

#include <EPrimitiveTypes.h>
#include <irrTypes.h>
#include <ITexture.h>
#include <rect.h>
#include <S3DVertex.h>
#include <SColor.h>
#include <SVertexIndex.h>

void preloadShaders();

void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
                        const irr::core::rect<irr::s32>& destRect,
                        const irr::core::rect<irr::s32>& sourceRect,
                        const irr::core::rect<irr::s32>* clipRect,
                        const irr::video::SColor &colors,
                        bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture,
                 const irr::core::rect<irr::s32>& destRect,
                 const irr::core::rect<irr::s32>& sourceRect,
                 const irr::core::rect<irr::s32>* clipRect,
                 const irr::video::SColor &color,
                 bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture,
                 const irr::core::rect<irr::s32>& destRect,
                 const irr::core::rect<irr::s32>& sourceRect,
                 const irr::core::rect<irr::s32>* clipRect,
                 const irr::video::SColor* const colors,
                 bool useAlphaChannelOfTexture);

void draw2DImageRotationColor(irr::video::ITexture* texture,
                 const irr::core::rect<irr::s32>& destRect,
                 const irr::core::rect<irr::s32>& sourceRect,
                 const irr::core::rect<irr::s32>* clipRect,
                 float rotation, const irr::video::SColor& color);

void draw2DVertexPrimitiveList(irr::video::ITexture *t, const void* vertices,
                  irr::u32 vertexCount, const void* indexList,
                  irr::u32 primitiveCount,
                  irr::video::E_VERTEX_TYPE vType = irr::video::EVT_STANDARD,
                  irr::scene::E_PRIMITIVE_TYPE pType = irr::scene::EPT_TRIANGLES,
                  irr::video::E_INDEX_TYPE iType = irr::video::EIT_16BIT        );

void GL32_draw2DRectangle(irr::video::SColor color,
                          const irr::core::rect<irr::s32>& position,
                          const irr::core::rect<irr::s32>* clip = 0);

#endif
