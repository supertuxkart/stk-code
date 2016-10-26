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

#ifndef STK_TEXT_BILLBOARD_HPP
#define STK_TEXT_BILLBOARD_HPP

#include "graphics/stk_mesh_scene_node.hpp"
#include "font/font_with_face.hpp"
#include "utils/cpp2011.hpp"

#include "../lib/irrlicht/source/Irrlicht/CBillboardSceneNode.h"
#include <IBillboardSceneNode.h>
#include <irrTypes.h>
#include <IMesh.h>

class STKTextBillboardChar
{
public:
    irr::video::ITexture* m_texture;
    irr::core::rect<float> m_destRect;
    irr::core::rect<irr::s32> m_sourceRect;
    //irr::video::SColor m_colors[4];

    STKTextBillboardChar(irr::video::ITexture* texture,
        const irr::core::rect<float>& destRect,
        const irr::core::rect<irr::s32>& sourceRect,
        const irr::video::SColor* const colors)
    {
        m_texture = texture;
        m_destRect = destRect;
        m_sourceRect = sourceRect;
        //if (colors == NULL)
        //{
        //    m_colors[0] = m_colors[1] = m_colors[2] = m_colors[3] = NULL;
        //}
        //else
        //{
        //    m_colors[0] = colors[0];
        //    m_colors[1] = colors[1];
        //    m_colors[2] = colors[2];
        //    m_colors[3] = colors[3];
        //}
    }
};

class STKTextBillboard : public STKMeshSceneNode, FontWithFace::FontCharCollector
{
    std::vector<STKTextBillboardChar> m_chars;
    irr::video::SColor m_color_top;
    irr::video::SColor m_color_bottom;

    irr::scene::IMesh* getTextMesh(irr::core::stringw text, FontWithFace* font);

public:
    STKTextBillboard(irr::core::stringw text, FontWithFace* font,
        const irr::video::SColor& color_top,
        const irr::video::SColor& color_bottom,
        irr::scene::ISceneNode* parent,
        irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position,
        const irr::core::vector3df& size);

    virtual scene::ESCENE_NODE_TYPE getType() const OVERRIDE
    {
        return scene::ESNT_TEXT;
    }

    virtual void collectChar(irr::video::ITexture* texture,
        const irr::core::rect<float>& destRect,
        const irr::core::rect<irr::s32>& sourceRect,
        const irr::video::SColor* const colors) OVERRIDE;

    virtual void updateAbsolutePosition() OVERRIDE;
};

#endif
