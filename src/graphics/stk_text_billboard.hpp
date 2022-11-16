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

#include "font/font_with_face.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/sp/sp_instanced_data.hpp"
#include "utils/no_copy.hpp"

#include <ISceneNode.h>
#include <array>
#include <unordered_map>
#include <vector>

using namespace irr;
using namespace scene;

namespace irr
{
    namespace scene
    {
        class IMeshSceneNode;
        class IMeshBuffer;
    }
}

class STKTextBillboard : public ISceneNode, public NoCopy,
                         FontWithFace::FontCharCollector
{
public:
    struct GLTB
    {
        core::vector3df m_position;
        video::SColor m_color;
        short m_uv[2];
    };

private:
    struct STKTextBillboardChar
    {
        video::ITexture* m_texture;

        core::rect<float> m_dest_rect;

        core::rect<s32> m_source_rect;

        // ------------------------------------------------------------------------
        STKTextBillboardChar(video::ITexture* texture,
                             const core::rect<float>& dest_rect,
                             const core::rect<irr::s32>& source_rect,
                             const video::SColor* const colors)
        {
            m_texture = texture;
            m_dest_rect = dest_rect;
            m_source_rect = source_rect;
        }
    };

    SP::SPInstancedData m_instanced_data;

    GLuint m_instanced_array = 0;

    std::vector<STKTextBillboardChar>* m_chars = NULL;

    video::SColor m_color_top;

    video::SColor m_color_bottom;

    std::unordered_map<video::ITexture*, std::vector<std::array<GLTB, 4> > >
        m_gl_tbs;

    std::unordered_map<video::ITexture*, std::pair<GLuint, GLuint> >
        m_vao_vbos;

    std::unordered_map<video::ITexture*, scene::IMeshBuffer*> m_gl_mb;

    core::aabbox3df m_bbox;

    FontWithFace* m_face;

    core::stringw m_text;

    IMeshSceneNode* m_ge_node;

    // ------------------------------------------------------------------------
    float getDefaultScale(FontWithFace* face);
    // ------------------------------------------------------------------------
    void removeGENode();
public:
    // ------------------------------------------------------------------------
    STKTextBillboard(const video::SColor& color_top,
                     const video::SColor& color_bottom, ISceneNode* parent,
                     ISceneManager* mgr, s32 id,
                     const core::vector3df& position,
                     const core::vector3df& scale = core::vector3df(1, 1, 1));
    // ------------------------------------------------------------------------
    ~STKTextBillboard()
    {
        removeGENode();
        clearBuffer();
    }
    // ------------------------------------------------------------------------
    void clearBuffer();
    // ------------------------------------------------------------------------
    void reload();
    // ------------------------------------------------------------------------
    virtual void collectChar(video::ITexture* texture,
                             const core::rect<float>& dest_rect,
                             const core::rect<irr::s32>& source_rect,
                             const video::SColor* const colors);
    // ------------------------------------------------------------------------
    virtual void updateAbsolutePosition();
    // ------------------------------------------------------------------------
    virtual void OnRegisterSceneNode();
    // ------------------------------------------------------------------------
    virtual void render();
    // ------------------------------------------------------------------------
    virtual const core::aabbox3df& getBoundingBox() const    { return m_bbox; }
    // ------------------------------------------------------------------------
    void init(const core::stringw& text, FontWithFace* face);
    // ------------------------------------------------------------------------
    void initLegacy(const core::stringw& text, FontWithFace* face);
    // ------------------------------------------------------------------------
    void draw(video::ITexture* tex) const
    {
#ifndef SERVER_ONLY
        glBindVertexArray(m_vao_vbos.at(tex).first);
        for (unsigned i = 0; i < m_gl_tbs.at(tex).size(); i++)
        {
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, i * 4, 4, 1);
        }
#endif
    }
    // ------------------------------------------------------------------------
    std::vector<video::ITexture*> getAllTBTextures() const
    {
        std::vector<video::ITexture*> ret;
        for (auto& p : m_vao_vbos)
        {
            ret.push_back(p.first);
        }
        return ret;
    }
    // ------------------------------------------------------------------------
    void updateGLInstanceData() const
    {
#ifndef SERVER_ONLY
        glBindBuffer(GL_ARRAY_BUFFER, m_instanced_array);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 36, m_instanced_data.getData());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    }
    // ------------------------------------------------------------------------
    static void updateAllTextBillboards();
};

#endif
