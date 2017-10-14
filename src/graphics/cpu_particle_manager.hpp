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
#include <vector3d.h>
#include <SColor.h>

#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace irr;

struct CPUParticle
{
    core::vector3df m_position;
    video::SColor m_mixed_color;
    short m_lifetime_and_size[2];
    // ------------------------------------------------------------------------
    CPUParticle(const core::vector3df& position,
                const core::vector3df& color_from,
                const core::vector3df& color_to, float lifetime, float size)
         : m_position(position), m_mixed_color(-1)
    {
        core::vector3df ret = color_from + (color_to - color_from) * lifetime;
        m_mixed_color.setRed(core::clamp((int)(ret.X * 255.0f), 0, 255));
        m_mixed_color.setBlue(core::clamp((int)(ret.Y * 255.0f), 0, 255));
        m_mixed_color.setGreen(core::clamp((int)(ret.Z * 255.0f), 0, 255));
        m_lifetime_and_size[0] = MiniGLM::toFloat16(lifetime);
        m_lifetime_and_size[1] = MiniGLM::toFloat16(size);
    }
};

class STKParticle;
class Material;

class CPUParticleManager : public Singleton<CPUParticleManager>, NoCopy
{
private:
    std::unordered_map<std::string, std::vector<STKParticle*> >
        m_particles_queue;

    std::unordered_map<std::string, std::vector<CPUParticle> >
        m_particles_generated;

    std::unordered_map<std::string, std::tuple<GLuint/*VAO*/,
        GLuint/*VBO*/, unsigned/*VBO*/> > m_gl_particles;

    std::unordered_map<std::string, Material*> m_material_map;

public:
    // ------------------------------------------------------------------------
    CPUParticleManager() {}
    // ------------------------------------------------------------------------
    ~CPUParticleManager();
    // ------------------------------------------------------------------------
    void addParticleNode(STKParticle* node);
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
        for (auto& p : m_particles_generated)
        {
            p.second.clear();
        }
    }
    // ------------------------------------------------------------------------
    void cleanMaterialMap()
    {
        m_material_map.clear();
    }

};

#endif

#endif
