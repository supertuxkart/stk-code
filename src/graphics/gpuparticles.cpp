#include "graphics/irr_driver.hpp"
#include "gpuparticles.hpp"
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include <ICameraSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include "guiengine/engine.hpp"

#define COMPONENTCOUNT 8

scene::IParticleSystemSceneNode *ParticleSystemProxy::addParticleNode(
    bool withDefaultEmitter, ISceneNode* parent, s32 id,
    const core::vector3df& position,
    const core::vector3df& rotation,
    const core::vector3df& scale)
{
    if (!parent)
        parent = irr_driver->getSceneManager()->getRootSceneNode();

    IParticleSystemSceneNode* node = new ParticleSystemProxy(withDefaultEmitter,
        parent, irr_driver->getSceneManager(), id, position, rotation, scale);
    node->drop();

    return node;
}

GLuint ParticleSystemProxy::quad_vertex_buffer = 0;

ParticleSystemProxy::ParticleSystemProxy(bool createDefaultEmitter,
    ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
    const core::vector3df& position,
    const core::vector3df& rotation,
    const core::vector3df& scale) : CParticleSystemSceneNode(createDefaultEmitter, parent, mgr, id, position, rotation, scale), m_alpha_additive(false)
{
    glGenBuffers(1, &initial_values_buffer);
    glGenBuffers(2, tfb_buffers);
    glGenBuffers(1, &quaternionsbuffer);
    glGenVertexArrays(1, &current_rendering_vao);
    glGenVertexArrays(1, &non_current_rendering_vao);
    size_increase_factor = 0.;

    m_color_from[0] = m_color_from[1] = m_color_from[2] = 1.0;
    m_color_to[0] = m_color_to[1] = m_color_to[2] = 1.0;


    // We set these later but avoid coverity report them
    heighmapbuffer = 0;
    heightmaptexture = 0;
    current_simulation_vao = 0;
    has_height_map = false;
    flip = false;
    track_x = 0;
    track_z = 0;
    track_x_len = 0;
    track_z_len = 0;
    texture = 0;

    if (quad_vertex_buffer)
        return;
    static const GLfloat quad_vertex[] = {
        -.5, -.5, 0., 0.,
        .5, -.5, 1., 0.,
        -.5, .5, 0., 1.,
        .5, .5, 1., 1.,
    };
    glGenBuffers(1, &quad_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex), quad_vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ParticleSystemProxy::~ParticleSystemProxy()
{
    glDeleteBuffers(2, tfb_buffers);
    glDeleteBuffers(1, &initial_values_buffer);
    if (quaternionsbuffer)
        glDeleteBuffers(1, &quaternionsbuffer);
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
    track_x = f1, track_z = f2, track_x_len = f3, track_z_len = f4;

    unsigned width = hm.size();
    unsigned height = hm[0].size();
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
    glBufferData(GL_TEXTURE_BUFFER, width * height * sizeof(float), hm_array, GL_STATIC_DRAW);
    glGenTextures(1, &heightmaptexture);
    glBindTexture(GL_TEXTURE_BUFFER, heightmaptexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, heighmapbuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    delete[] hm_array;
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

void ParticleSystemProxy::generateParticlesFromPointEmitter(scene::IParticlePointEmitter *emitter)
{
    ParticleData *particles = new ParticleData[count], *initialvalue = new ParticleData[count];

    for (unsigned i = 0; i < count; i++)
    {
        particles[i].PositionX = 0;
        particles[i].PositionY = 0;
        particles[i].PositionZ = 0;
        // Initial lifetime is >1
        particles[i].Lifetime = 2.;

        memcpy(&(initialvalue[i].PositionX), &(particles[i].PositionX), 3 * sizeof(float));

        generateLifetimeSizeDirection(emitter, initialvalue[i].Lifetime, initialvalue[i].Size,
            initialvalue[i].DirectionX, initialvalue[i].DirectionY, initialvalue[i].DirectionZ);

        memcpy(&(particles[i].DirectionX), &(initialvalue[i].DirectionX), 4 * sizeof(float));
    }

    glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), initialvalue, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), particles, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), 0, GL_STREAM_DRAW);
    delete[] particles;
    delete[] initialvalue;
}

void ParticleSystemProxy::generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter *emitter)
{
    ParticleData *particles = new ParticleData[count], *initialvalue = new ParticleData[count];

    const core::vector3df& extent = emitter->getBox().getExtent();

    for (unsigned i = 0; i < count; i++) {
        particles[i].PositionX = emitter->getBox().MinEdge.X + os::Randomizer::frand() * extent.X;
        particles[i].PositionY = emitter->getBox().MinEdge.Y + os::Randomizer::frand() * extent.Y;
        particles[i].PositionZ = emitter->getBox().MinEdge.Z + os::Randomizer::frand() * extent.Z;
        // Initial lifetime is random
        particles[i].Lifetime = os::Randomizer::frand();

        memcpy(&(initialvalue[i].PositionX), &(particles[i].PositionX), 3 * sizeof(float));
        generateLifetimeSizeDirection(emitter, initialvalue[i].Lifetime, initialvalue[i].Size,
            initialvalue[i].DirectionX, initialvalue[i].DirectionY, initialvalue[i].DirectionZ);
        memcpy(&(particles[i].DirectionX), &(initialvalue[i].DirectionX), 4 * sizeof(float));
    }
    glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), initialvalue, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), particles, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), 0, GL_STREAM_DRAW);
    delete[] particles;
    delete[] initialvalue;
}

void ParticleSystemProxy::generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter *emitter)
{
    ParticleData *particles = new ParticleData[count], *initialvalue = new ParticleData[count];

    for (unsigned i = 0; i < count; i++) {
        // Random distance from center
        const f32 distance = os::Randomizer::frand() * emitter->getRadius();

        // Random direction from center
        vector3df pos = emitter->getCenter() + distance;
        pos.rotateXYBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateYZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
        pos.rotateXZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());

        particles[i].PositionX = pos.X;
        particles[i].PositionY = pos.Y;
        particles[i].PositionZ = pos.Z;
        // Initial lifetime is > 1
        particles[i].Lifetime = 2.;

        memcpy(&(initialvalue[i].PositionX), &(particles[i].PositionX), 3 * sizeof(float));
        generateLifetimeSizeDirection(emitter, initialvalue[i].Lifetime, initialvalue[i].Size,
            initialvalue[i].DirectionX, initialvalue[i].DirectionY, initialvalue[i].DirectionZ);
        memcpy(&(particles[i].DirectionX), &(initialvalue[i].DirectionX), 4 * sizeof(float));
    }
    glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), initialvalue, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), particles, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(ParticleData), 0, GL_STREAM_DRAW);
    delete[] particles;
    delete[] initialvalue;
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

template<typename T>
void setPositionQuadAttributes(GLuint quad_vbo, GLuint position_vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glEnableVertexAttribArray(T::attrib_quadcorner);
    glVertexAttribPointer(T::attrib_quadcorner, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(T::attrib_texcoord);
    glVertexAttribPointer(T::attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glEnableVertexAttribArray(T::attrib_pos);
    glVertexAttribPointer(T::attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), 0);
    glVertexAttribDivisor(T::attrib_pos, 1);
    glEnableVertexAttribArray(T::attrib_lf);
    glVertexAttribPointer(T::attrib_lf, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(3 * sizeof(float)));
    glVertexAttribDivisor(T::attrib_lf, 1);
    glEnableVertexAttribArray(T::attrib_sz);
    glVertexAttribPointer(T::attrib_sz, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(7 * sizeof(float)));
    glVertexAttribDivisor(T::attrib_sz, 1);
}

void ParticleSystemProxy::FlipParticleVAOBind(GLuint PositionBuffer, GLuint QuaternionBuffer)
{
    setPositionQuadAttributes<ParticleShader::FlipParticleRender>(quad_vertex_buffer, PositionBuffer);
    glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_rotationvec);
    glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_anglespeed);

    glBindBuffer(GL_ARRAY_BUFFER, QuaternionBuffer);
    glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_rotationvec, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_anglespeed, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

    glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_rotationvec, 1);
    glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_anglespeed, 1);
}

void ParticleSystemProxy::SimpleParticleVAOBind(GLuint PositionBuffer)
{
    setPositionQuadAttributes<ParticleShader::SimpleParticleRender>(quad_vertex_buffer, PositionBuffer);
}

template<typename T>
void setSimulationBind(GLuint position_vbo, GLuint initialValues_vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
    glEnableVertexAttribArray(T::attrib_position);
    glVertexAttribPointer(T::attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
    glEnableVertexAttribArray(T::attrib_lifetime);
    glVertexAttribPointer(T::attrib_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(T::attrib_velocity);
    glVertexAttribPointer(T::attrib_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
    if (T::attrib_size < 30)
    {
        glEnableVertexAttribArray(T::attrib_size);
        glVertexAttribPointer(T::attrib_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
    }

    glBindBuffer(GL_ARRAY_BUFFER, initialValues_vbo);
    glEnableVertexAttribArray(T::attrib_initial_position);
    glVertexAttribPointer(T::attrib_initial_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
    glEnableVertexAttribArray(T::attrib_initial_lifetime);
    glVertexAttribPointer(T::attrib_initial_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(T::attrib_initial_velocity);
    glVertexAttribPointer(T::attrib_initial_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
    glEnableVertexAttribArray(T::attrib_initial_size);
    glVertexAttribPointer(T::attrib_initial_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
}

void ParticleSystemProxy::SimpleSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer)
{
    setSimulationBind<ParticleShader::SimpleSimulationShader>(PositionBuffer, InitialValuesBuffer);
}

void ParticleSystemProxy::HeightmapSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer)
{
    setSimulationBind<ParticleShader::HeightmapSimulationShader>(PositionBuffer, InitialValuesBuffer);
}


void ParticleSystemProxy::setEmitter(scene::IParticleEmitter* emitter)
{
    CParticleSystemSceneNode::setEmitter(emitter);
    if (!emitter || !isGPUParticleType(emitter->getType()))
        return;
    has_height_map = false;
    flip = false;

    count = emitter->getMaxParticlesPerSecond() * emitter->getMaxLifeTime() / 1000;
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    video::ITexture *tex = getMaterial(0).getTexture(0);
    compressTexture(tex, true, true);
    texture = getTextureGLuint(getMaterial(0).getTexture(0));
}

void ParticleSystemProxy::simulateHeightmap()
{
    int timediff = int(GUIEngine::getLatestDt() * 1000.f);
    int active_count = getEmitter()->getMaxLifeTime() * getEmitter()->getMaxParticlesPerSecond() / 1000;
    core::matrix4 matrix = getAbsoluteTransformation();
    glUseProgram(ParticleShader::HeightmapSimulationShader::Program);
    glEnable(GL_RASTERIZER_DISCARD);

    glUniform1i(ParticleShader::HeightmapSimulationShader::uniform_dt, timediff);
    glUniform1i(ParticleShader::HeightmapSimulationShader::uniform_level, active_count);
    glUniformMatrix4fv(ParticleShader::HeightmapSimulationShader::uniform_sourcematrix, 1, GL_FALSE, matrix.pointer());
    glUniform1f(ParticleShader::HeightmapSimulationShader::uniform_size_increase_factor, size_increase_factor);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, heightmaptexture);
    glUniform1i(ParticleShader::HeightmapSimulationShader::uniform_heightmap, 2);
    glUniform1f(ParticleShader::HeightmapSimulationShader::uniform_track_x, track_x);
    glUniform1f(ParticleShader::HeightmapSimulationShader::uniform_track_z, track_z);
    glUniform1f(ParticleShader::HeightmapSimulationShader::uniform_track_x_len, track_x_len);
    glUniform1f(ParticleShader::HeightmapSimulationShader::uniform_track_z_len, track_z_len);

    glBindVertexArray(current_simulation_vao);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, count);
    glEndTransformFeedback();
    glBindVertexArray(0);

    glDisable(GL_RASTERIZER_DISCARD);
    std::swap(tfb_buffers[0], tfb_buffers[1]);
    std::swap(current_rendering_vao, non_current_rendering_vao);
    std::swap(current_simulation_vao, non_current_simulation_vao);
}

void ParticleSystemProxy::simulateNoHeightmap()
{
    int timediff = int(GUIEngine::getLatestDt() * 1000.f);
    int active_count = getEmitter()->getMaxLifeTime() * getEmitter()->getMaxParticlesPerSecond() / 1000;
    core::matrix4 matrix = getAbsoluteTransformation();
    glUseProgram(ParticleShader::SimpleSimulationShader::Program);
    glEnable(GL_RASTERIZER_DISCARD);

    glUniform1i(ParticleShader::SimpleSimulationShader::uniform_dt, timediff);
    glUniform1i(ParticleShader::SimpleSimulationShader::uniform_level, active_count);
    glUniformMatrix4fv(ParticleShader::SimpleSimulationShader::uniform_sourcematrix, 1, GL_FALSE, matrix.pointer());
    glUniform1f(ParticleShader::SimpleSimulationShader::uniform_size_increase_factor, size_increase_factor);

    glBindVertexArray(current_simulation_vao);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, count);
    glEndTransformFeedback();
    glBindVertexArray(0);

    glDisable(GL_RASTERIZER_DISCARD);
    std::swap(tfb_buffers[0], tfb_buffers[1]);
    std::swap(current_rendering_vao, non_current_rendering_vao);
    std::swap(current_simulation_vao, non_current_simulation_vao);
}

void ParticleSystemProxy::simulate()
{
    if (has_height_map)
        simulateHeightmap();
    else
        simulateNoHeightmap();
}

void ParticleSystemProxy::drawFlip()
{
    glBlendFunc(GL_ONE, GL_ONE);
    glUseProgram(ParticleShader::FlipParticleRender::Program);

    float screen[2] = {
        (float)UserConfigParams::m_width,
        (float)UserConfigParams::m_height
    };

    setTexture(0, texture, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);

    ParticleShader::FlipParticleRender::setUniforms(irr_driver->getViewMatrix(), irr_driver->getProjMatrix(), irr_driver->getInvProjMatrix(), screen[0], screen[1], 0, 1);

    glBindVertexArray(current_rendering_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

void ParticleSystemProxy::drawNotFlip()
{
    if (m_alpha_additive)
        glBlendFunc(GL_ONE, GL_ONE);
    else
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(ParticleShader::SimpleParticleRender::Program);

    float screen[2] = {
        (float)UserConfigParams::m_width,
        (float)UserConfigParams::m_height
    };

    setTexture(0, texture, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);

    ParticleShader::SimpleParticleRender::setUniforms(irr_driver->getViewMatrix(), irr_driver->getProjMatrix(),
        irr_driver->getInvProjMatrix(), screen[0], screen[1], 0, 1, this);

    glBindVertexArray(current_rendering_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
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
    glGenVertexArrays(1, &current_rendering_vao);
    glGenVertexArrays(1, &non_current_rendering_vao);
    glGenVertexArrays(1, &current_simulation_vao);
    glGenVertexArrays(1, &non_current_simulation_vao);

    glBindVertexArray(current_simulation_vao);
    if (has_height_map)
        HeightmapSimulationBind(tfb_buffers[0], initial_values_buffer);
    else
        SimpleSimulationBind(tfb_buffers[0], initial_values_buffer);
    glBindVertexArray(non_current_simulation_vao);
    if (has_height_map)
        HeightmapSimulationBind(tfb_buffers[1], initial_values_buffer);
    else
        SimpleSimulationBind(tfb_buffers[1], initial_values_buffer);

    float *quaternions = new float[4 * count];
    glBindVertexArray(0);
    if (flip)
    {
        for (unsigned i = 0; i < count; i++)
        {
            core::vector3df rotationdir(0., 1., 0.);

            quaternions[4 * i] = rotationdir.X;
            quaternions[4 * i + 1] = rotationdir.Y;
            quaternions[4 * i + 2] = rotationdir.Z;
            quaternions[4 * i + 3] = 3.14f * 3.f * (2.f * os::Randomizer::frand() - 1.f); // 3 half rotation during lifetime at max
        }
        glGenBuffers(1, &quaternionsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quaternionsbuffer);
        glBufferData(GL_ARRAY_BUFFER, 4 * count * sizeof(float), quaternions, GL_STATIC_DRAW);
    }

    glBindVertexArray(current_rendering_vao);
    if (flip)
        FlipParticleVAOBind(tfb_buffers[0], quaternionsbuffer);
    else
        SimpleParticleVAOBind(tfb_buffers[0]);

    glBindVertexArray(non_current_rendering_vao);
    if (flip)
        FlipParticleVAOBind(tfb_buffers[1], quaternionsbuffer);
    else
        SimpleParticleVAOBind(tfb_buffers[1]);
    glBindVertexArray(0);

    delete[] quaternions;
}

void ParticleSystemProxy::render() {
    if (!getEmitter() || !isGPUParticleType(getEmitter()->getType()))
    {
        CParticleSystemSceneNode::render();
        return;
    }
    if (!current_rendering_vao || !non_current_rendering_vao || !current_simulation_vao || !non_current_simulation_vao)
        generateVAOs();
    simulate();
    draw();
}

void ParticleSystemProxy::OnRegisterSceneNode()
{
    doParticleSystem(os::Timer::getTime());

    if (IsVisible && (Particles.size() != 0))
    {
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT_EFFECT);
        ISceneNode::OnRegisterSceneNode();
    }
}
