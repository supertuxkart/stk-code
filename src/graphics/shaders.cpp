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

 \subsection shader_declaration_uniform_names Declare uniforms

 \subsection shader_declaration_bind_texture_unit Bind texture unit and name

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

GLuint getUniformLocation(GLuint program, const char* name)
{
    return glGetUniformLocation(program, name);
}

Shaders::Shaders()
{
    // Callbacks
    memset(m_callbacks, 0, sizeof(m_callbacks));

    m_callbacks[ES_SKYBOX] = new SkyboxProvider();
    m_callbacks[ES_WATER] = new WaterShaderProvider();
    m_callbacks[ES_GRASS] = new GrassShaderProvider();
    m_callbacks[ES_BUBBLES] = new BubbleEffectProvider();
    m_callbacks[ES_MOTIONBLUR] = new MotionBlurProvider();
    m_callbacks[ES_GAUSSIAN3V] = m_callbacks[ES_GAUSSIAN3H] = new GaussianBlurProvider();
    m_callbacks[ES_MIPVIZ] = new MipVizProvider();
    m_callbacks[ES_COLORIZE] = new ColorizeProvider();
    m_callbacks[ES_SUNLIGHT] = new SunLightProvider();
    m_callbacks[ES_DISPLACE] = new DisplaceProvider();

    for (s32 i = 0; i < ES_COUNT; i++)
        m_shaders[i] = -1;

    loadShaders();
}

GLuint quad_vbo, tri_vbo;

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
    glBufferData(GL_UNIFORM_BUFFER, (16 * 8 + 2) * sizeof(float), 0, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

    m_shaders[ES_BUBBLES] = glslmat(dir + "pass.vert", dir + "pass.frag",
                                    m_callbacks[ES_BUBBLES], EMT_TRANSPARENT_ALPHA_CHANNEL);

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
    initQuadVBO();
    initQuadBuffer();
    initBillboardVBO();
    initCubeVBO();
    initFrustrumVBO();
    initShadowVPMUBO();
    FullScreenShader::BloomBlendShader::init();
    FullScreenShader::BloomShader::init();
    FullScreenShader::DepthOfFieldShader::init();
    FullScreenShader::FogShader::init();
    FullScreenShader::Gaussian17TapHShader::init();
    FullScreenShader::ComputeGaussian17TapHShader::init();
    FullScreenShader::Gaussian3HBlurShader::init();
    FullScreenShader::Gaussian3VBlurShader::init();
    FullScreenShader::Gaussian17TapVShader::init();
    FullScreenShader::ComputeGaussian17TapVShader::init();
    FullScreenShader::Gaussian6HBlurShader::init();
    FullScreenShader::Gaussian6VBlurShader::init();
    FullScreenShader::GlowShader::init();
    FullScreenShader::PassThroughShader::init();
    FullScreenShader::LayerPassThroughShader::init();
    FullScreenShader::LinearizeDepthShader::init();
    FullScreenShader::SSAOShader::init();
    FullScreenShader::SunLightShader::init();
    FullScreenShader::DiffuseEnvMapShader::init();
    FullScreenShader::ShadowedSunLightShader::init();
    FullScreenShader::ShadowedSunLightDebugShader::init();
    FullScreenShader::RadianceHintsConstructionShader::init();
    FullScreenShader::RHDebug::init();
    FullScreenShader::GlobalIlluminationReconstructionShader::init();
    FullScreenShader::MotionBlurShader::init();
    FullScreenShader::GodFadeShader::init();
    FullScreenShader::GodRayShader::init();
    FullScreenShader::ToneMapShader::init();
    FullScreenShader::MLAAColorEdgeDetectionSHader::init();
    FullScreenShader::MLAABlendWeightSHader::init();
    FullScreenShader::MLAAGatherSHader::init();
    MeshShader::ColorizeShader::init();
    MeshShader::InstancedObjectPass1ShaderInstance = new MeshShader::InstancedObjectPass1Shader();
    MeshShader::InstancedObjectRefPass1ShaderInstance = new MeshShader::InstancedObjectRefPass1Shader();
    MeshShader::InstancedGrassPass1ShaderInstance = new MeshShader::InstancedGrassPass1Shader();
    MeshShader::InstancedObjectPass2ShaderInstance = new MeshShader::InstancedObjectPass2Shader();
    MeshShader::InstancedObjectRefPass2ShaderInstance = new MeshShader::InstancedObjectRefPass2Shader();
    MeshShader::InstancedGrassPass2ShaderInstance = new MeshShader::InstancedGrassPass2Shader();
    MeshShader::BubbleShader::init();
    MeshShader::BillboardShader::init();
    LightShader::PointLightShader::init();
    MeshShader::DisplaceShaderInstance = new MeshShader::DisplaceShader();
    MeshShader::DisplaceMaskShaderInstance = new MeshShader::DisplaceMaskShader();
    MeshShader::ShadowShaderInstance = new MeshShader::ShadowShader();
    MeshShader::RSMShader::init();
    MeshShader::InstancedShadowShaderInstance = new MeshShader::InstancedShadowShader();
    MeshShader::RefShadowShaderInstance = new MeshShader::RefShadowShader();
    MeshShader::InstancedRefShadowShaderInstance = new MeshShader::InstancedRefShadowShader();
    MeshShader::GrassShadowShaderInstance = new MeshShader::GrassShadowShader();
    MeshShader::InstancedGrassShadowShaderInstance = new MeshShader::InstancedGrassShadowShader();
    MeshShader::SkyboxShader::init();
    MeshShader::ViewFrustrumShader::init();
    ParticleShader::FlipParticleRender::init();
    ParticleShader::HeightmapSimulationShader::init();
    ParticleShader::SimpleParticleRender::init();
    ParticleShader::SimpleSimulationShader::init();
    UIShader::ColoredRectShader::init();
    UIShader::ColoredTextureRectShader::init();
    UIShader::TextureRectShader::init();
    UIShader::UniformColoredTextureRectShader::init();
    UtilShader::ColoredLine::init();
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
}

namespace UtilShader
{
    GLuint ColoredLine::Program;
    GLuint ColoredLine::uniform_color;
    GLuint ColoredLine::vao;
    GLuint ColoredLine::vbo;

    void ColoredLine::init()
    {
        Program = LoadProgram(
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
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void ColoredLine::setUniforms(const irr::video::SColor &col)
    {
        if (UserConfigParams::m_ubo_disabled)
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

void glUniformMatrix4fvWraper(GLuint a, size_t b, unsigned c, const float *d)
{
    glUniformMatrix4fv(a, b, c, d);
}

void glUniform3fWraper(GLuint a, float b, float c, float d)
{
    glUniform3f(a, b, c, d);
}

void glUniform2fWraper(GLuint a, float b, float c)
{
    glUniform2f(a, b, c);
}

void glUniform1fWrapper(GLuint a, float b)
{
    glUniform1f(a, b);
}

namespace MeshShader
{
    // Solid Normal and depth pass shaders
    ObjectPass1Shader::ObjectPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    ObjectRefPass1Shader::ObjectRefPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix", "TextureMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    GrassPass1Shader::GrassPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/grass_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix", "windDir");
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    NormalMapShader::NormalMapShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/normalmap.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/normalmap.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_normalmap = 1;
        TU_glossy = 0;
        AssignTextureUnit(Program, TexUnit(TU_normalmap, "normalMap"), TexUnit(TU_glossy, "DiffuseForAlpha"));
    }

    InstancedObjectPass1Shader::InstancedObjectPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass1.frag").c_str());
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    InstancedObjectPass1Shader *InstancedObjectPass1ShaderInstance;

    InstancedObjectRefPass1Shader::InstancedObjectRefPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    InstancedObjectRefPass1Shader *InstancedObjectRefPass1ShaderInstance;

    InstancedGrassPass1Shader::InstancedGrassPass1Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_grass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/encode_normal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
        AssignUniforms("windDir");
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    InstancedGrassPass1Shader *InstancedGrassPass1ShaderInstance;

    // Solid Lit pass shaders
    ObjectPass2Shader::ObjectPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix", "ambient");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_Albedo = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo")
        );
    }

    ObjectPass2Shader *ObjectPass2ShaderInstance;

    InstancedObjectPass2Shader::InstancedObjectPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_pass2.frag").c_str());
        AssignUniforms("ambient");
        TU_Albedo = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo")
        );

        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    InstancedObjectPass2Shader *InstancedObjectPass2ShaderInstance;

    InstancedObjectRefPass2Shader::InstancedObjectRefPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass2.frag").c_str());
        AssignUniforms("ambient");
        TU_Albedo = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"), 
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo")
        );

        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    InstancedObjectRefPass2Shader *InstancedObjectRefPass2ShaderInstance;

    DetailledObjectPass2Shader::DetailledObjectPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/detailledobject_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "ambient");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        TU_Albedo = 3;
        TU_detail = 4;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo"),
            TexUnit(TU_detail, "Detail")
        );
    }

    DetailledObjectPass2Shader *DetailledObjectPass2ShaderInstance;

    ObjectUnlitShader::ObjectUnlitShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        AssignUniforms("ModelMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 3;

        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    ObjectUnlitShader *ObjectUnlitShaderInstance;

    ObjectRefPass2Shader::ObjectRefPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectref_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix", "ambient");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_Albedo = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo")
        );
    }

    ObjectRefPass2Shader *ObjectRefPass2ShaderInstance;

    GrassPass2Shader::GrassPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/grass_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/grass_pass2.frag").c_str());
        AssignUniforms("ModelMatrix", "windDir", "ambient");
        TU_Albedo = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo")
        );
    }

    GrassPass2Shader *GrassPass2ShaderInstance;

    InstancedGrassPass2Shader::InstancedGrassPass2Shader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanced_grass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/grass_pass2.frag").c_str());
        AssignUniforms("windDir", "SunDir", "ambient");
        TU_Albedo = 3;
        TU_dtex = 4;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_Albedo, "Albedo"),
            TexUnit(TU_dtex, "dtex")
        );

        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    InstancedGrassPass2Shader *InstancedGrassPass2ShaderInstance;

    SphereMapShader::SphereMapShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getLightFactor.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/objectpass_spheremap.frag").c_str());
        AssignUniforms("ModelMatrix", "InverseModelMatrix", "ambient");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 3;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_tex, "tex")
        );
    }

    SphereMapShader *SphereMapShaderInstance;

    SplattingShader::SplattingShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/splatting.frag").c_str());
        AssignUniforms("ModelMatrix", "ambient");
        TU_tex_layout = 3;
        TU_tex_detail0 = 4;
        TU_tex_detail1 = 5;
        TU_tex_detail2 = 6;
        TU_tex_detail3 = 7;

        AssignTextureUnit(Program,
            TexUnit(0, "DiffuseMap"),
            TexUnit(1, "SpecularMap"),
            TexUnit(2, "SSAO"),
            TexUnit(TU_tex_layout, "tex_layout"),
            TexUnit(TU_tex_detail0, "tex_detail0"),
            TexUnit(TU_tex_detail1, "tex_detail1"),
            TexUnit(TU_tex_detail2, "tex_detail2"),
            TexUnit(TU_tex_detail3, "tex_detail3")
        );
    }

    SplattingShader *SplattingShaderInstance;

    GLuint BubbleShader::Program;
    GLuint BubbleShader::uniform_MVP;
    GLuint BubbleShader::uniform_tex;
    GLuint BubbleShader::uniform_time;
    GLuint BubbleShader::uniform_transparency;

    void BubbleShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/bubble.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bubble.frag").c_str());
        uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_time = glGetUniformLocation(Program, "time");
        uniform_transparency = glGetUniformLocation(Program, "transparency");
    }
    void BubbleShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex, float time, float transparency)
    {
        glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
        glUniform1i(uniform_tex, TU_tex);
        glUniform1f(uniform_time, time);
        glUniform1f(uniform_transparency, transparency);
    }

    TransparentShader::TransparentShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/transparent.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 0;

        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    TransparentShader *TransparentShaderInstance;

    TransparentFogShader::TransparentFogShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/transparentfog.frag").c_str());
        AssignUniforms("ModelMatrix", "TextureMatrix", "fogmax", "startH", "endH", "start", "end", "col");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
        TU_tex = 0;

        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));
    }

    TransparentFogShader *TransparentFogShaderInstance;

    GLuint BillboardShader::Program;
    GLuint BillboardShader::attrib_corner;
    GLuint BillboardShader::attrib_texcoord;
    GLuint BillboardShader::uniform_MV;
    GLuint BillboardShader::uniform_P;
    GLuint BillboardShader::uniform_tex;
    GLuint BillboardShader::uniform_Position;
    GLuint BillboardShader::uniform_Size;

    void BillboardShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/billboard.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/billboard.frag").c_str());
        attrib_corner = glGetAttribLocation(Program, "Corner");
        attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
        uniform_MV = glGetUniformLocation(Program, "ModelViewMatrix");
        uniform_P = glGetUniformLocation(Program, "ProjectionMatrix");
        uniform_Position = glGetUniformLocation(Program, "Position");
        uniform_Size = glGetUniformLocation(Program, "Size");
        uniform_tex = glGetUniformLocation(Program, "tex");
    }

    void BillboardShader::setUniforms(const core::matrix4 &ModelViewMatrix,
                                      const core::matrix4 &ProjectionMatrix,
                                      const core::vector3df &Position,
                                      const core::dimension2d<float> &size,
                                      unsigned TU_tex)
    {
        glUniformMatrix4fv(uniform_MV, 1, GL_FALSE, ModelViewMatrix.pointer());
        glUniformMatrix4fv(uniform_P, 1, GL_FALSE, ProjectionMatrix.pointer());
        glUniform3f(uniform_Position, Position.X, Position.Y, Position.Z);
        glUniform2f(uniform_Size, size.Width, size.Height);
        glUniform1i(uniform_tex, TU_tex);
    }

    GLuint ColorizeShader::Program;
    GLuint ColorizeShader::uniform_MM;
    GLuint ColorizeShader::uniform_col;

    void ColorizeShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/colorize.frag").c_str());
        uniform_MM = glGetUniformLocation(Program, "ModelMatrix");
        uniform_col = glGetUniformLocation(Program, "col");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void ColorizeShader::setUniforms(const core::matrix4 &ModelMatrix, float r, float g, float b)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniformMatrix4fv(uniform_MM, 1, GL_FALSE, ModelMatrix.pointer());
        glUniform3f(uniform_col, r, g, b);
    }

    ShadowShader::ShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        AssignUniforms("ModelMatrix");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    ShadowShader *ShadowShaderInstance;

    GLuint RSMShader::Program;
    GLuint RSMShader::uniform_MM;
    GLuint RSMShader::uniform_RSMMatrix;
    GLuint RSMShader::TU_tex;

    void RSMShader::init()
    {
        if (irr_driver->getGLSLVersion() < 150)
            return;
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/rsm.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rsm.frag").c_str());
        uniform_MM = glGetUniformLocation(Program, "ModelMatrix");
        uniform_RSMMatrix = glGetUniformLocation(Program, "RSMMatrix");
        AssignTextureUnit(Program, { TexUnit(TU_tex, "tex") });
    }

    void RSMShader::setUniforms(const core::matrix4 &RSMMatrix, const core::matrix4 &ModelMatrix)
    {
        glUniformMatrix4fv(uniform_RSMMatrix, 1, GL_FALSE, RSMMatrix.pointer());
        glUniformMatrix4fv(uniform_MM, 1, GL_FALSE, ModelMatrix.pointer());
    }

    InstancedShadowShader::InstancedShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        }
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    InstancedShadowShader *InstancedShadowShaderInstance;

    RefShadowShader::RefShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        AssignUniforms("ModelMatrix");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        TU_tex = 0;

        AssignTextureUnit(Program, { TexUnit(TU_tex, "tex") });
    }

    RefShadowShader *RefShadowShaderInstance;

    InstancedRefShadowShader::InstancedRefShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        TU_tex = 0;
        AssignTextureUnit(Program, { TexUnit(TU_tex, "tex") });
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    InstancedRefShadowShader *InstancedRefShadowShaderInstance;

    GrassShadowShader::GrassShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow_grass.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/shadow_grass.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        AssignUniforms("ModelMatrix", "windDir");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        TU_tex = 0;

        AssignTextureUnit(Program, { TexUnit(TU_tex, "tex") });
    }

    GrassShadowShader *GrassShadowShaderInstance;

    InstancedGrassShadowShader::InstancedGrassShadowShader()
    {
        // Geometry shader needed
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedgrassshadow.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/utils/getworldmatrix.vert").c_str(),
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/instanciedgrassshadow.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/shadow.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/object_unlit.frag").c_str());
        }
        TU_tex = 0;
        AssignTextureUnit(Program, TexUnit(TU_tex, "tex"));

        AssignUniforms("windDir");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    InstancedGrassShadowShader *InstancedGrassShadowShaderInstance;

    DisplaceMaskShader::DisplaceMaskShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/displace.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/white.frag").c_str());
        AssignUniforms("ModelMatrix");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    DisplaceMaskShader *DisplaceMaskShaderInstance;


    DisplaceShader::DisplaceShader()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/displace.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/displace.frag").c_str());
        AssignUniforms("ModelMatrix", "dir", "dir2");
        TU_displacement_tex = 0;
        TU_color_tex = 1;
        TU_mask_tex = 2;
        TU_tex = 3;
        AssignTextureUnit(Program,
            TexUnit(TU_displacement_tex, "displacement_tex"),
            TexUnit(TU_color_tex, "color_tex"),
            TexUnit(TU_mask_tex, "mask_tex"),
            TexUnit(TU_tex, "tex")
        );
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    DisplaceShader *DisplaceShaderInstance;

    GLuint SkyboxShader::Program;
    GLuint SkyboxShader::attrib_position;
    GLuint SkyboxShader::uniform_MM;
    GLuint SkyboxShader::uniform_tex;
    GLuint SkyboxShader::cubevao;

    void SkyboxShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/object_pass.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sky.frag").c_str());
        attrib_position = glGetAttribLocation(Program, "Position");
        uniform_MM = glGetUniformLocation(Program, "ModelMatrix");
        uniform_tex = glGetUniformLocation(Program, "tex");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }

        glGenVertexArrays(1, &cubevao);
        glBindVertexArray(cubevao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedObject::cubevbo);
        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SharedObject::cubeindexes);
        glBindVertexArray(0);
    }

    void SkyboxShader::setUniforms(const core::matrix4 &ModelMatrix, const core::vector2df &screen, unsigned TU_tex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniformMatrix4fv(uniform_MM, 1, GL_FALSE, ModelMatrix.pointer());
        glUniform1i(uniform_tex, TU_tex);
    }

    GLuint ViewFrustrumShader::Program;
    GLuint ViewFrustrumShader::attrib_position;
    GLuint ViewFrustrumShader::uniform_color;
    GLuint ViewFrustrumShader::uniform_idx;
    GLuint ViewFrustrumShader::frustrumvao;

    void ViewFrustrumShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/frustrum.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        attrib_position = glGetAttribLocation(Program, "Position");
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
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

    GLuint PointLightShader::Program;
    GLuint PointLightShader::attrib_Position;
    GLuint PointLightShader::attrib_Color;
    GLuint PointLightShader::attrib_Energy;
    GLuint PointLightShader::attrib_Radius;
    GLuint PointLightShader::uniform_ntex;
    GLuint PointLightShader::uniform_dtex;
    GLuint PointLightShader::uniform_spec;
    GLuint PointLightShader::vbo;
    GLuint PointLightShader::vao;

    void PointLightShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/pointlight.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/pointlight.frag").c_str());
        attrib_Position = glGetAttribLocation(Program, "Position");
        attrib_Color = glGetAttribLocation(Program, "Color");
        attrib_Energy = glGetAttribLocation(Program, "Energy");
        attrib_Radius = glGetAttribLocation(Program, "Radius");
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_spec = glGetUniformLocation(Program, "spec");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, MAXLIGHT * sizeof(PointLightInfo), 0, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(attrib_Position);
        glVertexAttribPointer(attrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), 0);
        glEnableVertexAttribArray(attrib_Energy);
        glVertexAttribPointer(attrib_Energy, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Color);
        glVertexAttribPointer(attrib_Color, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(4 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Radius);
        glVertexAttribPointer(attrib_Radius, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInfo), (GLvoid*)(7 * sizeof(float)));

        glVertexAttribDivisor(attrib_Position, 1);
        glVertexAttribDivisor(attrib_Energy, 1);
        glVertexAttribDivisor(attrib_Color, 1);
        glVertexAttribDivisor(attrib_Radius, 1);
    }

    void PointLightShader::setUniforms(const core::vector2df &screen, unsigned spec, unsigned TU_ntex, unsigned TU_dtex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform1f(uniform_spec, 200);

        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
    }

}


namespace ParticleShader
{
    GLuint SimpleSimulationShader::Program;
    GLuint SimpleSimulationShader::attrib_position;
    GLuint SimpleSimulationShader::attrib_velocity;
    GLuint SimpleSimulationShader::attrib_lifetime;
    GLuint SimpleSimulationShader::attrib_initial_position;
    GLuint SimpleSimulationShader::attrib_initial_velocity;
    GLuint SimpleSimulationShader::attrib_initial_lifetime;
    GLuint SimpleSimulationShader::attrib_size;
    GLuint SimpleSimulationShader::attrib_initial_size;
    GLuint SimpleSimulationShader::uniform_sourcematrix;
    GLuint SimpleSimulationShader::uniform_dt;
    GLuint SimpleSimulationShader::uniform_level;
    GLuint SimpleSimulationShader::uniform_size_increase_factor;

    void SimpleSimulationShader::init()
    {
        const char *varyings[] = {
            "new_particle_position",
            "new_lifetime",
            "new_particle_velocity",
            "new_size",
        };
        Program = LoadTFBProgram(file_manager->getAsset("shaders/pointemitter.vert").c_str(), varyings, 4);

        uniform_dt = glGetUniformLocation(Program, "dt");
        uniform_sourcematrix = glGetUniformLocation(Program, "sourcematrix");
        uniform_level = glGetUniformLocation(Program, "level");
        uniform_size_increase_factor = glGetUniformLocation(Program, "size_increase_factor");

        attrib_position = glGetAttribLocation(Program, "particle_position");
        attrib_lifetime = glGetAttribLocation(Program, "lifetime");
        attrib_velocity = glGetAttribLocation(Program, "particle_velocity");
        attrib_size = glGetAttribLocation(Program, "size");
        attrib_initial_position = glGetAttribLocation(Program, "particle_position_initial");
        attrib_initial_lifetime = glGetAttribLocation(Program, "lifetime_initial");
        attrib_initial_velocity = glGetAttribLocation(Program, "particle_velocity_initial");
        attrib_initial_size = glGetAttribLocation(Program, "size_initial");
    }

    GLuint HeightmapSimulationShader::Program;
    GLuint HeightmapSimulationShader::attrib_position;
    GLuint HeightmapSimulationShader::attrib_velocity;
    GLuint HeightmapSimulationShader::attrib_lifetime;
    GLuint HeightmapSimulationShader::attrib_initial_position;
    GLuint HeightmapSimulationShader::attrib_initial_velocity;
    GLuint HeightmapSimulationShader::attrib_initial_lifetime;
    GLuint HeightmapSimulationShader::attrib_size;
    GLuint HeightmapSimulationShader::attrib_initial_size;
    GLuint HeightmapSimulationShader::uniform_sourcematrix;
    GLuint HeightmapSimulationShader::uniform_dt;
    GLuint HeightmapSimulationShader::uniform_level;
    GLuint HeightmapSimulationShader::uniform_size_increase_factor;
    GLuint HeightmapSimulationShader::uniform_track_x;
    GLuint HeightmapSimulationShader::uniform_track_z;
    GLuint HeightmapSimulationShader::uniform_track_x_len;
    GLuint HeightmapSimulationShader::uniform_track_z_len;
    GLuint HeightmapSimulationShader::uniform_heightmap;

    void HeightmapSimulationShader::init()
    {
        const char *varyings[] = {
            "new_particle_position",
            "new_lifetime",
            "new_particle_velocity",
            "new_size",
        };
        Program = LoadTFBProgram(file_manager->getAsset("shaders/particlesimheightmap.vert").c_str(), varyings, 4);

        uniform_dt = glGetUniformLocation(Program, "dt");
        uniform_sourcematrix = glGetUniformLocation(Program, "sourcematrix");
        uniform_level = glGetUniformLocation(Program, "level");
        uniform_size_increase_factor = glGetUniformLocation(Program, "size_increase_factor");

        attrib_position = glGetAttribLocation(Program, "particle_position");
        attrib_lifetime = glGetAttribLocation(Program, "lifetime");
        attrib_velocity = glGetAttribLocation(Program, "particle_velocity");
        attrib_size = glGetAttribLocation(Program, "size");
        attrib_initial_position = glGetAttribLocation(Program, "particle_position_initial");
        attrib_initial_lifetime = glGetAttribLocation(Program, "lifetime_initial");
        attrib_initial_velocity = glGetAttribLocation(Program, "particle_velocity_initial");
        attrib_initial_size = glGetAttribLocation(Program, "size_initial");

        uniform_heightmap = glGetUniformLocation(Program, "heightmap");
        uniform_track_x = glGetUniformLocation(Program, "track_x");
        uniform_track_x_len = glGetUniformLocation(Program, "track_x_len");
        uniform_track_z = glGetUniformLocation(Program, "track_z");
        uniform_track_z_len = glGetUniformLocation(Program, "track_z_len");
    }

    GLuint SimpleParticleRender::Program;
    GLuint SimpleParticleRender::attrib_pos;
    GLuint SimpleParticleRender::attrib_lf;
    GLuint SimpleParticleRender::attrib_quadcorner;
    GLuint SimpleParticleRender::attrib_texcoord;
    GLuint SimpleParticleRender::attrib_sz;
    GLuint SimpleParticleRender::uniform_matrix;
    GLuint SimpleParticleRender::uniform_viewmatrix;
    GLuint SimpleParticleRender::uniform_tex;
    GLuint SimpleParticleRender::uniform_dtex;
    GLuint SimpleParticleRender::uniform_invproj;
    GLuint SimpleParticleRender::uniform_color_from;
    GLuint SimpleParticleRender::uniform_color_to;

    void SimpleParticleRender::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/particle.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/particle.frag").c_str());
        attrib_pos = glGetAttribLocation(Program, "position");
        attrib_sz = glGetAttribLocation(Program, "size");
        attrib_lf = glGetAttribLocation(Program, "lifetime");
        attrib_quadcorner = glGetAttribLocation(Program, "quadcorner");
        attrib_texcoord = glGetAttribLocation(Program, "texcoord");


        uniform_matrix = glGetUniformLocation(Program, "ProjectionMatrix");
        uniform_viewmatrix = glGetUniformLocation(Program, "ViewMatrix");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_invproj = glGetUniformLocation(Program, "invproj");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_color_from = glGetUniformLocation(Program, "color_from");
        assert(uniform_color_from != -1);
        uniform_color_to = glGetUniformLocation(Program, "color_to");
        assert(uniform_color_to != -1);
    }

    void SimpleParticleRender::setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix,
                                           const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_dtex,
                                           const ParticleSystemProxy* particle_system)
    {
        glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
        glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());
        glUniformMatrix4fv(uniform_viewmatrix, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());
        glUniform1i(uniform_tex, TU_tex);
        glUniform1i(uniform_dtex, TU_dtex);

        const float* color_from = particle_system->getColorFrom();
        const float* color_to = particle_system->getColorTo();
        glUniform3f(uniform_color_from, color_from[0], color_from[1], color_from[2]);
        glUniform3f(uniform_color_to, color_to[0], color_to[1], color_to[2]);
    }

    GLuint FlipParticleRender::Program;
    GLuint FlipParticleRender::attrib_pos;
    GLuint FlipParticleRender::attrib_lf;
    GLuint FlipParticleRender::attrib_quadcorner;
    GLuint FlipParticleRender::attrib_texcoord;
    GLuint FlipParticleRender::attrib_sz;
    GLuint FlipParticleRender::attrib_rotationvec;
    GLuint FlipParticleRender::attrib_anglespeed;
    GLuint FlipParticleRender::uniform_matrix;
    GLuint FlipParticleRender::uniform_viewmatrix;
    GLuint FlipParticleRender::uniform_tex;
    GLuint FlipParticleRender::uniform_dtex;
    GLuint FlipParticleRender::uniform_invproj;

    void FlipParticleRender::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/flipparticle.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/particle.frag").c_str());
        attrib_pos = glGetAttribLocation(Program, "position");
        attrib_sz = glGetAttribLocation(Program, "size");
        attrib_lf = glGetAttribLocation(Program, "lifetime");
        attrib_quadcorner = glGetAttribLocation(Program, "quadcorner");
        attrib_texcoord = glGetAttribLocation(Program, "texcoord");
        attrib_anglespeed = glGetAttribLocation(Program, "anglespeed");
        attrib_rotationvec = glGetAttribLocation(Program, "rotationvec");

        uniform_matrix = glGetUniformLocation(Program, "ProjectionMatrix");
        uniform_viewmatrix = glGetUniformLocation(Program, "ViewMatrix");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_invproj = glGetUniformLocation(Program, "invproj");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
    }

    void FlipParticleRender::setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix, const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_dtex)
    {
        glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
        glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());
        glUniformMatrix4fv(uniform_viewmatrix, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());
        glUniform1i(uniform_tex, TU_tex);
        glUniform1i(uniform_dtex, TU_dtex);
    }
}

static GLuint createFullScreenVAO(GLuint Program)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint attrib_position = glGetAttribLocation(Program, "Position");
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
    return vao;
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
    GLuint BloomShader::Program;
    GLuint BloomShader::uniform_texture;
    GLuint BloomShader::vao;
    void BloomShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getCIEXYZ.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bloom.frag").c_str());
        uniform_texture = glGetUniformLocation(Program, "tex");
        vao = createFullScreenVAO(Program);
    }

    void BloomShader::setUniforms(unsigned TU_tex)
    {
        glUniform1i(FullScreenShader::BloomShader::uniform_texture, TU_tex);
    }

    GLuint BloomBlendShader::Program;
    GLuint BloomBlendShader::uniform_tex_128;
    GLuint BloomBlendShader::uniform_tex_256;
    GLuint BloomBlendShader::uniform_tex_512;
    GLuint BloomBlendShader::vao;

    void BloomBlendShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bloomblend.frag").c_str());
        uniform_tex_128 = glGetUniformLocation(Program, "tex_128");
        uniform_tex_256 = glGetUniformLocation(Program, "tex_256");
        uniform_tex_512 = glGetUniformLocation(Program, "tex_512");
        vao = createFullScreenVAO(Program);
    }

    void BloomBlendShader::setUniforms(unsigned TU_tex_128, unsigned TU_tex_256, unsigned TU_tex_512)
    {
        glUniform1i(uniform_tex_128, TU_tex_128);
        glUniform1i(uniform_tex_256, TU_tex_256);
        glUniform1i(uniform_tex_512, TU_tex_512);
    }

    GLuint ToneMapShader::Program;
    GLuint ToneMapShader::uniform_tex;
    GLuint ToneMapShader::uniform_logluminancetex;
    GLuint ToneMapShader::uniform_exposure;
    GLuint ToneMapShader::uniform_lwhite;
    GLuint ToneMapShader::vao;

    void ToneMapShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getRGBfromCIEXxy.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getCIEXYZ.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/tonemap.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_logluminancetex = glGetUniformLocation(Program, "logluminancetex");
        uniform_exposure = glGetUniformLocation(Program, "exposure");
        uniform_lwhite = glGetUniformLocation(Program, "Lwhite");
        vao = createFullScreenVAO(Program);
    }

    void ToneMapShader::setUniforms(float exposure, float Lwhite, unsigned TU_tex, unsigned TU_loglum)
    {
        glUniform1i(uniform_tex, TU_tex);
        glUniform1i(uniform_logluminancetex, TU_loglum);
        glUniform1f(uniform_exposure, exposure);
        glUniform1f(uniform_lwhite, Lwhite);
    }

    GLuint DepthOfFieldShader::Program;
    GLuint DepthOfFieldShader::uniform_tex;
    GLuint DepthOfFieldShader::uniform_depth;
    GLuint DepthOfFieldShader::vao;

    void DepthOfFieldShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/dof.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_depth = glGetUniformLocation(Program, "dtex");
        vao = createFullScreenVAO(Program);
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    void DepthOfFieldShader::setUniforms(unsigned TU_tex, unsigned TU_dtex)
    {
        glUniform1i(uniform_tex, TU_tex);
        glUniform1i(uniform_depth, TU_dtex);
    }

    GLuint SunLightShader::Program;
    GLuint SunLightShader::uniform_ntex;
    GLuint SunLightShader::uniform_dtex;
    GLuint SunLightShader::uniform_direction;
    GLuint SunLightShader::uniform_col;
    GLuint SunLightShader::vao;

    void SunLightShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sunlight.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_direction = glGetUniformLocation(Program, "direction");
        uniform_col = glGetUniformLocation(Program, "col");
        vao = createFullScreenVAO(Program);
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void SunLightShader::setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform3f(uniform_direction, direction.X, direction.Y, direction.Z);
        glUniform3f(uniform_col, r, g, b);
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
    }

    GLuint DiffuseEnvMapShader::Program;
    GLuint DiffuseEnvMapShader::uniform_ntex;
    GLuint DiffuseEnvMapShader::uniform_blueLmn;
    GLuint DiffuseEnvMapShader::uniform_greenLmn;
    GLuint DiffuseEnvMapShader::uniform_redLmn;
    GLuint DiffuseEnvMapShader::uniform_TVM;
    GLuint DiffuseEnvMapShader::vao;

    void DiffuseEnvMapShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/diffuseenvmap.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_blueLmn = glGetUniformLocation(Program, "blueLmn[0]");
        uniform_greenLmn = glGetUniformLocation(Program, "greenLmn[0]");
        uniform_redLmn = glGetUniformLocation(Program, "redLmn[0]");
        uniform_TVM = glGetUniformLocation(Program, "TransposeViewMatrix");
        vao = createFullScreenVAO(Program);
    }

    void DiffuseEnvMapShader::setUniforms(const core::matrix4 &TransposeViewMatrix, const float *blueSHCoeff, const float *greenSHCoeff, const float *redSHCoeff, unsigned TU_ntex)
    {
        glUniformMatrix4fv(uniform_TVM, 1, GL_FALSE, TransposeViewMatrix.pointer());
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1fv(uniform_blueLmn, 9, blueSHCoeff);
        glUniform1fv(uniform_greenLmn, 9, greenSHCoeff);
        glUniform1fv(uniform_redLmn, 9, redSHCoeff);
    }

    GLuint ShadowedSunLightShader::Program;
    GLuint ShadowedSunLightShader::uniform_ntex;
    GLuint ShadowedSunLightShader::uniform_dtex;
    GLuint ShadowedSunLightShader::uniform_shadowtex;
    GLuint ShadowedSunLightShader::uniform_direction;
    GLuint ShadowedSunLightShader::uniform_col;
    GLuint ShadowedSunLightShader::vao;

    void ShadowedSunLightShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sunlightshadow.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_shadowtex = glGetUniformLocation(Program, "shadowtex");
        uniform_direction = glGetUniformLocation(Program, "direction");
        uniform_col = glGetUniformLocation(Program, "col");
        vao = createFullScreenVAO(Program);
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void ShadowedSunLightShader::setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_shadowtex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform3f(uniform_direction, direction.X, direction.Y, direction.Z);
        glUniform3f(uniform_col, r, g, b);
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
        glUniform1i(uniform_shadowtex, TU_shadowtex);
    }

    GLuint ShadowedSunLightDebugShader::Program;
    GLuint ShadowedSunLightDebugShader::uniform_ntex;
    GLuint ShadowedSunLightDebugShader::uniform_dtex;
    GLuint ShadowedSunLightDebugShader::uniform_shadowtex;
    GLuint ShadowedSunLightDebugShader::uniform_direction;
    GLuint ShadowedSunLightDebugShader::uniform_col;
    GLuint ShadowedSunLightDebugShader::vao;

    void ShadowedSunLightDebugShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getSpecular.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/sunlightshadowdebug.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_shadowtex = glGetUniformLocation(Program, "shadowtex");
        uniform_direction = glGetUniformLocation(Program, "direction");
        uniform_col = glGetUniformLocation(Program, "col");
        vao = createVAO(Program);
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void ShadowedSunLightDebugShader::setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_shadowtex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform3f(uniform_direction, direction.X, direction.Y, direction.Z);
        glUniform3f(uniform_col, r, g, b);
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
        glUniform1i(uniform_shadowtex, TU_shadowtex);
    }

    GLuint RadianceHintsConstructionShader::Program;
    GLuint RadianceHintsConstructionShader::uniform_ctex;
    GLuint RadianceHintsConstructionShader::uniform_ntex;
    GLuint RadianceHintsConstructionShader::uniform_dtex;
    GLuint RadianceHintsConstructionShader::uniform_extents;
    GLuint RadianceHintsConstructionShader::uniform_RHMatrix;
    GLuint RadianceHintsConstructionShader::uniform_RSMMatrix;
    GLuint RadianceHintsConstructionShader::vao;

    void RadianceHintsConstructionShader::init()
    {
        if (irr_driver->getGLSLVersion() < 150)
            return;
        if (irr_driver->hasVSLayerExtension())
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/slicedscreenquad.vert").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rh.frag").c_str());
        }
        else
        {
            Program = LoadProgram(
                GL_VERTEX_SHADER, file_manager->getAsset("shaders/slicedscreenquad.vert").c_str(),
                GL_GEOMETRY_SHADER, file_manager->getAsset("shaders/rhpassthrough.geom").c_str(),
                GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rh.frag").c_str());
        }

        uniform_ctex = glGetUniformLocation(Program, "ctex");
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_extents = glGetUniformLocation(Program, "extents");
        uniform_RHMatrix = glGetUniformLocation(Program, "RHMatrix");
        uniform_RSMMatrix = glGetUniformLocation(Program, "RSMMatrix");
        vao = createFullScreenVAO(Program);
    }

    void RadianceHintsConstructionShader::setUniforms(const core::matrix4 &RSMMatrix, const core::matrix4 &RHMatrix, const core::vector3df &extents, unsigned TU_ctex, unsigned TU_ntex, unsigned TU_dtex)
    {
        glUniformMatrix4fv(uniform_RSMMatrix, 1, GL_FALSE, RSMMatrix.pointer());
        glUniformMatrix4fv(uniform_RHMatrix, 1, GL_FALSE, RHMatrix.pointer());
        glUniform1i(uniform_ctex, TU_ctex);
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
        glUniform3f(uniform_extents, extents.X, extents.Y, extents.Z);
    }

    GLuint RHDebug::Program;
    GLuint RHDebug::uniform_extents;
    GLuint RHDebug::uniform_SHR;
    GLuint RHDebug::uniform_SHG;
    GLuint RHDebug::uniform_SHB;
    GLuint RHDebug::uniform_RHMatrix;

    void RHDebug::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/rhdebug.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/rhdebug.frag").c_str());
        uniform_extents = glGetUniformLocation(Program, "extents");
        uniform_SHR = glGetUniformLocation(Program, "SHR");
        uniform_SHG = glGetUniformLocation(Program, "SHG");
        uniform_SHB = glGetUniformLocation(Program, "SHB");
        uniform_RHMatrix = glGetUniformLocation(Program, "RHMatrix");
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    void RHDebug::setUniforms(const core::matrix4 &RHMatrix, const core::vector3df &extents, unsigned TU_SHR, unsigned TU_SHG, unsigned TU_SHB)
    {
        glUniformMatrix4fv(uniform_RHMatrix, 1, GL_FALSE, RHMatrix.pointer());
        glUniform3f(uniform_extents, extents.X, extents.Y, extents.Z);
        glUniform1i(uniform_SHR, TU_SHR);
        glUniform1i(uniform_SHG, TU_SHG);
        glUniform1i(uniform_SHB, TU_SHB);
    }

    GLuint GlobalIlluminationReconstructionShader::Program;
    GLuint GlobalIlluminationReconstructionShader::uniform_ntex;
    GLuint GlobalIlluminationReconstructionShader::uniform_dtex;
    GLuint GlobalIlluminationReconstructionShader::uniform_SHR;
    GLuint GlobalIlluminationReconstructionShader::uniform_SHG;
    GLuint GlobalIlluminationReconstructionShader::uniform_SHB;
    GLuint GlobalIlluminationReconstructionShader::uniform_extents;
    GLuint GlobalIlluminationReconstructionShader::uniform_RHMatrix;
    GLuint GlobalIlluminationReconstructionShader::uniform_InvRHMatrix;
    GLuint GlobalIlluminationReconstructionShader::vao;

    void GlobalIlluminationReconstructionShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gi.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_SHR = glGetUniformLocation(Program, "SHR");
        uniform_SHG = glGetUniformLocation(Program, "SHG");
        uniform_SHB = glGetUniformLocation(Program, "SHB");
        uniform_RHMatrix = glGetUniformLocation(Program, "RHMatrix");
        uniform_InvRHMatrix = glGetUniformLocation(Program, "InvRHMatrix");
        uniform_extents = glGetUniformLocation(Program, "extents");
        vao = createFullScreenVAO(Program);
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    void GlobalIlluminationReconstructionShader::setUniforms(const core::matrix4 &RHMatrix, const core::matrix4 &InvRHMatrix, const core::vector3df &extents, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_SHR, unsigned TU_SHG, unsigned TU_SHB)
    {
        glUniformMatrix4fv(uniform_RHMatrix, 1, GL_FALSE, RHMatrix.pointer());
        glUniformMatrix4fv(uniform_InvRHMatrix, 1, GL_FALSE, InvRHMatrix.pointer());
        glUniform1i(uniform_ntex, TU_ntex);
        glUniform1i(uniform_dtex, TU_dtex);
        glUniform1i(uniform_SHR, TU_SHR);
        glUniform1i(uniform_SHG, TU_SHG);
        glUniform1i(uniform_SHB, TU_SHB);
        glUniform3f(uniform_extents, extents.X, extents.Y, extents.Z);
    }

    GLuint Gaussian17TapHShader::Program;
    GLuint Gaussian17TapHShader::uniform_tex;
    GLuint Gaussian17TapHShader::uniform_depth;
    GLuint Gaussian17TapHShader::uniform_pixel;
    GLuint Gaussian17TapHShader::vao;
    void Gaussian17TapHShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bilateralH.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        uniform_depth = glGetUniformLocation(Program, "depth");
        vao = createFullScreenVAO(Program);
    }

    GLuint ComputeGaussian17TapHShader::Program;
    GLuint ComputeGaussian17TapHShader::uniform_source;
    GLuint ComputeGaussian17TapHShader::uniform_depth;
    GLuint ComputeGaussian17TapHShader::uniform_dest;
    void ComputeGaussian17TapHShader::init()
    {
#if WIN32
        if (irr_driver->getGLSLVersion() < 420)
            return;
        Program = LoadProgram(
            GL_COMPUTE_SHADER, file_manager->getAsset("shaders/bilateralH.comp").c_str());
        uniform_source = glGetUniformLocation(Program, "source");
        uniform_depth = glGetUniformLocation(Program, "depth");
        uniform_dest = glGetUniformLocation(Program, "dest");
#endif
    }

    GLuint Gaussian6HBlurShader::Program;
    GLuint Gaussian6HBlurShader::uniform_tex;
    GLuint Gaussian6HBlurShader::uniform_pixel;
    GLuint Gaussian6HBlurShader::vao;
    void Gaussian6HBlurShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian6h.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        vao = createFullScreenVAO(Program);
    }

    GLuint Gaussian3HBlurShader::Program;
    GLuint Gaussian3HBlurShader::uniform_tex;
    GLuint Gaussian3HBlurShader::uniform_pixel;
    GLuint Gaussian3HBlurShader::vao;
    void Gaussian3HBlurShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian3h.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        vao = createFullScreenVAO(Program);
    }

    GLuint Gaussian17TapVShader::Program;
    GLuint Gaussian17TapVShader::uniform_tex;
    GLuint Gaussian17TapVShader::uniform_depth;
    GLuint Gaussian17TapVShader::uniform_pixel;
    GLuint Gaussian17TapVShader::vao;
    void Gaussian17TapVShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/bilateralV.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        uniform_depth = glGetUniformLocation(Program, "depth");
        vao = createFullScreenVAO(Program);
    }

    GLuint ComputeGaussian17TapVShader::Program;
    GLuint ComputeGaussian17TapVShader::uniform_source;
    GLuint ComputeGaussian17TapVShader::uniform_depth;
    GLuint ComputeGaussian17TapVShader::uniform_dest;
    void ComputeGaussian17TapVShader::init()
    {
#if WIN32
        if (irr_driver->getGLSLVersion() < 420)
            return;
        Program = LoadProgram(
            GL_COMPUTE_SHADER, file_manager->getAsset("shaders/bilateralV.comp").c_str());
        uniform_source = glGetUniformLocation(Program, "source");
        uniform_depth = glGetUniformLocation(Program, "depth");
        uniform_dest = glGetUniformLocation(Program, "dest");
#endif
    }

    GLuint Gaussian6VBlurShader::Program;
    GLuint Gaussian6VBlurShader::uniform_tex;
    GLuint Gaussian6VBlurShader::uniform_pixel;
    GLuint Gaussian6VBlurShader::vao;
    void Gaussian6VBlurShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian6v.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        vao = createFullScreenVAO(Program);
    }

    GLuint Gaussian3VBlurShader::Program;
    GLuint Gaussian3VBlurShader::uniform_tex;
    GLuint Gaussian3VBlurShader::uniform_pixel;
    GLuint Gaussian3VBlurShader::vao;
    void Gaussian3VBlurShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/gaussian3v.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_pixel = glGetUniformLocation(Program, "pixel");
        vao = createFullScreenVAO(Program);
    }

    GLuint PassThroughShader::Program;
    GLuint PassThroughShader::uniform_texture;
    GLuint PassThroughShader::vao;
    void PassThroughShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/texturedquad.frag").c_str());
        uniform_texture = glGetUniformLocation(Program, "texture");
        vao = createVAO(Program);
    }

    GLuint LayerPassThroughShader::Program;
    GLuint LayerPassThroughShader::uniform_texture;
    GLuint LayerPassThroughShader::uniform_layer;
    GLuint LayerPassThroughShader::vao;
    void LayerPassThroughShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/layertexturequad.frag").c_str());
        uniform_texture = glGetUniformLocation(Program, "tex");
        uniform_layer = glGetUniformLocation(Program, "layer");
        vao = createVAO(Program);
    }

    GLuint LinearizeDepthShader::Program;
    GLuint LinearizeDepthShader::uniform_zn;
    GLuint LinearizeDepthShader::uniform_zf;
    GLuint LinearizeDepthShader::uniform_texture;
    GLuint LinearizeDepthShader::vao;
    void LinearizeDepthShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/linearizedepth.frag").c_str());
        uniform_texture = glGetUniformLocation(Program, "texture");
        uniform_zf = glGetUniformLocation(Program, "zf");
        uniform_zn = glGetUniformLocation(Program, "zn");
        vao = createFullScreenVAO(Program);
    }

    void LinearizeDepthShader::setUniforms(float zn, float zf, unsigned TU_tex)
    {
        glUniform1f(uniform_zn, zn);
        glUniform1f(uniform_zf, zf);
        glUniform1i(uniform_texture, TU_tex);
    }

    GLuint GlowShader::Program;
    GLuint GlowShader::uniform_tex;
    GLuint GlowShader::vao;
    void GlowShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/glow.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        vao = createVAO(Program);
    }

    GLuint SSAOShader::Program;
    GLuint SSAOShader::uniform_ntex;
    GLuint SSAOShader::uniform_dtex;
    GLuint SSAOShader::uniform_noise_texture;
    GLuint SSAOShader::uniform_samplePoints;
    GLuint SSAOShader::vao;
    float SSAOShader::SSAOSamples[64];

    void SSAOShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/decodeNormal.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/ssao.frag").c_str());
        uniform_ntex = glGetUniformLocation(Program, "ntex");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_noise_texture = glGetUniformLocation(Program, "noise_texture");
        uniform_samplePoints = glGetUniformLocation(Program, "samplePoints[0]");
        vao = createFullScreenVAO(Program);
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }

        // SSAOSamples[4 * i] and SSAOSamples[4 * i + 1] can be negative

        SSAOSamples[0] = 0.135061f;
        SSAOSamples[1] = 0.207948f;
        SSAOSamples[2] = 0.968770f;
        SSAOSamples[3] = 0.983032f;

        SSAOSamples[4] = 0.273456f;
        SSAOSamples[5] = -0.805390f;
        SSAOSamples[6] = 0.525898f;
        SSAOSamples[7] = 0.942808f;

        SSAOSamples[8] = 0.443450f;
        SSAOSamples[9] = -0.803786f;
        SSAOSamples[10] = 0.396585f;
        SSAOSamples[11] = 0.007996f;

        SSAOSamples[12] = 0.742420f;
        SSAOSamples[13] = -0.620072f;
        SSAOSamples[14] = 0.253621f;
        SSAOSamples[15] = 0.284829f;

        SSAOSamples[16] = 0.892464f;
        SSAOSamples[17] = 0.046221f;
        SSAOSamples[18] = 0.448744f;
        SSAOSamples[19] = 0.753655f;

        SSAOSamples[20] = 0.830350f;
        SSAOSamples[21] = -0.043593f;
        SSAOSamples[22] = 0.555535f;
        SSAOSamples[23] = 0.357463f;

        SSAOSamples[24] = -0.600612f;
        SSAOSamples[25] = -0.536421f;
        SSAOSamples[26] = 0.592889f;
        SSAOSamples[27] = 0.670583f;

        SSAOSamples[28] = -0.280658f;
        SSAOSamples[29] = 0.674894f;
        SSAOSamples[30] = 0.682458f;
        SSAOSamples[31] = 0.553362f;

        SSAOSamples[32] = -0.654493f;
        SSAOSamples[33] = -0.140866f;
        SSAOSamples[34] = 0.742830f;
        SSAOSamples[35] = 0.699820f;

        SSAOSamples[36] = 0.114730f;
        SSAOSamples[37] = 0.873130f;
        SSAOSamples[38] = 0.473794f;
        SSAOSamples[39] = 0.483901f;

        SSAOSamples[40] = 0.699167f;
        SSAOSamples[41] = 0.632210f;
        SSAOSamples[42] = 0.333879f;
        SSAOSamples[43] = 0.010956f;

        SSAOSamples[44] = 0.904603f;
        SSAOSamples[45] = 0.393410f;
        SSAOSamples[46] = 0.164080f;
        SSAOSamples[47] = 0.780297f;

        SSAOSamples[48] = 0.631662f;
        SSAOSamples[49] = -0.405195f;
        SSAOSamples[50] = 0.660924f;
        SSAOSamples[51] = 0.865596f;

        SSAOSamples[52] = -0.195668f;
        SSAOSamples[53] = 0.629185f;
        SSAOSamples[54] = 0.752223f;
        SSAOSamples[55] = 0.019013f;

        SSAOSamples[56] = -0.511316f;
        SSAOSamples[57] = 0.635504f;
        SSAOSamples[58] = 0.578524f;
        SSAOSamples[59] = 0.605457f;

        SSAOSamples[60] = -0.898843f;
        SSAOSamples[61] = 0.067382f;
        SSAOSamples[62] = 0.433061f;
        SSAOSamples[63] = 0.772942f;

        // Generate another random distribution, if needed
        /*      for (unsigned i = 0; i < 16; i++) {
        // Use double to avoid denorm and get a true uniform distribution
        // Generate z component between [0.1; 1] to avoid being too close from surface
        double z = rand();
        z /= RAND_MAX;
        z = 0.1 + 0.9 * z;

        // Now generate x,y on the unit circle
        double x = rand();
        x /= RAND_MAX;
        x = 2 * x - 1;
        double y = rand();
        y /= RAND_MAX;
        y = 2 * y - 1;
        double xynorm = sqrt(x * x + y * y);
        x /= xynorm;
        y /= xynorm;
        // Now resize x,y so that norm(x,y,z) is one
        x *= sqrt(1. - z * z);
        y *= sqrt(1. - z * z);

        // Norm factor
        double w = rand();
        w /= RAND_MAX;
        SSAOSamples[4 * i] = (float)x;
        SSAOSamples[4 * i + 1] = (float)y;
        SSAOSamples[4 * i + 2] = (float)z;
        SSAOSamples[4 * i + 3] = (float)w;
        }*/
    }

    void SSAOShader::setUniforms(const core::vector2df &screen, unsigned TU_dtex, unsigned TU_noise)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform4fv(uniform_samplePoints, 16, SSAOSamples);

        glUniform1i(uniform_dtex, TU_dtex);
        glUniform1i(uniform_noise_texture, TU_noise);
    }

    GLuint FogShader::Program;
    GLuint FogShader::uniform_tex;
    GLuint FogShader::uniform_fogmax;
    GLuint FogShader::uniform_startH;
    GLuint FogShader::uniform_endH;
    GLuint FogShader::uniform_start;
    GLuint FogShader::uniform_end;
    GLuint FogShader::uniform_col;
    GLuint FogShader::vao;

    void FogShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/fog.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_fogmax = glGetUniformLocation(Program, "fogmax");
        uniform_startH = glGetUniformLocation(Program, "startH");
        uniform_endH = glGetUniformLocation(Program, "endH");
        uniform_start = glGetUniformLocation(Program, "start");
        uniform_end = glGetUniformLocation(Program, "end");
        uniform_col = glGetUniformLocation(Program, "col");
        vao = createFullScreenVAO(Program);
        if (!UserConfigParams::m_ubo_disabled)
        {
            GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
        }
    }

    void FogShader::setUniforms(float fogmax, float startH, float endH, float start, float end, const core::vector3df &col, unsigned TU_ntex)
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        glUniform1f(uniform_fogmax, fogmax);
        glUniform1f(uniform_startH, startH);
        glUniform1f(uniform_endH, endH);
        glUniform1f(uniform_start, start);
        glUniform1f(uniform_end, end);
        glUniform3f(uniform_col, col.X, col.Y, col.Z);
        glUniform1i(uniform_tex, TU_ntex);
    }

    GLuint MotionBlurShader::Program;
    GLuint MotionBlurShader::uniform_boost_amount;
    GLuint MotionBlurShader::uniform_center;
    GLuint MotionBlurShader::uniform_color_buffer;
    GLuint MotionBlurShader::uniform_dtex;
    GLuint MotionBlurShader::uniform_previous_viewproj;
    GLuint MotionBlurShader::uniform_mask_radius;
    GLuint MotionBlurShader::vao;

    void MotionBlurShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/utils/getPosFromUVDepth.frag").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/motion_blur.frag").c_str());
        uniform_boost_amount = glGetUniformLocation(Program, "boost_amount");
        uniform_center = glGetUniformLocation(Program, "center");
        uniform_color_buffer = glGetUniformLocation(Program, "color_buffer");
        uniform_mask_radius = glGetUniformLocation(Program, "mask_radius");
        uniform_dtex = glGetUniformLocation(Program, "dtex");
        uniform_previous_viewproj = glGetUniformLocation(Program, "previous_viewproj");
        vao = createFullScreenVAO(Program);
    }

    void MotionBlurShader::setUniforms(float boost_amount, const core::matrix4 &previousVP, const core::vector2df &center, float mask_radius, unsigned TU_cb, unsigned TU_dtex)
    {
        glUniformMatrix4fv(uniform_previous_viewproj, 1, GL_FALSE, previousVP.pointer());
        glUniform1f(uniform_boost_amount, boost_amount);
        glUniform2f(uniform_center, center.X, center.Y);
        glUniform1f(uniform_mask_radius, mask_radius);
        glUniform1i(uniform_color_buffer, TU_cb);
        glUniform1i(uniform_dtex, TU_dtex);
    }

    GLuint GodFadeShader::Program;
    GLuint GodFadeShader::uniform_tex;
    GLuint GodFadeShader::uniform_col;
    GLuint GodFadeShader::vao;

    void GodFadeShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/godfade.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_col = glGetUniformLocation(Program, "col");
        vao = createVAO(Program);
    }

    void GodFadeShader::setUniforms(const SColor &col, unsigned TU_tex)
    {
        glUniform3f(uniform_col, col.getRed() / 255.f, col.getGreen() / 255.f, col.getBlue() / 255.f);
        glUniform1i(uniform_tex, TU_tex);
    }

    GLuint GodRayShader::Program;
    GLuint GodRayShader::uniform_tex;
    GLuint GodRayShader::uniform_sunpos;
    GLuint GodRayShader::vao;

    void GodRayShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/godray.frag").c_str());
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_sunpos = glGetUniformLocation(Program, "sunpos");
        vao = createVAO(Program);
    }

    void GodRayShader::setUniforms(const core::vector2df &sunpos, unsigned TU_tex)
    {
        glUniform2f(uniform_sunpos, sunpos.X, sunpos.Y);
        glUniform1i(uniform_tex, TU_tex);
    }

    GLuint MLAAColorEdgeDetectionSHader::Program;
    GLuint MLAAColorEdgeDetectionSHader::uniform_colorMapG;
    GLuint MLAAColorEdgeDetectionSHader::uniform_PIXEL_SIZE;
    GLuint MLAAColorEdgeDetectionSHader::vao;

    void MLAAColorEdgeDetectionSHader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/mlaa_offset.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_color1.frag").c_str());
        uniform_colorMapG = glGetUniformLocation(Program, "colorMapG");
        uniform_PIXEL_SIZE = glGetUniformLocation(Program, "PIXEL_SIZE");
        vao = createVAO(Program);
    }

    void MLAAColorEdgeDetectionSHader::setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_colorMapG)
    {
        glUniform1i(uniform_colorMapG, TU_colorMapG);
        glUniform2f(uniform_PIXEL_SIZE, PIXEL_SIZE.X, PIXEL_SIZE.Y);
    }

    GLuint MLAABlendWeightSHader::Program;
    GLuint MLAABlendWeightSHader::uniform_edgesMap;
    GLuint MLAABlendWeightSHader::uniform_areaMap;
    GLuint MLAABlendWeightSHader::uniform_PIXEL_SIZE;
    GLuint MLAABlendWeightSHader::vao;

    void MLAABlendWeightSHader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/screenquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_blend2.frag").c_str());
        uniform_edgesMap = glGetUniformLocation(Program, "edgesMap");
        uniform_areaMap = glGetUniformLocation(Program, "areaMap");
        uniform_PIXEL_SIZE = glGetUniformLocation(Program, "PIXEL_SIZE");
        vao = createVAO(Program);
    }

    void MLAABlendWeightSHader::setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_edgesMap, unsigned TU_areaMap)
    {
        glUniform1i(uniform_edgesMap, TU_edgesMap);
        glUniform1i(uniform_areaMap, TU_areaMap);
        glUniform2f(uniform_PIXEL_SIZE, PIXEL_SIZE.X, PIXEL_SIZE.Y);
    }

    GLuint MLAAGatherSHader::Program;
    GLuint MLAAGatherSHader::uniform_colorMap;
    GLuint MLAAGatherSHader::uniform_blendMap;
    GLuint MLAAGatherSHader::uniform_PIXEL_SIZE;
    GLuint MLAAGatherSHader::vao;

    void MLAAGatherSHader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/mlaa_offset.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/mlaa_neigh3.frag").c_str());
        uniform_colorMap = glGetUniformLocation(Program, "colorMap");
        uniform_blendMap = glGetUniformLocation(Program, "blendMap");
        uniform_PIXEL_SIZE = glGetUniformLocation(Program, "PIXEL_SIZE");
        vao = createVAO(Program);
    }

    void MLAAGatherSHader::setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_colormap, unsigned TU_blendmap)
    {
        glUniform1i(uniform_colorMap, TU_colormap);
        glUniform1i(uniform_blendMap, TU_blendmap);
        glUniform2f(uniform_PIXEL_SIZE, PIXEL_SIZE.X, PIXEL_SIZE.Y);
    }
}

namespace UIShader
{
    GLuint TextureRectShader::Program;
    GLuint TextureRectShader::attrib_position;
    GLuint TextureRectShader::attrib_texcoord;
    GLuint TextureRectShader::uniform_tex;
    GLuint TextureRectShader::uniform_center;
    GLuint TextureRectShader::uniform_size;
    GLuint TextureRectShader::uniform_texcenter;
    GLuint TextureRectShader::uniform_texsize;
    GLuint TextureRectShader::vao;

    void TextureRectShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/texturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/texturedquad.frag").c_str());

        attrib_position = glGetAttribLocation(Program, "position");
        attrib_texcoord = glGetAttribLocation(Program, "texcoord");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_center = glGetUniformLocation(Program, "center");
        uniform_size = glGetUniformLocation(Program, "size");
        uniform_texcenter = glGetUniformLocation(Program, "texcenter");
        uniform_texsize = glGetUniformLocation(Program, "texsize");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(attrib_position);
        glEnableVertexAttribArray(attrib_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    void TextureRectShader::setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, unsigned TU_tex)
    {
        glUniform1i(uniform_tex, TU_tex);
        glUniform2f(uniform_center, center_pos_x, center_pos_y);
        glUniform2f(uniform_size, width, height);
        glUniform2f(uniform_texcenter, tex_center_pos_x, tex_center_pos_y);
        glUniform2f(uniform_texsize, tex_width, tex_height);
    }

    GLuint UniformColoredTextureRectShader::Program;
    GLuint UniformColoredTextureRectShader::attrib_position;
    GLuint UniformColoredTextureRectShader::attrib_texcoord;
    GLuint UniformColoredTextureRectShader::uniform_tex;
    GLuint UniformColoredTextureRectShader::uniform_color;
    GLuint UniformColoredTextureRectShader::uniform_center;
    GLuint UniformColoredTextureRectShader::uniform_size;
    GLuint UniformColoredTextureRectShader::uniform_texcenter;
    GLuint UniformColoredTextureRectShader::uniform_texsize;
    GLuint UniformColoredTextureRectShader::vao;

    void UniformColoredTextureRectShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/texturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/uniformcolortexturedquad.frag").c_str());

        attrib_position = glGetAttribLocation(Program, "position");
        attrib_texcoord = glGetAttribLocation(Program, "texcoord");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_color = glGetUniformLocation(Program, "color");
        uniform_center = glGetUniformLocation(Program, "center");
        uniform_size = glGetUniformLocation(Program, "size");
        uniform_texcenter = glGetUniformLocation(Program, "texcenter");
        uniform_texsize = glGetUniformLocation(Program, "texsize");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(attrib_position);
        glEnableVertexAttribArray(attrib_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    void UniformColoredTextureRectShader::setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, const SColor &color, unsigned TU_tex)
    {
        glUniform1i(uniform_tex, TU_tex);
        glUniform2f(uniform_center, center_pos_x, center_pos_y);
        glUniform2f(uniform_size, width, height);
        glUniform2f(uniform_texcenter, tex_center_pos_x, tex_center_pos_y);
        glUniform2f(uniform_texsize, tex_width, tex_height);
        glUniform4i(uniform_color, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
    }

    GLuint ColoredTextureRectShader::Program;
    GLuint ColoredTextureRectShader::attrib_position;
    GLuint ColoredTextureRectShader::attrib_texcoord;
    GLuint ColoredTextureRectShader::attrib_color;
    GLuint ColoredTextureRectShader::uniform_tex;
    GLuint ColoredTextureRectShader::uniform_center;
    GLuint ColoredTextureRectShader::uniform_size;
    GLuint ColoredTextureRectShader::uniform_texcenter;
    GLuint ColoredTextureRectShader::uniform_texsize;
    GLuint ColoredTextureRectShader::colorvbo;
    GLuint ColoredTextureRectShader::vao;

    void ColoredTextureRectShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/colortexturedquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/colortexturedquad.frag").c_str());

        attrib_position = glGetAttribLocation(Program, "position");
        attrib_texcoord = glGetAttribLocation(Program, "texcoord");
        attrib_color = glGetAttribLocation(Program, "color");
        uniform_tex = glGetUniformLocation(Program, "tex");
        uniform_center = glGetUniformLocation(Program, "center");
        uniform_size = glGetUniformLocation(Program, "size");
        uniform_texcenter = glGetUniformLocation(Program, "texcenter");
        uniform_texsize = glGetUniformLocation(Program, "texsize");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(attrib_position);
        glEnableVertexAttribArray(attrib_texcoord);
        glEnableVertexAttribArray(attrib_color);
        glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid *)(2 * sizeof(float)));
        const unsigned quad_color[] = {
            0, 0, 0, 255,
            255, 0, 0, 255,
            0, 255, 0, 255,
            0, 0, 255, 255,
        };
        glGenBuffers(1, &colorvbo);
        glBindBuffer(GL_ARRAY_BUFFER, colorvbo);
        glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(unsigned), quad_color, GL_DYNAMIC_DRAW);
        glVertexAttribIPointer(attrib_color, 4, GL_UNSIGNED_INT, 4 * sizeof(unsigned), 0);
        glBindVertexArray(0);
    }

    void ColoredTextureRectShader::setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, unsigned TU_tex)
    {
        glUniform1i(uniform_tex, TU_tex);
        glUniform2f(uniform_center, center_pos_x, center_pos_y);
        glUniform2f(uniform_size, width, height);
        glUniform2f(uniform_texcenter, tex_center_pos_x, tex_center_pos_y);
        glUniform2f(uniform_texsize, tex_width, tex_height);
    }

    GLuint ColoredRectShader::Program;
    GLuint ColoredRectShader::attrib_position;
    GLuint ColoredRectShader::uniform_center;
    GLuint ColoredRectShader::uniform_size;
    GLuint ColoredRectShader::uniform_color;
    GLuint ColoredRectShader::vao;

    void ColoredRectShader::init()
    {
        Program = LoadProgram(
            GL_VERTEX_SHADER, file_manager->getAsset("shaders/coloredquad.vert").c_str(),
            GL_FRAGMENT_SHADER, file_manager->getAsset("shaders/coloredquad.frag").c_str());
        attrib_position = glGetAttribLocation(Program, "position");
        uniform_color = glGetUniformLocation(Program, "color");
        uniform_center = glGetUniformLocation(Program, "center");
        uniform_size = glGetUniformLocation(Program, "size");
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glEnableVertexAttribArray(attrib_position);
        glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindVertexArray(0);
    }

    void ColoredRectShader::setUniforms(float center_pos_x, float center_pos_y, float width, float height, const video::SColor &color)
    {
        glUniform2f(uniform_center, center_pos_x, center_pos_y);
        glUniform2f(uniform_size, width, height);
        glUniform4i(uniform_color, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
    }
}
