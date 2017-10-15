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

#ifndef HEADER_CPU_PARTICLE_MANAGER_HPP
#define HEADER_CPU_PARTICLE_MANAGER_HPP

#ifndef SERVER_ONLY

#include "graphics/gl_headers.hpp"
#include "utils/mini_glm.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <dimension2d.h>
#include <IBillboardSceneNode.h>
#include <vector3d.h>
#include <SColor.h>

#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace irr;

struct CPUParticle
{
    core::vector3df m_position;
    video::SColor m_color_lifetime;
    short m_size[2];
    // ------------------------------------------------------------------------
    CPUParticle(const core::vector3df& position,
                const core::vector3df& color_from,
                const core::vector3df& color_to, float lf_time, float size)
         : m_position(position)
    {
        core::vector3df ret = color_from + (color_to - color_from) * lf_time;
        m_color_lifetime.setRed(core::clamp((int)(ret.X * 255.0f), 0, 255));
        m_color_lifetime.setBlue(core::clamp((int)(ret.Y * 255.0f), 0, 255));
        m_color_lifetime.setGreen(core::clamp((int)(ret.Z * 255.0f), 0, 255));
        m_color_lifetime.setAlpha(core::clamp((int)(lf_time * 255.0f), 0, 255));
        m_size[0] = MiniGLM::toFloat16(size);
        m_size[1] = m_size[0];
    }
    // ------------------------------------------------------------------------
    CPUParticle(scene::IBillboardSceneNode* node)
    {
        m_position = node->getAbsolutePosition();
        video::SColor unused_bottom;
        node->getColor(m_color_lifetime, unused_bottom);
        m_size[0] = MiniGLM::toFloat16(node->getSize().Width);
        m_size[1] = MiniGLM::toFloat16(node->getSize().Height);
    }
};

class STKParticle;
class Material;

class CPUParticleManager : public Singleton<CPUParticleManager>, NoCopy
{
private:
    std::unordered_map<std::string, std::vector<STKParticle*> >
        m_particles_queue;

    std::unordered_map<std::string, std::vector<scene::IBillboardSceneNode*> >
        m_billboards_queue;

    std::unordered_map<std::string, std::vector<CPUParticle> >
        m_particles_generated;

    std::unordered_map<std::string, std::tuple<GLuint/*VAO*/,
        GLuint/*VBO*/, unsigned/*VBO size*/> > m_gl_particles;

    std::unordered_map<std::string, Material*> m_material_map;

    std::unordered_set<std::string> m_flips_material;

    GLuint m_particle_quad;

    // ------------------------------------------------------------------------
    bool isFlipsMaterial(const std::string& name)
              { return m_flips_material.find(name) != m_flips_material.end(); }

public:
    // ------------------------------------------------------------------------
    CPUParticleManager()
    {
        const float vertices[] =
        {
            -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 1.0f, 1.0f,
        };
        glGenBuffers(1, &m_particle_quad);
        glBindBuffer(GL_ARRAY_BUFFER, m_particle_quad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    // ------------------------------------------------------------------------
    ~CPUParticleManager()
    {
        for (auto& p : m_gl_particles)
        {
            glDeleteVertexArrays(1, &std::get<0>(p.second));
            glDeleteBuffers(1, &std::get<1>(p.second));
        }
        glDeleteBuffers(1, &m_particle_quad);
    }
    // ------------------------------------------------------------------------
    void addParticleNode(STKParticle* node);
    // ------------------------------------------------------------------------
    void addBillboardNode(scene::IBillboardSceneNode* node);
    // ------------------------------------------------------------------------
    void generateAll();
    // ------------------------------------------------------------------------
    void uploadAll();
    // ------------------------------------------------------------------------
    void drawAll();
    // ------------------------------------------------------------------------
    void reset()
    {
        for (auto& p : m_particles_queue)
        {
            p.second.clear();
        }
        for (auto& p : m_billboards_queue)
        {
            p.second.clear();
        }
        for (auto& p : m_particles_generated)
        {
            p.second.clear();
        }
    }
    // ------------------------------------------------------------------------
    void cleanMaterialMap()
    {
        m_material_map.clear();
        m_flips_material.clear();
    }

};

#endif

#endif
