#ifndef GPUPARTICLES_H
#define GPUPARTICLES_H

#ifndef _IRR_WINDOWS_API_
#define GL_GLEXT_PROTOTYPES 1
#endif
#include "graphics/glwrap.hpp"
#include "../lib/irrlicht/source/Irrlicht/CParticleSystemSceneNode.h"
#include <ISceneManager.h>
#include <IParticleSystemSceneNode.h>

void initGL();
GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path);
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);
GLuint getTextureGLuint(irr::video::ITexture *tex);
void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit);


class GPUParticle : public scene::ISceneNode {
protected:
	video::SMaterial fakemat;
	virtual void simulate() = 0;
	virtual void draw() = 0;
public:
	GPUParticle(scene::ISceneNode *parent, scene::ISceneManager* mgr, ITexture *tex);
	virtual void render();
	virtual void OnRegisterSceneNode();
};

class ParticleSystemProxy : public scene::CParticleSystemSceneNode {
protected:
	GLuint SimulationProgram, RenderProgram;
	GLuint loc_duration, loc_sourcematrix, loc_dt, loc_matrix, loc_texture, loc_normal_and_depths, loc_screen, loc_invproj;
	GLuint loc_position, loc_velocity, loc_lifetime;
	GLuint tfb_buffers[2];
	GLuint texture, normal_and_depth;
	unsigned duration, count;

	virtual void simulate();
	virtual void draw();
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

	virtual void setEmitter(scene::IParticleEmitter* emitter);
	virtual void render();
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
    scene::ISceneManager* mgr, ITexture *tex,
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
	RainNode(scene::ISceneManager* mgr, ITexture *tex);
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount() const { return 1; }
};

#endif // GPUPARTICLES_H
