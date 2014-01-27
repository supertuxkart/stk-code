#ifndef GPUPARTICLES_H
#define GPUPARTICLES_H

#include "graphics/glwrap.hpp"

#include "../lib/irrlicht/source/Irrlicht/CParticleSystemSceneNode.h"
#include <ISceneManager.h>
#include <IParticleSystemSceneNode.h>

namespace irr { namespace video{ class ITexture; } }

GLuint getTextureGLuint(irr::video::ITexture *tex);

class GPUParticle : public scene::ISceneNode
{
protected:
	video::SMaterial fakemat;
	virtual void simulate() = 0;
	virtual void draw() = 0;
public:
	GPUParticle(scene::ISceneNode *parent, scene::ISceneManager* mgr, 
                video::ITexture *tex);
	virtual void render();
	virtual void OnRegisterSceneNode();
};

class ParticleSystemProxy : public scene::CParticleSystemSceneNode {
protected:
	video::SMaterial fakemat;
	GLuint tfb_buffers[2], initial_values_buffer, heighmapbuffer, heightmaptexture, quaternionsbuffer;
	GLuint current_simulation_vao, non_current_simulation_vao;
	GLuint current_hm_simulation_vao, non_currenthm__simulation_vao;
	GLuint current_rendering_vao, non_current_rendering_vao;
	GLuint current_rendering_flip_vao, non_current_rendering_flip_vao;
	bool m_alpha_additive, has_height_map, flip;
	float size_increase_factor, track_x, track_z, track_x_len, track_z_len;

	static GLuint quad_vertex_buffer;

	GLuint texture;
	unsigned count;
	static void SimpleParticleVAOBind(GLuint PositionBuffer);
	static void FlipParticleVAOBind(GLuint PositionBuffer, GLuint QuaternionBuffer);
	static void SimpleSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer);
	static void HeightmapSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer);

	void simulateHeightmap();
	void simulateNoHeightmap();
	void drawFlip();
	void drawNotFlip();
	virtual void simulate();
	virtual void draw();
	void generateParticlesFromPointEmitter(scene::IParticlePointEmitter *);
	void generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter *);
	void generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter *);
public:
	static IParticleSystemSceneNode *addParticleNode(
		bool withDefaultEmitter = true, ISceneNode* parent = 0, s32 id = -1,
		const core::vector3df& position = core::vector3df(0, 0, 0),
		const core::vector3df& rotation = core::vector3df(0, 0, 0),
		const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));

	ParticleSystemProxy(bool createDefaultEmitter,
		ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
		const core::vector3df& position,
		const core::vector3df& rotation,
		const core::vector3df& scale);
	~ParticleSystemProxy();

	virtual void setEmitter(scene::IParticleEmitter* emitter);
	virtual void render();
	virtual void OnRegisterSceneNode();
	void setAlphaAdditive(bool);
	void setIncreaseFactor(float);
	void setHeightmap(const std::vector<std::vector<float> >&, float, float, float, float);
	void setFlip();
};

class RainNode : public GPUParticle
{
protected:
	GLuint SimulationProgram, RenderProgram, tfb_vertex_buffer[2];
	unsigned count;
	GLuint texture, normal_and_depth;
	GLuint loc_campos, loc_viewm, loc_time;
	GLuint loc_screenw, loc_screen, loc_invproj, texloc_tex, texloc_normal_and_depths;
	s32 area;
	core::aabbox3d<f32> box;

	virtual void simulate();
	virtual void draw();
public:
	RainNode(scene::ISceneManager* mgr, video::ITexture *tex);
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount() const { return 1; }
};

#endif // GPUPARTICLES_H
