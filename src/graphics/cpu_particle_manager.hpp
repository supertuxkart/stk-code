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
#include "utils/constants.hpp"
#include "mini_glm.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <dimension2d.h>
#include <IBillboardSceneNode.h>
#include <vector3d.h>
#include <SColor.h>

#include <cassert>
#include <string>
#include <memory>
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
                const core::vector3df& color_to, float lf_time, float size);
    // ------------------------------------------------------------------------
    CPUParticle(scene::IBillboardSceneNode* node);
};


class STKParticle;
class Material;

class CPUParticleManager : public Singleton<CPUParticleManager>, NoCopy
{
private:
    struct GLParticle : public NoCopy
    {
        GLuint m_vao;
        GLuint m_vbo;
        unsigned m_size;
        // --------------------------------------------------------------------
        GLParticle(bool flips);
        // --------------------------------------------------------------------
        ~GLParticle()
        {
            glDeleteVertexArrays(1, &m_vao);
            glDeleteBuffers(1, &m_vbo);
        }
    };

    std::unordered_map<std::string, std::vector<STKParticle*> >
        m_particles_queue;

    std::unordered_map<std::string, std::vector<scene::IBillboardSceneNode*> >
        m_billboards_queue;

    std::unordered_map<std::string, std::vector<CPUParticle> >
        m_particles_generated;

    std::unordered_map<std::string, std::unique_ptr<GLParticle> >
        m_gl_particles;

    std::unordered_map<std::string, Material*> m_material_map;

    std::unordered_set<std::string> m_flips_material, m_sky_material;

    static GLuint m_particle_quad;

    // ------------------------------------------------------------------------
    bool isFlipsMaterial(const std::string& name)
              { return m_flips_material.find(name) != m_flips_material.end(); }

public:
    // ------------------------------------------------------------------------
    CPUParticleManager();
    // ------------------------------------------------------------------------
    ~CPUParticleManager()
    {
        glDeleteBuffers(1, &m_particle_quad);
        m_particle_quad = 0;
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
        m_sky_material.clear();
    }

};

#endif

#endif
