#include "graphics/irr_driver.hpp"
#include "gpuparticles.h"
#include <fstream>
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include <ICameraSceneNode.h>
#include <IParticleSystemSceneNode.h>

#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(X) wglGetProcAddress(reinterpret_cast<const char*>(X))


PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
#endif

static bool is_gl_init = false;
void initGL()
{
	if (is_gl_init)
		return;
	is_gl_init = true;
#ifdef _IRR_WINDOWS_API_
    glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)IRR_OGL_LOAD_EXTENSION("glGenTransformFeedbacks");
    glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBindTransformFeedback");
    glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glDrawTransformFeedback");
    glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBeginTransformFeedback");
    glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glEndTransformFeedback");
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)IRR_OGL_LOAD_EXTENSION("glBindBufferBase");
    glGenBuffers = (PFNGLGENBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)IRR_OGL_LOAD_EXTENSION("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribPointer");
    glCreateShader = (PFNGLCREATESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCreateShader");
    glCompileShader = (PFNGLCOMPILESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCompileShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)IRR_OGL_LOAD_EXTENSION("glShaderSource");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)IRR_OGL_LOAD_EXTENSION("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glUseProgram");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glEnableVertexAttribArray");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)IRR_OGL_LOAD_EXTENSION("glUniformMatrix4fv");
    glUniform1f = (PFNGLUNIFORM1FPROC)IRR_OGL_LOAD_EXTENSION("glUniform1f");
    glUniform3f = (PFNGLUNIFORM3FPROC)IRR_OGL_LOAD_EXTENSION("glUniform3f");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glDisableVertexAttribArray");
    glDeleteShader = (PFNGLDELETESHADERPROC)IRR_OGL_LOAD_EXTENSION("glDeleteShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderInfoLog");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
    glUniform2f = (PFNGLUNIFORM2FPROC)IRR_OGL_LOAD_EXTENSION("glUniform2f");
    glUniform1i = (PFNGLUNIFORM1IPROC)IRR_OGL_LOAD_EXTENSION("glUniform1i");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramInfoLog");
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)IRR_OGL_LOAD_EXTENSION("glTransformFeedbackVaryings");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetAttribLocation");
	glBlendEquation = (PFNGLBLENDEQUATIONPROC)IRR_OGL_LOAD_EXTENSION("glBlendEquation");
	glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribDivisor");
	glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)IRR_OGL_LOAD_EXTENSION("glDrawArraysInstanced");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteBuffers");
#endif
}

// Mostly from shader tutorial
static
GLuint LoadShader(const char * file, unsigned type) {
    GLuint Id = glCreateShader(type);
    std::string Code;
    std::ifstream Stream(file, std::ios::in);
    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            Code += "\n" + Line;
        Stream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    printf("Compiling shader : %s\n", file);
    char const * SourcePointer = Code.c_str();
    int length = strlen(SourcePointer);
    glShaderSource(Id, 1, &SourcePointer, &length);
    glCompileShader(Id);

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
    if (Result == GL_FALSE) {
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }

    return Id;
}

GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path) {
    GLuint VertexShaderID = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint FragmentShaderID = LoadShader(fragment_file_path, GL_FRAGMENT_SHADER);

    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) {
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount) {
    GLuint Shader = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint Program = glCreateProgram();
    glAttachShader(Program, Shader);
    glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(Program);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(Program, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) {
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }
    glDeleteShader(Shader);
    return Program;
}

GLuint getTextureGLuint(irr::video::ITexture *tex) {
    return static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName();
}

void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texid);
    glUniform1i(location, textureUnit);
}

#define COMPONENTCOUNT 8

GPUParticle::GPUParticle(scene::ISceneNode *parent, scene::ISceneManager* mgr, ITexture *tex)
    : scene::ISceneNode(parent, mgr, -1) {
    initGL();
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
	initGL();
	fakemat.Lighting = false;
	fakemat.ZWriteEnable = false;
	fakemat.MaterialType = irr_driver->getShader(ES_RAIN);
	fakemat.setTexture(0, getMaterial(0).getTexture(0));
	fakemat.BlendOperation = video::EBO_NONE;
	fakemat.FrontfaceCulling = false;
	fakemat.BackfaceCulling = false;
	glGenBuffers(1, &initial_values_buffer);
	glGenBuffers(2, tfb_buffers);
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
}

void ParticleSystemProxy::setAlphaAdditive(bool val) { m_alpha_additive = val;  }

void ParticleSystemProxy::generateParticlesFromPointEmitter(scene::IParticlePointEmitter *emitter)
{
	float *particles = new float[COMPONENTCOUNT * count], *initialvalue = new float[COMPONENTCOUNT * count];
	unsigned lifetime_range = emitter->getMaxLifeTime() - emitter->getMinLifeTime();

	float sizeMin = emitter->getMinStartSize().Height;
	float sizeMax = emitter->getMaxStartSize().Height;

	for (unsigned i = 0; i < count; i++) {
		particles[COMPONENTCOUNT * i] = 0;
		particles[COMPONENTCOUNT * i + 1] = 0;
		particles[COMPONENTCOUNT * i + 2] = 0;
		// Initial lifetime is >1
		particles[COMPONENTCOUNT * i + 3] = 2.;

		initialvalue[COMPONENTCOUNT * i] = 0.;
		initialvalue[COMPONENTCOUNT * i + 1] = 0.;
		initialvalue[COMPONENTCOUNT * i + 2] = 0.;
		initialvalue[COMPONENTCOUNT * i + 3] = rand() % lifetime_range;
		initialvalue[COMPONENTCOUNT * i + 3] += emitter->getMinLifeTime();

		core::vector3df particledir = emitter->getDirection();
		particledir.rotateXYBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateYZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateXZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());

		float size = rand();
		size /= RAND_MAX;
		size *= (sizeMax - sizeMin);
		size += sizeMin;

		initialvalue[COMPONENTCOUNT * i + 7] = size;

		particles[COMPONENTCOUNT * i + 4] = particledir.X / size;
		particles[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		particles[COMPONENTCOUNT * i + 6] = particledir.Z / size;
		initialvalue[COMPONENTCOUNT * i + 4] = particledir.X / size;
		initialvalue[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		initialvalue[COMPONENTCOUNT * i + 6] = particledir.Z / size;

	}

	glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), initialvalue, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), particles, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), 0, GL_STREAM_DRAW);
	delete[] particles;
	delete[] initialvalue;
}

void ParticleSystemProxy::generateParticlesFromBoxEmitter(scene::IParticleBoxEmitter *emitter)
{
	float *particles = new float[COMPONENTCOUNT * count], *initialvalue = new float[COMPONENTCOUNT * count];
	unsigned lifetime_range = emitter->getMaxLifeTime() - emitter->getMinLifeTime();

	float sizeMin = emitter->getMinStartSize().Height;
	float sizeMax = emitter->getMaxStartSize().Height;

	const core::vector3df& extent = emitter->getBox().getExtent();

	for (unsigned i = 0; i < count; i++) {
		particles[COMPONENTCOUNT * i] = emitter->getBox().MinEdge.X + os::Randomizer::frand() * extent.X;
		particles[COMPONENTCOUNT * i + 1] = emitter->getBox().MinEdge.Y + os::Randomizer::frand() * extent.Y;
		particles[COMPONENTCOUNT * i + 2] = emitter->getBox().MinEdge.Z + os::Randomizer::frand() * extent.Z;
		// Initial lifetime is > 1
		particles[COMPONENTCOUNT * i + 3] = os::Randomizer::frand();

		initialvalue[COMPONENTCOUNT * i] = particles[COMPONENTCOUNT * i];
		initialvalue[COMPONENTCOUNT * i + 1] = particles[COMPONENTCOUNT * i + 1];
		initialvalue[COMPONENTCOUNT * i + 2] = particles[COMPONENTCOUNT * i + 2];
		initialvalue[COMPONENTCOUNT * i + 3] = (lifetime_range > 0) ? rand() % lifetime_range : 0;
		initialvalue[COMPONENTCOUNT * i + 3] += emitter->getMinLifeTime();

		core::vector3df particledir = emitter->getDirection();
		particledir.rotateXYBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateYZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateXZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());

		float size = rand();
		size /= RAND_MAX;
		size *= (sizeMax - sizeMin);
		size += sizeMin;

		initialvalue[COMPONENTCOUNT * i + 7] = size;

		particles[COMPONENTCOUNT * i + 4] = particledir.X / size;
		particles[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		particles[COMPONENTCOUNT * i + 6] = particledir.Z / size;
		initialvalue[COMPONENTCOUNT * i + 4] = particledir.X / size;
		initialvalue[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		initialvalue[COMPONENTCOUNT * i + 6] = particledir.Z / size;
	}
	glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), initialvalue, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), particles, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), 0, GL_STREAM_DRAW);
	delete[] particles;
	delete[] initialvalue;
}

void ParticleSystemProxy::generateParticlesFromSphereEmitter(scene::IParticleSphereEmitter *emitter)
{
	float *particles = new float[COMPONENTCOUNT * count], *initialvalue = new float[COMPONENTCOUNT * count];
	unsigned lifetime_range = emitter->getMaxLifeTime() - emitter->getMinLifeTime();

	float sizeMin = emitter->getMinStartSize().Height;
	float sizeMax = emitter->getMaxStartSize().Height;

	for (unsigned i = 0; i < count; i++) {
		// Random distance from center
		const f32 distance = os::Randomizer::frand() * emitter->getRadius();

		// Random direction from center
		vector3df pos = emitter->getCenter() + distance;
		pos.rotateXYBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
		pos.rotateYZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());
		pos.rotateXZBy(os::Randomizer::frand() * 360.f, emitter->getCenter());

		particles[COMPONENTCOUNT * i] = pos.X;
		particles[COMPONENTCOUNT * i + 1] = pos.Y;
		particles[COMPONENTCOUNT * i + 2] = pos.Z;
		// Initial lifetime is > 1
		particles[COMPONENTCOUNT * i + 3] = 2.;

		initialvalue[COMPONENTCOUNT * i] = particles[COMPONENTCOUNT * i];
		initialvalue[COMPONENTCOUNT * i + 1] = particles[COMPONENTCOUNT * i + 1];
		initialvalue[COMPONENTCOUNT * i + 2] = particles[COMPONENTCOUNT * i + 2];
		initialvalue[COMPONENTCOUNT * i + 3] = (lifetime_range > 0) ? rand() % lifetime_range : 0;
		initialvalue[COMPONENTCOUNT * i + 3] += emitter->getMinLifeTime();

		core::vector3df particledir = emitter->getDirection();
		particledir.rotateXYBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateYZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());
		particledir.rotateXZBy(os::Randomizer::frand() * emitter->getMaxAngleDegrees());

		float size = rand();
		size /= RAND_MAX;
		size *= (sizeMax - sizeMin);
		size += sizeMin;

		initialvalue[COMPONENTCOUNT * i + 7] = size;

		particles[COMPONENTCOUNT * i + 4] = particledir.X / size;
		particles[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		particles[COMPONENTCOUNT * i + 6] = particledir.Z / size;
		initialvalue[COMPONENTCOUNT * i + 4] = particledir.X / size;
		initialvalue[COMPONENTCOUNT * i + 5] = particledir.Y / size;
		initialvalue[COMPONENTCOUNT * i + 6] = particledir.Z / size;
	}
	glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), initialvalue, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), particles, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), 0, GL_STREAM_DRAW);
	delete[] particles;
	delete[] initialvalue;
}

GLuint ParticleSystemProxy::SimulationProgram = 0;
GLuint ParticleSystemProxy::RenderProgram = 0;

GLuint ParticleSystemProxy::attrib_position;
GLuint ParticleSystemProxy::attrib_velocity;
GLuint ParticleSystemProxy::attrib_lifetime;
GLuint ParticleSystemProxy::attrib_initial_position;
GLuint ParticleSystemProxy::attrib_initial_velocity;
GLuint ParticleSystemProxy::attrib_initial_lifetime;
GLuint ParticleSystemProxy::attrib_size;
GLuint ParticleSystemProxy::attrib_initial_size;
GLuint ParticleSystemProxy::uniform_sourcematrix;
GLuint ParticleSystemProxy::uniform_tinvsourcematrix;
GLuint ParticleSystemProxy::uniform_dt;


GLuint ParticleSystemProxy::attrib_pos;
GLuint ParticleSystemProxy::attrib_lf;
GLuint ParticleSystemProxy::attrib_quadcorner;
GLuint ParticleSystemProxy::attrib_texcoord;
GLuint ParticleSystemProxy::attrib_sz;
GLuint ParticleSystemProxy::uniform_matrix;
GLuint ParticleSystemProxy::uniform_viewmatrix;
GLuint ParticleSystemProxy::uniform_texture;
GLuint ParticleSystemProxy::uniform_normal_and_depths;
GLuint ParticleSystemProxy::uniform_screen;
GLuint ParticleSystemProxy::uniform_invproj;

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
	// Pass a fake material type to force irrlicht to update its internal states on rendering
	setMaterialType(irr_driver->getShader(ES_RAIN));
	setAutomaticCulling(0);
	LastEmitTime = 0;

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

	texture = getTextureGLuint(getMaterial(0).getTexture(0));
	normal_and_depth = getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));

	if (SimulationProgram && RenderProgram)
		return;

	const char *varyings[] = {
		"new_particle_position",
		"new_lifetime",
		"new_particle_velocity",
		"new_size",
	};

	SimulationProgram = LoadTFBProgram(file_manager->getAsset("shaders/pointemitter.vert").c_str(), varyings, 4);

	uniform_dt = glGetUniformLocation(SimulationProgram, "dt");
	uniform_sourcematrix = glGetUniformLocation(SimulationProgram, "sourcematrix");
	uniform_tinvsourcematrix = glGetUniformLocation(SimulationProgram, "tinvsourcematrix");

	attrib_position = glGetAttribLocation(SimulationProgram, "particle_position");
	attrib_lifetime = glGetAttribLocation(SimulationProgram, "lifetime");
	attrib_velocity = glGetAttribLocation(SimulationProgram, "particle_velocity");
	attrib_size = glGetAttribLocation(SimulationProgram, "size");
	attrib_initial_position = glGetAttribLocation(SimulationProgram, "particle_position_initial");
	attrib_initial_lifetime = glGetAttribLocation(SimulationProgram, "lifetime_initial");
	attrib_initial_velocity = glGetAttribLocation(SimulationProgram, "particle_velocity_initial");
	attrib_initial_size = glGetAttribLocation(SimulationProgram, "size_initial");

	RenderProgram = LoadProgram(file_manager->getAsset("shaders/particle.vert").c_str(), file_manager->getAsset("shaders/particle.frag").c_str());
	attrib_pos = glGetAttribLocation(RenderProgram, "position");
	attrib_sz = glGetAttribLocation(RenderProgram, "size");
	attrib_lf = glGetAttribLocation(RenderProgram, "lifetime");
	attrib_quadcorner = glGetAttribLocation(RenderProgram, "quadcorner");
	attrib_texcoord = glGetAttribLocation(RenderProgram, "texcoord");

	uniform_matrix = glGetUniformLocation(RenderProgram, "ProjectionMatrix");
	uniform_viewmatrix = glGetUniformLocation(RenderProgram, "ViewMatrix");
	uniform_texture = glGetUniformLocation(RenderProgram, "texture");
	uniform_invproj = glGetUniformLocation(RenderProgram, "invproj");
	uniform_screen = glGetUniformLocation(RenderProgram, "screen");
	uniform_normal_and_depths = glGetUniformLocation(RenderProgram, "normals_and_depth");
}

void ParticleSystemProxy::simulate()
{
	unsigned time = os::Timer::getTime();
	if (LastEmitTime == 0)
	{
		LastEmitTime = time;
		return;
	}

	u32 now = time;
	u32 timediff = time - LastEmitTime;
	LastEmitTime = time;

	core::matrix4 matrix = getAbsoluteTransformation();
	core::matrix4 tinvmatrix;
	matrix.getInverse(tinvmatrix);
	tinvmatrix = tinvmatrix.getTransposed();
	glUseProgram(SimulationProgram);
	glEnable(GL_RASTERIZER_DISCARD);
	glEnableVertexAttribArray(attrib_position);
	glEnableVertexAttribArray(attrib_lifetime);
	glEnableVertexAttribArray(attrib_velocity);
	glEnableVertexAttribArray(attrib_size);
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
	glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)0);
	glVertexAttribPointer(attrib_lifetime, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(attrib_velocity, 4, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(4 * sizeof(float)));
	glVertexAttribPointer(attrib_size, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(7 * sizeof(float)));
	glEnableVertexAttribArray(attrib_initial_position);
	glEnableVertexAttribArray(attrib_initial_lifetime);
	glEnableVertexAttribArray(attrib_initial_velocity);
	glEnableVertexAttribArray(attrib_initial_size);
	glBindBuffer(GL_ARRAY_BUFFER, initial_values_buffer);
	glVertexAttribPointer(attrib_initial_position, 3, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)0);
	glVertexAttribPointer(attrib_initial_lifetime, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glVertexAttribPointer(attrib_initial_velocity, 4, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(4 * sizeof(float)));
	glVertexAttribPointer(attrib_initial_size, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(7 * sizeof(float)));
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

	glUniform1i(uniform_dt, timediff);
	glUniformMatrix4fv(uniform_sourcematrix, 1, GL_FALSE, matrix.pointer());
	glUniformMatrix4fv(uniform_tinvsourcematrix, 1, GL_FALSE, tinvmatrix.pointer());
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, count);
	glEndTransformFeedback();
	glDisableVertexAttribArray(attrib_position);
	glDisableVertexAttribArray(attrib_lifetime);
	glDisableVertexAttribArray(attrib_velocity);
	glDisableVertexAttribArray(attrib_size);
	glDisableVertexAttribArray(attrib_initial_position);
	glDisableVertexAttribArray(attrib_initial_lifetime);
	glDisableVertexAttribArray(attrib_initial_velocity);
	glDisableVertexAttribArray(attrib_initial_size);
	glDisable(GL_RASTERIZER_DISCARD);
	std::swap(tfb_buffers[0], tfb_buffers[1]);
}

void ParticleSystemProxy::draw()
{
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	core::matrix4 projm = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION);
	core::matrix4 viewm = irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
	if (m_alpha_additive)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(RenderProgram);
	glEnableVertexAttribArray(attrib_pos);
	glEnableVertexAttribArray(attrib_lf);
	glEnableVertexAttribArray(attrib_quadcorner);
	glEnableVertexAttribArray(attrib_texcoord);
	glEnableVertexAttribArray(attrib_sz);
	
	float screen[2] = {
		(float)UserConfigParams::m_width,
		(float)UserConfigParams::m_height
	};
	irr::core::matrix4 invproj = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_PROJECTION);
	invproj.makeInverse();

	bindUniformToTextureUnit(uniform_texture, texture, 0);
	bindUniformToTextureUnit(uniform_normal_and_depths, normal_and_depth, 1);

	glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, invproj.pointer());
	glUniform2f(uniform_screen, screen[0], screen[1]);
	glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, projm.pointer());
	glUniformMatrix4fv(uniform_viewmatrix, 1, GL_FALSE, viewm.pointer());

	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glVertexAttribPointer(attrib_quadcorner, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), 0);
	glVertexAttribPointer(attrib_lf, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid *) (3 * sizeof(float)));
	glVertexAttribPointer(attrib_sz, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid *)(7 * sizeof(float)));

	glVertexAttribDivisor(attrib_lf, 1);
	glVertexAttribDivisor(attrib_pos, 1);
	glVertexAttribDivisor(attrib_sz, 1);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
	glVertexAttribDivisor(attrib_lf, 0);
	glVertexAttribDivisor(attrib_pos, 0);
	glVertexAttribDivisor(attrib_sz, 0);
	glDisableVertexAttribArray(attrib_pos);
	glDisableVertexAttribArray(attrib_lf);
	glDisableVertexAttribArray(attrib_quadcorner);
	glDisableVertexAttribArray(attrib_texcoord);
	glDisableVertexAttribArray(attrib_sz);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_BLEND);
}

void ParticleSystemProxy::render() {
	if (!getEmitter() || !isGPUParticleType(getEmitter()->getType()))
	{
		CParticleSystemSceneNode::render();
		return;
	}
	simulate();
	draw();
	// We need to force irrlicht to update its internal states
	irr::video::IVideoDriver * const drv = irr_driver->getVideoDriver();
	drv->setMaterial(fakemat);
	static_cast<irr::video::COpenGLDriver*>(drv)->setRenderStates3DMode();
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
