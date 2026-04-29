//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "graphics/moving_texture.hpp"

#include "graphics/material.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include "ITexture.h"

/** Constructor for an animated texture.
 *  \param matrix The texture matrix to modify.
 *  \param node An XML node containing dx and dy attributes to set the
 *         speed of the animation.
 */
MovingTexture::MovingTexture(core::matrix4 *matrix, const XMLNode &node)
             : m_matrix(matrix)
{
    m_dx = 0.0f;
    m_dy = 0.0f;
    m_dt = 0.0f;
    m_x = 0.0f;
    m_y = 0.0f;
    m_count = 0.0f;
    m_sp_tm = NULL;

    if (m_matrix)
    {
        core::vector3df v = m_matrix->getTranslation();
        m_x = v.X;
        m_y = v.Y;
    }

    // by default the animation by step is disabled
    m_isAnimatedByStep = false;

    node.get("dx", &m_dx);
    node.get("dy", &m_dy);
    node.get("dt", &m_dt);

    node.get("animByStep", &m_isAnimatedByStep);
}   // MovingTexture

//-----------------------------------------------------------------------------
/** Constructor for an animated texture, specifying the speed of the animation
 *  directly.
 *  \param matrix The texture matrix to modify.
 *  \param dx Speed of the animation in X direction.
 *  \param dy Speed of the animation in Y direction.
 */
MovingTexture::MovingTexture(core::matrix4 *matrix, float dx, float dy)
             : m_matrix(matrix)
{
    // by default the animation by step is disabled
    m_isAnimatedByStep = false;

    m_dx = dx;
    m_dy = dy;
    core::vector3df v = m_matrix->getTranslation();
    m_x = v.X;
    m_y = v.Y;
    m_count = 0.0f;
    m_dt = 0.0f;
    m_sp_tm = NULL;
}   // MovingTexture

//-----------------------------------------------------------------------------
MovingTexture::MovingTexture(float dx, float dy, float dt,
                             bool animated_by_step)
{
    m_isAnimatedByStep = animated_by_step;

    m_dx     = dx;
    m_dy     = dy;
    m_x      = 0;
    m_y      = 0;
    m_count  = 0.0f;
    m_dt     = dt;
    m_matrix = NULL;
    m_sp_tm = NULL;
}   // MovingTexture

//-----------------------------------------------------------------------------
/** Destructor for an animated texture.
 */
MovingTexture::~MovingTexture()
{
}   // ~MovingTexture

//-----------------------------------------------------------------------------
/** Resets at (re)start of a race.
 */
void MovingTexture::reset()
{
    m_x = m_y = 0;
    if (m_matrix)
    {
        m_matrix->setTextureTranslate(m_x, m_y);
    }
    else if (m_sp_tm)
    {
        m_sp_tm[0] = 0.0f;
        m_sp_tm[1] = 0.0f;
    }
}   // reset

//-----------------------------------------------------------------------------
/** Updates the transform of an animated texture.
 *  \param dt Time step size.
 */
void MovingTexture::update(float dt)
{

    if (m_isAnimatedByStep)
    {
        m_count += dt;
        if(m_count > m_dt)
        {
            m_count -= m_dt;

            m_x = m_x + 1.0f*m_dx;
            m_y = m_y + 1.0f*m_dy;
            if(m_x>1.0f) m_x = fmod(m_x, 1.0f);
            if(m_y>1.0f) m_y = fmod(m_y, 1.0f);

            if (m_matrix)
            {
                m_matrix->setTextureTranslate(m_x, m_y);
            }
            else if (m_sp_tm)
            {
                m_sp_tm[0] = m_x;
                m_sp_tm[1] = m_y;
            }
        }
    }
    else
    {
        m_x = m_x + dt*m_dx;
        m_y = m_y + dt*m_dy;
        if(m_x>1.0f) m_x = fmod(m_x, 1.0f);
        if(m_y>1.0f) m_y = fmod(m_y, 1.0f);
        if (m_matrix)
        {
            m_matrix->setTextureTranslate(m_x, m_y);
        }
        else if (m_sp_tm)
        {
            m_sp_tm[0] = m_x;
            m_sp_tm[1] = m_y;
        }
    }
}   // update

namespace MovingTextureUtils
{
    // The ident is used for logging reports, to know what's the source of a missing texture
    std::vector<MovingTexture*> processTextures(scene::ISceneNode *node, const XMLNode &xml,
                                                const std::string& ident)
    {
        std::vector<MovingTexture*> animated_textures;

        for(unsigned int node_number = 0; node_number<xml.getNumNodes(); node_number++)
        {
            const XMLNode *texture_node = xml.getNode(node_number);
            if(texture_node->getName()!="animated-texture") continue;
            std::string name;
            texture_node->get("name", &name);
            if(name=="")
            {
                Log::error("AnimTexture",
                    "An animated texture for '%s' has no name specified!", ident.c_str());
                continue;
            }

            // to lower case, for case-insensitive comparison
            name = StringUtils::toLowerCase(name);

            int moving_textures_found = 0;
            SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(node);
            if (spmn)
            {
                for (unsigned i = 0; i < spmn->getSPM()->getMeshBufferCount(); i++)
                {
                    SP::SPMeshBuffer* spmb = spmn->getSPM()->getSPMeshBuffer(i);
                    const std::vector<Material*>& m = spmb->getAllSTKMaterials();
                    bool found = false;
                    for (unsigned j = 0; j < m.size(); j++)
                    {
                        Material* mat = m[j];
                        std::string mat_name =
                            StringUtils::getBasename(mat->getSamplerPath(0));
                        mat_name = StringUtils::toLowerCase(mat_name);
                        if (mat_name == name)
                        {
                            found = true;
                            moving_textures_found++;
                            spmb->enableTextureMatrix(j);
                            MovingTexture* mt =
                                new MovingTexture(NULL, *texture_node);
                            mt->setSPTM(spmn->getTextureMatrix(i).data());
                            animated_textures.push_back(mt);
                            // For spm only 1 texture matrix per mesh buffer is possible
                            break;
                        }
                    }
                    if (found)
                    {
                        break;
                    }
                }
            }
            else
            {
                Log::warn("AnimTexture", "Non-SPM meshes are deprecated, '%s'", name.c_str());
                for(unsigned int i=0; i<node->getMaterialCount(); i++)
                {
                    video::SMaterial &irrMaterial=node->getMaterial(i);
                    for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
                    {
                        video::ITexture* t=irrMaterial.getTexture(j);
                        if(!t) continue;
                        std::string texture_name =
                            StringUtils::getBasename(t->getName().getPtr());

                        // to lower case, for case-insensitive comparison
                        texture_name = StringUtils::toLowerCase(texture_name);

                        if (texture_name != name) continue;
                        core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
                        animated_textures.push_back(new MovingTexture(m, *texture_node));
                        moving_textures_found++;
                    }   // for j<MATERIAL_MAX_TEXTURES
                }   // for i<getMaterialCount
            }
            if (moving_textures_found == 0)
                Log::warn("AnimTexture", "Did not find animated texture '%s'", name.c_str());
        }   // for node_number < xml->getNumNodes

        return animated_textures;
    }   // processTextures
} // namespace