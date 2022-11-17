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

#include "graphics/stk_particle.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/cpu_particle_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"

#include <cmath>
#include "../../lib/irrlicht/source/Irrlicht/os.h"
#include <ISceneManager.h>
#include <IVideoDriver.h>

// ----------------------------------------------------------------------------
std::vector<float> STKParticle::m_flips_data;
GLuint STKParticle::m_flips_buffer = 0;
// ----------------------------------------------------------------------------
STKParticle::STKParticle(bool randomize_initial_y, ISceneNode* parent, s32 id,
                         const core::vector3df& position,
                         const core::vector3df& rotation,
                         const core::vector3df& scale)
           : CParticleSystemSceneNode(true,
                                      parent ? parent :
                                      irr_driver->getSceneManager()
                                      ->getRootSceneNode(),
                                      irr_driver->getSceneManager(), id,
                                      position, rotation, scale)
{
    m_hm = NULL;
    m_color_to = core::vector3df(1.0f);
    m_color_from = m_color_to;
    m_size_increase_factor = 0.0f;
    m_first_execution = true;
    m_pre_generating = true;
    m_randomize_initial_y = randomize_initial_y;
    m_flips = false;
    m_max_count = 0;
    drop();
}   // STKParticle

// ----------------------------------------------------------------------------
static void generateLifetimeSizeDirection(scene::IParticleEmitter *emitter,
                                          float& lifetime, float& size,
                                          core::vector3df& direction)
{
    float size_min = emitter->getMinStartSize().Height;
    float size_max = emitter->getMaxStartSize().Height;
    float lifetime_range =
        float(emitter->getMaxLifeTime() - emitter->getMinLifeTime());

    lifetime = os::Randomizer::frand() * lifetime_range;
    lifetime += emitter->getMinLifeTime();

    size = os::Randomizer::frand();
    size *= (size_max - size_min);
    size += size_min;

    core::vector3df particledir = emitter->getDirection();
    particledir.rotateXYBy(os::Randomizer::frand() *
        emitter->getMaxAngleDegrees());
    particledir.rotateYZBy(os::Randomizer::frand() *
        emitter->getMaxAngleDegrees());
    particledir.rotateXZBy(os::Randomizer::frand() *
        emitter->getMaxAngleDegrees());
    direction = particledir;
}   // generateLifetimeSizeDirection

// ----------------------------------------------------------------------------
void STKParticle::generateParticlesFromPointEmitter
    (scene::IParticlePointEmitter *emitter)
{
    m_particles_generating.clear();
    m_initial_particles.clear();
    m_particles_generating.resize(m_max_count);
    m_initial_particles.resize(m_max_count);
    for (unsigned i = 0; i < m_max_count; i++)
    {
        // Initial lifetime is > 1
        m_particles_generating[i].m_lifetime = 2.0f;

        generateLifetimeSizeDirection(emitter,
            m_initial_particles[i].m_lifetime,
            m_particles_generating[i].m_size,
            m_particles_generating[i].m_direction);

        m_initial_particles[i].m_direction =
            m_particles_generating[i].m_direction;
        m_initial_particles[i].m_size = m_particles_generating[i].m_size;
    }
}   // generateParticlesFromPointEmitter

// ----------------------------------------------------------------------------
void STKParticle::generateParticlesFromBoxEmitter
    (scene::IParticleBoxEmitter *emitter)
{
    m_particles_generating.clear();
    m_initial_particles.clear();
    m_particles_generating.resize(m_max_count);
    m_initial_particles.resize(m_max_count);
    const core::vector3df& extent = emitter->getBox().getExtent();
    for (unsigned i = 0; i < m_max_count; i++)
    {
        m_particles_generating[i].m_position.X =
            emitter->getBox().MinEdge.X + os::Randomizer::frand() * extent.X;
        m_particles_generating[i].m_position.Y =
            emitter->getBox().MinEdge.Y + os::Randomizer::frand() * extent.Y;
        m_particles_generating[i].m_position.Z =
            emitter->getBox().MinEdge.Z + os::Randomizer::frand() * extent.Z;

        // Initial lifetime is random
        m_particles_generating[i].m_lifetime = os::Randomizer::frand();
        if (!m_randomize_initial_y)
        {
            m_particles_generating[i].m_lifetime += 1.0f;
        }
        m_initial_particles[i].m_position =
            m_particles_generating[i].m_position;

        generateLifetimeSizeDirection(emitter,
            m_initial_particles[i].m_lifetime,
            m_particles_generating[i].m_size,
            m_particles_generating[i].m_direction);

        m_initial_particles[i].m_direction =
            m_particles_generating[i].m_direction;
        m_initial_particles[i].m_size = m_particles_generating[i].m_size;

        if (m_randomize_initial_y)
        {
            m_initial_particles[i].m_position.Y =
                os::Randomizer::frand() * 50.0f; // -100.0f;
        }
    }
}   // generateParticlesFromBoxEmitter

// ----------------------------------------------------------------------------
void STKParticle::generateParticlesFromSphereEmitter
    (scene::IParticleSphereEmitter *emitter)
{
    m_particles_generating.clear();
    m_initial_particles.clear();
    m_particles_generating.resize(m_max_count);
    m_initial_particles.resize(m_max_count);
    for (unsigned i = 0; i < m_max_count; i++)
    {
        // Random distance from center
        const f32 distance = os::Randomizer::frand() * emitter->getRadius();

        // Random direction from center
        vector3df pos = emitter->getCenter() + distance;
        pos.rotateXYBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateYZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateXZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());

        m_particles_generating[i].m_position = pos;

        // Initial lifetime is > 1
        m_particles_generating[i].m_lifetime = 2.0f;
        m_initial_particles[i].m_position =
            m_particles_generating[i].m_position;

        generateLifetimeSizeDirection(emitter,
            m_initial_particles[i].m_lifetime,
            m_particles_generating[i].m_size,
            m_particles_generating[i].m_direction);

        m_initial_particles[i].m_direction =
            m_particles_generating[i].m_direction;
        m_initial_particles[i].m_size = m_particles_generating[i].m_size;
    }
}   // generateParticlesFromSphereEmitter

// ----------------------------------------------------------------------------
static bool isSTKParticleType(scene::E_PARTICLE_EMITTER_TYPE type)
{
    switch (type)
    {
    case scene::EPET_POINT:
    case scene::EPET_BOX:
    case scene::EPET_SPHERE:
        return true;
    default:
        return false;
    }
}   // isSTKParticleType

// ----------------------------------------------------------------------------
void STKParticle::setEmitter(scene::IParticleEmitter* emitter)
{
    CParticleSystemSceneNode::setEmitter(emitter);
    if (!emitter || !isSTKParticleType(emitter->getType()))
    {
        CParticleSystemSceneNode::setEmitter(NULL);
        return;
    }

    delete m_hm;
    m_hm = NULL;
    m_first_execution = true;
    m_pre_generating = true;
    m_flips = false;
    m_max_count = emitter->getMaxParticlesPerSecond() * emitter->getMaxLifeTime() / 1000;

    switch (emitter->getType())
    {
    case scene::EPET_POINT:
        generateParticlesFromPointEmitter(emitter);
        break;
    case scene::EPET_BOX:
        generateParticlesFromBoxEmitter
            (static_cast<scene::IParticleBoxEmitter*>(emitter));
        break;
    case scene::EPET_SPHERE:
        generateParticlesFromSphereEmitter
            (static_cast<scene::IParticleSphereEmitter*>(emitter));
        break;
    default:
        assert(false && "Wrong particle type");
    }
}   // setEmitter

// ----------------------------------------------------------------------------
void STKParticle::generate(std::vector<CPUParticle>* out)
{
    if (!getEmitter())
    {
        return;
    }

    Buffer->BoundingBox.reset(AbsoluteTransformation.getTranslation());
    int active_count = getEmitter()->getMaxLifeTime() *
        getEmitter()->getMaxParticlesPerSecond() / 1000;
    if (m_first_execution)
    {
        m_previous_frame_matrix = AbsoluteTransformation;
        for (int i = 0; i <
            (m_max_count > 5000 ? 5 : m_pre_generating ? 100 : 0); i++)
        {
            if (m_hm != NULL)
            {
                stimulateHeightMap((float)i, active_count, NULL);
            }
            else
            {
                stimulateNormal((float)i, active_count, NULL);
            }
        }
        m_first_execution = false;
    }

    float dt = GUIEngine::getLatestDt() * 1000.f;
    if (m_hm != NULL)
    {
        stimulateHeightMap(dt, active_count, out);
    }
    else
    {
        stimulateNormal(dt, active_count, out);
    }
    m_previous_frame_matrix = AbsoluteTransformation;

    core::matrix4 inv(AbsoluteTransformation, core::matrix4::EM4CONST_INVERSE);
    inv.transformBoxEx(Buffer->BoundingBox);

}   // generate

// ----------------------------------------------------------------------------
inline float glslFract(float val)
{
    return val - (float)floor(val);
}   // glslFract

// ----------------------------------------------------------------------------
inline float glslMix(float x, float y, float a)
{
    return x * (1.0f - a) + y * a;
}   // glslMix

// ----------------------------------------------------------------------------
void STKParticle::stimulateHeightMap(float dt, unsigned int active_count,
                                     std::vector<CPUParticle>* out)
{
    assert(m_hm != NULL);
    const core::matrix4 cur_matrix = AbsoluteTransformation;
    for (unsigned i = 0; i < m_max_count; i++)
    {
        core::vector3df new_particle_position;
        core::vector3df new_particle_direction;
        float new_size = 0.0f;
        float new_lifetime = 0.0f;

        const core::vector3df particle_position =
            m_particles_generating[i].m_position;
        const float lifetime = m_particles_generating[i].m_lifetime;
        const core::vector3df particle_direction =
            m_particles_generating[i].m_direction;

        const core::vector3df particle_position_initial =
            m_initial_particles[i].m_position;
        const float lifetime_initial = m_initial_particles[i].m_lifetime;
        const core::vector3df particle_direction_initial =
            m_initial_particles[i].m_direction;
        const float size_initial = m_initial_particles[i].m_size;

        bool reset = false;
        const int px = core::clamp((int)(256.0f *
            (particle_position.X - m_hm->m_x) / m_hm->m_x_len), 0, 255);
        const int py = core::clamp((int)(256.0f *
            (particle_position.Z - m_hm->m_z) / m_hm->m_z_len), 0, 255);
        const float h = particle_position.Y - m_hm->m_array[px][py];
        reset = h < 0.0f;

        core::vector3df initial_position, initial_new_position;
        cur_matrix.transformVect(initial_position, particle_position_initial);
        cur_matrix.transformVect(initial_new_position,
            particle_position_initial + particle_direction_initial);

        core::vector3df adjusted_initial_direction =
            initial_new_position - initial_position;
        float adjusted_lifetime = lifetime + (dt / lifetime_initial);
        reset = reset || adjusted_lifetime > 1.0f;
        reset = reset || lifetime < 0.0f;

        new_particle_position = !reset ?
            (particle_position + particle_direction * dt) : initial_position;
        new_lifetime = !reset ? adjusted_lifetime : 0.0f;
        new_particle_direction = !reset ?
            particle_direction : adjusted_initial_direction;
        new_size = !reset ?
            glslMix(size_initial, size_initial * m_size_increase_factor,
            adjusted_lifetime) : 0.0f;

        m_particles_generating[i].m_position = new_particle_position;
        m_particles_generating[i].m_lifetime = new_lifetime;
        m_particles_generating[i].m_direction = new_particle_direction;
        m_particles_generating[i].m_size = new_size;
        if (out != NULL)
        {
            if (m_flips || new_size != 0.0f)
            {
                if (new_size != 0.0f)
                {
                    Buffer->BoundingBox.addInternalPoint
                        (new_particle_position);
                }
                out->emplace_back(new_particle_position, m_color_from,
                    m_color_to, new_lifetime, new_size);
            }
        }
    }
}   // stimulateHeightMap

// ----------------------------------------------------------------------------
void STKParticle::stimulateNormal(float dt, unsigned int active_count,
                                  std::vector<CPUParticle>* out)
{
    const core::matrix4 cur_matrix = AbsoluteTransformation;
    core::vector3df previous_frame_position, current_frame_position,
        previous_frame_direction, current_frame_direction;
    for (unsigned i = 0; i < m_max_count; i++)
    {
        core::vector3df new_particle_position;
        core::vector3df new_particle_direction;
        float new_size = 0.0f;
        float new_lifetime = 0.0f;

        const core::vector3df particle_position =
            m_particles_generating[i].m_position;
        const float lifetime = m_particles_generating[i].m_lifetime;
        const core::vector3df particle_direction =
            m_particles_generating[i].m_direction;
        const float size = m_particles_generating[i].m_size;

        const core::vector3df particle_position_initial =
            m_initial_particles[i].m_position;
        const float lifetime_initial = m_initial_particles[i].m_lifetime;
        const core::vector3df particle_direction_initial =
            m_initial_particles[i].m_direction;
        const float size_initial = m_initial_particles[i].m_size;

        float updated_lifetime = lifetime + (dt / lifetime_initial);
        if (updated_lifetime > 1.0f)
        {
            if (i < active_count)
            {
                float dt_from_last_frame =
                    glslFract(updated_lifetime) * lifetime_initial;
                float coeff = 0.0f;
                if (dt > 0.0f)
                    coeff = dt_from_last_frame / dt;

                m_previous_frame_matrix.transformVect(previous_frame_position,
                    particle_position_initial);
                cur_matrix.transformVect(current_frame_position,
                    particle_position_initial);

                core::vector3df updated_position = previous_frame_position
                    .getInterpolated(current_frame_position, coeff);

                m_previous_frame_matrix.rotateVect(previous_frame_direction,
                    particle_direction_initial);
                cur_matrix.rotateVect(current_frame_direction,
                    particle_direction_initial);

                core::vector3df updated_direction = previous_frame_direction
                    .getInterpolated(current_frame_direction, coeff);
                // + (current_frame_position - previous_frame_position) / dt;

                // To be accurate, emitter speed should be added.
                // But the simple formula
                // ( (current_frame_position - previous_frame_position) / dt )
                // with a constant speed between 2 frames creates visual
                // artifacts when the framerate is low, and a more accurate
                // formula would need more complex computations.

                new_particle_position = updated_position + dt_from_last_frame *
                    updated_direction;
                new_particle_direction = updated_direction;

                new_lifetime = glslFract(updated_lifetime);
                new_size = glslMix(size_initial,
                    size_initial * m_size_increase_factor,
                    glslFract(updated_lifetime));
            }
            else
            {
                new_lifetime = glslFract(updated_lifetime);
                new_size = 0.0f;
            }
        }
        else
        {
            new_particle_position = particle_position +
                particle_direction * dt;
            new_particle_direction = particle_direction;
            new_lifetime = updated_lifetime;
            new_size = (size == 0.0f) ? 0.0f :
                glslMix(size_initial, size_initial * m_size_increase_factor,
                updated_lifetime);
        }
        m_particles_generating[i].m_position = new_particle_position;
        m_particles_generating[i].m_lifetime = new_lifetime;
        m_particles_generating[i].m_direction = new_particle_direction;
        m_particles_generating[i].m_size = new_size;
        if (out != NULL)
        {
            if (m_flips || new_size != 0.0f)
            {
                if (new_size != 0.0f)
                {
                    Buffer->BoundingBox.addInternalPoint
                        (new_particle_position);
                }
                out->emplace_back(new_particle_position, m_color_from,
                    m_color_to, new_lifetime, new_size);
            }
        }
    }
}   // stimulateNormal

// ----------------------------------------------------------------------------
void STKParticle::updateFlips(unsigned maximum_particle_count)
{
    bool updated = false;
    while (maximum_particle_count > m_flips_data.size())
    {
        if (m_flips_buffer == 0)
        {
            glGenBuffers(1, &m_flips_buffer);
        }
        updated = true;
        // 3 half rotation during lifetime at max
        m_flips_data.push_back(3.14f * 3.0f * (2.0f * os::Randomizer::frand()
            - 1.0f));
    }
    if (updated)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_flips_buffer);
        glBufferData(GL_ARRAY_BUFFER, m_flips_data.size() * 4,
            m_flips_data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}   // updateFlips

// ----------------------------------------------------------------------------
void STKParticle::OnRegisterSceneNode()
{
    if (CVS->isGLSL())
    {
        Log::warn("STKParticle", "Don't call OnRegisterSceneNode with GLSL");
        return;
    }
    generate(NULL);
    Particles.clear();
    Buffer->BoundingBox.reset(AbsoluteTransformation.getTranslation());
    for (unsigned i = 0; i < m_particles_generating.size(); i++)
    {
        if (m_particles_generating[i].m_size == 0.0f ||
            std::isnan(m_particles_generating[i].m_position.X) ||
            std::isnan(m_particles_generating[i].m_position.Y) ||
            std::isnan(m_particles_generating[i].m_position.Z))
        {
            continue;
        }
        scene::SParticle p;
        p.startTime = 0;
        p.endTime = 0;
        p.color = 0;
        p.startColor = 0;
        p.pos = m_particles_generating[i].m_position;
        Buffer->BoundingBox.addInternalPoint(p.pos);
        p.size = core::dimension2df(m_particles_generating[i].m_size,
            m_particles_generating[i].m_size);
        core::vector3df ret = m_color_from + (m_color_to - m_color_from) *
            m_particles_generating[i].m_lifetime;
        float alpha = 1.0f - m_particles_generating[i].m_lifetime;
        alpha = glslSmoothstep(0.0f, 0.35f, alpha);
        p.color.setRed(core::clamp((int)(ret.X * 255.0f), 0, 255));
        p.color.setGreen(core::clamp((int)(ret.Y * 255.0f), 0, 255));
        p.color.setBlue(core::clamp((int)(ret.Z * 255.0f), 0, 255));
        p.color.setAlpha(core::clamp((int)(alpha * 255.0f), 0, 255));
        if (irr_driver->getVideoDriver()->getDriverType() == video::EDT_VULKAN)
        {
            // Only used in ge_vulkan_draw_call.cpp
            p.startTime = i;
            p.startSize.Width = m_particles_generating[i].m_lifetime;
        }
        Particles.push_back(p);
    }
    core::matrix4 inv(AbsoluteTransformation, core::matrix4::EM4CONST_INVERSE);
    inv.transformBoxEx(Buffer->BoundingBox);
    if (IsVisible && (!Particles.empty()))
    {
        SceneManager->registerNodeForRendering(this);
        ISceneNode::OnRegisterSceneNode();
    }
}   // OnRegisterSceneNode

#endif   // SERVER_ONLY
