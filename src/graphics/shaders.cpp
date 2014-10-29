//  SuperTuxKart - a fun racing game with go-kart
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

 Use the AssignUniforms() function to pass name of the uniforms in the program.
 The order of name declaration is the same as the argument passed to setUniforms function.

 \subsection shader_declaration_bind_texture_unit Bind texture unit and name

 Texture are optional but if you have one, you must give them determined texture unit (up to 32).
 You can do this using the AssignTextureUnit function that takes pair of texture unit and sampler name
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
#include "graphics/irr_driver.hpp"
#include "graphics/gpuparticles.hpp"
#include "graphics/shaders.hpp"
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
    m_callbacks[ES_SUNLIGHT] = new SunLightProvider();
    m_callbacks[ES_DISPLACE] = new DisplaceProvider();

    for (s32 i = 0; i < ES_COUNT; i++)
        m_shaders[i] = -1;

    loadShaders();
}

// Shader loading  related hook

static std::string LoadHeader()
{
    std::string result;
    std::ifstream Stream(file_manager->getAsset("shaders/header.txt").c_str(), std::ios::in);

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
    sprintf(versionString, "#version %d\n", irr_driver->getGLSLVersion());
    std::string Code = versionString;
    if (irr_driver->hasVSLayerExtension())
        Code += "#extension GL_AMD_vertex_shader_layer : enable\n";
    if (UserConfigParams::m_azdo)
        Code += "#extension GL_ARB_bindless_texture : enable\n";
    else
    {
        Code += "#extension GL_ARB_bindless_texture : disable\n";
        Code += "#undef GL_ARB_bindless_texture\n";
    }
    std::ifstream Stream(file, std::ios::in);
    Code += "//" + std::string(file) + "\n";
    if (irr_driver->needUBOWorkaround())
        Code += "#define UBO_DISABLED\n";
    if (irr_driver->hasVSLayerExtension())
        Code += "#define VSLayer\n";
    if (irr_driver->needsRGBBindlessWorkaround())
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

void setAttribute(AttributeType Tp, GLuint ProgramID)
{
    switch (Tp)
    {
    case OBJECT:
        glBindAttribLocation(ProgramID, 0, "Position");
        glBindAttribLocation(ProgramID, 1, "Normal");
        glBindAttribLocation(ProgramID, 2, "Color");
        glBindAttribLocation(ProgramID, 3, "Texcoord");
        glBindAttribLocation(ProgramID, 4, "SecondTexcoord");
        glBindAttribLocation(ProgramID, 5, "Tangent");
        glBindAttribLocation(ProgramID, 6, "Bitangent");
        glBindAttribLocation(ProgramID, 7, "Origin");
        glBindAttribLocation(ProgramID, 8, "Orientation");
        glBindAttribLocation(ProgramID, 9, "Scale");
        break;
    case PARTICLES_SIM:
        glBindAttribLocation(ProgramID, 0, "particle_position");
        glBindAttribLocation(ProgramID, 1, "lifetime");
        glBindAttribLocation(ProgramID, 2, "particle_velocity");
        glBindAttribLocation(ProgramID, 3, "size");
        glBindAttribLocation(ProgramID, 4, "particle_position_initial");
        glBindAttribLocation(ProgramID, 5, "lifetime_initial");
        glBindAttribLocation(ProgramID, 6, "particle_velocity_initial");
        glBindAttribLocation(ProgramID, 7, "size_initial");
        break;
    case PARTICLES_RENDERING:
        glBindAttribLocation(ProgramID, 1, "lifetime");
        glBindAttribLocation(ProgramID, 2, "size");
        glBindAttribLocation(ProgramID, 4, "quadcorner");
        glBindAttribLocation(ProgramID, 5, "rotationvec");
        glBindAttribLocation(ProgramID, 6, "anglespeed");
        break;
    }
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount)
{
    GLuint Program = glCreateProgram();
    loadAndAttach(Program, GL_VERTEX_SHADER, vertex_file_path);
    if (irr_driver->getGLSLVersion() < 330)
        setAttribute(PARTICLES_SIM, Program);
    glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(Program);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(Program, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrap", ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Program;
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

GLuint SharedObject::cubevbo = 0;
GLuint SharedObject::cubeindexes = 0;

static void initCubeVBO()
{
    // From CSkyBoxSceneNode
    float corners[] =
        {
            // top side
            1., 1., -1.,
            1., 1., 1.,
            -1., 1., 1.,
            -1., 1., -1.,

            // Bottom side
            1., -1., 1.,
            1., -1., -1.,
            -1., -1., -1.,
            -1., -1., 1.,

            // right side
            1., -1, -1,
            1., -1, 1,
            1., 1., 1.,
            1., 1., -1.,

            // left side
            -1., -1., 1.,
            -1., -1., -1.,
            -1., 1., -1.,
            -1., 1., 1.,

            // back side
            -1., -1., -1.,
            1., -1, -1.,
            1, 1, -1.,
            -1, 1, -1.,

            // front side
            1., -1., 1.,
            -1., -1., 1.,
            -1, 1., 1.,
            1., 1., 1.,
        };
    int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    glGenBuffers(1, &SharedObject::cubevbo);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::cubevbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), corners, GL_STATIC_DRAW);

    glGenBuffers(1, &SharedObject::cubeindexes);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::cubeindexes);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 6 * sizeof(int), indices, GL_STATIC_DRAW);
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

    m_shaders[ES_COLORIZE] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                     m_callbacks[ES_COLORIZE], EMT_SOLID);

    m_shaders[ES_OBJECTPASS] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECT_UNLIT] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECTPASS_REF] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");
    m_shaders[ES_OBJECTPASS_RIMLIT] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_SUNLIGHT] = glsl_noinput(dir + "pass.vert", dir + "pass.frag");

    m_shaders[ES_DISPLACE] = glslmat(dir + "pass.vert", dir + "pass.frag",
        m_callbacks[ES_DISPLACE], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_PASSFAR] = glsl(dir + "pass.vert", dir + "pass.frag",
                                 m_callbacks[ES_COLORIZE]);

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
    initCubeVBO();
    initFrustrumVBO();
    initShadowVPMUBO();
    initParticleQuadVBO();
    MeshShader::ViewFrustrumShader::init();
    UtilShader::ColoredLine::init();
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
}

namespace UtilShader
{
    GLuint ColoredLine::Program;
    GLuint ColoredLine::uniform_color;
    GLuint ColoredLine::vao;
    GLuint ColoredLine::vbo;

    void ColoredLine::init()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * 1024 * sizeof(float), 0, GL_DYNAMIC_DRAW);
        GLuint attrib_position = glGetAttribLocation(Program, "Position");
        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        uniform_color = glGetUniformLocation(Program, "color");
    }

    void ColoredLine::setUniforms(const irr::video::SColor &col)
    {
        if (irr_driver->needUBOWorkaround())
            bypassUBO(Program);
        glUniform4i(uniform_color, col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
        glUniformMatrix4fv(glGetUniformLocation(Program, "ModelMatrix"), 1, GL_FALSE, core::IdentityMatrix.pointer());
    }

    struct TexUnit
    {
        GLuint m_index;
        const char* m_uniform;

        TexUnit(GLuint index, const char* uniform)
        {
            m_index = index;
            m_uniform = uniform;
        }
    };

    template <typename T>
    std::vector<TexUnit> TexUnits(T curr) // required on older clang versions
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        return v;
    }

    template <typename T, typename... R>
    std::vector<TexUnit> TexUnits(T curr, R... rest) // required on older clang versions
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        VTexUnits(v, rest...);
        return v;
    }

    template <typename T, typename... R>
    void VTexUnits(std::vector<TexUnit>& v, T curr, R... rest) // required on older clang versions
    {
        v.push_back(curr);
        VTexUnits(v, rest...);
    }

    template <typename T>
    void VTexUnits(std::vector<TexUnit>& v, T curr)
    {
        v.push_back(curr);
    }

    static void
    AssignTextureUnit(GLuint Program, TexUnit texUnit)
    {
        glUseProgram(Program);
        GLuint uniform = glGetUniformLocation(Program, texUnit.m_uniform);
        glUniform1i(uniform, texUnit.m_index);
        glUseProgram(0);
    }

    template<typename... T>
    static void AssignTextureUnit(GLuint Program, TexUnit texUnit, T... rest)
    {
        glUseProgram(Program);
        GLuint uniform = glGetUniformLocation(Program, texUnit.m_uniform);
        glUniform1i(uniform, texUnit.m_index);
        AssignTextureUnit_Sub(Program, rest...);
        glUseProgram(0);
    }

    static void AssignTextureUnit_Sub(GLuint Program) {}

    template<typename... T>
    static void AssignTextureUnit_Sub(GLuint Program, TexUnit texUnit, T... rest)
    {
        GLuint uniform = glGetUniformLocation(Program, texUnit.m_uniform);
        glUniform1i(uniform, texUnit.m_index);
        AssignTextureUnit_Sub(Program, rest...);
    }
}
using namespace UtilShader;

bool needsUBO()
{
    return irr_driver->needUBOWorkaround();
}

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

void BindCubemapTrilinear(unsigned TU, unsigned tex)
{
    glActiveTexture(GL_TEXTURE0 + TU);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)aniso);
}

GLuint createShadowSampler()
{
#ifdef GL_VERSION_3_3
    unsigned id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
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
    return irr_driver->getGLSLVersion();
}

namespace MeshShader
{
    // Solid Normal and depth pass shaders
    ObjectPass1Shader::ObjectPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix");
        AssignSamplerNames(Program, 0, "tex");
    }

    ObjectRefPass1Shader::ObjectRefPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "tex", 1, "glosstex");
    }

    GrassPass1Shader::GrassPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/grass_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix", "windDir");
        AssignSamplerNames(Program, 0, "tex", 1, "glosstex");
    }

    NormalMapShader::NormalMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/normalmap.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix");
        AssignSamplerNames(Program, 1, "normalMap", 0, "DiffuseForAlpha");
    }

    InstancedObjectPass1Shader::InstancedObjectPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_object_pass1.frag").c_str());

        AssignUniforms();
        AssignSamplerNames(Program, 0, "glosstex");
    }

    InstancedObjectRefPass1Shader::InstancedObjectRefPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_objectref_pass1.frag").c_str());

        AssignUniforms();
        AssignSamplerNames(Program, 0, "tex", 1, "glosstex");
    }

    InstancedGrassPass1Shader::InstancedGrassPass1Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_grass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_objectref_pass1.frag").c_str());
        AssignUniforms("windDir");
        AssignSamplerNames(Program, 0, "tex", 1, "glosstex");
    }

    InstancedNormalMapShader::InstancedNormalMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_normalmap.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "normalMap", 1, "glossMap");
    }

    // Solid Lit pass shaders
    ObjectPass2Shader::ObjectPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedObjectPass2Shader::InstancedObjectPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_object_pass2.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedObjectRefPass2Shader::InstancedObjectRefPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_objectref_pass2.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    DetailledObjectPass2Shader::DetailledObjectPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/detailledobject_pass2.frag").c_str());
        AssignUniforms("ModelMatrix");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "Detail", 5, "SpecMap");
    }

    InstancedDetailledObjectPass2Shader::InstancedDetailledObjectPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_detailledobject_pass2.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "Detail", 5, "SpecMap");
    }

    ObjectUnlitShader::ObjectUnlitShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    InstancedObjectUnlitShader::InstancedObjectUnlitShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_object_unlit.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    ObjectRefPass2Shader::ObjectRefPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    GrassPass2Shader::GrassPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/grass_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/grass_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "windDir");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "Albedo", 4, "SpecMap");
    }

    InstancedGrassPass2Shader::InstancedGrassPass2Shader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_grass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_grass_pass2.frag").c_str());
        AssignUniforms("windDir", "SunDir");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "dtex", 4, "Albedo", 5, "SpecMap");
    }

    SphereMapShader::SphereMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectpass_spheremap.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix");
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    InstancedSphereMapShader::InstancedSphereMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_objectpass_spheremap.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "DiffuseMap", 1, "SpecularMap", 2, "SSAO", 3, "tex");
    }

    SplattingShader::SplattingShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/splatting.frag").c_str());
        AssignUniforms("ModelMatrix");

        AssignSamplerNames(Program,
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
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/transparent.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "tex");
    }

    TransparentFogShader::TransparentFogShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/transparentfog.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix", "fogmax", "startH", "endH", "start", "end", "col");
        AssignSamplerNames(Program, 0, "tex");
    }

    BillboardShader::BillboardShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/billboard.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/billboard.frag").c_str());

        AssignUniforms("ModelViewMatrix", "ProjectionMatrix", "Position", "Size");
        AssignSamplerNames(Program, 0, "tex");
    }

    ColorizeShader::ColorizeShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/colorize.frag").c_str());
        AssignUniforms("ModelMatrix", "col");
    }

    InstancedColorizeShader::InstancedColorizeShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/glow_object.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/glow_object.frag").c_str());
        AssignUniforms();
    }

    ShadowShader::ShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        AssignUniforms("layer", "ModelMatrix");
    }

    RSMShader::RSMShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/rsm.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rsm.frag").c_str());

        AssignUniforms("RSMMatrix", "ModelMatrix", "TextureMatrix");
        AssignSamplerNames(Program, 0, "tex");
    }

    InstancedRSMShader::InstancedRSMShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_rsm.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_rsm.frag").c_str());

        AssignUniforms("RSMMatrix");
        AssignSamplerNames(Program, 0, "tex");
    }

    SplattingRSMShader::SplattingRSMShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/rsm.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/splatting_rsm.frag").c_str());

        AssignUniforms("RSMMatrix", "ModelMatrix");
        AssignSamplerNames(Program, 0, "tex_layout", 1, "tex_detail0", 2, "tex_detail1", 3, "tex_detail2", 4, "tex_detail3");
    }

    InstancedShadowShader::InstancedShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/instanced_shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        AssignUniforms("layer");
    }

    RefShadowShader::RefShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        AssignUniforms("layer", "ModelMatrix");
        AssignSamplerNames(Program, 0, "tex");
    }

    InstancedRefShadowShader::InstancedRefShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/instanced_shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }
        AssignUniforms("layer");
        AssignSamplerNames(Program, 0, "tex");
    }

    GrassShadowShader::GrassShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow_grass.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow_grass.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }
        AssignUniforms("layer", "ModelMatrix", "windDir");
        AssignSamplerNames(Program, 0, "tex");
    }

    InstancedGrassShadowShader::InstancedGrassShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedgrassshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedgrassshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/instanced_shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/instanced_shadowref.frag").c_str());
        }

        AssignSamplerNames(Program, 0, "tex");
        AssignUniforms("layer", "windDir");
    }

    DisplaceMaskShader::DisplaceMaskShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/displace.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        AssignUniforms("ModelMatrix");
    }


    DisplaceShader::DisplaceShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/displace.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/displace.frag").c_str());
        AssignUniforms("ModelMatrix", "dir", "dir2");

        AssignSamplerNames(Program,
            0, "displacement_tex",
            1, "color_tex",
            2, "mask_tex",
            3, "tex");
    }

    SkyboxShader::SkyboxShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sky.frag").c_str());
        AssignUniforms("ModelMatrix");
        AssignSamplerNames(Program, 0, "tex");

        glGenVertexArrays(1, &cubevao);
        glBindVertexArray(cubevao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedObject::cubevbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::cubeindexes);
        glBindVertexArray(0);
    }

    NormalVisualizer::NormalVisualizer()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/normal_visualizer.geom").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        AssignUniforms("color");
    }

    GLuint ViewFrustrumShader::Program;
    GLuint ViewFrustrumShader::attrib_position;
    GLuint ViewFrustrumShader::uniform_color;
    GLuint ViewFrustrumShader::uniform_idx;
    GLuint ViewFrustrumShader::frustrumvao;

    void ViewFrustrumShader::init()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/frustrum.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        attrib_position = glGetAttribLocation(Program, "Position");

        uniform_color = glGetUniformLocation(Program, "color");
        uniform_idx = glGetUniformLocation(Program, "idx");

        glGenVertexArrays(1, &frustrumvao);
        glBindVertexArray(frustrumvao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedObject::frustrumvbo);
        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::frustrumindexes);
        glBindVertexArray(0);
    }

    void ViewFrustrumShader::setUniforms(const video::SColor &color, unsigned idx)
    {
        glUniform4i(uniform_color, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
        glUniform1i(uniform_idx, idx);
    }
}

namespace LightShader
{
    PointLightShader::PointLightShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/pointlight.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/pointlight.frag").c_str());

        AssignUniforms();
        AssignSamplerNames(Program, 0, "ntex", 1, "dtex");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAXLIGHT * sizeof(PointLightInfo), 0, GL_DYNAMIC_DRAW);

        GLuint attrib_Position = glGetAttribLocation(Program, "Position");
        GLuint attrib_Color = glGetAttribLocation(Program, "Color");
        GLuint attrib_Energy = glGetAttribLocation(Program, "Energy");
        GLuint attrib_Radius = glGetAttribLocation(Program, "Radius");

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


namespace ParticleShader
{
    SimpleSimulationShader::SimpleSimulationShader()
    {
        const char *varyings[] = {
            "new_particle_position",
            "new_lifetime",
            "new_particle_velocity",
            "new_size",
        };
        Program = LoadTFBProgram(file_manager->getAsset("shaders/pointemitter.vert").c_str(), varyings, 4);
        AssignUniforms("sourcematrix", "dt", "level", "size_increase_factor");
    }

    HeightmapSimulationShader::HeightmapSimulationShader()
    {
        const char *varyings[] = {
            "new_particle_position",
            "new_lifetime",
            "new_particle_velocity",
            "new_size",
        };
        Program = LoadTFBProgram(file_manager->getAsset("shaders/particlesimheightmap.vert").c_str(), varyings, 4);
        TU_heightmap = 2;
        AssignTextureUnit(Program, TexUnit(TU_heightmap, "heightmap"));
        AssignUniforms("sourcematrix", "dt", "level", "size_increase_factor", "track_x", "track_x_len", "track_z", "track_z_len");
    }

    SimpleParticleRender::SimpleParticleRender()
    {
        Program = LoadProgram(PARTICLES_RENDERING,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/particle.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/particle.frag").c_str());
        AssignUniforms("color_from", "color_to");

        AssignSamplerNames(Program, 0, "tex", 1, "dtex");
    }

    FlipParticleRender::FlipParticleRender()
    {
        Program = LoadProgram(PARTICLES_RENDERING,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/flipparticle.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/particle.frag").c_str());
        AssignUniforms();

        AssignSamplerNames(Program, 0, "tex", 1, "dtex");
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
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getCIEXYZ.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getRGBfromCIEXxy.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bloom.frag").c_str());
        AssignUniforms();

        AssignSamplerNames(Program, 0, "tex");
    }

    BloomBlendShader::BloomBlendShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bloomblend.frag").c_str());
        AssignUniforms();

        AssignSamplerNames(Program, 0, "tex_128", 1, "tex_256", 2, "tex_512");
    }

    ToneMapShader::ToneMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getRGBfromCIEXxy.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getCIEXYZ.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/tonemap.frag").c_str());
        AssignUniforms();

        AssignSamplerNames(Program, 0, "text");
    }

    DepthOfFieldShader::DepthOfFieldShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/dof.frag").c_str());

        AssignUniforms();
        AssignSamplerNames(Program, 0, "tex", 1, "dtex");
    }

    SunLightShader::SunLightShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sunlight.frag").c_str());

        AssignSamplerNames(Program, 0, "ntex", 1, "dtex");
        AssignUniforms("direction", "col");
    }

    EnvMapShader::EnvMapShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/diffuseenvmap.frag").c_str());
        AssignUniforms("TransposeViewMatrix", "blueLmn[0]", "greenLmn[0]", "redLmn[0]");
        AssignSamplerNames(Program, 0, "ntex", 1, "dtex", 2, "tex");
    }

    ShadowedSunLightShader::ShadowedSunLightShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sunlightshadow.frag").c_str());

        // Use 8 to circumvent a catalyst bug when binding sampler
        AssignSamplerNames(Program, 0, "ntex", 1, "dtex", 8, "shadowtex");
        AssignUniforms("direction", "col");
    }

    RadianceHintsConstructionShader::RadianceHintsConstructionShader()
    {
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/slicedscreenquad.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rh.frag").c_str());
        }
        else
        {
            Program = LoadProgram(OBJECT,
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/slicedscreenquad.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/rhpassthrough.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rh.frag").c_str());
        }

        AssignUniforms("RSMMatrix", "RHMatrix", "extents", "suncol");
        AssignSamplerNames(Program, 0, "ctex", 1, "ntex", 2, "dtex");
    }

    NVWorkaroundRadianceHintsConstructionShader::NVWorkaroundRadianceHintsConstructionShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/slicedscreenquad_nvworkaround.vert").c_str(),
            GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/rhpassthrough.geom").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rh.frag").c_str());

        AssignUniforms("RSMMatrix", "RHMatrix", "extents", "slice", "suncol");

        AssignSamplerNames(Program, 0, "ctex", 1, "ntex", 2, "dtex");
    }

    RHDebug::RHDebug()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/rhdebug.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rhdebug.frag").c_str());
        AssignUniforms("RHMatrix", "extents");
        TU_SHR = 0;
        TU_SHG = 1;
        TU_SHB = 2;
        AssignTextureUnit(Program, TexUnit(TU_SHR, "SHR"), TexUnit(TU_SHG, "SHG"), TexUnit(TU_SHB, "SHB"));
    }

    GlobalIlluminationReconstructionShader::GlobalIlluminationReconstructionShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gi.frag").c_str());

        AssignUniforms("RHMatrix", "InvRHMatrix", "extents");
        AssignSamplerNames(Program, 0, "ntex", 1, "dtex", 2, "SHR", 3, "SHG", 4, "SHB");
    }

    Gaussian17TapHShader::Gaussian17TapHShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bilateralH.frag").c_str());
        AssignUniforms("pixel");
        AssignSamplerNames(Program, 0, "tex", 1, "depth");
    }

    ComputeGaussian17TapHShader::ComputeGaussian17TapHShader()
    {
        Program = LoadProgram(OBJECT,
            GL_COMPUTE_SHADER, file_manager->getAsset("shaders/bilateralH.comp").c_str());
        TU_dest = 2;
        AssignUniforms("pixel");
        AssignSamplerNames(Program, 0, "source", 1, "depth");
        AssignTextureUnit(Program, TexUnit(TU_dest, "dest"));
    }

    Gaussian6HBlurShader::Gaussian6HBlurShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian6h.frag").c_str());
        AssignUniforms("pixel");

        AssignSamplerNames(Program, 0, "tex");
    }

    Gaussian3HBlurShader::Gaussian3HBlurShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian3h.frag").c_str());
        AssignUniforms("pixel");

        AssignSamplerNames(Program, 0, "tex");
    }

    Gaussian17TapVShader::Gaussian17TapVShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bilateralV.frag").c_str());
        AssignUniforms("pixel");

        AssignSamplerNames(Program, 0, "tex", 1, "depth");
    }

    ComputeGaussian17TapVShader::ComputeGaussian17TapVShader()
    {
        Program = LoadProgram(OBJECT,
            GL_COMPUTE_SHADER, file_manager->getAsset("shaders/bilateralV.comp").c_str());
        TU_dest = 2;
        AssignUniforms("pixel");
        AssignSamplerNames(Program, 0, "source", 1, "depth");
        AssignTextureUnit(Program, TexUnit(TU_dest, "dest"));
    }

    Gaussian6VBlurShader::Gaussian6VBlurShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian6v.frag").c_str());
        AssignUniforms("pixel");

        AssignSamplerNames(Program, 0, "tex");
    }

    Gaussian3VBlurShader::Gaussian3VBlurShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian3v.frag").c_str());
        AssignUniforms("pixel");

        AssignSamplerNames(Program, 0, "tex");
    }

    PassThroughShader::PassThroughShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/texturedquad.frag").c_str());

        AssignUniforms();
        AssignSamplerNames(Program, 0, "texture");
        vao = createVAO(Program);
    }

    LayerPassThroughShader::LayerPassThroughShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/layertexturequad.frag").c_str());
        TU_texture = 0;
        AssignUniforms("layer");
        AssignTextureUnit(Program, TexUnit(TU_texture, "tex"));
        vao = createVAO(Program);
    }

    LinearizeDepthShader::LinearizeDepthShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/linearizedepth.frag").c_str());
        AssignUniforms("zn", "zf");

        AssignSamplerNames(Program, 0, "texture");
    }

    GlowShader::GlowShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/glow.frag").c_str());
        AssignUniforms();

        AssignSamplerNames(Program, 0, "tex");
        vao = createVAO(Program);
    }

    SSAOShader::SSAOShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/ssao.frag").c_str());

        AssignSamplerNames(Program, 0, "dtex");
        AssignUniforms("radius", "k", "sigma");
    }

    FogShader::FogShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/fog.frag").c_str());

        AssignUniforms("fogmax", "startH", "endH", "start", "end", "col");
        AssignSamplerNames(Program, 0, "tex");
    }

    MotionBlurShader::MotionBlurShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/motion_blur.frag").c_str());
        AssignUniforms("previous_viewproj", "center", "boost_amount", "mask_radius");
        AssignSamplerNames(Program, 0, "color_buffer", 1, "dtex");
    }

    GodFadeShader::GodFadeShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/godfade.frag").c_str());
        AssignUniforms("col");

        AssignSamplerNames(Program, 0, "tex");
        vao = createVAO(Program);
    }

    GodRayShader::GodRayShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/godray.frag").c_str());

        AssignUniforms("sunpos");
        AssignSamplerNames(Program, 0, "tex");
        vao = createVAO(Program);
    }

    MLAAColorEdgeDetectionSHader::MLAAColorEdgeDetectionSHader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/mlaa_offset.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_color1.frag").c_str());
        AssignUniforms("PIXEL_SIZE");

        AssignSamplerNames(Program, 0, "colorMapG");
        vao = createVAO(Program);
    }

    MLAABlendWeightSHader::MLAABlendWeightSHader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_blend2.frag").c_str());
        AssignUniforms("PIXEL_SIZE");

        AssignSamplerNames(Program, 0, "edgesMap", 1, "areaMap");
        vao = createVAO(Program);
    }

    MLAAGatherSHader::MLAAGatherSHader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/mlaa_offset.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_neigh3.frag").c_str());
        AssignUniforms("PIXEL_SIZE");

        AssignSamplerNames(Program, 0, "blendMap", 1, "colorMap");
        vao = createVAO(Program);
    }
}

namespace UIShader
{

    Primitive2DList::Primitive2DList()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/primitive2dlist.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/transparent.frag").c_str());
        AssignUniforms();
        AssignSamplerNames(Program, 0, "tex");
    }

    TextureRectShader::TextureRectShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/texturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/texturedquad.frag").c_str());
        AssignUniforms("center", "size", "texcenter", "texsize");

        AssignSamplerNames(Program, 0, "tex");
    }

    UniformColoredTextureRectShader::UniformColoredTextureRectShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/texturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/uniformcolortexturedquad.frag").c_str());

        AssignUniforms("center", "size", "texcenter", "texsize", "color");

        AssignSamplerNames(Program, 0, "tex");
    }

    ColoredTextureRectShader::ColoredTextureRectShader()
    {
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/colortexturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/colortexturedquad.frag").c_str());
        AssignUniforms("center", "size", "texcenter", "texsize");

        AssignSamplerNames(Program, 0, "tex");

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
        Program = LoadProgram(OBJECT,
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/coloredquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        AssignUniforms("center", "size", "color");
    }
}
