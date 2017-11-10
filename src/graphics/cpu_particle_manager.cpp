//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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


#include "graphics/cpu_particle_manager.hpp"
#include "graphics/stk_particle.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "utils/log.hpp"

#include <algorithm>

#ifndef SERVER_ONLY
#include "graphics/texture_shader.hpp"

// ============================================================================
/** A Shader to render particles.
*/
class ParticleRenderer : public TextureShader<ParticleRenderer, 2, int, float>
{
public:
    ParticleRenderer()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "simple_particle.vert",
                    GL_FRAGMENT_SHADER, "simple_particle.frag");
        assignUniforms("flips", "billboard");
        assignSamplerNames(0, "tex",  ST_TRILINEAR_ANISOTROPIC_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }   // ParticleRenderer
};   // ParticleRenderer

// ============================================================================
/** A Shader to render alpha-test particles.
*/
class AlphaTestParticleRenderer : public TextureShader
    <AlphaTestParticleRenderer, 1, int>
{
public:
    AlphaTestParticleRenderer()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "alphatest_particle.vert",
                    GL_FRAGMENT_SHADER, "alphatest_particle.frag");
        assignUniforms("flips");
        assignSamplerNames(0, "tex",  ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }   // AlphaTestParticleRenderer
};   // AlphaTestParticleRenderer

// ============================================================================
void CPUParticleManager::addParticleNode(STKParticle* node)
{
    if (node->getMaterialCount() != 1)
    {
        Log::error("CPUParticleManager", "More than 1 material");
        return;
    }
    video::ITexture* t = node->getMaterial(0).getTexture(0);
    assert(t != NULL);
    std::string tex_name = t->getName().getPtr();
    Material* m = NULL;
    if (m_material_map.find(tex_name) == m_material_map.end())
    {
        m = material_manager->getMaterialFor(t);
        m_material_map[tex_name] = m;
        if (m == NULL)
        {
            Log::error("CPUParticleManager", "Missing material for particle");
        }
    }
    m = m_material_map.at(tex_name);
    if (m == NULL)
    {
        return;
    }
    if (node->getFlips())
    {
        m_flips_material.insert(tex_name);
    }
    m_particles_queue[tex_name].push_back(node);
}   // addParticleNode

// ----------------------------------------------------------------------------
void CPUParticleManager::addBillboardNode(scene::IBillboardSceneNode* node)
{
    video::ITexture* t = node->getMaterial(0).getTexture(0);
    if (t == NULL)
    {
        return;
    }
    std::string tex_name = t->getName().getPtr();
    tex_name = std::string("_bb_") + tex_name;
    Material* m = NULL;
    if (m_material_map.find(tex_name) == m_material_map.end())
    {
        m = material_manager->getMaterialFor(t);
        m_material_map[tex_name] = m;
        if (m == NULL)
        {
            Log::error("CPUParticleManager", "Missing material for billboard");
        }
    }
    m = m_material_map.at(tex_name);
    if (m == NULL)
    {
        return;
    }
    m_billboards_queue[tex_name].push_back(node);
}   // addBillboardNode

// ----------------------------------------------------------------------------
void CPUParticleManager::generateAll()
{
    for (auto& p : m_particles_queue)
    {
        if (p.second.empty())
        {
            continue;
        }
        for (auto& q : p.second)
        {
            q->generate(&m_particles_generated[p.first]);
        }
        if (isFlipsMaterial(p.first))
        {
            STKParticle::updateFlips(unsigned
                (m_particles_queue.at(p.first).size() *
                m_particles_queue.at(p.first)[0]->getMaxCount()));
        }
    }
    for (auto& p : m_billboards_queue)
    {
        if (p.second.empty())
        {
            continue;
        }
        for (auto& q : p.second)
        {
            m_particles_generated[p.first].emplace_back(q);
        }
    }
}   // generateAll

// ----------------------------------------------------------------------------
void CPUParticleManager::uploadAll()
{
    for (auto& p : m_particles_generated)
    {
        if (p.second.empty())
        {
            continue;
        }
        bool first_upload = false;
        unsigned vbo_size = (unsigned)(m_particles_generated[p.first].size());
        if (m_gl_particles.find(p.first) == m_gl_particles.end())
        {
            first_upload = true;
            m_gl_particles[p.first] = std::make_tuple(0, 0, vbo_size * 2);
        }
        if (first_upload)
        {
            glGenBuffers(1, &std::get<1>(m_gl_particles[p.first]));
            glBindBuffer(GL_ARRAY_BUFFER,
                std::get<1>(m_gl_particles[p.first]));
            glBufferData(GL_ARRAY_BUFFER, vbo_size * 2 * 20, NULL,
                GL_DYNAMIC_DRAW);
            glGenVertexArrays(1, &std::get<0>(m_gl_particles[p.first]));
            glBindVertexArray(std::get<0>(m_gl_particles[p.first]));
            glBindBuffer(GL_ARRAY_BUFFER,
                std::get<1>(m_gl_particles[p.first]));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
            glVertexAttribDivisorARB(0, 1);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20,
                (void*)12);
            glVertexAttribDivisorARB(1, 1);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_HALF_FLOAT, GL_FALSE, 20,
                (void*)16);
            glVertexAttribDivisorARB(2, 1);
            glBindBuffer(GL_ARRAY_BUFFER, m_particle_quad);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 16, 0);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
            if (isFlipsMaterial(p.first))
            {
                glBindBuffer(GL_ARRAY_BUFFER, STKParticle::getFlipsBuffer());
                glEnableVertexAttribArray(6);
                glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 4, 0);
                glVertexAttribDivisorARB(6, 1);
            }
            glBindVertexArray(0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(m_gl_particles[p.first]));
        // Check "real" particle buffer size in opengl
        if (std::get<2>(m_gl_particles[p.first]) < vbo_size)
        {
            std::get<2>(m_gl_particles[p.first]) = vbo_size * 2;
            glBufferData(GL_ARRAY_BUFFER, vbo_size * 2 * 20, NULL,
                GL_DYNAMIC_DRAW);
        }
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, vbo_size * 20,
            GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
            GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, m_particles_generated[p.first].data(), vbo_size * 20);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}   // uploadAll

// ----------------------------------------------------------------------------
void CPUParticleManager::drawAll()
{
    std::vector<std::pair<Material*, std::string> > particle_drawn;
    for (auto& p : m_particles_generated)
    {
        if (!p.second.empty())
        {
            particle_drawn.emplace_back(m_material_map.at(p.first), p.first);
        }
    }
    std::sort(particle_drawn.begin(), particle_drawn.end(),
        [](const std::pair<Material*, std::string>& a,
           const std::pair<Material*, std::string>& b)->bool
        {
            return a.first->getShaderType() > b.first->getShaderType(); 
        });

    Material::ShaderType st = Material::SHADERTYPE_COUNT;
    for (auto& p : particle_drawn)
    {
        const bool flips = isFlipsMaterial(p.second);
        const float billboard = p.second.substr(0, 4) == "_bb_" ? 1.0f : 0.0f;
        Material* cur_mat = p.first;
        if (cur_mat->getShaderType() != st)
        {
            st = cur_mat->getShaderType();
            if (cur_mat->getShaderType() == Material::SHADERTYPE_ADDITIVE)
            {
                ParticleRenderer::getInstance()->use();
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE);
            }
            else if (cur_mat
                ->getShaderType() == Material::SHADERTYPE_ALPHA_BLEND)
            {
                ParticleRenderer::getInstance()->use();
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                AlphaTestParticleRenderer::getInstance()->use();
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_TRUE);
                glDisable(GL_CULL_FACE);
                glDisable(GL_BLEND);
            }
        }

        if (cur_mat->getShaderType() == Material::SHADERTYPE_ADDITIVE ||
            cur_mat->getShaderType() == Material::SHADERTYPE_ALPHA_BLEND)
        {
            ParticleRenderer::getInstance()->setTextureUnits
                (cur_mat->getTexture()->getOpenGLTextureName(),
                irr_driver->getDepthStencilTexture());
            ParticleRenderer::getInstance()->setUniforms(flips, billboard);
        }
        else
        {
            AlphaTestParticleRenderer::getInstance()->setTextureUnits
                (cur_mat->getTexture()->getOpenGLTextureName());
            AlphaTestParticleRenderer::getInstance()->setUniforms(flips);
        }
        glBindVertexArray(std::get<0>(m_gl_particles[p.second]));
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
            (unsigned)m_particles_generated.at(p.second).size());
    }

}   // drawAll

#endif
