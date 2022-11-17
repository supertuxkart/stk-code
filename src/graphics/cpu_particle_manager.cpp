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
#include "graphics/sp/sp_base.hpp"
#include "graphics/stk_particle.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "utils/log.hpp"

#include <algorithm>

#ifndef SERVER_ONLY
#include <ICameraSceneNode.h>
#include <ISceneManager.h>
#include <ITexture.h>

#include "graphics/texture_shader.hpp"

// ============================================================================
/** A Shader to render particles.
*/
class ParticleRenderer : public TextureShader
    <ParticleRenderer, 2, int, int, core::vector3df, float>
{
public:
    ParticleRenderer()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "simple_particle.vert",
                    GL_FRAGMENT_SHADER, "simple_particle.frag");
        assignUniforms("flips", "sky", "view_position", "billboard");
        assignSamplerNames(0, "tex",  ST_TRILINEAR_ANISOTROPIC_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }   // ParticleRenderer
};   // ParticleRenderer

// ============================================================================
/** A Shader to render alpha-test particles.
*/
class AlphaTestParticleRenderer : public TextureShader
    <AlphaTestParticleRenderer, 1, int, int, core::vector3df, float>
{
public:
    AlphaTestParticleRenderer()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "alphatest_particle.vert",
                    GL_FRAGMENT_SHADER, "alphatest_particle.frag");
        assignUniforms("flips", "sky", "view_position", "billboard");
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
    else if (node->isSkyParticle())
    {
        m_sky_material.insert(tex_name);
    }
    m_particles_queue[tex_name].push_back(node);
}   // addParticleNode

// ============================================================================
CPUParticle::CPUParticle(const core::vector3df& position,
                         const core::vector3df& color_from,
                         const core::vector3df& color_to, float lf_time,
                         float size)
           : m_position(position)
{
    core::vector3df ret = color_from + (color_to - color_from) * lf_time;
    float alpha = 1.0f - lf_time;
    alpha = glslSmoothstep(0.0f, 0.35f, alpha);
    int r = core::clamp((int)(ret.X * 255.0f), 0, 255);
    int g = core::clamp((int)(ret.Y * 255.0f), 0, 255);
    int b = core::clamp((int)(ret.Z * 255.0f), 0, 255);
    if (CVS->isDeferredEnabled() && CVS->isGLSL())
    {
        using namespace SP;
        r = srgb255ToLinear(r);
        g = srgb255ToLinear(g);
        b = srgb255ToLinear(b);
    }
    m_color_lifetime.setRed(r);
    m_color_lifetime.setGreen(g);
    m_color_lifetime.setBlue(b);
    m_color_lifetime.setAlpha(core::clamp((int)(alpha * 255.0f), 0, 255));
    if (size != 0.0f)
    {
        m_size[0] = MiniGLM::toFloat16(size);
        m_size[1] = MiniGLM::toFloat16(lf_time);
    }
    else
        memset(m_size, 0, sizeof(m_size));
}   // CPUParticle

// ============================================================================
CPUParticle::CPUParticle(scene::IBillboardSceneNode* node)
{
    m_position = node->getAbsolutePosition();
    video::SColor unused_bottom;
    node->getColor(m_color_lifetime, unused_bottom);
    if (CVS->isDeferredEnabled() && CVS->isGLSL())
    {
        using namespace SP;
        m_color_lifetime.setRed(srgb255ToLinear(m_color_lifetime.getRed()));
        m_color_lifetime.setGreen(srgb255ToLinear(m_color_lifetime.getGreen()));
        m_color_lifetime.setBlue(srgb255ToLinear(m_color_lifetime.getBlue()));
    }
    m_size[0] = MiniGLM::toFloat16(node->getSize().Width);
    m_size[1] = MiniGLM::toFloat16(node->getSize().Height);
}   // CPUParticle

// ============================================================================
GLuint CPUParticleManager::m_particle_quad = 0;
// ----------------------------------------------------------------------------
CPUParticleManager::GLParticle::GLParticle(bool flips)
{
    m_size = 1;
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_size * 20, NULL,
        GL_DYNAMIC_DRAW);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
    glVertexAttribDivisor(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20,
        (void*)12);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_HALF_FLOAT, GL_FALSE, 20,
        (void*)16);
    glVertexAttribDivisor(2, 1);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_quad);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 16, 0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
    if (flips)
    {
        glBindBuffer(GL_ARRAY_BUFFER, STKParticle::getFlipsBuffer());
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 4, 0);
        glVertexAttribDivisor(6, 1);
    }
    glBindVertexArray(0);
}   // GLParticle

// ----------------------------------------------------------------------------
CPUParticleManager::CPUParticleManager()
{
    assert(CVS->isGLSL());
    
    const float vertices[] =
    {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
    };
    glGenBuffers(1, &m_particle_quad);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // For preloading shaders
    ParticleRenderer::getInstance();
    AlphaTestParticleRenderer::getInstance();
}   // CPUParticleManager

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
        unsigned vbo_size = (unsigned)(m_particles_generated[p.first].size());
        if (m_gl_particles.find(p.first) == m_gl_particles.end())
        {
            m_gl_particles[p.first] = std::unique_ptr<GLParticle>
                (new GLParticle(isFlipsMaterial(p.first)));
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_gl_particles.at(p.first)->m_vbo);

        // Check "real" particle buffer size in opengl
        if (m_gl_particles.at(p.first)->m_size < vbo_size)
        {
            m_gl_particles.at(p.first)->m_size = vbo_size * 2;
            m_particles_generated.at(p.first).reserve(vbo_size * 2);
            glBufferData(GL_ARRAY_BUFFER, vbo_size * 2 * 20,
                m_particles_generated.at(p.first).data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            continue;
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
    using namespace SP;
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
            return a.first->getShaderName() > b.first->getShaderName();
        });

    std::string shader_name;
    core::vector3df view_position;
    scene::ICameraSceneNode* cam = irr_driver->getSceneManager()
        ->getActiveCamera();
    if (cam)
        view_position = cam->getPosition();
    for (auto& p : particle_drawn)
    {
        const bool flips = isFlipsMaterial(p.second);
        const bool sky = m_sky_material.find(p.second) != m_sky_material.end();
        const float billboard = p.second.substr(0, 4) == "_bb_" ? 1.0f : 0.0f;
        Material* cur_mat = p.first;
        if (cur_mat->getShaderName() != shader_name)
        {
            shader_name = cur_mat->getShaderName();
            if (cur_mat->getShaderName() == "additive")
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
            else if (cur_mat->getShaderName() == "alphablend")
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

        if (cur_mat->getShaderName() == "additive" ||
            cur_mat->getShaderName() == "alphablend")
        {
            ParticleRenderer::getInstance()->setTextureUnits
                (cur_mat->getTexture()->getTextureHandler(),
                CVS->isDeferredEnabled() ?
                irr_driver->getDepthStencilTexture() : 0);
            ParticleRenderer::getInstance()->setUniforms(flips, sky,
                view_position, billboard);
        }
        else
        {
            AlphaTestParticleRenderer::getInstance()->setTextureUnits
                (cur_mat->getTexture()->getTextureHandler());
            AlphaTestParticleRenderer::getInstance()->setUniforms(flips, sky,
                view_position, billboard);
        }
        glBindVertexArray(m_gl_particles.at(p.second)->m_vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
            (unsigned)m_particles_generated.at(p.second).size());
    }

}   // drawAll

#endif
