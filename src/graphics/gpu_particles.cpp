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

#include "graphics/gpu_particles.hpp"

#include "config/user_config.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"

#include <ICameraSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include "../../lib/irrlicht/source/Irrlicht/os.h"
#define COMPONENTCOUNT 8


// ============================================================================
/** Transform feedback shader that simulates the particles on GPU.
*/
class PointEmitterShader : public Shader
                           < PointEmitterShader, core::matrix4, core::matrix4, int, int, float >
{
public:
    PointEmitterShader()
    {
        const char *varyings[] = { "new_particle_position", "new_lifetime",
                                   "new_particle_velocity",  "new_size"     };
        loadTFBProgram("pointemitter.vert", varyings, 4);
        assignUniforms("previous_frame_sourcematrix", "sourcematrix",
                       "dt", "level", "size_increase_factor");
    }   // PointEmitterShader

};   // PointEmitterShader

// ============================================================================

/** A Shader to render particles.
*/
class SimpleParticleRender : public TextureShader<SimpleParticleRender, 2,
                                                video::SColorf, video::SColorf>
{
public:
    SimpleParticleRender()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "particle.vert",
                    GL_FRAGMENT_SHADER, "particle.frag");

        assignUniforms("color_from", "color_to");
        assignSamplerNames(0, "tex",  ST_TRILINEAR_ANISOTROPIC_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }   // SimpleParticleRender

};   // SimpleParticleRender

// ============================================================================

class FlipParticleRender : public TextureShader<FlipParticleRender, 2>
{
public:
    FlipParticleRender()
    {
        loadProgram(PARTICLES_RENDERING,
                    GL_VERTEX_SHADER,   "flipparticle.vert",
                    GL_FRAGMENT_SHADER, "particle.frag");
        assignUniforms();
        assignSamplerNames(0, "tex",  ST_TRILINEAR_ANISOTROPIC_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }

};   // FlipParticleShader

// ============================================================================
/** */
class HeightmapSimulationShader : public Shader <HeightmapSimulationShader,
                                                 core::matrix4, int, int,
                                                 float,float,float,float,float>
{
public:
    GLuint m_TU_heightmap;

    HeightmapSimulationShader()
    {
        const char *varyings[] = {"new_particle_position", "new_lifetime",
                                  "new_particle_velocity", "new_size"      };
        loadTFBProgram("particlesimheightmap.vert", varyings, 4);
        assignUniforms("sourcematrix", "dt", "level", "size_increase_factor",
                       "track_x", "track_x_len", "track_z", "track_z_len");
        m_TU_heightmap = 2;
        assignTextureUnit(m_TU_heightmap, "heightmap");
    }   // HeightmapSimulationShader


};   // class HeightmapSimulationShader

// ============================================================================

scene::IParticleSystemSceneNode *ParticleSystemProxy::addParticleNode(
    bool withDefaultEmitter, bool randomize_initial_y, ISceneNode* parent, s32 id,
    const core::vector3df& position,
    const core::vector3df& rotation,
    const core::vector3df& scale)
{
    if (!parent)
        parent = irr_driver->getSceneManager()->getRootSceneNode();

    IParticleSystemSceneNode* node = new ParticleSystemProxy(withDefaultEmitter,
        parent, irr_driver->getSceneManager(), id, position, rotation, scale, randomize_initial_y);
    node->drop();

    return node;
}

ParticleSystemProxy::ParticleSystemProxy(bool createDefaultEmitter,
    ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
    const core::vector3df& position,
    const core::vector3df& rotation,
    const core::vector3df& scale,
    bool randomize_initial_y) : CParticleSystemSceneNode(createDefaultEmitter, parent, mgr, id, position, rotation, scale), m_alpha_additive(false), m_first_execution(true)
{
    if (randomize_initial_y)
        m_randomize_initial_y = randomize_initial_y;

    m_randomize_initial_y = randomize_initial_y;
    size_increase_factor = 0.;
    ParticleParams = NULL;
    InitialValues = NULL;

    m_color_from[0] = m_color_from[1] = m_color_from[2] = 1.0;
    m_color_to[0] = m_color_to[1] = m_color_to[2] = 1.0;

    // We set these later but avoid coverity report them
    heighmapbuffer = 0;
    heightmaptexture = 0;
    has_height_map = false;
    flip = false;
    track_x = 0;
    track_z = 0;
    track_x_len = 0;
    track_z_len = 0;
    texture = 0;
}

ParticleSystemProxy::~ParticleSystemProxy()
{
    if (InitialValues)
        free(InitialValues);
    if (ParticleParams)
        free(ParticleParams);
    if (!m_first_execution)
        cleanGL();
    if (heighmapbuffer)
        glDeleteBuffers(1, &heighmapbuffer);
    if (heightmaptexture)
        glDeleteTextures(1, &heightmaptexture);
}

void ParticleSystemProxy::setFlip()
{
    flip = true;
}

void ParticleSystemProxy::setHeightmap(const std::vector<std::vector<float> > &hm,
    float f1, float f2, float f3, float f4)
{
#if !defined(USE_GLES2)
    track_x = f1, track_z = f2, track_x_len = f3, track_z_len = f4;

    unsigned width  = (unsigned)hm.size();
    unsigned height = (unsigned)hm[0].size();
    float *hm_array = new float[width * height];
    for (unsigned i = 0; i < width; i++)
    {
        for (unsigned j = 0; j < height; j++)
        {
            hm_array[i * height + j] = hm[i][j];
        }
    }
    has_height_map = true;
    glGenBuffers(1, &heighmapbuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, heighmapbuffer);
    glBufferData(GL_TEXTURE_BUFFER, width * height * sizeof(float), hm_array, GL_STREAM_COPY);
    glGenTextures(1, &heightmaptexture);
    glBindTexture(GL_TEXTURE_BUFFER, heightmaptexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, heighmapbuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    delete[] hm_array;
#endif
}

static
void generateLifetimeSizeDirection(scene::IParticleEmitter *emitter, float &lifetime, float &size, float &dirX, float &dirY, float &dirZ)
{
    float sizeMin = emitter->getMinStartSize().Height;
    float sizeMax = emitter->getMaxStartSize().Height;
    float lifetime_range = float(emitter->getMaxLifeTime() - emitter->getMinLifeTime());

    lifetime = os::Randomizer::frand() * lifetime_range;
    lifetime += emitter->getMinLifeTime();

    size = os::Randomizer::frand();
    size *= (sizeMax - sizeMin);
    size += sizeMin;

    core::vector3df particledir = emitter->getDirection();
    particledir.rotateXYBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
    particledir.rotateYZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
    particledir.rotateXZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());

    dirX = particledir.X;
    dirY = particledir.Y;
    dirZ = particledir.Z;
}

void ParticleSystemProxy::generateParticlesFromPointEmitter(scene::IParticlePointEmitter *emitter)
{
    ParticleData* ParticleParamsTmp = (ParticleData *) realloc(ParticleParams, sizeof(ParticleData) * m_count);
    ParticleData* InitialValuesTmp = (ParticleData *)realloc(InitialValues, sizeof(ParticleData)* m_count);

    if (ParticleParamsTmp != NULL)     // In case memory allocation succeeded
    {
        ParticleParams = ParticleParamsTmp;
    }
    else
    {
        Log::error("GPUParticles", "Not enough memory for %d from point particles.", m_count);
        m_count = m_previous_count;
    }
    if (InitialValuesTmp != NULL)
    {
        InitialValues = InitialValuesTmp;
    }
    else
    {
        Log::fatal("GPUParticles", "Not enough memory for %d from point particles.", m_count);
        m_count = m_previous_count;
    }

    for (unsigned i = 0; i < m_count; i++)
    {
        ParticleParams[i].PositionX = 0;
        ParticleParams[i].PositionY = 0;
        ParticleParams[i].PositionZ = 0;
        // Initial lifetime is >1
        InitialValues[i].Lifetime = 2.;

        memcpy(&(InitialValues[i].PositionX), &(ParticleParams[i].PositionX), 3 * sizeof(float));

        generateLifetimeSizeDirection(emitter, ParticleParams[i].Lifetime, ParticleParams[i].Size,
            ParticleParams[i].DirectionX, ParticleParams[i].DirectionY, ParticleParams[i].DirectionZ);

        memcpy(&(InitialValues[i].DirectionX), &(ParticleParams[i].DirectionX), 4 * sizeof(float));
    }
}

void ParticleSystemProxy::generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter *emitter)
{
    ParticleData* ParticleParamsTmp = (ParticleData *) realloc(ParticleParams, sizeof(ParticleData) * m_count);
    ParticleData* InitialValuesTmp = (ParticleData *)realloc(InitialValues, sizeof(ParticleData)* m_count);

    if (ParticleParamsTmp != NULL)     // In case memory allocation succeeded
    {
        ParticleParams = ParticleParamsTmp;
    }
    else
    {
        Log::error("GPUParticles", "Not enough memory for %d from box particles.", m_count);
        m_count = m_previous_count;
    }
    if (InitialValuesTmp != NULL)
    {
        InitialValues = InitialValuesTmp;
    }
    else
    {
        Log::error("GPUParticles", "Not enough memory for %d from box particles.", m_count);
        m_count = m_previous_count;
    }

    const core::vector3df& extent = emitter->getBox().getExtent();

    for (unsigned i = 0; i < m_count; i++)
    {
        ParticleParams[i].PositionX = emitter->getBox().MinEdge.X + os::Randomizer::frand() * extent.X;
        ParticleParams[i].PositionY = emitter->getBox().MinEdge.Y + os::Randomizer::frand() * extent.Y;
        ParticleParams[i].PositionZ = emitter->getBox().MinEdge.Z + os::Randomizer::frand() * extent.Z;
        // Initial lifetime is random
        InitialValues[i].Lifetime = os::Randomizer::frand();
        if (!m_randomize_initial_y)
            InitialValues[i].Lifetime += 1.;

        memcpy(&(InitialValues[i].PositionX), &(ParticleParams[i].PositionX), 3 * sizeof(float));
        generateLifetimeSizeDirection(emitter, ParticleParams[i].Lifetime, ParticleParams[i].Size,
            ParticleParams[i].DirectionX, ParticleParams[i].DirectionY, ParticleParams[i].DirectionZ);
        memcpy(&(InitialValues[i].DirectionX), &(ParticleParams[i].DirectionX), 4 * sizeof(float));

        if (m_randomize_initial_y)
            InitialValues[i].PositionY = os::Randomizer::frand()*50.0f; // -100.0f;
    }
}

void ParticleSystemProxy::generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter *emitter)
{
    ParticleData* ParticleParamsTmp = (ParticleData *) realloc(ParticleParams, sizeof(ParticleData) * m_count);
    ParticleData* InitialValuesTmp = (ParticleData *)realloc(InitialValues, sizeof(ParticleData)* m_count);

    if(ParticleParamsTmp != NULL)     // In case memory allocation succeeded
        ParticleParams = ParticleParamsTmp;
    if(InitialValuesTmp != NULL)
        InitialValues = InitialValuesTmp;

    for (unsigned i = 0; i < m_count; i++) {
        // Random distance from center
        const f32 distance = os::Randomizer::frand() * emitter->getRadius();

        // Random direction from center
        vector3df pos = emitter->getCenter() + distance;
        pos.rotateXYBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateYZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateXZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());

        ParticleParams[i].PositionX = pos.X;
        ParticleParams[i].PositionY = pos.Y;
        ParticleParams[i].PositionZ = pos.Z;
        // Initial lifetime is > 1
        InitialValues[i].Lifetime = 2.;

        memcpy(&(InitialValues[i].PositionX), &(ParticleParams[i].PositionX), 3 * sizeof(float));
        generateLifetimeSizeDirection(emitter, ParticleParams[i].Lifetime, ParticleParams[i].Size,
            ParticleParams[i].DirectionX, ParticleParams[i].DirectionY, ParticleParams[i].DirectionZ);
        memcpy(&(InitialValues[i].DirectionX), &(ParticleParams[i].DirectionX), 4 * sizeof(float));
    }
}

static bool isGPUParticleType(scene::E_PARTICLE_EMITTER_TYPE type)
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
}

void ParticleSystemProxy::setEmitter(scene::IParticleEmitter* emitter)
{
    CParticleSystemSceneNode::setEmitter(emitter);
    if (!emitter || !isGPUParticleType(emitter->getType()))
        return;
    if (!m_first_execution)
        cleanGL();
    has_height_map = false;
    flip = false;
    m_first_execution = true;

    m_previous_count = m_count;   // save to handle out of memory errors
    m_count = emitter->getMaxParticlesPerSecond() * emitter->getMaxLifeTime() / 1000;
    switch (emitter->getType())
    {
    case scene::EPET_POINT:
        generateParticlesFromPointEmitter(emitter);
        break;
    case scene::EPET_BOX:
        generateParticlesFromBoxEmitter(static_cast<scene::IParticleBoxEmitter *>(emitter));
        break;
    case scene::EPET_SPHERE:
        generateParticlesFromSphereEmitter(static_cast<scene::IParticleSphereEmitter *>(emitter));
        break;
    default:
        assert(0 && "Wrong particle type");
    }

    video::ITexture *tex = getMaterial(0).getTexture(0);
    compressTexture(tex, true, true);
    texture = getTextureGLuint(getMaterial(0).getTexture(0));
}

void ParticleSystemProxy::cleanGL()
{
    if (flip)
        glDeleteBuffers(1, &quaternionsbuffer);
    glDeleteBuffers(2, tfb_buffers);
    glDeleteBuffers(1, &initial_values_buffer);
    glDeleteVertexArrays(1, &current_rendering_vao);
    glDeleteVertexArrays(1, &non_current_rendering_vao);
    glDeleteVertexArrays(1, &current_simulation_vao);
    glDeleteVertexArrays(1, &non_current_simulation_vao);
}

void ParticleSystemProxy::CommonRenderingVAO(GLuint PositionBuffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getParticleQuadVBO());
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), 0);
    glVertexAttribDivisorARB(0, 1);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribDivisorARB(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(7 * sizeof(float)));
    glVertexAttribDivisorARB(2, 1);
}

void ParticleSystemProxy::AppendQuaternionRenderingVAO(GLuint QuaternionBuffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, QuaternionBuffer);
    glEnableVertexAttribArray(5);

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribDivisorARB(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribDivisorARB(6, 1);
}

void ParticleSystemProxy::CommonSimulationVAO(GLuint position_vbo, GLuint initialValues_vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(4 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(7 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, initialValues_vbo);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)0);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(4 * sizeof(float)));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleSystemProxy::ParticleData), (GLvoid*)(7 * sizeof(float)));
}

void ParticleSystemProxy::simulate()
{
    int timediff = int(GUIEngine::getLatestDt() * 1000.f);
    int active_count = getEmitter()->getMaxLifeTime() * getEmitter()->getMaxParticlesPerSecond() / 1000;
    core::matrix4 matrix = getAbsoluteTransformation();

    glEnable(GL_RASTERIZER_DISCARD);
    if (has_height_map)
    {
#if !defined(USE_GLES2)
        HeightmapSimulationShader::getInstance()->use();
        glActiveTexture(GL_TEXTURE0 + HeightmapSimulationShader::getInstance()->m_TU_heightmap);
        glBindTexture(GL_TEXTURE_BUFFER, heightmaptexture);
        HeightmapSimulationShader::getInstance()->setUniforms(matrix, timediff, active_count, size_increase_factor, track_x, track_x_len, track_z, track_z_len);
#endif        
    }
    else
    {
        PointEmitterShader::getInstance()->use();
        PointEmitterShader::getInstance()->setUniforms(m_previous_frame_matrix, matrix, timediff, active_count, size_increase_factor);
    }
    m_previous_frame_matrix = matrix;

    glBindVertexArray(current_simulation_vao);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, m_count);
    glEndTransformFeedback();
    glBindVertexArray(0);

    glDisable(GL_RASTERIZER_DISCARD);
#ifdef DEBUG_PARTICLES
    // This code maps the data from the particles (initial data, current data
    // and output of transform feedback shader) into memory. Useful for debugging.
    if (std::string(getName()) == std::string("particles(C:/Users/jhenrich/supertuxkart/stk-assets/textures/skid-particle1.png)"))
    {
        glBindVertexArray(current_simulation_vao);
        glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
        ParticleData *p_initial = (ParticleData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_count*sizeof(ParticleData), GL_MAP_READ_BIT);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
        ParticleData *p_prev = (ParticleData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_count*sizeof(ParticleData), GL_MAP_READ_BIT);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
        ParticleData *p_new = (ParticleData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_count*sizeof(ParticleData), GL_MAP_READ_BIT);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
#endif

    std::swap(tfb_buffers[0], tfb_buffers[1]);
    std::swap(current_rendering_vao, non_current_rendering_vao);
    std::swap(current_simulation_vao, non_current_simulation_vao);
}

void ParticleSystemProxy::drawFlip()
{
    glBlendFunc(GL_ONE, GL_ONE);
    FlipParticleRender::getInstance()->use();

    FlipParticleRender::getInstance()->setTextureUnits(texture, irr_driver->getDepthStencilTexture());
    FlipParticleRender::getInstance()->setUniforms();

    glBindVertexArray(current_rendering_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_count);
}

void ParticleSystemProxy::drawNotFlip()
{
    if (m_alpha_additive)
        glBlendFunc(GL_ONE, GL_ONE);
    else
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    SimpleParticleRender::getInstance()->use();

    SimpleParticleRender::getInstance()->setTextureUnits(texture, irr_driver->getDepthStencilTexture());
    video::SColorf ColorFrom = video::SColorf(getColorFrom()[0], getColorFrom()[1], getColorFrom()[2]);
    video::SColorf ColorTo = video::SColorf(getColorTo()[0], getColorTo()[1], getColorTo()[2]);

    SimpleParticleRender::getInstance()->setUniforms(ColorFrom, ColorTo);

    glBindVertexArray(current_rendering_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_count);
}

void ParticleSystemProxy::draw()
{
    if (flip)
        drawFlip();
    else
        drawNotFlip();
}

void ParticleSystemProxy::generateVAOs()
{
    glBindVertexArray(0);
    glGenBuffers(1, &initial_values_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(ParticleData), ParticleParams, GL_STREAM_COPY);
    glGenBuffers(2, tfb_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(ParticleData), InitialValues, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, m_count * sizeof(ParticleData), 0, GL_STREAM_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &current_rendering_vao);
    glGenVertexArrays(1, &non_current_rendering_vao);
    glGenVertexArrays(1, &current_simulation_vao);
    glGenVertexArrays(1, &non_current_simulation_vao);

    glBindVertexArray(current_simulation_vao);
    CommonSimulationVAO(tfb_buffers[0], initial_values_buffer);
    glBindVertexArray(non_current_simulation_vao);
    CommonSimulationVAO(tfb_buffers[1], initial_values_buffer);


    glBindVertexArray(0);
    if (flip)
    {
        float *quaternions = new float[4 * m_count];
        for (unsigned i = 0; i < m_count; i++)
        {
            core::vector3df rotationdir(0., 1., 0.);

            quaternions[4 * i] = rotationdir.X;
            quaternions[4 * i + 1] = rotationdir.Y;
            quaternions[4 * i + 2] = rotationdir.Z;
            quaternions[4 * i + 3] = 3.14f * 3.f * (2.f * os::Randomizer::frand() - 1.f); // 3 half rotation during lifetime at max
        }
        glGenBuffers(1, &quaternionsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quaternionsbuffer);
        glBufferData(GL_ARRAY_BUFFER, 4 * m_count * sizeof(float), quaternions, GL_STREAM_COPY);
        delete[] quaternions;
    }

    glBindVertexArray(current_rendering_vao);
    CommonRenderingVAO(tfb_buffers[0]);
    if (flip)
        AppendQuaternionRenderingVAO(quaternionsbuffer);

    glBindVertexArray(non_current_rendering_vao);
    CommonRenderingVAO(tfb_buffers[1]);
    if (flip)
        AppendQuaternionRenderingVAO(quaternionsbuffer);
    glBindVertexArray(0);
}

void ParticleSystemProxy::render() {
    if (!getEmitter() || !isGPUParticleType(getEmitter()->getType()))
    {
        CParticleSystemSceneNode::render();
        return;
    }
    if (m_first_execution)
    {
        generateVAOs();
        m_previous_frame_matrix = getAbsoluteTransformation();
        m_first_execution = false;
    }
    else
    {
        simulate();
        draw();
    }
}
