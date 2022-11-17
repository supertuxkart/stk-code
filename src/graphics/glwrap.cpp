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

#include "graphics/glwrap.hpp"

#include "config/hardware_stats.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"
#include "graphics/sp/sp_base.hpp"
#include "utils/profiler.hpp"
#include "utils/cpp2011.hpp"
#include "utils/string_utils.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <ge_gl_utils.hpp>

#include <IVideoDriver.h>

#ifdef DEBUG
#if !defined(__APPLE__) && !defined(ANDROID)
#define ARB_DEBUG_OUTPUT
#endif
#endif

#if defined(USE_GLES2)
#ifndef __APPLE__
#include <SDL_video.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#endif

static bool is_gl_init = false;

#if DEBUG
bool GLContextDebugBit = true;
#else
bool GLContextDebugBit = false;
#endif

#ifdef ARB_DEBUG_OUTPUT
static void
#ifdef WIN32
CALLBACK
#endif
debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
              const GLchar* msg, const void *userparam)
{
#ifdef GL_DEBUG_SEVERITY_NOTIFICATION
    // ignore minor notifications sent by some drivers (notably the nvidia one)
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;
#endif

    // suppress minor performance warnings (emitted mostly by nvidia drivers)
    if ((severity == GL_DEBUG_SEVERITY_MEDIUM_ARB || severity == GL_DEBUG_SEVERITY_LOW_ARB) &&
        type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
    {
        return;
    }

    switch(source)
    {
    case GL_DEBUG_SOURCE_API_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - API");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - WINDOW_SYSTEM");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - SHADER_COMPILER");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - THIRD_PARTY");
        break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - APPLICATION");
        break;
    case GL_DEBUG_SOURCE_OTHER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - OTHER");
        break;
    }

    switch(type)
    {
    case GL_DEBUG_TYPE_ERROR_ARB:
        Log::warn("GLWrap", "    Error type : ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
        Log::warn("GLWrap", "    Error type : PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        Log::warn("GLWrap", "    Error type : PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER_ARB:
        Log::warn("GLWrap", "    Error type : OTHER");
        break;
    }

    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
        Log::warn("GLWrap", "    Severity : HIGH");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        Log::warn("GLWrap", "    Severity : MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_LOW_ARB:
        Log::warn("GLWrap", "    Severity : LOW");
        break;
    }
    
    if (msg)
        Log::warn("GLWrap", "    Message : %s", msg);
}
#endif

#ifdef USE_GLES2

#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_TYPE_MARKER 0x8268
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#endif

void initGL()
{
    if (is_gl_init)
        return;

    is_gl_init = true;
#ifdef ARB_DEBUG_OUTPUT
    if (glDebugMessageCallback)
        glDebugMessageCallback((GLDEBUGPROCARB)debugCallback, NULL);
#endif

    if (SP::sp_apitrace && GE::hasGLExtension("GL_KHR_debug"))
    {
        if (glDebugMessageControl)
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    else
    {
        SP::sp_apitrace = false;
    }
}

ScopedGPUTimer::ScopedGPUTimer(GPUTimer &t) : timer(t)
{
    if (SP::sp_apitrace)
    {
        std::string msg = timer.getName();
        msg += " begin";
        if (glDebugMessageInsert)
        {
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 100,
                GL_DEBUG_SEVERITY_NOTIFICATION, -1, msg.c_str());
        }
    }
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;
    if (!timer.canSubmitQuery) return;
#ifdef GL_TIME_ELAPSED
    if (!timer.initialised)
    {
        glGenQueries(1, &timer.query);
        timer.initialised = true;
    }
    glBeginQuery(GL_TIME_ELAPSED, timer.query);
#endif
}
ScopedGPUTimer::~ScopedGPUTimer()
{
    if (SP::sp_apitrace)
    {
        std::string msg = timer.getName();
        msg += " end";
        if (glDebugMessageInsert)
        {
            glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 100,
                GL_DEBUG_SEVERITY_NOTIFICATION, -1, msg.c_str());
        }
    }
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;
    if (!timer.canSubmitQuery) return;
#ifdef GL_TIME_ELAPSED
    glEndQuery(GL_TIME_ELAPSED);
    timer.canSubmitQuery = false;
#endif
}

GPUTimer::GPUTimer(const char* name)
        : m_name(name)
{
    reset();
}

unsigned GPUTimer::elapsedTimeus()
{
    if (!initialised)
        return 0;
    GLuint result;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &result);
    if (result == GL_FALSE)
        return lastResult;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &result);
    lastResult = result / 1000;
    canSubmitQuery = true;
    return result / 1000;
}

void draw3DLine(const core::vector3df& start,
                const core::vector3df& end, irr::video::SColor color)
{
    if (!CVS->isGLSL()) {
        irr_driver->getVideoDriver()->draw3DLine(start, end, color);
        return;
    }

    float vertex[6] = {
        start.X, start.Y, start.Z,
        end.X, end.Y, end.Z
    };

    Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
    line->bindVertexArray();
    line->bindBuffer();
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(float), vertex);

    line->use();
    line->setUniforms(color);
    glDrawArrays(GL_LINES, 0, 2);

    glGetError();
}

// ----------------------------------------------------------------------------
/** Returns a space-separated list of all GL extensions. Used for hardware
 *  reporting.
 */
const std::string getGLExtensions()
{
    std::string result;
#if !defined(USE_GLES2)
    if (glGetStringi != NULL)
    {
        GLint num_extensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        for (GLint i = 0; i < num_extensions; i++)
        {
            const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
            if(result.size()>0)
                result += " ";
            result += extension;
        }
    }
    else
#endif
    {
        const char* extensions = (const char*) glGetString(GL_EXTENSIONS);
        result = extensions;
    }

    return result;
}   // getGLExtensions

// ----------------------------------------------------------------------------
/** Adds GL limits to the json data structure.
 *  (C) 2014-2015 Wildfire Games (0 A.D.), ported by Joerg Henrichs
 */
void getGLLimits(HardwareStats::Json *json)
{
    // Various macros to make the data assembly shorter.
#define INTEGER(id)                               \
    do {                                          \
        GLint i = -1;                             \
        glGetIntegerv(GL_##id, &i);               \
        if (glGetError()==GL_NO_ERROR)            \
            json->add("GL_"#id, i);               \
    } while (false)
#define INTEGER2(id)                              \
    do {                                          \
        GLint i[2] = { -1, -1 };                  \
        glGetIntegerv(GL_##id, i);                \
        if (glGetError()==GL_NO_ERROR) {          \
            json->add("GL_"#id"[0]", i[0]);       \
            json->add("GL_"#id"[1]", i[1]);       \
        }                                         \
    } while (false)
#define FLOAT(id)                                 \
    do {                                          \
        GLfloat f = -1.0f;                        \
        glGetFloatv(GL_##id, &f);                 \
        if (glGetError()==GL_NO_ERROR)            \
            json->add("GL_"#id, f);               \
    } while (false)
#define FLOAT2(id)                                \
    do {                                          \
        GLfloat f[2] = {-1.0f, -1.0f};            \
        glGetFloatv(GL_##id,  f);                 \
        if (glGetError()==GL_NO_ERROR)  {         \
            json->add("GL_"#id"[0]", f[0]);       \
            json->add("GL_"#id"[1]", f[1]);       \
        }                                         \
    } while (false)
#define STRING(id)                                         \
    do {                                                   \
        const char* c = (const char*)glGetString(GL_##id); \
        if(!c) c="";                                       \
        json->add("GL_"#id, c);                            \
    } while (false)


    STRING(VERSION);
    STRING(VENDOR);
    STRING(RENDERER);
    INTEGER(SUBPIXEL_BITS);
    INTEGER(MAX_TEXTURE_SIZE);
    INTEGER(MAX_CUBE_MAP_TEXTURE_SIZE);
    INTEGER2(MAX_VIEWPORT_DIMS);
    FLOAT2(ALIASED_POINT_SIZE_RANGE);
    FLOAT2(ALIASED_LINE_WIDTH_RANGE);
    INTEGER(SAMPLE_BUFFERS);
    INTEGER(SAMPLES);
    // TODO: compressed texture formats
    INTEGER(RED_BITS);
    INTEGER(GREEN_BITS);
    INTEGER(BLUE_BITS);
    INTEGER(ALPHA_BITS);
    INTEGER(DEPTH_BITS);
    INTEGER(STENCIL_BITS);

    return;

#ifdef NOT_DONE_YET
#define QUERY(target, pname) do { \
    GLint i = -1; \
    pglGetQueryivARB(GL_##target, GL_##pname, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_" #target ".GL_" #pname, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_" #target ".GL_" #pname, i); \
        } while (false)
#define VERTEXPROGRAM(id) do { \
    GLint i = -1; \
    pglGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_##id, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_VERTEX_PROGRAM_ARB.GL_" #id, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_VERTEX_PROGRAM_ARB.GL_" #id, i); \
        } while (false)
#define FRAGMENTPROGRAM(id) do { \
    GLint i = -1; \
    pglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_##id, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_FRAGMENT_PROGRAM_ARB.GL_" #id, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_FRAGMENT_PROGRAM_ARB.GL_" #id, i); \
        } while (false)
#define BOOL(id) INTEGER(id)


#if !CONFIG2_GLES
        INTEGER(MAX_LIGHTS);
        INTEGER(MAX_CLIP_PLANES);
        // Skip MAX_COLOR_MATRIX_STACK_DEPTH (only in imaging subset)
        INTEGER(MAX_MODELVIEW_STACK_DEPTH);
        INTEGER(MAX_PROJECTION_STACK_DEPTH);
        INTEGER(MAX_TEXTURE_STACK_DEPTH);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_3D_TEXTURE_SIZE);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_PIXEL_MAP_TABLE);
        INTEGER(MAX_NAME_STACK_DEPTH);
        INTEGER(MAX_LIST_NESTING);
        INTEGER(MAX_EVAL_ORDER);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_ATTRIB_STACK_DEPTH);
        INTEGER(MAX_CLIENT_ATTRIB_STACK_DEPTH);
        INTEGER(AUX_BUFFERS);
        BOOL(RGBA_MODE);
        BOOL(INDEX_MODE);
        BOOL(DOUBLEBUFFER);
        BOOL(STEREO);
#endif
#if !CONFIG2_GLES
        FLOAT2(SMOOTH_POINT_SIZE_RANGE);
        FLOAT(SMOOTH_POINT_SIZE_GRANULARITY);
#endif
#if !CONFIG2_GLES
        FLOAT2(SMOOTH_LINE_WIDTH_RANGE);
        FLOAT(SMOOTH_LINE_WIDTH_GRANULARITY);
        // Skip MAX_CONVOLUTION_WIDTH, MAX_CONVOLUTION_HEIGHT (only in imaging subset)
        INTEGER(MAX_ELEMENTS_INDICES);
        INTEGER(MAX_ELEMENTS_VERTICES);
        INTEGER(MAX_TEXTURE_UNITS);
#endif
#if !CONFIG2_GLES
        INTEGER(INDEX_BITS);
#endif
#if !CONFIG2_GLES
        INTEGER(ACCUM_RED_BITS);
        INTEGER(ACCUM_GREEN_BITS);
        INTEGER(ACCUM_BLUE_BITS);
        INTEGER(ACCUM_ALPHA_BITS);
#endif
#if !CONFIG2_GLES
        // Core OpenGL 2.0 (treated as extensions):
        if (ogl_HaveExtension("GL_EXT_texture_lod_bias"))
        {
            FLOAT(MAX_TEXTURE_LOD_BIAS_EXT);
        }
        if (ogl_HaveExtension("GL_ARB_occlusion_query"))
        {
            QUERY(SAMPLES_PASSED, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_ARB_shading_language_100"))
        {
            STRING(SHADING_LANGUAGE_VERSION_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_shader"))
        {
            INTEGER(MAX_VERTEX_ATTRIBS_ARB);
            INTEGER(MAX_VERTEX_UNIFORM_COMPONENTS_ARB);
            INTEGER(MAX_VARYING_FLOATS_ARB);
            INTEGER(MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_fragment_shader"))
        {
            INTEGER(MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_shader") || ogl_HaveExtension("GL_ARB_fragment_shader") ||
            ogl_HaveExtension("GL_ARB_vertex_program") || ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            INTEGER(MAX_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_TEXTURE_COORDS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_draw_buffers"))
        {
            INTEGER(MAX_DRAW_BUFFERS_ARB);
        }
        // Core OpenGL 3.0:
        if (ogl_HaveExtension("GL_EXT_gpu_shader4"))
        {
            INTEGER(MIN_PROGRAM_TEXEL_OFFSET); // no _EXT version of these in glext.h
            INTEGER(MAX_PROGRAM_TEXEL_OFFSET);
        }
        if (ogl_HaveExtension("GL_EXT_framebuffer_object"))
        {
            INTEGER(MAX_COLOR_ATTACHMENTS_EXT);
            INTEGER(MAX_RENDERBUFFER_SIZE_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_framebuffer_multisample"))
        {
            INTEGER(MAX_SAMPLES_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_texture_array"))
        {
            INTEGER(MAX_ARRAY_TEXTURE_LAYERS_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_transform_feedback"))
        {
            INTEGER(MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT);
            INTEGER(MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT);
            INTEGER(MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT);
        }
        // Other interesting extensions:
        if (ogl_HaveExtension("GL_EXT_timer_query") || ogl_HaveExtension("GL_ARB_timer_query"))
        {
            QUERY(TIME_ELAPSED, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_ARB_timer_query"))
        {
            QUERY(TIMESTAMP, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_EXT_texture_filter_anisotropic"))
        {
            FLOAT(MAX_TEXTURE_MAX_ANISOTROPY_EXT);
        }
        if (ogl_HaveExtension("GL_ARB_texture_rectangle"))
        {
            INTEGER(MAX_RECTANGLE_TEXTURE_SIZE_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_program") || ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            INTEGER(MAX_PROGRAM_MATRICES_ARB);
            INTEGER(MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_program"))
        {
            VERTEXPROGRAM(MAX_PROGRAM_ENV_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_INSTRUCTIONS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_TEMPORARIES_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_ATTRIBS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
            if (ogl_HaveExtension("GL_ARB_fragment_program"))
            {
                // The spec seems to say these should be supported, but
                // Mesa complains about them so let's not bother
                /*
                VERTEXPROGRAM(MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
                */
            }
        }
        if (ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            FRAGMENTPROGRAM(MAX_PROGRAM_ENV_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEMPORARIES_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_ATTRIBS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
            if (ogl_HaveExtension("GL_ARB_vertex_program"))
            {
                // The spec seems to say these should be supported, but
                // Intel drivers on Windows complain about them so let's not bother
                /*
                FRAGMENTPROGRAM(MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
                FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
                */
            }
        }
        if (ogl_HaveExtension("GL_ARB_geometry_shader4"))
        {
            INTEGER(MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_GEOMETRY_OUTPUT_VERTICES_ARB);
            INTEGER(MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB);
            INTEGER(MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB);
            INTEGER(MAX_GEOMETRY_VARYING_COMPONENTS_ARB);
            INTEGER(MAX_VERTEX_VARYING_COMPONENTS_ARB);
        }
#else // CONFIG2_GLES
        // Core OpenGL ES 2.0:
        STRING(SHADING_LANGUAGE_VERSION);
        INTEGER(MAX_VERTEX_ATTRIBS);
        INTEGER(MAX_VERTEX_UNIFORM_VECTORS);
        INTEGER(MAX_VARYING_VECTORS);
        INTEGER(MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_VERTEX_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_FRAGMENT_UNIFORM_VECTORS);
        INTEGER(MAX_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_RENDERBUFFER_SIZE);
#endif // CONFIG2_GLES
#ifdef SDL_VIDEO_DRIVER_X11
#define GLXQCR_INTEGER(id) do { \
    unsigned int i = UINT_MAX; \
    if (pglXQueryCurrentRendererIntegerMESA(id, &i)) \
    scriptInterface.SetProperty(settings, #id, i); \
        } while (false)
#define GLXQCR_INTEGER2(id) do { \
    unsigned int i[2] = { UINT_MAX, UINT_MAX }; \
    if (pglXQueryCurrentRendererIntegerMESA(id, i)) { \
    scriptInterface.SetProperty(settings, #id "[0]", i[0]); \
    scriptInterface.SetProperty(settings, #id "[1]", i[1]); \
    } \
        } while (false)
#define GLXQCR_INTEGER3(id) do { \
    unsigned int i[3] = { UINT_MAX, UINT_MAX, UINT_MAX }; \
    if (pglXQueryCurrentRendererIntegerMESA(id, i)) { \
    scriptInterface.SetProperty(settings, #id "[0]", i[0]); \
    scriptInterface.SetProperty(settings, #id "[1]", i[1]); \
    scriptInterface.SetProperty(settings, #id "[2]", i[2]); \
    } \
        } while (false)
#define GLXQCR_STRING(id) do { \
    const char* str = pglXQueryCurrentRendererStringMESA(id); \
    if (str) \
    scriptInterface.SetProperty(settings, #id ".string", str); \
        } while (false)
        SDL_SysWMinfo wminfo;
        SDL_VERSION(&wminfo.version);
        if (SDL_GetWMInfo(&wminfo) && wminfo.subsystem == SDL_SYSWM_X11)
        {
            Display* dpy = wminfo.info.x11.gfxdisplay;
            int scrnum = DefaultScreen(dpy);
            const char* glxexts = glXQueryExtensionsString(dpy, scrnum);
            scriptInterface.SetProperty(settings, "glx_extensions", glxexts);
            if (strstr(glxexts, "GLX_MESA_query_renderer") && pglXQueryCurrentRendererIntegerMESA && pglXQueryCurrentRendererStringMESA)
            {
                GLXQCR_INTEGER(GLX_RENDERER_VENDOR_ID_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_DEVICE_ID_MESA);
                GLXQCR_INTEGER3(GLX_RENDERER_VERSION_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_ACCELERATED_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_VIDEO_MEMORY_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_PREFERRED_PROFILE_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA);
                GLXQCR_STRING(GLX_RENDERER_VENDOR_ID_MESA);
                GLXQCR_STRING(GLX_RENDERER_DEVICE_ID_MESA);
            }
        }
#endif // SDL_VIDEO_DRIVER_X11

#endif  // ifdef XX
}   // getGLLimits


// ----------------------------------------------------------------------------
/** Executes glGetError and prints error to the console
 * \return True if error ocurred
 */
bool checkGLError()
{
    GLenum err = glGetError();
    
    switch (err)
    {
    case GL_NO_ERROR:
        break;
    case GL_INVALID_ENUM:
        Log::warn("GLWrap", "glGetError: GL_INVALID_ENUM");
        break;
    case GL_INVALID_VALUE:
        Log::warn("GLWrap", "glGetError: GL_INVALID_VALUE");
        break;
    case GL_INVALID_OPERATION:
        Log::warn("GLWrap", "glGetError: GL_INVALID_OPERATION");
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        Log::warn("GLWrap", "glGetError: GL_INVALID_FRAMEBUFFER_OPERATION");
        break;
    case GL_OUT_OF_MEMORY:
        Log::warn("GLWrap", "glGetError: GL_OUT_OF_MEMORY");
        break;
#if !defined(USE_GLES2)
    case GL_STACK_UNDERFLOW:
        Log::warn("GLWrap", "glGetError: GL_STACK_UNDERFLOW");
        break;
    case GL_STACK_OVERFLOW:
        Log::warn("GLWrap", "glGetError: GL_STACK_OVERFLOW");
        break;
#endif
    default:
        Log::warn("GLWrap", "glGetError: %i", (int)err);
        break;
    }
    
    return err != GL_NO_ERROR;
}

#ifdef WIN32
// Tell system that it should use nvidia on optimus devices and amd on enduro devices
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif

#endif   // !SERVER_ONLY

