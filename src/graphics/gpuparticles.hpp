#ifndef GPUPARTICLES_H
#define GPUPARTICLES_H

#include "../lib/irrlicht/source/Irrlicht/CParticleSystemSceneNode.h"
#include <ISceneManager.h>
#include <IParticleSystemSceneNode.h>

namespace irr { namespace video{ class ITexture; } }

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
    unsigned count;
    static void CommonRenderingVAO(GLuint PositionBuffer);
    static void AppendQuaternionRenderingVAO(GLuint QuaternionBuffer);
    static void CommonSimulationVAO(GLuint position_vbo, GLuint initialValues_vbo);

    void generateVAOs();
    void cleanGL();

    void drawFlip();
    void drawNotFlip();
    virtual void simulate();
    virtual void draw();

public:
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
