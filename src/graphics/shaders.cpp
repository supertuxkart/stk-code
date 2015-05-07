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


/**
\page shaders_overview Shaders Overview

 \section shader_declaration Shader declaration
 You need to create a class for each shader in shaders.cpp
 This class should inherit from the template ShaderHelper<>.
 The template first parameter is the shader class being declared and the following ones are the types
 of every uniform (except samplers) required by the shaders.

 The template inheritance will provide the shader with a setUniforms() variadic function which calls
 the glUniform*() that pass uniforms value to the shader according to the type given as parameter
 to the template.

 The class constructor is used to
 \li \ref shader_declaration_compile
 \li \ref shader_declaration_uniform_names
 \li \ref shader_declaration_bind_texture_unit

 Of course it's possible to use the constructor to declare others things if needed.

 \subsection shader_declaration_compile Compile the shader

 The LoadProgram() function is provided to ease shader compilation and link.
 It takes a flat sequence of SHADER_TYPE, filename pairs that will be linked together.
 This way you can add any shader stage you want (geometry, domain/hull shader)

 It is highly recommended to use explicit attribute location for a program input.
 However as not all hardware support this extension, default location are provided for
 input whose name is either Position (location 0) or Normal (location 1) or
 Texcoord (location 3) or Color (location 2) or SecondTexcoord (location 4).
 You can use these predefined name and location in your vao for shader
 that needs GL pre 3.3 support.

 \subsection shader_declaration_uniform_names Declare uniforms

 Use the assignUniforms() function to pass name of the uniforms in the program.
 The order of name declaration is the same as the argument passed to setUniforms function.

 \subsection shader_declaration_bind_texture_unit Bind texture unit and name

 Texture are optional but if you have one, you must give them determined texture unit (up to 32).
 You can do this using the assignTextureUnit function that takes pair of texture unit and sampler name
 as argument.

 \section shader_usage

 Shader's class are singleton that can be retrieved using ShaderClassName::getInstance() which automatically
 creates an instance the first time it is called.

 As the program id of a shader instance is public it can be used to bind a program :
 \code
 glUseProgram(MyShaderClass::getInstance()->Program);
 \endcode

 To set uniforms use the automatically generated function setUniforms:

 \code
 MyShaderClass::getInstance()->setUniforms(Args...)
 \endcode

 A Vertex Array must be bound (VAO creation process is currently left to the reader) :

 \code
 glBindVertexAttrib(vao);
 \endcode

 To actually perform the rendering you also need to call a glDraw* function (left to the reader as well) :

 \code
 glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT);
 \endcode

*/

#define SHADER_NAMES

#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/gpuparticles.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shaders_util.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "graphics/glwrap.hpp"
#include <assert.h>
#include <IGPUProgrammingServices.h>

using namespace video;

std::vector<void(*)()> CleanTable;

Shaders::Shaders()
{
    // Callbacks
    memset(m_callbacks, 0, sizeof(m_callbacks));

    m_callbacks[ES_WATER] = new WaterShaderProvider();
    m_callbacks[ES_GRASS] = new GrassShaderProvider();
    m_callbacks[ES_MOTIONBLUR] = new MotionBlurProvider();
    m_callbacks[ES_MIPVIZ] = new MipVizProvider();
    m_callbacks[ES_DISPLACE] = new DisplaceProvider();

    for (s32 i = 0; i < ES_COUNT; i++)
        m_shaders[i] = -1;

    loadShaders();
}

// Shader loading  related hook

static std::string LoadHeader()
{
    std::string result;
    std::ifstream Stream("header.txt", std::ios::in);

    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            result += "\n" + Line;
        Stream.close();
    }

    return result;
}

// Mostly from shader tutorial
GLuint LoadShader(const char * file, unsigned type)
{
    GLuint Id = glCreateShader(type);
    char versionString[20];
    sprintf(versionString, "#version %d\n", CVS->getGLSLVersion());
    std::string Code = versionString;
    if (CVS->isAMDVertexShaderLayerUsable())
        Code += "#extension GL_AMD_vertex_shader_layer : enable\n";
    if (CVS->isAZDOEnabled())
    {
        Code += "#extension GL_ARB_bindless_texture : enable\n";
        Code += "#define Use_Bindless_Texture\n";
    }
    std::ifstream Stream(file, std::ios::in);
    Code += "//" + std::string(file) + "\n";
    if (!CVS->isARBUniformBufferObjectUsable())
        Code += "#define UBO_DISABLED\n";
    if (CVS->isAMDVertexShaderLayerUsable())
        Code += "#define VSLayer\n";
    if (CVS->needsRGBBindlessWorkaround())
        Code += "#define SRGBBindlessFix\n";
    Code += LoadHeader();
    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            Code += "\n" + Line;
        Stream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    Log::info("GLWrap", "Compiling shader : %s", file);
    char const * SourcePointer = Code.c_str();
    int length = (int)strlen(SourcePointer);
    glShaderSource(Id, 1, &SourcePointer, &length);
    glCompileShader(Id);

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        Log::error("GLWrap", "Error in shader %s", file);
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength<0)
            InfoLogLength = 1024;
        char *ErrorMessage = new char[InfoLogLength];
        ErrorMessage[0] = 0;
        glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrap", ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Id;
}



GLuint quad_vbo, tri_vbo;

GLuint SharedObject::FullScreenQuadVAO = 0;
GLuint SharedObject::UIVAO = 0;

static void initQuadVBO()
{
    const float quad_vertex[] = {
        -1., -1., 0., 0., // UpperLeft
        -1., 1., 0., 1., // LowerLeft
        1., -1., 1., 0., // UpperRight
        1., 1., 1., 1., // LowerRight
    };
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quad_vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const float tri_vertex[] = {
        -1., -1.,
        -1., 3.,
        3., -1.,
    };
    glGenBuffers(1, &tri_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), tri_vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &SharedObject::FullScreenQuadVAO);
    glBindVertexArray(SharedObject::FullScreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}

// It should be possible to merge it with previous one...
GLuint quad_buffer;

static void initQuadBuffer()
{
    const float quad_vertex[] = {
        -1., -1., -1., 1., // UpperLeft
        -1., 1., -1., -1., // LowerLeft
        1., -1., 1., 1., // UpperRight
        1., 1., 1., -1., // LowerRight
    };
    glGenBuffers(1, &quad_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quad_vertex, GL_STATIC_DRAW);

    glGenVertexArrays(1, &SharedObject::UIVAO);
    glBindVertexArray(SharedObject::UIVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

GLuint SharedObject::billboardvbo = 0;

static void initBillboardVBO()
{
    float quad[] = {
        -.5, -.5, 0., 1.,
        -.5, .5, 0., 0.,
        .5, -.5, 1., 1.,
        .5, .5, 1., 0.,
    };
    glGenBuffers(1, &(SharedObject::billboardvbo));
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::billboardvbo);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quad, GL_STATIC_DRAW);
}

GLuint SharedObject::skytrivbo = 0;

static void initSkyTriVBO()
{
  const float tri_vertex[] = {
      -1., -1., 1.,
      -1., 3., 1.,
      3., -1., 1.,
  };

    glGenBuffers(1, &SharedObject::skytrivbo);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::skytrivbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), tri_vertex, GL_STATIC_DRAW);
}

GLuint SharedObject::frustrumvbo = 0;
GLuint SharedObject::frustrumindexes = 0;

static void initFrustrumVBO()
{
    glGenBuffers(1, &SharedObject::frustrumvbo);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::frustrumvbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);

    int indices[24] = {
        0, 1, 1, 3, 3, 2, 2, 0,
        4, 5, 5, 7, 7, 6, 6, 4,
        0, 4, 1, 5, 2, 6, 3, 7,
    };

    glGenBuffers(1, &SharedObject::frustrumindexes);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::frustrumindexes);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 2 * sizeof(int), indices, GL_STATIC_DRAW);
}

GLuint SharedObject::ViewProjectionMatrixesUBO;

static void initShadowVPMUBO()
{
    glGenBuffers(1, &SharedObject::ViewProjectionMatrixesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, SharedObject::ViewProjectionMatrixesUBO);
    glBufferData(GL_UNIFORM_BUFFER, (16 * 9 + 2) * sizeof(float), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GLuint SharedObject::LightingDataUBO;

static void initLightingDataUBO()
{
    glGenBuffers(1, &SharedObject::LightingDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, SharedObject::LightingDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, 36 * sizeof(float), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GLuint SharedObject::ParticleQuadVBO = 0;

static void initParticleQuadVBO()
{
    static const GLfloat quad_vertex[] = {
        -.5, -.5, 0., 0.,
        .5, -.5, 1., 0.,
        -.5, .5, 0., 1.,
        .5, .5, 1., 1.,
    };
    glGenBuffers(1, &SharedObject::ParticleQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::ParticleQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex), quad_vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shaders::loadShaders()
{
    const std::string &dir = file_manager->getAsset(FileManager::SHADER, "");

    IGPUProgrammingServices * const gpu = irr_driver->getVideoDriver()->getGPUProgrammingServices();

#define glsl(a, b, c) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c)
#define glslmat(a, b, c, d) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c, d)
#define glsl_noinput(a, b) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) 0)

    // Save previous shaders (used in case some shaders don't compile)
    int saved_shaders[ES_COUNT];
    memcpy(saved_shaders, m_shaders, sizeof(m_shaders));

    // Ok, go
    m_shaders[ES_NORMAL_MAP] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_NORMAL_MAP_LIGHTMAP] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_SKYBOX] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                   m_callbacks[ES_SKYBOX], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_SPLATTING] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_WATER] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                  m_callbacks[ES_WATER], EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_shaders[ES_WATER_SURFACE] = glsl(dir + "pass.vert", dir + "pass.frag",
                                       m_callbacks[ES_WATER]);

    m_shaders[ES_SPHERE_MAP] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_GRASS] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                  m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_shaders[ES_GRASS_REF] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                      m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    m_shaders[ES_MOTIONBLUR] = glsl(dir + "pass.vert", dir + "pass.frag",
                                    m_callbacks[ES_MOTIONBLUR]);

    m_shaders[ES_GAUSSIAN3H] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                       m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
    m_shaders[ES_GAUSSIAN3V] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                       m_callbacks[ES_GAUSSIAN3V], EMT_SOLID);

    m_shaders[ES_MIPVIZ] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                   m_callbacks[ES_MIPVIZ], EMT_SOLID);

    m_shaders[ES_OBJECTPASS] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECT_UNLIT] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECTPASS_REF] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECTPASS_RIMLIT] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_DISPLACE] = glslmat(dir + "pass.vert", dir + "pass.frag",
        m_callbacks[ES_DISPLACE], EMT_TRANSPARENT_ALPHA_CHANNEL);

    // Check that all successfully loaded
    for (s32 i = 0; i < ES_COUNT; i++) {

        // Old Intel Windows drivers fail here.
        // It's an artist option, so not necessary to play.
        if (i == ES_MIPVIZ)
            continue;

        check(i);
    }

#undef glsl
#undef glslmat
#undef glsl_noinput

    // In case we're reloading and a shader didn't compile: keep the previous, working one
    for (s32 i = 0; i < ES_COUNT; i++)
    {
        if (m_shaders[i] == -1)
            m_shaders[i] = saved_shaders[i];
    }

    initGL();
    CleanTable.clear();
    initQuadVBO();
    initQuadBuffer();
    initBillboardVBO();
    initSkyTriVBO();
    initFrustrumVBO();
    initShadowVPMUBO();
    initLightingDataUBO();
    initParticleQuadVBO();
}

void Shaders::killShaders()
{
    for (unsigned i = 0; i < CleanTable.size(); i++)
        CleanTable[i]();
}

Shaders::~Shaders()
{
    u32 i;
    for (i = 0; i < ES_COUNT; i++)
    {
        if (i == ES_GAUSSIAN3V || !m_callbacks[i]) continue;
        delete m_callbacks[i];
    }
}

E_MATERIAL_TYPE Shaders::getShader(const ShaderType num) const
{
    assert(num < ES_COUNT);

    return (E_MATERIAL_TYPE)m_shaders[num];
}

void Shaders::check(const int num) const
{
    if (m_shaders[num] == -1)
    {
        Log::error("shaders", "Shader %s failed to load. Update your drivers, if the issue "
                   "persists, report a bug to us.", shader_names[num] + 3);
    }
}

void bypassUBO(GLuint Program)
{
    GLint VM = glGetUniformLocation(Program, "ViewMatrix");
    glUniformMatrix4fv(VM, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());
    GLint PM = glGetUniformLocation(Program, "ProjectionMatrix");
    glUniformMatrix4fv(PM, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());
    GLint IVM = glGetUniformLocation(Program, "InverseViewMatrix");
    glUniformMatrix4fv(IVM, 1, GL_FALSE, irr_driver->getInvViewMatrix().pointer());
    GLint IPM = glGetUniformLocation(Program, "InverseProjectionMatrix");
    glUniformMatrix4fv(IPM, 1, GL_FALSE, irr_driver->getInvProjMatrix().pointer());
    GLint Screen = glGetUniformLocation(Program, "screen");
    glUniform2f(Screen, irr_driver->getCurrentScreenSize().X, irr_driver->getCurrentScreenSize().Y);
    GLint bLmn = glGetUniformLocation(Program, "blueLmn[0]");
    glUniform1fv(bLmn, 9, irr_driver->blueSHCoeff);
    GLint gLmn = glGetUniformLocation(Program, "greenLmn[0]");
    glUniform1fv(gLmn, 9, irr_driver->greenSHCoeff);
    GLint rLmn = glGetUniformLocation(Program, "redLmn[0]");
    glUniform1fv(rLmn, 9, irr_driver->redSHCoeff);
    GLint sundir = glGetUniformLocation(Program, "sun_direction");
    glUniform3f(sundir, irr_driver->getSunDirection().X, irr_driver->getSunDirection().Y, irr_driver->getSunDirection().Z);
    GLint suncol = glGetUniformLocation(Program, "sun_col");
    glUniform3f(suncol, irr_driver->getSunColor().getRed(), irr_driver->getSunColor().getGreen(), irr_driver->getSunColor().getBlue());
    GLint sunangle = glGetUniformLocation(Program, "sun_angle");
    glUniform1f(sunangle, 0.54f);
}

namespace UtilShader
{
    ColoredLine::ColoredLine()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "coloredquad.frag");

        assignUniforms("color");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * 1024 * sizeof(float), 0, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}


using namespace UtilShader;


void setTextureSampler(GLenum tp, GLuint texunit, GLuint tid, GLuint sid)
{
#ifdef GL_VERSION_3_3
    glActiveTexture(GL_TEXTURE0 + texunit);
    glBindTexture(tp, tid);
    glBindSampler(texunit, sid);
#endif
}


GLuint createNearestSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
#endif
}

void BindTextureNearest(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}

GLuint createBilinearSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
#endif
}

void BindTextureBilinear(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}

GLuint createBilinearClampedSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
#endif
}

void BindTextureBilinearClamped(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}

GLuint createSemiTrilinearSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
    return id;
#endif
}

void BindTextureSemiTrilinear(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}

GLuint createTrilinearSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
    return id;
#endif
}

void BindTextureTrilinearAnisotropic(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
}


GLuint createShadowSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameterf(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameterf(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    return id;
#endif
}

void BindTextureShadow(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}


GLuint createTrilinearClampedArray()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
    return id;
#endif
}

void BindTrilinearClampedArrayTexture(unsigned TU, unsigned tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
}

void BindTextureVolume(GLuint TU, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
}

unsigned getGLSLVersion()
{
    return CVS->getGLSLVersion();
}

namespace UtilShader
{
    SpecularIBLGenerator::SpecularIBLGenerator()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "importance_sampling_specular.frag");
        assignUniforms("PermutationMatrix", "ViewportSize");
        TU_Samples = 1;
        assignSamplerNames(m_program, 0, "tex", ST_TRILINEAR_CUBEMAP);
        assignTextureUnit(TU_Samples, "samples");
    }
}

namespace MeshShader
{
    // Solid Normal and depth pass shaders
    ObjectPass1Shader::ObjectPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "object_pass1.frag");
        assignUniforms("ModelMatrix", "InverseModelMatrix");
        assignSamplerNames(m_program, 0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    ObjectRefPass1Shader::ObjectRefPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "objectref_pass1.frag");
        assignUniforms("ModelMatrix", "InverseModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "tex",     ST_TRILINEAR_ANISOTROPIC_FILTERED,
                                      1, "glosstex",ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    GrassPass1Shader::GrassPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "grass_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "objectref_pass1.frag");
        assignUniforms("ModelMatrix", "InverseModelMatrix", "windDir");
        assignSamplerNames(m_program, 0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                                      1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    NormalMapShader::NormalMapShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "normalmap.frag");
        assignUniforms("ModelMatrix", "InverseModelMatrix");
        assignSamplerNames(m_program, 1, "normalMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                                      0, "DiffuseForAlpha", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    InstancedObjectPass1Shader::InstancedObjectPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "instanced_object_pass1.frag");

        assignUniforms();
        assignSamplerNames(m_program, 0, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    InstancedObjectRefPass1Shader::InstancedObjectRefPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "instanced_objectref_pass1.frag");

        assignUniforms();
        assignSamplerNames(m_program, 0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                                      1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    }

    InstancedGrassPass1Shader::InstancedGrassPass1Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_grass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "instanced_objectref_pass1.frag");
        assignUniforms("windDir");
        assignSamplerNames(m_program, 0, "tex", 1, "glosstex");
    }

    InstancedNormalMapShader::InstancedNormalMapShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/encode_normal.frag",
            GL_FRAGMENT_SHADER, "instanced_normalmap.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "normalMap", 1, "glossMap");
    }

    // Solid Lit pass shaders
    ObjectPass2Shader::ObjectPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "object_pass2.frag");
        assignUniforms("ModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedObjectPass2Shader::InstancedObjectPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "instanced_object_pass2.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedObjectRefPass2Shader::InstancedObjectRefPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "instanced_objectref_pass2.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    DetailledObjectPass2Shader::DetailledObjectPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "detailledobject_pass2.frag");
        assignUniforms("ModelMatrix");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "Detail", 5, "SpecMap");
    }

    InstancedDetailledObjectPass2Shader::InstancedDetailledObjectPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "instanced_detailledobject_pass2.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "Detail", 5, "SpecMap");
    }

    ObjectUnlitShader::ObjectUnlitShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "object_unlit.frag");
        assignUniforms("ModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    InstancedObjectUnlitShader::InstancedObjectUnlitShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "instanced_object_unlit.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    ObjectRefPass2Shader::ObjectRefPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "objectref_pass2.frag");
        assignUniforms("ModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    GrassPass2Shader::GrassPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "grass_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "grass_pass2.frag");
        assignUniforms("ModelMatrix", "windDir");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedGrassPass2Shader::InstancedGrassPass2Shader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_grass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "instanced_grass_pass2.frag");
        assignUniforms("windDir", "SunDir");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "dtex", 4, "Albedo", 5, "SpecMap");
    }

    SphereMapShader::SphereMapShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "objectpass_spheremap.frag");
        assignUniforms("ModelMatrix", "InverseModelMatrix");
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    InstancedSphereMapShader::InstancedSphereMapShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "instanced_objectpass_spheremap.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    SplattingShader::SplattingShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "utils/getLightFactor.frag",
            GL_FRAGMENT_SHADER, "splatting.frag");
        assignUniforms("ModelMatrix");

        assignSamplerNames(m_program,
            0, "DiffuseMap",
            1, "SpecularMap",
            2, "SSAO",
            3, "tex_layout",
            4, "tex_detail0",
            5, "tex_detail1",
            6, "tex_detail2",
            7, "tex_detail3");
    }

    TransparentShader::TransparentShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "transparent.frag");
        assignUniforms("ModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "tex");
    }

    TransparentFogShader::TransparentFogShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "transparentfog.frag");
        assignUniforms("ModelMatrix", "TextureMatrix", "fogmax", "startH", "endH", "start", "end", "col");
        assignSamplerNames(m_program, 0, "tex");
    }

    BillboardShader::BillboardShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "billboard.vert",
            GL_FRAGMENT_SHADER, "billboard.frag");

        assignUniforms("ModelViewMatrix", "ProjectionMatrix", "Position", "Size");
        assignSamplerNames(m_program, 0, "tex");
    }

    ColorizeShader::ColorizeShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "object_pass.vert",
            GL_FRAGMENT_SHADER, "colorize.frag");
        assignUniforms("ModelMatrix", "col");
    }

    InstancedColorizeShader::InstancedColorizeShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "glow_object.vert",
            GL_FRAGMENT_SHADER, "glow_object.frag");
        assignUniforms();
    }

    ShadowShader::ShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow.vert",
                GL_FRAGMENT_SHADER, "shadow.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow.vert",
                GL_GEOMETRY_SHADER, "shadow.geom",
                GL_FRAGMENT_SHADER, "shadow.frag");
        }
        assignUniforms("layer", "ModelMatrix");
    }

    RSMShader::RSMShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "rsm.vert",
            GL_FRAGMENT_SHADER, "rsm.frag");

        assignUniforms("RSMMatrix", "ModelMatrix", "TextureMatrix");
        assignSamplerNames(m_program, 0, "tex");
    }

    InstancedRSMShader::InstancedRSMShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_rsm.vert",
            GL_FRAGMENT_SHADER, "instanced_rsm.frag");

        assignUniforms("RSMMatrix");
        assignSamplerNames(m_program, 0, "tex");
    }

    SplattingRSMShader::SplattingRSMShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "rsm.vert",
            GL_FRAGMENT_SHADER, "splatting_rsm.frag");

        assignUniforms("RSMMatrix", "ModelMatrix");
        assignSamplerNames(m_program, 0, "tex_layout", 1, "tex_detail0", 2, "tex_detail1", 3, "tex_detail2", 4, "tex_detail3");
    }

    InstancedShadowShader::InstancedShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedshadow.vert",
                GL_FRAGMENT_SHADER, "shadow.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedshadow.vert",
                GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                GL_FRAGMENT_SHADER, "shadow.frag");
        }
        assignUniforms("layer");
    }

    RefShadowShader::RefShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow.vert",
                GL_FRAGMENT_SHADER, "shadowref.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow.vert",
                GL_GEOMETRY_SHADER, "shadow.geom",
                GL_FRAGMENT_SHADER, "shadowref.frag");
        }
        assignUniforms("layer", "ModelMatrix");
        assignSamplerNames(m_program, 0, "tex");
    }

    InstancedRefShadowShader::InstancedRefShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedshadow.vert",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedshadow.vert",
                GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }
        assignUniforms("layer");
        assignSamplerNames(m_program, 0, "tex");
    }

    GrassShadowShader::GrassShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow_grass.vert",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "shadow_grass.vert",
                GL_GEOMETRY_SHADER, "shadow.geom",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }
        assignUniforms("layer", "ModelMatrix", "windDir");
        assignSamplerNames(m_program, 0, "tex");
    }

    InstancedGrassShadowShader::InstancedGrassShadowShader()
    {
        // Geometry shader needed
        if (CVS->getGLSLVersion() < 150)
            return;
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedgrassshadow.vert",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
                GL_VERTEX_SHADER, "instanciedgrassshadow.vert",
                GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
        }

        assignSamplerNames(m_program, 0, "tex");
        assignUniforms("layer", "windDir");
    }

    DisplaceMaskShader::DisplaceMaskShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "displace.vert",
            GL_FRAGMENT_SHADER, "white.frag");
        assignUniforms("ModelMatrix");
    }


    DisplaceShader::DisplaceShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "displace.vert",
            GL_FRAGMENT_SHADER, "displace.frag");
        assignUniforms("ModelMatrix", "dir", "dir2");

        assignSamplerNames(m_program,
            0, "displacement_tex",
            1, "color_tex",
            2, "mask_tex",
            3, "tex");
    }

    SkyboxShader::SkyboxShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "sky.vert",
            GL_FRAGMENT_SHADER, "sky.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "tex", ST_TRILINEAR_CUBEMAP);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedObject::skytrivbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindVertexArray(0);
    }

    NormalVisualizer::NormalVisualizer()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "utils/getworldmatrix.vert",
            GL_VERTEX_SHADER, "instanced_object_pass.vert",
            GL_GEOMETRY_SHADER, "normal_visualizer.geom",
            GL_FRAGMENT_SHADER, "coloredquad.frag");
        assignUniforms("color");
    }

    ViewFrustrumShader::ViewFrustrumShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "frustrum.vert",
            GL_FRAGMENT_SHADER, "coloredquad.frag");

        assignUniforms("color", "idx");

        glGenVertexArrays(1, &frustrumvao);
        glBindVertexArray(frustrumvao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedObject::frustrumvbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::frustrumindexes);
        glBindVertexArray(0);
    }
}

namespace LightShader
{
    PointLightShader::PointLightShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "pointlight.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "pointlight.frag");

        assignUniforms();
        assignSamplerNames(m_program, 0, "ntex", 1, "dtex");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAXLIGHT * sizeof(PointLightInfo), 0, GL_DYNAMIC_DRAW);

        GLuint attrib_Position = glGetAttribLocation(m_program, "Position");
        GLuint attrib_Color = glGetAttribLocation(m_program, "Color");
        GLuint attrib_Energy = glGetAttribLocation(m_program, "Energy");
        GLuint attrib_Radius = glGetAttribLocation(m_program, "Radius");

        glEnableVertexAttribArray(attrib_Position);
        glVertexAttribPointer(attrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), 0);
        glEnableVertexAttribArray(attrib_Energy);
        glVertexAttribPointer(attrib_Energy, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Color);
        glVertexAttribPointer(attrib_Color, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(4 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Radius);
        glVertexAttribPointer(attrib_Radius, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(7 * sizeof(float)));

        glVertexAttribDivisorARB(attrib_Position, 1);
        glVertexAttribDivisorARB(attrib_Energy, 1);
        glVertexAttribDivisorARB(attrib_Color, 1);
        glVertexAttribDivisorARB(attrib_Radius, 1);
    }

    PointLightScatterShader::PointLightScatterShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "pointlight.vert",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "pointlightscatter.frag");

        assignUniforms("density", "fogcol");
        assignSamplerNames(m_program, 0, "dtex");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, PointLightShader::getInstance()->vbo);

        GLuint attrib_Position = glGetAttribLocation(m_program, "Position");
        GLuint attrib_Color = glGetAttribLocation(m_program, "Color");
        GLuint attrib_Energy = glGetAttribLocation(m_program, "Energy");
        GLuint attrib_Radius = glGetAttribLocation(m_program, "Radius");

        glEnableVertexAttribArray(attrib_Position);
        glVertexAttribPointer(attrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), 0);
        glEnableVertexAttribArray(attrib_Energy);
        glVertexAttribPointer(attrib_Energy, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Color);
        glVertexAttribPointer(attrib_Color, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(4 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Radius);
        glVertexAttribPointer(attrib_Radius, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(7 * sizeof(float)));

        glVertexAttribDivisorARB(attrib_Position, 1);
        glVertexAttribDivisorARB(attrib_Energy, 1);
        glVertexAttribDivisorARB(attrib_Color, 1);
        glVertexAttribDivisorARB(attrib_Radius, 1);
    }
}



static GLuint createVAO(GLuint Program)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint attrib_position = glGetAttribLocation(Program, "Position");
    GLuint attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_texcoord);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glBindVertexArray(0);
    return vao;
}

namespace FullScreenShader
{
    BloomShader::BloomShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/getCIEXYZ.frag",
            GL_FRAGMENT_SHADER, "utils/getRGBfromCIEXxy.frag",
            GL_FRAGMENT_SHADER, "bloom.frag");
        assignUniforms();

        assignSamplerNames(m_program, 0, "tex");
    }

    BloomBlendShader::BloomBlendShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "bloomblend.frag");
        assignUniforms();

        assignSamplerNames(m_program, 0, "tex_128", 1, "tex_256", 2, "tex_512");
    }
	
	LensBlendShader::LensBlendShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "lensblend.frag");
        assignUniforms();

        assignSamplerNames(m_program, 0, "tex_128", 1, "tex_256", 2, "tex_512");
    }

    ToneMapShader::ToneMapShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/getRGBfromCIEXxy.frag",
            GL_FRAGMENT_SHADER, "utils/getCIEXYZ.frag",
            GL_FRAGMENT_SHADER, "tonemap.frag");
        assignUniforms("vignette_weight");

        assignSamplerNames(m_program, 0, "text");
    }

    DepthOfFieldShader::DepthOfFieldShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "dof.frag");

        assignUniforms();
        assignSamplerNames(m_program, 0, "tex", 1, "dtex");
    }

    SunLightShader::SunLightShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "utils/SunMRP.frag",
            GL_FRAGMENT_SHADER, "sunlight.frag");

        assignSamplerNames(m_program, 0, "ntex", 1, "dtex");
        assignUniforms("direction", "col");
    }

    IBLShader::IBLShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseIBL.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularIBL.frag",
            GL_FRAGMENT_SHADER, "IBL.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "ntex",  ST_NEAREST_FILTERED,
                                      1, "dtex",  ST_NEAREST_FILTERED,
                                      2, "probe", ST_TRILINEAR_CUBEMAP);
    }

    DegradedIBLShader::DegradedIBLShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseIBL.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularIBL.frag",
            GL_FRAGMENT_SHADER, "degraded_ibl.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "ntex");
    }

    ShadowedSunLightShaderPCF::ShadowedSunLightShaderPCF()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "utils/SunMRP.frag",
            GL_FRAGMENT_SHADER, "sunlightshadow.frag");

        // Use 8 to circumvent a catalyst bug when binding sampler
        assignSamplerNames(m_program, 0, "ntex", 1, "dtex", 8, "shadowtex");
        assignUniforms("split0", "split1", "split2", "splitmax", "shadow_res");
    }

    ShadowedSunLightShaderESM::ShadowedSunLightShaderESM()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "utils/SunMRP.frag",
            GL_FRAGMENT_SHADER, "sunlightshadowesm.frag");

        // Use 8 to circumvent a catalyst bug when binding sampler
        assignSamplerNames(m_program, 0, "ntex", 1, "dtex", 8, "shadowtex");
        assignUniforms("split0", "split1", "split2", "splitmax");
    }

    RadianceHintsConstructionShader::RadianceHintsConstructionShader()
    {
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "slicedscreenquad.vert",
                GL_FRAGMENT_SHADER, "rh.frag");
        }
        else
        {
            loadProgram(OBJECT,
                GL_VERTEX_SHADER, "slicedscreenquad.vert",
                GL_GEOMETRY_SHADER, "rhpassthrough.geom",
                GL_FRAGMENT_SHADER, "rh.frag");
        }

        assignUniforms("RSMMatrix", "RHMatrix", "extents", "suncol");
        assignSamplerNames(m_program, 0, "ctex", 1, "ntex", 2, "dtex");
    }

    NVWorkaroundRadianceHintsConstructionShader::NVWorkaroundRadianceHintsConstructionShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "slicedscreenquad_nvworkaround.vert",
            GL_GEOMETRY_SHADER, "rhpassthrough.geom",
            GL_FRAGMENT_SHADER, "rh.frag");

        assignUniforms("RSMMatrix", "RHMatrix", "extents", "slice", "suncol");

        assignSamplerNames(m_program, 0, "ctex", 1, "ntex", 2, "dtex");
    }

    RHDebug::RHDebug()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "rhdebug.vert",
            GL_FRAGMENT_SHADER, "rhdebug.frag");
        assignUniforms("RHMatrix", "extents");
        TU_SHR = 0;
        TU_SHG = 1;
        TU_SHB = 2;
        assignTextureUnit(TU_SHR, "SHR",  TU_SHG, "SHG",  TU_SHB, "SHB");
    }

    GlobalIlluminationReconstructionShader::GlobalIlluminationReconstructionShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "gi.frag");

        assignUniforms("RHMatrix", "InvRHMatrix", "extents");
        assignSamplerNames(m_program, 0, "ntex", 1, "dtex", 2, "SHR", 3, "SHG", 4, "SHB");
    }

    Gaussian17TapHShader::Gaussian17TapHShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "bilateralH.frag");
        assignUniforms("pixel");
        assignSamplerNames(m_program, 0, "tex", 1, "depth");
    }

    ComputeGaussian17TapHShader::ComputeGaussian17TapHShader()
    {
        loadProgram(OBJECT,  GL_COMPUTE_SHADER, "bilateralH.comp");
        TU_dest = 2;
        assignUniforms("pixel");
        assignSamplerNames(m_program, 0, "source", 1, "depth");
        assignTextureUnit(TU_dest, "dest");
    }

    ComputeGaussian6HBlurShader::ComputeGaussian6HBlurShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "gaussian6h.comp");
        TU_dest = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source");
        assignTextureUnit(TU_dest, "dest");
    }

    ComputeShadowBlurHShader::ComputeShadowBlurHShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "blurshadowH.comp");
        TU_dest = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source");
        assignTextureUnit(TU_dest, "dest");
    }

    Gaussian6HBlurShader::Gaussian6HBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "gaussian6h.frag");
        assignUniforms("pixel", "sigma");

        assignSamplerNames(m_program, 0, "tex");
    }
	
	HorizontalBlurShader::HorizontalBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "gaussian6h.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex");
    }

    Gaussian3HBlurShader::Gaussian3HBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "gaussian3h.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex");
    }

    Gaussian17TapVShader::Gaussian17TapVShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "bilateralV.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex", 1, "depth");
    }

    ComputeGaussian17TapVShader::ComputeGaussian17TapVShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "bilateralV.comp");
        TU_dest = 2;
        assignUniforms("pixel");
        assignSamplerNames(m_program, 0, "source", 1, "depth");
        assignTextureUnit(TU_dest, "dest");
    }

    ComputeGaussian6VBlurShader::ComputeGaussian6VBlurShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "gaussian6v.comp");
        TU_dest = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source");
        assignTextureUnit(TU_dest, "dest");
    }

    ComputeShadowBlurVShader::ComputeShadowBlurVShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "blurshadowV.comp");
        TU_dest = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source");
        assignTextureUnit(TU_dest, "dest");
    }

    Gaussian6VBlurShader::Gaussian6VBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "gaussian6v.frag");
        assignUniforms("pixel", "sigma");

        assignSamplerNames(m_program, 0, "tex");
    }

    Gaussian3VBlurShader::Gaussian3VBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "gaussian3v.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex");
    }

    PassThroughShader::PassThroughShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "passthrough.frag");

        assignUniforms("width", "height");
        assignSamplerNames(m_program, 0, "tex");
    }

    LayerPassThroughShader::LayerPassThroughShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "layertexturequad.frag");
        TU_texture = 0;
        assignUniforms("layer");
        assignTextureUnit(TU_texture, "tex");
        vao = createVAO(m_program);
    }

    LinearizeDepthShader::LinearizeDepthShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "linearizedepth.frag");
        assignUniforms("zn", "zf");

        assignSamplerNames(m_program, 0, "texture");
    }


    LightspaceBoundingBoxShader::LightspaceBoundingBoxShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "Lightspaceboundingbox.comp",
            GL_COMPUTE_SHADER, "utils/getPosFromUVDepth.frag");
        assignSamplerNames(m_program, 0, "depth");
        assignUniforms("SunCamMatrix", "split0", "split1", "split2", "splitmax");
        GLuint block_idx = glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK, "BoundingBoxes");
        glShaderStorageBlockBinding(m_program, block_idx, 2);
    }

    ShadowMatrixesGenerationShader::ShadowMatrixesGenerationShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "shadowmatrixgeneration.comp");
        assignUniforms("SunCamMatrix");
        GLuint block_idx = glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK, "BoundingBoxes");
        glShaderStorageBlockBinding(m_program, block_idx, 2);
        block_idx = glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK, "NewMatrixData");
        glShaderStorageBlockBinding(m_program, block_idx, 1);
    }

    DepthHistogramShader::DepthHistogramShader()
    {
        loadProgram(OBJECT,
            GL_COMPUTE_SHADER, "depthhistogram.comp",
            GL_COMPUTE_SHADER, "utils/getPosFromUVDepth.frag");
        assignSamplerNames(m_program, 0, "depth");

        GLuint block_idx = glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK, "Histogram");
        glShaderStorageBlockBinding(m_program, block_idx, 1);
    }

    GlowShader::GlowShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "glow.frag");
        assignUniforms();

        assignSamplerNames(m_program, 0, "tex");
        vao = createVAO(m_program);
    }

    SSAOShader::SSAOShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "ssao.frag");

        assignUniforms("radius", "k", "sigma");
        assignSamplerNames(m_program, 0, "dtex");
    }

    FogShader::FogShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "fog.frag");

        assignUniforms("density", "col");
        assignSamplerNames(m_program, 0, "tex");
    }

    MotionBlurShader::MotionBlurShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
            GL_FRAGMENT_SHADER, "motion_blur.frag");
        assignUniforms("previous_viewproj", "center", "boost_amount", "mask_radius");
        assignSamplerNames(m_program, 0, "color_buffer", 1, "dtex");
    }

    GodFadeShader::GodFadeShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "godfade.frag");
        assignUniforms("col");

        assignSamplerNames(m_program, 0, "tex");
    }

    GodRayShader::GodRayShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "godray.frag");

        assignUniforms("sunpos");
        assignSamplerNames(m_program, 0, "tex");
    }

    MLAAColorEdgeDetectionSHader::MLAAColorEdgeDetectionSHader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "mlaa_color1.frag");
        assignUniforms("PIXEL_SIZE");

        assignSamplerNames(m_program, 0, "colorMapG");
    }

    MLAABlendWeightSHader::MLAABlendWeightSHader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "mlaa_blend2.frag");
        assignUniforms("PIXEL_SIZE");

        assignSamplerNames(m_program, 0, "edgesMap", 1, "areaMap");
    }

    MLAAGatherSHader::MLAAGatherSHader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "screenquad.vert",
            GL_FRAGMENT_SHADER, "mlaa_neigh3.frag");
        assignUniforms("PIXEL_SIZE");

        assignSamplerNames(m_program, 0, "blendMap", 1, "colorMap");
    }
}

namespace UIShader
{

    Primitive2DList::Primitive2DList()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "primitive2dlist.vert",
            GL_FRAGMENT_SHADER, "transparent.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "tex");
    }

    TextureRectShader::TextureRectShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "texturedquad.vert",
            GL_FRAGMENT_SHADER, "texturedquad.frag");
        assignUniforms("center", "size", "texcenter", "texsize");

        assignSamplerNames(m_program, 0, "tex");
    }

    UniformColoredTextureRectShader::UniformColoredTextureRectShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "texturedquad.vert",
            GL_FRAGMENT_SHADER, "uniformcolortexturedquad.frag");

        assignUniforms("center", "size", "texcenter", "texsize", "color");

        assignSamplerNames(m_program, 0, "tex");
    }

    ColoredTextureRectShader::ColoredTextureRectShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "colortexturedquad.vert",
            GL_FRAGMENT_SHADER, "colortexturedquad.frag");
        assignUniforms("center", "size", "texcenter", "texsize");

        assignSamplerNames(m_program, 0, "tex");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
        const unsigned quad_color[] = {
            0, 0, 0, 255,
            255, 0, 0, 255,
            0, 255, 0, 255,
            0, 0, 255, 255,
        };
        glGenBuffers(1, &colorvbo);
        glBindBuffer(GL_ARRAY_BUFFER, colorvbo);
        glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(unsigned), quad_color, GL_DYNAMIC_DRAW);
        glVertexAttribIPointer(2, 4, GL_UNSIGNED_INT, 4 * sizeof(unsigned), 0);
        glBindVertexArray(0);
    }

    ColoredRectShader::ColoredRectShader()
    {
        loadProgram(OBJECT,
            GL_VERTEX_SHADER, "coloredquad.vert",
            GL_FRAGMENT_SHADER, "coloredquad.frag");
        assignUniforms("center", "size", "color");
    }
}
