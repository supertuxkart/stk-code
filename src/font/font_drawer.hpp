//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#ifndef HEADER_FONT_DRAWER_HPP
#define HEADER_FONT_DRAWER_HPP

#ifndef SERVER_ONLY

#include <array>

#include <ITexture.h>
#include <rect.h>
#include <SColor.h>

using namespace irr;

namespace FontDrawer
{
    // ------------------------------------------------------------------------
    void startBatching();
    // ------------------------------------------------------------------------
    bool isBatching();
    // ------------------------------------------------------------------------
    void endBatching();
    // ------------------------------------------------------------------------
    void addGlyph(video::ITexture* texture,
                  const core::rect<float>& dest_rect,
                  const core::rect<s32>& source_rect,
                  const core::rect<s32>* clip_rect,
                  const video::SColor* color);
    // ------------------------------------------------------------------------
    inline void addGlyph(video::ITexture* texture,
                         const core::rect<float>& dest_rect,
                         const core::rect<s32>& source_rect,
                         const core::rect<s32>* clip_rect,
                         const video::SColor& color)
    {
        std::array<video::SColor, 4> colors = {{ color, color, color, color }};
        addGlyph(texture, dest_rect, source_rect, clip_rect, colors.data());
    }   // addGlyph
    // ------------------------------------------------------------------------
    void draw();
};

#endif

#endif
