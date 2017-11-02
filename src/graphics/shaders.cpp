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


#ifndef SERVER_ONLY

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

#include "graphics/shaders.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <assert.h>
#include <IGPUProgrammingServices.h>


bool                               Shaders::m_has_been_initialised = false;
video::IShaderConstantSetCallBack *Shaders::m_callbacks[ES_COUNT];
int                                Shaders::m_shaders[ES_COUNT];

// Use macro FOREACH_SHADER from shaders.hpp to create an array
// with all shader names.
#define STR(a) #a,
const char *Shaders::shader_names[] = { FOREACH_SHADER(STR) };  
#undef STR

using namespace video;

void Shaders::init()
{
    assert(!m_has_been_initialised);
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
    m_has_been_initialised = true;
}   // init

// ----------------------------------------------------------------------------
/** Frees all memory used by the shader manager.
 */
void Shaders::destroy()
{
    assert(m_has_been_initialised);
    u32 i;
    for (i = 0; i < ES_COUNT; i++)
    {
        if (i == ES_GAUSSIAN3V || !m_callbacks[i]) continue;
        m_callbacks[i]->drop();
        m_callbacks[i] = NULL;
    }
    m_has_been_initialised = false;
    SharedGPUObjects::reset();
}   // destroy

// ----------------------------------------------------------------------------
// Shader loading  related hook

static std::string loadHeader()
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
}   // loadHeader

// ----------------------------------------------------------------------------
void Shaders::loadShaders()
{
    const std::string &dir = file_manager->getAsset(FileManager::SHADER, "");

    IGPUProgrammingServices * const gpu = irr_driver->getVideoDriver()
                                        ->getGPUProgrammingServices();

#define glsl(a, b, c) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c)
#define glslmat(a, b, c, d) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c, d)
#define glsl_noinput(a, b) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) 0)

    // Save previous shaders (used in case some shaders don't compile)
    int saved_shaders[ES_COUNT];
    memcpy(saved_shaders, m_shaders, sizeof(m_shaders));
    
#if !defined(USE_GLES2)
    std::string name = "pass";
#else
    std::string name = "pass_gles";
#endif

    // Ok, go
    m_shaders[ES_NORMAL_MAP] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");
    m_shaders[ES_NORMAL_MAP_LIGHTMAP] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");

    m_shaders[ES_SKYBOX] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                   m_callbacks[ES_SKYBOX], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_SPLATTING] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");

    m_shaders[ES_WATER] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                  m_callbacks[ES_WATER], EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_shaders[ES_WATER_SURFACE] = glsl(dir + name + ".vert", dir + name + ".frag",
                                       m_callbacks[ES_WATER]);

    m_shaders[ES_SPHERE_MAP] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");

    m_shaders[ES_GRASS] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                  m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_shaders[ES_GRASS_REF] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                      m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    m_shaders[ES_MOTIONBLUR] = glsl(dir + name + ".vert", dir + name + ".frag",
                                    m_callbacks[ES_MOTIONBLUR]);

    m_shaders[ES_GAUSSIAN3H] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                       m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
    m_shaders[ES_GAUSSIAN3V] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                       m_callbacks[ES_GAUSSIAN3V], EMT_SOLID);

    m_shaders[ES_MIPVIZ] = glslmat(dir + name + ".vert", dir + name + ".frag",
                                   m_callbacks[ES_MIPVIZ], EMT_SOLID);

    m_shaders[ES_OBJECTPASS] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");
    m_shaders[ES_OBJECT_UNLIT] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");
    m_shaders[ES_OBJECTPASS_REF] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");
    m_shaders[ES_OBJECTPASS_RIMLIT] = glsl_noinput(dir + name + ".vert", dir + name + ".frag");

    m_shaders[ES_DISPLACE] = glslmat(dir + name + ".vert", dir + name + ".frag",
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
}   // loadShaders

// ----------------------------------------------------------------------------
void Shaders::check(const int num)
{
    if (m_shaders[num] == -1)
    {
        Log::error("shaders",
                   "Shader %s failed to load. Update your drivers, if the issue "
                   "persists, report a bug to us.", shader_names[num] + 3);
    }
}   // check

// ============================================================================
Shaders::SkinnedTransparentShader::SkinnedTransparentShader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                        GL_FRAGMENT_SHADER, "transparent.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "texture_trans", "skinning_offset",  "custom_alpha");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedTransparentShader

// ============================================================================
Shaders::TransparentShader::TransparentShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "transparent.frag");
    assignUniforms("ModelMatrix", "texture_trans", "custom_alpha");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // TransparentShader

// ============================================================================
Shaders::TransparentFogShader::TransparentFogShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "transparentfog.frag");
    assignUniforms("ModelMatrix", "texture_trans", "fogmax", "startH",
                   "endH", "start", "end", "col");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // TransparentFogShader

// ============================================================================
Shaders::ColoredLine::ColoredLine()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER,   "object_pass.vert",
                        GL_FRAGMENT_SHADER, "coloredquad.frag");

    assignUniforms("color");

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 1024 * sizeof(float), 0, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}   // Shaders::ColoredLine

#endif   // !SERVER_ONLY
