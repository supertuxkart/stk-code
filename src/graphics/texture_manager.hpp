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


#ifndef HEADER_TEXTURE_MANAGER_HPP
#define HEADER_TEXTURE_MANAGER_HPP

#include "graphics/gl_headers.hpp"

#include <ITexture.h>
#include <SColor.h>
#include <string>

GLuint getTextureGLuint(irr::video::ITexture *tex);

void resetTextureTable();
void cleanUnicolorTextures();
void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha = false);
bool loadCompressedTexture(const std::string& compressed_tex);
void saveCompressedTexture(const std::string& compressed_tex);
void insertTextureHandle(uint64_t handle);
bool reloadSingleTexture(irr::video::ITexture *tex);
irr::core::stringw reloadTexture(const irr::core::stringw& name);

#endif
