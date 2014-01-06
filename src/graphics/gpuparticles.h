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
	GLuint tfb_buffers[2], initial_values_buffer;
	bool m_alpha_additive;

	static GLuint SimulationProgram;
	static GLuint attrib_position, attrib_velocity, attrib_lifetime, attrib_initial_position, attrib_initial_velocity, attrib_initial_lifetime, attrib_size, attrib_initial_size;
	static GLuint uniform_sourcematrix, uniform_tinvsourcematrix, uniform_dt;

	static GLuint RenderProgram;
	static GLuint attrib_pos, attrib_lf, attrib_quadcorner, attrib_texcoord, attrib_sz;
	static GLuint uniform_matrix, uniform_viewmatrix, uniform_texture, uniform_normal_and_depths, uniform_screen, uniform_invproj;

	static GLuint quad_vertex_buffer;

	GLuint texture, normal_and_depth;
	unsigned duration, count, LastEmitTime;

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
	void setAlphaAdditive(bool);
};

class PointEmitter : public GPUParticle
{
protected:
  GLuint SimulationProgram, RenderProgram;
  GLuint loc_duration, loc_sourcematrix, loc_dt, loc_matrix, loc_texture, loc_normal_and_depths, loc_screen, loc_invproj;
  GLuint loc_position, loc_velocity, loc_lifetime;
  GLuint tfb_buffers[2];
  GLuint texture, normal_and_depth;
  unsigned duration, count;
  core::vector3df direction;
  core::aabbox3d<f32> box;
  scene::IParticleSystemSceneNode *m_node;

  virtual void simulate();
  virtual void draw();
public:
  PointEmitter(scene::ISceneNode *parent,
    scene::ISceneManager* mgr, video::ITexture *tex,
    const core::vector3df& dir,
    u32 minParticlesPerSecond,
    u32 maxParticlesPerSecond,
    const video::SColor& minStartColor,
    const video::SColor& maxStartColor,
    u32 lifeTimeMin, u32 lifeTimeMax,
    s32 maxAngleDegrees
//    const core::dimension2df& minStartSize,
//    const core::dimension2df& maxStartSize
  );
  void set_m_node(scene::IParticleSystemSceneNode *nd) { m_node = nd; }
  virtual const core::aabbox3d<f32>& getBoundingBox() const { return box; }
  virtual u32 getMaterialCount() const { return 1; }
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
