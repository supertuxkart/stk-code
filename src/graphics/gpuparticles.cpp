#include "graphics/irr_driver.hpp"
#include "gpuparticles.h"
#include <fstream>
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include <ICameraSceneNode.h>

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
#endif

void initGL()
{
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

GPUParticle::GPUParticle(scene::ISceneManager* mgr, ITexture *tex)
    : scene::ISceneNode(mgr->getRootSceneNode(), mgr, -1) {
    initGL();
	fakemat.Lighting = false;
	fakemat.ZWriteEnable = false;
	fakemat.MaterialType = irr_driver->getShader(ES_RAIN);
	fakemat.Thickness = 200;
	fakemat.setTexture(0, tex);
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

#define COMPONENTCOUNT 8

PointEmitter::PointEmitter(scene::ISceneManager* mgr, ITexture *tex,
  const core::vector3df& direction,
  u32 minParticlesPerSecond,
  u32 maxParticlesPerSecond,
  const video::SColor& minStartColor,
  const video::SColor& maxStartColor,
  u32 lifeTimeMin, u32 lifeTimeMax,
  s32 maxAngleDegrees
//  const core::dimension2df& minStartSize,
//  const core::dimension2df& maxStartSize
) : GPUParticle(mgr, tex) {
	count = maxParticlesPerSecond;
	duration = lifeTimeMax;
	float initial_lifetime_incr = 1000.;
	initial_lifetime_incr /= maxParticlesPerSecond;
  const char *varyings[] = {
    "new_particle_position",
	"new_lifetime",
    "new_particle_velocity",
  };
  texture = getTextureGLuint(tex);
  normal_and_depth = getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));

  SimulationProgram = LoadTFBProgram(file_manager->getAsset("shaders/pointemitter.vert").c_str(), varyings, 3);
  loc_duration = glGetUniformLocation(SimulationProgram, "duration");
  loc_dt = glGetUniformLocation(SimulationProgram, "dt");
  loc_source = glGetUniformLocation(SimulationProgram, "source");
  loc_position = glGetAttribLocation(SimulationProgram, "particle_position");
  loc_lifetime = glGetAttribLocation(SimulationProgram, "lifetime");
  loc_velocity = glGetAttribLocation(SimulationProgram, "particle_velocity");
  printf("count:%d\nduration:%d\ninitial_lifetine:%f\n", count, duration, initial_lifetime_incr);

  RenderProgram = LoadProgram(file_manager->getAsset("shaders/particle.vert").c_str(), file_manager->getAsset("shaders/particle.frag").c_str());
  loc_matrix = glGetUniformLocation(RenderProgram, "matrix");
  loc_texture = glGetUniformLocation(RenderProgram, "texture");
  loc_invproj = glGetUniformLocation(RenderProgram, "invproj");
  loc_screen = glGetUniformLocation(RenderProgram, "screen");
  loc_normal_and_depths = glGetUniformLocation(RenderProgram, "normals_and_depth");

  float *particles = new float[COMPONENTCOUNT * count];
  for (unsigned i = 0; i < count; i++) {
	  particles[COMPONENTCOUNT * i] = 0.;
	  particles[COMPONENTCOUNT * i + 1] = 0.;
	  particles[COMPONENTCOUNT * i + 2] = 0.;
	  particles[COMPONENTCOUNT * i + 3] = rand() % duration;
	  core::vector3df particledir = direction;
	  particledir.rotateXYBy(os::Randomizer::frand() * maxAngleDegrees);
	  particledir.rotateYZBy(os::Randomizer::frand() * maxAngleDegrees);
	  particledir.rotateXZBy(os::Randomizer::frand() * maxAngleDegrees);

	  particles[COMPONENTCOUNT * i + 4] = particledir.X;
	  particles[COMPONENTCOUNT * i + 5] = particledir.Y;
	  particles[COMPONENTCOUNT * i + 6] = particledir.Z;
  }
  glGenBuffers(2, tfb_buffers);
  glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
  glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), particles, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[1]);
  glBufferData(GL_ARRAY_BUFFER, COMPONENTCOUNT * count * sizeof(float), 0, GL_STREAM_DRAW);
  delete [] particles;
}

void PointEmitter::simulate()
{
  glUseProgram(SimulationProgram);
  glEnable(GL_RASTERIZER_DISCARD);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
  glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)0);
  glVertexAttribPointer(loc_lifetime, 1, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  glVertexAttribPointer(loc_velocity, 4, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), (GLvoid*)(4 * sizeof(float)));
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_buffers[1]);

  glUniform1i(loc_dt, 1);
  glUniform1i(loc_duration, duration);
  glUniform3f(loc_source, getPosition().X, getPosition().Y, getPosition().Z);
  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, count);
  glEndTransformFeedback();
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glDisable(GL_RASTERIZER_DISCARD);
  std::swap(tfb_buffers[0], tfb_buffers[1]);
}

void PointEmitter::draw()
{
  glDisable(GL_ALPHA_TEST);
  glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	core::matrix4 matrix = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION);
	matrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
	matrix *= getAbsoluteTransformation();
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SPRITE);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glUseProgram(RenderProgram);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, tfb_buffers[0]);
  float screen[2] = {
	  (float)UserConfigParams::m_width,
	  (float)UserConfigParams::m_height
  };
  irr::core::matrix4 invproj = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_PROJECTION);
  invproj.makeInverse();

  bindUniformToTextureUnit(loc_texture, texture, 0);
  bindUniformToTextureUnit(loc_normal_and_depths, normal_and_depth, 1);

  glUniformMatrix4fv(loc_invproj, 1, GL_FALSE, invproj.pointer());
  glUniform2f(loc_screen, screen[0], screen[1]);
  glUniformMatrix4fv(loc_matrix, 1, GL_FALSE, matrix.pointer());

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, COMPONENTCOUNT * sizeof(float), 0);
  glDrawArrays(GL_POINTS, 0, count);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_ALPHA_TEST);
  glDepthMask(GL_TRUE);
}

RainNode::RainNode(scene::ISceneManager* mgr, ITexture *tex)
    : GPUParticle(mgr, tex)
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
