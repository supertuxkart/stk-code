#include "graphics/irr_driver.hpp"
#include "gpuparticles.h"
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include <ICameraSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include "guiengine/engine.hpp"

GLuint getTextureGLuint(irr::video::ITexture *tex) {
	return static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName();
}

#define COMPONENTCOUNT 8

GPUParticle::GPUParticle(scene::ISceneNode *parent, scene::ISceneManager* mgr, ITexture *tex)
    : scene::ISceneNode(parent, mgr, -1) {
	fakemat.Lighting = false;
	fakemat.ZWriteEnable = false;
	fakemat.MaterialType = irr_driver->getShader(ES_RAIN);
	fakemat.Thickness = 200;
	fakemat.setTexture(0, tex);
	fakemat.BlendOperation = video::EBO_NONE;
	setAutomaticCulling(0);
  }

void GPUParticle::render() {
	simulate();
	draw();
	// We need to force irrlicht to update its internal states
	irr::video::IVideoDriver * const drv = irr_driver->getVideoDriver();
	drv->setMaterial(fakemat);
	static_cast<irr::video::COpenGLDriver*>(drv)->setRenderStates3DMode();
}

void GPUParticle::OnRegisterSceneNode() {
	if (
		(irr_driver->getRenderPass() & irr::scene::ESNRP_TRANSPARENT) == irr::scene::ESNRP_TRANSPARENT)
	{
		SceneManager->registerNodeForRendering(this, irr::scene::ESNRP_TRANSPARENT);
	}
	ISceneNode::OnRegisterSceneNode();
}

scene::IParticleSystemSceneNode *ParticleSystemProxy::addParticleNode(
	bool withDefaultEmitter, ISceneNode* parent, s32 id,
	const core::vector3df& position,
	const core::vector3df& rotation,
	const core::vector3df& scale) {
	if (!irr_driver->isGLSL())
		return irr_driver->addParticleNode();
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
	const core::vector3df& scale) : CParticleSystemSceneNode(createDefaultEmitter, parent, mgr, id, position, rotation, scale), m_alpha_additive(false) {
	fakemat.Lighting = false;
	fakemat.ZWriteEnable = false;
	fakemat.MaterialType = irr_driver->getShader(ES_RAIN);
	fakemat.setTexture(0, getMaterial(0).getTexture(0));
	fakemat.BlendOperation = video::EBO_NONE;
	fakemat.FrontfaceCulling = false;
	fakemat.BackfaceCulling = false;
	glGenBuffers(1, &initial_values_buffer);
	glGenBuffers(2, tfb_buffers);
	glGenBuffers(1, &quaternionsbuffer);
	glGenVertexArrays(1, &current_rendering_vao);
	glGenVertexArrays(1, &non_current_rendering_vao);
	size_increase_factor = 0.;
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
	glBufferData(GL_ARRAY_BUFFER,  sizeof(quad_vertex), quad_vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ParticleSystemProxy::~ParticleSystemProxy()
{
	glDeleteBuffers(2, tfb_buffers);
	glDeleteBuffers(1, &initial_values_buffer);
	glDeleteBuffers(1, &quaternionsbuffer);
	if (has_height_map)
	{
		glDeleteBuffers(1, &heighmapbuffer);
		glDeleteTextures(1, &heightmaptexture);
	}
		
}

void ParticleSystemProxy::setAlphaAdditive(bool val) { m_alpha_additive = val; }

void ParticleSystemProxy::setIncreaseFactor(float val) { size_increase_factor = val; }

void ParticleSystemProxy::setFlip() {
	flip = true;
	float *quaternions = new float[4 * count];
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

	glGenVertexArrays(1, &current_rendering_flip_vao);
	glBindVertexArray(current_rendering_flip_vao);
	FlipParticleVAOBind(tfb_buffers[0], quaternionsbuffer);
	glGenVertexArrays(1, &non_current_rendering_flip_vao);
	glBindVertexArray(non_current_rendering_flip_vao);
	FlipParticleVAOBind(tfb_buffers[1], quaternionsbuffer);
	glBindVertexArray(0);

	delete[] quaternions;
}

void ParticleSystemProxy::setHeightmap(const std::vector<std::vector<float> > &hm,
	float f1, float f2, float f3, float f4) {
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

	glGenVertexArrays(1, &current_hm_simulation_vao);
	glBindVertexArray(current_hm_simulation_vao);
	HeightmapSimulationBind(tfb_buffers[0], initial_values_buffer);

	glGenVertexArrays(1, &non_currenthm__simulation_vao);
	glBindVertexArray(non_currenthm__simulation_vao);
	HeightmapSimulationBind(tfb_buffers[1], initial_values_buffer);

	glBindVertexArray(0);

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

	for (unsigned i = 0; i < count; i++) {
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
		memcpy(&(particles[i].DirectionX), &(initialvalue[i].DirectionZ), 4 * sizeof(float));
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

void ParticleSystemProxy::FlipParticleVAOBind(GLuint PositionBuffer, GLuint QuaternionBuffer)
{
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_pos);
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_lf);
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_quadcorner);
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_texcoord);
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_sz);

	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_rotationvec);
	glEnableVertexAttribArray(ParticleShader::FlipParticleRender::attrib_anglespeed);
	glBindBuffer(GL_ARRAY_BUFFER, QuaternionBuffer);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_rotationvec, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_anglespeed, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_quadcorner, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), 0);
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_lf, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::FlipParticleRender::attrib_sz, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(7 * sizeof(float)));

	glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_lf, 1);
	glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_pos, 1);
	glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_sz, 1);
	glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_rotationvec, 1);
	glVertexAttribDivisor(ParticleShader::FlipParticleRender::attrib_anglespeed, 1);
}

void ParticleSystemProxy::SimpleParticleVAOBind(GLuint PositionBuffer)
{
	glEnableVertexAttribArray(ParticleShader::SimpleParticleRender::attrib_pos);
	glEnableVertexAttribArray(ParticleShader::SimpleParticleRender::attrib_lf);
	glEnableVertexAttribArray(ParticleShader::SimpleParticleRender::attrib_quadcorner);
	glEnableVertexAttribArray(ParticleShader::SimpleParticleRender::attrib_texcoord);
	glEnableVertexAttribArray(ParticleShader::SimpleParticleRender::attrib_sz);

	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(ParticleShader::SimpleParticleRender::attrib_quadcorner, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(ParticleShader::SimpleParticleRender::attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
	glVertexAttribPointer(ParticleShader::SimpleParticleRender::attrib_pos, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), 0);
	glVertexAttribPointer(ParticleShader::SimpleParticleRender::attrib_lf, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::SimpleParticleRender::attrib_sz, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid *)(7 * sizeof(float)));

	glVertexAttribDivisor(ParticleShader::SimpleParticleRender::attrib_lf, 1);
	glVertexAttribDivisor(ParticleShader::SimpleParticleRender::attrib_pos, 1);
	glVertexAttribDivisor(ParticleShader::SimpleParticleRender::attrib_sz, 1);
}

void ParticleSystemProxy::SimpleSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer)
{
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_position);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_lifetime);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_velocity);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_size);
	glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_initial_position);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_initial_lifetime);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_initial_velocity);
	glEnableVertexAttribArray(ParticleShader::SimpleSimulationShader::attrib_initial_size);
	glBindBuffer(GL_ARRAY_BUFFER, InitialValuesBuffer);
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_initial_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_initial_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_initial_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::SimpleSimulationShader::attrib_initial_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
}

void ParticleSystemProxy::HeightmapSimulationBind(GLuint PositionBuffer, GLuint InitialValuesBuffer)
{
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_position);
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_lifetime);
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_velocity);
	//	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_size);
	glBindBuffer(GL_ARRAY_BUFFER, PositionBuffer);
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
	//glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_initial_position);
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_initial_lifetime);
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_initial_velocity);
	glEnableVertexAttribArray(ParticleShader::HeightmapSimulationShader::attrib_initial_size);
	glBindBuffer(GL_ARRAY_BUFFER, InitialValuesBuffer);
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_initial_position, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)0);
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_initial_lifetime, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_initial_velocity, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(4 * sizeof(float)));
	glVertexAttribPointer(ParticleShader::HeightmapSimulationShader::attrib_initial_size, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (GLvoid*)(7 * sizeof(float)));
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

	glBindVertexArray(current_rendering_vao);
	SimpleParticleVAOBind(tfb_buffers[0]);

	glBindVertexArray(non_current_rendering_vao);
	SimpleParticleVAOBind(tfb_buffers[1]);

	glGenVertexArrays(1, &current_simulation_vao);
	glBindVertexArray(current_simulation_vao);
	SimpleSimulationBind(tfb_buffers[0], initial_values_buffer);

	glGenVertexArrays(1, &non_current_simulation_vao);
	glBindVertexArray(non_current_simulation_vao);
	SimpleSimulationBind(tfb_buffers[1], initial_values_buffer);

	glBindVertexArray(0);

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

	glBindVertexArray(current_hm_simulation_vao);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, count);
	glEndTransformFeedback();
	glBindVertexArray(0);

	glDisable(GL_RASTERIZER_DISCARD);
	std::swap(tfb_buffers[0], tfb_buffers[1]);
	std::swap(current_rendering_flip_vao, non_current_rendering_flip_vao);
	std::swap(current_hm_simulation_vao, non_currenthm__simulation_vao);
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glUseProgram(ParticleShader::FlipParticleRender::Program);

	float screen[2] = {
		(float)UserConfigParams::m_width,
		(float)UserConfigParams::m_height
	};

	setTexture(0, texture, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	setTexture(1, static_cast<video::COpenGLFBOTexture *>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->DepthBufferTexture, GL_NEAREST, GL_NEAREST);

	ParticleShader::FlipParticleRender::setUniforms(irr_driver->getViewMatrix(), irr_driver->getProjMatrix(), irr_driver->getInvProjMatrix(), screen[0], screen[1], 0, 1);

	glBindVertexArray(current_rendering_flip_vao);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

void ParticleSystemProxy::drawNotFlip()
{
	if (m_alpha_additive)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(ParticleShader::SimpleParticleRender::Program);

	float screen[2] = {
		(float)UserConfigParams::m_width,
		(float)UserConfigParams::m_height
	};

	setTexture(0, texture, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	setTexture(1, static_cast<video::COpenGLFBOTexture *>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->DepthBufferTexture, GL_NEAREST, GL_NEAREST);

	ParticleShader::SimpleParticleRender::setUniforms(irr_driver->getViewMatrix(), irr_driver->getProjMatrix(), irr_driver->getInvProjMatrix(), screen[0], screen[1], 0, 1);

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

void ParticleSystemProxy::render() {
	if (!getEmitter() || !isGPUParticleType(getEmitter()->getType()))
	{
		CParticleSystemSceneNode::render();
		return;
	}
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

RainNode::RainNode(scene::ISceneManager* mgr, ITexture *tex)
    : GPUParticle(0, mgr, tex)
{
	RenderProgram = LoadProgram(file_manager->getAsset("shaders/rain.vert").c_str(), file_manager->getAsset("shaders/rain.frag").c_str());
	loc_screenw = glGetUniformLocation(RenderProgram, "screenw");
	loc_screen = glGetUniformLocation(RenderProgram, "screen");
	loc_invproj = glGetUniformLocation(RenderProgram, "invproj");
	texloc_tex = glGetUniformLocation(RenderProgram, "tex");
	texloc_normal_and_depths = glGetUniformLocation(RenderProgram, "normals_and_depth");

	const char *varyings[] = { "currentPosition" };
	SimulationProgram = LoadTFBProgram(file_manager->getAsset("shaders/rainsim.vert").c_str(), varyings, 1);
	loc_campos = glGetUniformLocation(SimulationProgram, "campos");
	loc_viewm = glGetUniformLocation(SimulationProgram, "viewm");
	loc_time = glGetUniformLocation(SimulationProgram, "time");
	count = 2500;
	area = 3500;

	u32 i;
	float x, y, z, vertices[7500];
	for (i = 0; i < count; i++)
	{
		x = ((rand() % area) - area / 2) / 100.0f;
		y = ((rand() % 2400)) / 100.0f;
		z = ((rand() % area) - area / 2) / 100.0f;

		vertices[3 * i] = x;
		vertices[3 * i + 1] = y;
		vertices[3 * i + 2] = z;
	}
	
	texture = getTextureGLuint(tex);
	normal_and_depth = getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));
	glGenBuffers(2, tfb_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), vertices, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), 0, GL_STREAM_DRAW);

	box.addInternalPoint(vector3df((float)(-area / 2)));
	box.addInternalPoint(vector3df((float)(area / 2)));
}

void RainNode::simulate() {
	glUseProgram(SimulationProgram);
	const float time = irr_driver->getDevice()->getTimer()->getTime() / 90.0f;
	const irr::core::matrix4 viewm = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_VIEW);
	const irr::core::vector3df campos = irr_driver->getSceneManager()->getActiveCamera()->getPosition();

	glEnable(GL_RASTERIZER_DISCARD);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_vertex_buffer[1]);

	glUniformMatrix4fv(loc_viewm, 1, GL_FALSE, viewm.pointer());
	glUniform1f(loc_time, time);
	glUniform3f(loc_campos, campos.X, campos.Y, campos.Z);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, count);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);
}

void RainNode::draw() {
	const float screenw = (float)UserConfigParams::m_width;

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	glUseProgram(RenderProgram);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	float screen[2] = {
		(float)UserConfigParams::m_width,
		(float)UserConfigParams::m_height
	};
	irr::core::matrix4 invproj = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_PROJECTION);
	invproj.makeInverse();

	bindUniformToTextureUnit(texloc_tex, texture, 0);
	bindUniformToTextureUnit(texloc_normal_and_depths, normal_and_depth, 1);

	glUniformMatrix4fv(loc_invproj, 1, GL_FALSE, invproj.pointer());
	glUniform2f(loc_screen, screen[0], screen[1]);
	glUniform1f(loc_screenw, screenw);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

const core::aabbox3d<f32>& RainNode::getBoundingBox() const
{
	return box;
}
