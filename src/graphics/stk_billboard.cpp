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

#include "graphics/stk_billboard.hpp"

#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"

#include <ISceneManager.h>

using namespace irr;


GLuint STKBillboard::m_billboard_vao = 0;


class BillboardShader : public TextureShader<BillboardShader, 1,
                                      core::matrix4, core::vector3df,
                                      core::dimension2df>
{
public:
    BillboardShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "billboard.vert",
                            GL_FRAGMENT_SHADER, "billboard.frag");

        assignUniforms("color_matrix", "Position", "Size");
        assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }   // BillboardShader
};   // BillboardShader

// ============================================================================
void STKBillboard::createBillboardVAO()
{
    glGenVertexArrays(1, &m_billboard_vao);
    glBindVertexArray(m_billboard_vao);
    glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getBillboardVBO());
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                          (GLvoid*) (2 * sizeof(float)));
    glBindVertexArray(0);
}   // createBillboardVAO

// ----------------------------------------------------------------------------
void STKBillboard::destroyBillboardVAO()
{
    if (m_billboard_vao != 0)
    {
        glDeleteVertexArrays(1, &m_billboard_vao);
        m_billboard_vao = 0;
    }
}   // destroyBillboardVAO

// ----------------------------------------------------------------------------
STKBillboard::STKBillboard(irr::scene::ISceneNode* parent,
                           irr::scene::ISceneManager* mgr, irr::s32 id,
                           const irr::core::vector3df& position,
                           const irr::core::dimension2d<irr::f32>& size,
                           irr::video::SColor colorTop, 
                           irr::video::SColor colorBottom)
            : IBillboardSceneNode(parent, mgr, id, position),
              CBillboardSceneNode(parent, mgr, id, position, size, 
                                  colorTop, colorBottom)
{
    if (!m_billboard_vao)
        createBillboardVAO();
}   // STKBillboard

// ----------------------------------------------------------------------------
void STKBillboard::OnRegisterSceneNode()
{
    if (IsVisible)
    {
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
    }

    ISceneNode::OnRegisterSceneNode();
}   // OnRegisterSceneNode

// ----------------------------------------------------------------------------
void STKBillboard::render()
{
    if (irr_driver->getPhase() != TRANSPARENT_PASS)
        return;

    core::vector3df pos = getAbsolutePosition();
    glBindVertexArray(m_billboard_vao);
    video::ITexture *tex = Material.getTexture(0);
    if (!tex)
        return;

    ::Material* material = material_manager->getMaterialFor(tex, 
        video::E_MATERIAL_TYPE::EMT_ONETEXTURE_BLEND);
    if (material->getShaderType() == Material::SHADERTYPE_ADDITIVE)
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    else
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    video::SColor col[2];
    getColor(col[0], col[1]);
    const float colors[] =
    {
        col[1].getRed() / 255.f, col[1].getGreen() / 255.f,
        col[1].getBlue() / 255.f, col[1].getAlpha() / 255.f,
        col[0].getRed() / 255.f, col[0].getGreen() / 255.f,
        col[0].getBlue() / 255.f, col[0].getAlpha() / 255.f,
        col[1].getRed() / 255.f, col[1].getGreen() / 255.f,
        col[1].getBlue() / 255.f, col[1].getAlpha() / 255.f,
        col[0].getRed() / 255.f, col[0].getGreen() / 255.f,
        col[0].getBlue() / 255.f, col[0].getAlpha() / 255.f,
    };
    core::matrix4 color_matrix;
    color_matrix.setM(colors);
    BillboardShader::getInstance()->use();
    BillboardShader::getInstance()->setTextureUnits(tex->getOpenGLTextureName());
    BillboardShader::getInstance()->setUniforms(color_matrix, pos, Size);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}   // render

#endif   // !SERVER_ONLY

