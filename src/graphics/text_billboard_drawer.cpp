//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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
#include "graphics/texture_shader.hpp"

#include <unordered_set>
#include <ITexture.h>

namespace TextBillboardDrawer
{
// ----------------------------------------------------------------------------
std::unordered_map<video::ITexture*, std::vector<STKTextBillboard*> > g_tbs;
// ----------------------------------------------------------------------------
std::unordered_set<STKTextBillboard*> g_tbs_update;
// ============================================================================
/** A Shader to render text billboard.
*/
class TBRenderer : public TextureShader<TBRenderer, 1>
{
public:
    TBRenderer()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "sp_pass.vert",
                    GL_FRAGMENT_SHADER, "sp_text_billboard.frag");
        assignUniforms();
        assignSamplerNames(0, "font_texture",
            ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }   // TBRenderer
};   // TBRenderer

// ============================================================================
void addTextBillboard(STKTextBillboard* tb)
{
    g_tbs_update.insert(tb);
    const auto& tex = tb->getAllTBTextures();
    for (video::ITexture* t : tex)
    {
        g_tbs[t].push_back(tb);
    }
}   // addTextBillboard

// ----------------------------------------------------------------------------
void drawAll()
{
    if (g_tbs.empty())
    {
        return;
    }
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    TBRenderer::getInstance()->use();
    for (auto& p : g_tbs)
    {
        TBRenderer::getInstance()
            ->setTextureUnits(p.first->getTextureHandler());
        for (auto* q : p.second)
        {
            q->draw(p.first);
        }
    }
}   // drawAll

// ----------------------------------------------------------------------------
void reset()
{
    g_tbs.clear();
    g_tbs_update.clear();
}   // reset

// ----------------------------------------------------------------------------
void updateAll()
{
    for (STKTextBillboard* tb : g_tbs_update)
    {
        tb->updateGLInstanceData();
    }
}   // updateAll

}

#endif
