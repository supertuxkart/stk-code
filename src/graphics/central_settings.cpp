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
#include "graphics/central_settings.hpp"

#include "config/user_config.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "guiengine/engine.hpp"
#include <ge_main.hpp>
#include <ge_gl_utils.hpp>
#include <ge_vulkan_features.hpp>

using namespace GE;
bool CentralVideoSettings::m_supports_sp = true;

CentralVideoSettings *CVS = new CentralVideoSettings();

void CentralVideoSettings::init()
{
    m_gl_major_version = 2;
    m_gl_minor_version = 1;
    m_gl_mem = 0;
    m_glsl = false;

    // Parse extensions
    hasBufferStorage = false;
    hasComputeShaders = false;
    hasArraysOfArrays = false;
    hasTextureStorage = false;
    hasTextureView = false;
    hasAtomics = false;
    hasSSBO = false;
    hasImageLoadStore = false;
    hasTextureCompression = false;
    hasTextureCompressionSRGB = false;
    hasUBO = false;
    hasExplicitAttribLocation = false;
    hasGS = false;
    hasTextureFilterAnisotropic = false;
    hasTextureSwizzle = false;
    hasPixelBufferObject = false;
    hasSamplerObjects = false;
    hasVertexType2101010Rev = false;
    hasInstancedArrays = false;
    hasBGRA = false;
    hasColorBufferFloat = false;
    hasTextureBufferObject = false;
    m_need_vertex_id_workaround = false;

    // Call to glGetIntegerv should not be made if --no-graphics is used
    if (!GUIEngine::isNoGraphics())
    {
        if (GE::getDriver()->getDriverType() != video::EDT_OPENGL &&
            GE::getDriver()->getDriverType() != video::EDT_OGLES2)
        {
            GraphicsRestrictions::init("", "", GE::getDriver()->getVendorInfo().c_str());
            GE::getGEConfig()->m_disable_npot_texture =
                GraphicsRestrictions::isDisabled(
                GraphicsRestrictions::GR_NPOT_TEXTURES);
            if (GE::getDriver()->getDriverType() == video::EDT_VULKAN)
            {
                hasTextureCompression = GEVulkanFeatures::supportsS3TCBC3() ||
                    GEVulkanFeatures::supportsBPTCBC7() ||
                    GEVulkanFeatures::supportsASTC4x4();
            }
            return;
        }

        glGetIntegerv(GL_MAJOR_VERSION, &m_gl_major_version);
        glGetIntegerv(GL_MINOR_VERSION, &m_gl_minor_version);
        const char *vendor = (const char *)glGetString(GL_VENDOR);
        const char *renderer = (const char *)glGetString(GL_RENDERER);
        const char *version = (const char *)glGetString(GL_VERSION);
        Log::info("IrrDriver", "OpenGL version: %d.%d", m_gl_major_version, m_gl_minor_version);
        Log::info("IrrDriver", "OpenGL vendor: %s", vendor);
        Log::info("IrrDriver", "OpenGL renderer: %s", renderer);
        Log::info("IrrDriver", "OpenGL version string: %s", version);

        if (strstr(vendor, "NVIDIA"))
            glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &m_gl_mem);

        if (m_gl_mem > 0)
            Log::info("IrrDriver", "OpenGL total memory: %d", m_gl_mem/1024);
    }
#if !defined(USE_GLES2)
    m_glsl = (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 1))
           && !UserConfigParams::m_force_legacy_device && m_supports_sp;
#else
    m_glsl = m_gl_major_version >= 3 && !UserConfigParams::m_force_legacy_device;
#endif
    if (!GUIEngine::isNoGraphics())
        initGL();

    if (!GUIEngine::isNoGraphics())
    {
        std::string driver((char*)(glGetString(GL_VERSION)));
        std::string card((char*)(glGetString(GL_RENDERER)));
        std::string vendor((char*)(glGetString(GL_VENDOR)));
        GraphicsRestrictions::init(driver, card, vendor);
        GE::getGEConfig()->m_disable_npot_texture =
            GraphicsRestrictions::isDisabled(
            GraphicsRestrictions::GR_NPOT_TEXTURES);

        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FORCE_LEGACY_DEVICE))
        {
            m_glsl = false;
        }

#if !defined(USE_GLES2)
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_BUFFER_STORAGE) &&
            hasGLExtension("GL_ARB_buffer_storage")  )
        {
            hasBufferStorage = true;
            Log::info("GLDriver", "ARB Buffer Storage Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_COMPUTE_SHADER) &&
            hasGLExtension("GL_ARB_compute_shader")) {
            hasComputeShaders = true;
            Log::info("GLDriver", "ARB Compute Shader Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_ARRAYS_OF_ARRAYS) &&
            hasGLExtension("GL_ARB_arrays_of_arrays")) {
            hasArraysOfArrays = true;
            Log::info("GLDriver", "ARB Arrays of Arrays Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_TEXTURE_STORAGE) &&
            hasGLExtension("GL_ARB_texture_storage")) {
            hasTextureStorage = true;
            Log::info("GLDriver", "ARB Texture Storage Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_TEXTURE_VIEW) &&
            hasGLExtension("GL_ARB_texture_view")) {
            hasTextureView = true;
            Log::info("GLDriver", "ARB Texture View Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_IMAGE_LOAD_STORE) &&
            hasGLExtension("GL_ARB_shader_image_load_store")) {
            hasImageLoadStore = true;
            Log::info("GLDriver", "ARB Image Load Store Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_SHADER_ATOMIC_COUNTERS) &&
            hasGLExtension("GL_ARB_shader_atomic_counters")) {
            hasAtomics = true;
            Log::info("GLDriver", "ARB Shader Atomic Counters Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_SHADER_STORAGE_BUFFER_OBJECT) &&
            hasGLExtension("GL_ARB_shader_storage_buffer_object")) {
            hasSSBO = true;
            Log::info("GLDriver", "ARB Shader Storage Buffer Object Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXT_TEXTURE_COMPRESSION_S3TC) &&
            hasGLExtension("GL_EXT_texture_compression_s3tc"))
        {
            hasTextureCompression = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_UNIFORM_BUFFER_OBJECT) &&
            hasGLExtension("GL_ARB_uniform_buffer_object")) {
            hasUBO = true;
            Log::info("GLDriver", "ARB Uniform Buffer Object Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXPLICIT_ATTRIB_LOCATION) &&
            hasGLExtension("GL_ARB_explicit_attrib_location")) {
            hasExplicitAttribLocation = true;
            Log::info("GLDriver", "ARB Explicit Attrib Location Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_TEXTURE_FILTER_ANISOTROPIC) &&
            hasGLExtension("GL_EXT_texture_filter_anisotropic")) {
            hasTextureFilterAnisotropic = true;
            Log::info("GLDriver", "EXT Texture Filter Anisotropic Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_GEOMETRY_SHADER) &&
            (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 2))) {
            hasGS = true;
            Log::info("GLDriver", "Geometry Shaders Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_TEXTURE_BUFFER_OBJECT) &&
            m_glsl == true) 
        {
            hasTextureBufferObject = true;
            Log::info("GLDriver", "ARB Texture Buffer Object Present");
        }
        if (hasGLExtension("GL_ARB_texture_swizzle"))
        {
            hasTextureSwizzle = true;
            Log::info("GLDriver", "ARB Texture Swizzle Present");
        }
        if (hasGLExtension("GL_ARB_pixel_buffer_object"))
        {
            hasPixelBufferObject = true;
            Log::info("GLDriver", "ARB Pixel Buffer Object Present");
        }
        if (hasGLExtension("GL_ARB_sampler_objects"))
        {
            hasSamplerObjects = true;
            Log::info("GLDriver", "ARB Sampler Objects Present");
        }
        if (hasGLExtension("GL_ARB_vertex_type_2_10_10_10_rev"))
        {
            hasVertexType2101010Rev = true;
            Log::info("GLDriver", "ARB Vertex Type 2_10_10_10_rev Present");
        }
        if (hasGLExtension("GL_ARB_instanced_arrays"))
        {
            hasInstancedArrays = true;
            Log::info("GLDriver", "ARB Instanced Arrays Present");
        }

        // Check all extensions required by SP
        m_supports_sp = isARBInstancedArraysUsable() &&
            isARBVertexType2101010RevUsable() && isARBSamplerObjectsUsable() &&
            isARBExplicitAttribLocationUsable();
            
        hasTextureCompressionSRGB = true;
        hasBGRA = true;
        hasColorBufferFloat = true;

#else
        if (m_glsl == true)
        {
            hasTextureStorage = true;
            hasTextureSwizzle = true;
            hasSamplerObjects = true;
            hasVertexType2101010Rev = true;
            hasInstancedArrays = true;
            hasPixelBufferObject = true;
        }

        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXPLICIT_ATTRIB_LOCATION) &&
            m_glsl == true)
        {
            Log::info("GLDriver", "Explicit Attrib Location Present");
            hasExplicitAttribLocation = true;
        }

        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_UNIFORM_BUFFER_OBJECT) &&
            m_glsl == true) 
        {
            hasUBO = true;
            Log::info("GLDriver", "ARB Uniform Buffer Object Present");
        }
        
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_TEXTURE_FORMAT_BGRA8888) &&
            (hasGLExtension("GL_IMG_texture_format_BGRA8888") ||
             hasGLExtension("GL_EXT_texture_format_BGRA8888")))
        {
            hasBGRA = true;
            Log::info("GLDriver", "EXT texture format BGRA8888 Present");
        }

        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_COLOR_BUFFER_FLOAT) &&
            hasGLExtension("GL_EXT_color_buffer_float"))
        {
            hasColorBufferFloat = true;
            Log::info("GLDriver", "EXT Color Buffer Float Present");
        }
        
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXT_TEXTURE_COMPRESSION_S3TC) &&
            (hasGLExtension("GL_EXT_texture_compression_s3tc") || 
             hasGLExtension("GL_ANGLE_texture_compression_dxt5")))
        {
            hasTextureCompression = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC Present");
        }
        
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXT_TEXTURE_COMPRESSION_S3TC) &&
            (hasGLExtension("GL_EXT_texture_compression_s3tc_srgb") || 
             hasGLExtension("GL_NV_sRGB_formats")))
        {
            hasTextureCompressionSRGB = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC sRGB Present");
        }

        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_VERTEX_ID_WORKING))
        {
            m_need_vertex_id_workaround = true;
        }
#endif

        // Only unset the high def textures if they are set as default. If the
        // user has enabled them (bit 1 set), then leave them enabled.
        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_HIGHDEFINITION_TEXTURES) &&
            (UserConfigParams::m_high_definition_textures & 0x02) == 0)
        {
            UserConfigParams::m_high_definition_textures = 0x00;
        }

        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_HIGHDEFINITION_TEXTURES_256))
        {
            UserConfigParams::m_high_definition_textures = 0;
            if (UserConfigParams::m_max_texture_size > 256)
            {
                UserConfigParams::m_max_texture_size = 256;
            }
        }
#ifndef ANDROID
        if (SP::sp_apitrace)
        {
            Log::info("IrrDriver", "Writing GPU query strings to apitrace and"
                " disable buffer storage");
            hasBufferStorage = false;
        }
#endif
    }
}

unsigned CentralVideoSettings::getGLSLVersion() const
{
#if defined(USE_GLES2)
    if (m_gl_major_version >= 3)
        return 300;
    else
        return 100;
#else
    if (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version == 3))
        return m_gl_major_version * 100 + m_gl_minor_version * 10;
    else if (m_gl_major_version == 3)
        return 100 + (m_gl_minor_version + 3) * 10;
    else
        return 120;
#endif
}

bool CentralVideoSettings::isGLSL() const
{
    return m_glsl;
}

bool CentralVideoSettings::needsVertexIdWorkaround() const
{
    return m_need_vertex_id_workaround;
}

bool CentralVideoSettings::isARBGeometryShadersUsable() const
{
    return hasGS;
}

bool CentralVideoSettings::isARBUniformBufferObjectUsable() const
{
    return hasUBO ||
        (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 1));
}

bool CentralVideoSettings::isARBExplicitAttribLocationUsable() const
{
    return hasExplicitAttribLocation ||
        (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 3));
}

bool CentralVideoSettings::isEXTTextureCompressionS3TCUsable() const
{
    return hasTextureCompression;
}

bool CentralVideoSettings::isEXTTextureCompressionS3TCSRGBUsable() const
{
    return hasTextureCompression && hasTextureCompressionSRGB;
}


bool CentralVideoSettings::isARBBufferStorageUsable() const
{
    return hasBufferStorage;
}

bool CentralVideoSettings::isARBComputeShaderUsable() const
{
    return hasComputeShaders;
}

bool CentralVideoSettings::isARBArraysOfArraysUsable() const
{
    return hasArraysOfArrays;
}

bool CentralVideoSettings::isARBTextureStorageUsable() const
{
    return hasTextureStorage;
}

bool CentralVideoSettings::isARBTextureViewUsable() const
{
    return hasTextureView;
}

bool CentralVideoSettings::isARBShaderAtomicCountersUsable() const
{
    return hasAtomics;
}

bool CentralVideoSettings::isARBShaderStorageBufferObjectUsable() const
{
    return hasSSBO;
}

bool CentralVideoSettings::isARBImageLoadStoreUsable() const
{
    return hasImageLoadStore;
}

bool CentralVideoSettings::isEXTTextureFilterAnisotropicUsable() const
{
    return hasTextureFilterAnisotropic;
}

bool CentralVideoSettings::isEXTTextureFormatBGRA8888Usable() const
{
    return hasBGRA;
}

bool CentralVideoSettings::isEXTColorBufferFloatUsable() const
{
    return hasColorBufferFloat;
}

bool CentralVideoSettings::supportsComputeShadersFiltering() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable() && isARBComputeShaderUsable() && isARBArraysOfArraysUsable();
}

bool CentralVideoSettings::supportsTextureCompression() const
{
    return isEXTTextureCompressionS3TCUsable();
}

bool CentralVideoSettings::isShadowEnabled() const
{
    return UserConfigParams::m_shadows_resolution > 0;
}

bool CentralVideoSettings::isTextureCompressionEnabled() const
{
#ifdef MOBILE_STK
    // MOBILE_STK currently doesn't handle libsquish in SP
    return false;
#else
    return supportsTextureCompression() && UserConfigParams::m_texture_compression;
#endif
}

bool CentralVideoSettings::isDeferredEnabled() const
{
    return UserConfigParams::m_dynamic_lights && !GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_ADVANCED_PIPELINE);
}

bool CentralVideoSettings::supportsHardwareSkinning() const
{
    return !GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_HARDWARE_SKINNING);
}

bool CentralVideoSettings::isARBTextureSwizzleUsable() const
{
    return m_glsl && hasTextureSwizzle;
}

bool CentralVideoSettings::isARBPixelBufferObjectUsable() const
{
    return hasPixelBufferObject;
}

bool CentralVideoSettings::isARBSamplerObjectsUsable() const
{
    return hasSamplerObjects ||
        (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 3));
}

bool CentralVideoSettings::isARBVertexType2101010RevUsable() const
{
    return hasVertexType2101010Rev ||
        (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 3));
}

bool CentralVideoSettings::isARBInstancedArraysUsable() const
{
    return hasInstancedArrays ||
        (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 2));
}

bool CentralVideoSettings::isARBTextureBufferObjectUsable() const
{
    return hasTextureBufferObject;
}

bool CentralVideoSettings::supportsColorization() const
{
    return isGLSL() || GE::getDriver()->getDriverType() == video::EDT_VULKAN ||
        GE::getDriver()->getDriverType() == video::EDT_OGLES2;
}

#endif   // !SERVER_ONLY
