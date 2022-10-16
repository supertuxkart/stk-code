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

#ifndef SERVER_ONLY

#ifndef HEADER_STK_PARTICLE_HPP
#define HEADER_STK_PARTICLE_HPP

#include "graphics/gl_headers.hpp"
#include "../lib/irrlicht/source/Irrlicht/CParticleSystemSceneNode.h"
#include <cassert>
#include <vector>

using namespace irr;

struct CPUParticle;

inline float glslSmoothstep(float edge0, float edge1, float x)
{
    float t = core::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

class STKParticle : public scene::CParticleSystemSceneNode
{
private:
    // ------------------------------------------------------------------------
    struct HeightMapData
    {
        const std::vector<std::vector<float> > m_array;
        const float m_x;
        const float m_z;
        const float m_x_len;
        const float m_z_len;
        // --------------------------------------------------------------------
        HeightMapData(std::vector<std::vector<float> >& array,
                      float track_x, float track_z, float track_x_len,
                      float track_z_len)
            : m_array(std::move(array)), m_x(track_x), m_z(track_z),
              m_x_len(track_x_len), m_z_len(track_z_len) {}
    };
    // ------------------------------------------------------------------------
    struct ParticleData
    {
        core::vector3df m_position;
        float m_lifetime;
        core::vector3df m_direction;
        float m_size;
    };
    // ------------------------------------------------------------------------
    HeightMapData* m_hm;

    std::vector<ParticleData> m_particles_generating, m_initial_particles;

    core::vector3df m_color_from, m_color_to;

    float m_size_increase_factor;

    bool m_first_execution, m_randomize_initial_y, m_flips, m_pre_generating;

    /** Previous frame particles emitter source matrix */
    core::matrix4 m_previous_frame_matrix;

    /** Maximum count of particles. */
    unsigned m_max_count;

    static std::vector<float> m_flips_data;

    static GLuint m_flips_buffer;

    // ------------------------------------------------------------------------
    void generateParticlesFromPointEmitter(scene::IParticlePointEmitter*);
    // ------------------------------------------------------------------------
    void generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter*);
    // ------------------------------------------------------------------------
    void generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter*);
    // ------------------------------------------------------------------------
    void stimulateHeightMap(float, unsigned int, std::vector<CPUParticle>*);
    // ------------------------------------------------------------------------
    void stimulateNormal(float, unsigned int, std::vector<CPUParticle>*);

public:
    // ------------------------------------------------------------------------
    STKParticle(bool randomize_initial_y = false,
        ISceneNode* parent = 0, s32 id = -1,
        const core::vector3df& position = core::vector3df(0, 0, 0),
        const core::vector3df& rotation = core::vector3df(0, 0, 0),
        const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));
    // ------------------------------------------------------------------------
    ~STKParticle()
    {
        delete m_hm;
    }
    // ------------------------------------------------------------------------
    void setColorFrom(float r, float g, float b)
    {
        m_color_from.X = r;
        m_color_from.Y = g;
        m_color_from.Z = b;
    }
    // ------------------------------------------------------------------------
    void setColorTo(float r, float g, float b)
    {
        m_color_to.X = r;
        m_color_to.Y = g;
        m_color_to.Z = b;
    }
    // ------------------------------------------------------------------------
    virtual void setEmitter(scene::IParticleEmitter* emitter);
    // ------------------------------------------------------------------------
    virtual void OnRegisterSceneNode();
    // ------------------------------------------------------------------------
    void setIncreaseFactor(float val)         { m_size_increase_factor = val; }
    // ------------------------------------------------------------------------
    void setHeightmap(std::vector<std::vector<float> >& array, float track_x,
                      float track_z, float track_x_len, float track_z_len)
    {
        m_hm = new HeightMapData(array, track_x, track_z, track_x_len,
            track_z_len);
    }
    // ------------------------------------------------------------------------
    void generate(std::vector<CPUParticle>* out);
    // ------------------------------------------------------------------------
    void setFlips()                                         { m_flips = true; }
    // ------------------------------------------------------------------------
    virtual bool getFlips() const                           { return m_flips; }
    // ------------------------------------------------------------------------
    unsigned getMaxCount() const                        { return m_max_count; }
    // ------------------------------------------------------------------------
    void setPreGenerating(bool val)                 { m_pre_generating = val; }
    // ------------------------------------------------------------------------
    static void updateFlips(unsigned maximum_particle_count);
    // ------------------------------------------------------------------------
    static void destroyFlipsBuffer()
    {
        if (m_flips_buffer != 0)
        {
            glDeleteBuffers(1, &m_flips_buffer);
            m_flips_buffer = 0;
        }
        m_flips_data.clear();
    }
    // ------------------------------------------------------------------------
    static GLuint getFlipsBuffer()
    {
        assert(m_flips_buffer != 0);
        return m_flips_buffer;
    }
    // ------------------------------------------------------------------------
    virtual bool isSkyParticle() const                 { return m_hm != NULL; }
};

#endif

#endif  // !SERVER_ONLY
