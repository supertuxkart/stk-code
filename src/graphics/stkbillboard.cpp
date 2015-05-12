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

#include "graphics/stkbillboard.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/irr_driver.hpp"
#include <ISceneManager.h>

using namespace irr;


static GLuint billboardvao = 0;


class BillboardShader : public Shader<BillboardShader, core::matrix4,
                                      core::matrix4, core::vector3df,
                                      core::dimension2df>,
                       public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    BillboardShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "billboard.vert",
                            GL_FRAGMENT_SHADER, "billboard.frag");

        assignUniforms("ModelViewMatrix", "ProjectionMatrix", "Position",
                       "Size");
        assignSamplerNames(m_program, 
                           0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }   // BillboardShader
};   // BillboardShader

// ============================================================================

static void createBillboardVAO()
{
    glGenVertexArrays(1, &billboardvao);
    glBindVertexArray(billboardvao);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::billboardvbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                          (GLvoid*) (2 * sizeof(float)));
    glBindVertexArray(0);
}   // createBillboardVAO

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
    if (!billboardvao)
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
    glBindVertexArray(billboardvao);
    video::ITexture *tex = Material.getTexture(0);
    if (!tex )
        return;

    compressTexture(tex, true, true);
    GLuint texid = getTextureGLuint(tex);
    BillboardShader::getInstance()->use();
    BillboardShader::getInstance()->setTextureUnits(texid);
    BillboardShader::getInstance()->setUniforms(irr_driver->getViewMatrix(), 
                                                irr_driver->getProjMatrix(),
                                                pos, Size);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}   // render
