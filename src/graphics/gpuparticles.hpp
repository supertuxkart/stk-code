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

#ifndef HEADER_GPU_PARTICLES_HPP
#define HEADER_GPU_PARTICLES_HPP

#include "graphics/shader.hpp"

#include "../lib/irrlicht/source/Irrlicht/CParticleSystemSceneNode.h"
#include <ISceneManager.h>
#include <IParticleSystemSceneNode.h>

namespace irr { namespace video{ class ITexture; } }

using namespace irr;

class ParticleSystemProxy : public scene::CParticleSystemSceneNode
{
protected:
    GLuint tfb_buffers[2], initial_values_buffer, heighmapbuffer, heightmaptexture, quaternionsbuffer;
    GLuint current_simulation_vao, non_current_simulation_vao;
    GLuint current_rendering_vao, non_current_rendering_vao;
    bool m_alpha_additive, has_height_map, flip;
    float size_increase_factor, track_x, track_z, track_x_len, track_z_len;
    float m_color_from[3];
    float m_color_to[3];
    bool m_first_execution;
    bool m_randomize_initial_y;

    GLuint texture;

    /** Current count of particles. */
    unsigned m_count;
    /** Previous count - for error handling only. */
    unsigned m_previous_count;

    static void CommonRenderingVAO(GLuint PositionBuffer);
    static void AppendQuaternionRenderingVAO(GLuint QuaternionBuffer);
    static void CommonSimulationVAO(GLuint position_vbo, GLuint initialValues_vbo);

    void generateVAOs();
    void cleanGL();

    void drawFlip();
    void drawNotFlip();
    virtual void simulate();
    virtual void draw();

    struct ParticleData
    {
        float PositionX;
        float PositionY;
        float PositionZ;
        float Lifetime;
        float DirectionX;
        float DirectionY;
        float DirectionZ;
        float Size;
    };

private:

    ParticleData *ParticleParams, *InitialValues;
    void generateParticlesFromPointEmitter(scene::IParticlePointEmitter *);
    void generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter *);
    void generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter *);
public:
    static IParticleSystemSceneNode *addParticleNode(
        bool withDefaultEmitter = true, bool randomize_initial_y = false, ISceneNode* parent = 0, s32 id = -1,
        const core::vector3df& position = core::vector3df(0, 0, 0),
        const core::vector3df& rotation = core::vector3df(0, 0, 0),
        const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));

    ParticleSystemProxy(bool createDefaultEmitter,
        ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
        const core::vector3df& position,
        const core::vector3df& rotation,
        const core::vector3df& scale,
        bool randomize_initial_y);
    ~ParticleSystemProxy();

    virtual void setEmitter(scene::IParticleEmitter* emitter);
    virtual void render();
    void setAlphaAdditive(bool val) { m_alpha_additive = val; }
    void setIncreaseFactor(float val) { size_increase_factor = val; }
    void setColorFrom(float r, float g, float b) { m_color_from[0] = r; m_color_from[1] = g; m_color_from[2] = b; }
    void setColorTo(float r, float g, float b) { m_color_to[0] = r; m_color_to[1] = g; m_color_to[2] = b; }
    const float* getColorFrom() const { return m_color_from; }
    const float* getColorTo() const { return m_color_to; }
    void setHeightmap(const std::vector<std::vector<float> >&, float, float, float, float);
    void setFlip();
};

#endif // GPUPARTICLES_H
