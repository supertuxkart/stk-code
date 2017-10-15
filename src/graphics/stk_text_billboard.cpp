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

#include "graphics/stk_text_billboard.hpp"
#include "graphics/shaders.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/stk_tex_manager.hpp"
#include <SMesh.h>
#include <SMeshBuffer.h>
#include <ISceneManager.h>
#include <ICameraSceneNode.h>

using namespace irr;

STKTextBillboard::STKTextBillboard(core::stringw text, FontWithFace* font,
    const video::SColor& color_top, const video::SColor& color_bottom,
    irr::scene::ISceneNode* parent,
    irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position, const irr::core::vector3df& size) :
    STKMeshSceneNode(new scene::SMesh(),
        parent, irr_driver->getSceneManager(), -1, "text_billboard",
        position, core::vector3df(0.0f, 0.0f, 0.0f), size, false)
{
    m_color_top = color_top;
    m_color_bottom = color_bottom;
    getTextMesh(text, font);
    createGLMeshes();
    Mesh->drop();
    //setAutomaticCulling(0);
    updateAbsolutePosition();
}

void STKTextBillboard::updateAbsolutePosition()
{
    // Make billboard always face the camera
    scene::ICameraSceneNode* curr_cam =
        irr_driver->getSceneManager()->getActiveCamera();
    if (!curr_cam) return;
    core::quaternion q(curr_cam->getViewMatrix());
    q.W = -q.W;

    if (Parent)
    {
        // Override to not use the parent's rotation
        core::vector3df wc = RelativeTranslation;
        Parent->getAbsoluteTransformation().transformVect(wc);
        AbsoluteTransformation.setTranslation(wc);
        q.getMatrix(AbsoluteTransformation, wc);
    }
    else
    {
        q.getMatrix(AbsoluteTransformation, RelativeTranslation);
    }
    core::matrix4 m;
    m.setScale(RelativeScale);
    AbsoluteTransformation *= m;
}

scene::IMesh* STKTextBillboard::getTextMesh(core::stringw text, FontWithFace* font)
{
    core::dimension2du size = font->getDimension(text.c_str());
    font->render(text, core::rect<s32>(0, 0, size.Width, size.Height), video::SColor(255,255,255,255),
        false, false, NULL, NULL, this);

    const float scale = 0.02f;

    //scene::SMesh* mesh = new scene::SMesh();
    std::map<video::ITexture*, scene::SMeshBuffer*> buffers;

    float max_x = 0;
    float min_y = 0;
    float max_y = 0;
    for (unsigned int i = 0; i < m_chars.size(); i++)
    {
        float char_x = m_chars[i].m_destRect.LowerRightCorner.X;
        if (char_x > max_x)
            max_x = char_x;

        float char_min_y = m_chars[i].m_destRect.UpperLeftCorner.Y;
        float char_max_y = m_chars[i].m_destRect.LowerRightCorner.Y;
        if (char_min_y < min_y)
            min_y = char_min_y;
        if (char_max_y > min_y)
            max_y = char_max_y;
    }
    float scaled_center_x = (max_x / 2.0f) * scale;
    float scaled_y = (max_y / 2.0f) * scale; // -max_y * scale;

    for (unsigned int i = 0; i < m_chars.size(); i++)
    {
        core::vector3df char_pos(m_chars[i].m_destRect.UpperLeftCorner.X,
            m_chars[i].m_destRect.UpperLeftCorner.Y, 0);
        char_pos *= scale;

        core::vector3df char_pos2(m_chars[i].m_destRect.LowerRightCorner.X,
            m_chars[i].m_destRect.LowerRightCorner.Y, 0);
        char_pos2 *= scale;

        //core::dimension2di char_size_i = m_chars[i].m_destRect.getSize();
        //core::dimension2df char_size(char_size_i.Width*scale, char_size_i.Height*scale);

        std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.find(m_chars[i].m_texture);
        scene::SMeshBuffer* buffer;
        if (map_itr == buffers.end())
        {
            buffer = new scene::SMeshBuffer();
            buffer->getMaterial().setTexture(0, m_chars[i].m_texture);
            buffer->getMaterial().setTexture(1, STKTexManager::getInstance()->getUnicolorTexture(video::SColor(0, 0, 0, 0)));
            buffer->getMaterial().setTexture(2, STKTexManager::getInstance()->getUnicolorTexture(video::SColor(0, 0, 0, 0)));
            buffer->getMaterial().MaterialType = Shaders::getShader(ES_OBJECT_UNLIT);
            buffers[m_chars[i].m_texture] = buffer;
        }
        else
        {
            buffer = map_itr->second;
        }

        float tex_width = (float) m_chars[i].m_texture->getSize().Width;
        float tex_height = (float)m_chars[i].m_texture->getSize().Height;


        video::S3DVertex vertices[] =
        {
            video::S3DVertex(char_pos.X - scaled_center_x, char_pos.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_bottom,
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

            video::S3DVertex(char_pos2.X - scaled_center_x, char_pos.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_bottom,
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.LowerRightCorner.Y / tex_height),

            video::S3DVertex(char_pos2.X - scaled_center_x, char_pos2.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_top,
                m_chars[i].m_sourceRect.LowerRightCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height),

            video::S3DVertex(char_pos.X - scaled_center_x, char_pos2.Y - scaled_y, 0.0f,
                0.0f, 0.0f, 1.0f,
                m_color_top,
                m_chars[i].m_sourceRect.UpperLeftCorner.X / tex_width,
                m_chars[i].m_sourceRect.UpperLeftCorner.Y / tex_height)
        };

        irr::u16 indices[] = { 2, 1, 0, 3, 2, 0 };

        buffer->append(vertices, 4, indices, 6);
    }

    for (std::map<video::ITexture*, scene::SMeshBuffer*>::iterator map_itr = buffers.begin();
        map_itr != buffers.end(); map_itr++)
    {
        ((scene::SMesh*)Mesh)->addMeshBuffer(map_itr->second);

        map_itr->second->recalculateBoundingBox();
        Mesh->setBoundingBox(map_itr->second->getBoundingBox()); // TODO: wrong if several buffers

        map_itr->second->drop();
    }

    getMaterial(0).MaterialType = Shaders::getShader(ES_OBJECT_UNLIT);

    return Mesh;
}

void STKTextBillboard::collectChar(video::ITexture* texture,
    const core::rect<float>& destRect,
    const core::rect<s32>& sourceRect,
    const video::SColor* const colors)
{
    m_chars.push_back(STKTextBillboardChar(texture, destRect, sourceRect, colors));
}

#endif   // !SERVER_ONLY

