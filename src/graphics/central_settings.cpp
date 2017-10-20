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
#include "modes/profile_world.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/graphics_restrictions.hpp"


CentralVideoSettings *CVS = new CentralVideoSettings();

void CentralVideoSettings::init()
{
    m_gl_major_version = 2;
    m_gl_minor_version = 1;

    // Parse extensions
    hasVSLayer = false;
    hasBaseInstance = false;
    hasBufferStorage = false;
    hasDrawIndirect = false;
    hasComputeShaders = false;
    hasArraysOfArrays = false;
    hasTextureStorage = false;
    hasTextureView = false;
    hasBindlessTexture = false;
    hasAtomics = false;
    hasSSBO = false;
    hasImageLoadStore = false;
    hasMultiDrawIndirect = false;
    hasTextureCompression = false;
    hasUBO = false;
    hasExplicitAttribLocation = false;
    hasGS = false;
    hasTextureFilterAnisotropic = false;
    hasTextureSwizzle = false;
    hasPixelBufferObject = false;
    hasSRGBFramebuffer = false;

#if defined(USE_GLES2)
    hasBGRA = false;
    hasColorBufferFloat = false;
#endif

    m_GI_has_artifact = false;
    m_need_rh_workaround = false;
    m_need_srgb_workaround = false;
    m_need_srgb_visual_workaround = false;
    m_need_vertex_id_workaround = false;

    // Call to glGetIntegerv should not be made if --no-graphics is used
    if (!ProfileWorld::isNoGraphics())
    {

    }
    if (!ProfileWorld::isNoGraphics())
    {
        glGetIntegerv(GL_MAJOR_VERSION, &m_gl_major_version);
        glGetIntegerv(GL_MINOR_VERSION, &m_gl_minor_version);
        Log::info("IrrDriver", "OpenGL version: %d.%d", m_gl_major_version, m_gl_minor_version);
        Log::info("IrrDriver", "OpenGL vendor: %s", glGetString(GL_VENDOR));
        Log::info("IrrDriver", "OpenGL renderer: %s", glGetString(GL_RENDERER));
        Log::info("IrrDriver", "OpenGL version string: %s", glGetString(GL_VERSION));
    }
#if !defined(USE_GLES2)
    m_glsl = (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 1))
           && !UserConfigParams::m_force_legacy_device;
#else
    m_glsl = m_gl_major_version >= 3 && !UserConfigParams::m_force_legacy_device;
#endif
    if (!ProfileWorld::isNoGraphics())
        initGL();

    if (!ProfileWorld::isNoGraphics())
    {
        std::string driver((char*)(glGetString(GL_VERSION)));
        std::string card((char*)(glGetString(GL_RENDERER)));
        std::string vendor((char*)(glGetString(GL_VENDOR)));
        GraphicsRestrictions::init(driver, card, vendor);

        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FORCE_LEGACY_DEVICE))
        {
            m_glsl = false;
        }

#if !defined(USE_GLES2)
        if (hasGLExtension("GL_AMD_vertex_shader_layer")) {
            hasVSLayer = true;
            Log::info("GLDriver", "AMD Vertex Shader Layer Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_BUFFER_STORAGE) &&
            hasGLExtension("GL_ARB_buffer_storage")  )
        {
            hasBufferStorage = true;
            Log::info("GLDriver", "ARB Buffer Storage Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_BASE_INSTANCE) &&
            hasGLExtension("GL_ARB_base_instance")) {
            hasBaseInstance = true;
            Log::info("GLDriver", "ARB Base Instance Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_DRAW_INDIRECT) &&
            hasGLExtension("GL_ARB_draw_indirect")) {
            hasDrawIndirect = true;
            Log::info("GLDriver", "ARB Draw Indirect Present");
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
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_BINDLESS_TEXTURE) &&
            hasGLExtension("GL_ARB_bindless_texture")) {
            hasBindlessTexture = true;
            Log::info("GLDriver", "ARB Bindless Texture Present");
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
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_MULTI_DRAW_INDIRECT) &&
            hasGLExtension("GL_ARB_multi_draw_indirect")) {
            hasMultiDrawIndirect = true;
            Log::info("GLDriver", "ARB Multi Draw Indirect Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_EXT_TEXTURE_COMPRESSION_S3TC) &&
            hasGLExtension("GL_EXT_texture_compression_s3tc") &&
            hasGLExtension("GL_ARB_texture_compression_rgtc"))
        {
            hasTextureCompression = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC Present");
            Log::info("GLDriver", "ARB Texture Compression RGTC Present");
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
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB) &&
            (hasGLExtension("GL_ARB_framebuffer_sRGB") || m_glsl == true))
        {
            hasSRGBFramebuffer = true;
            Log::info("GLDriver", "ARB framebuffer sRGB Present");
        }

        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_GI))
        {
            m_GI_has_artifact = true;
        }

        // Specific disablement
        if (strstr((const char *)glGetString(GL_VENDOR), "NVIDIA") != NULL)
        {
            // Fix for Nvidia and instanced RH
            // Compiler crashes with a big loop in RH or GI shaders
            m_need_rh_workaround = true;
        }

        if (strstr((const char *)glGetString(GL_VENDOR), "ATI") != NULL)
        {
            // Bindless textures are all treated RGB even sRGB one
            m_need_srgb_workaround = true;
        }

        // Check if visual is sRGB-capable
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB) &&
            GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB_WORKAROUND2) &&
            m_glsl == true)
        {
            GLint param = GL_SRGB;
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_BACK_LEFT,
                              GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &param);
            m_need_srgb_visual_workaround = (param != GL_SRGB);
            
            if (m_need_srgb_visual_workaround)
            {
                hasSRGBFramebuffer = false;
            }
        }
#else
        if (m_glsl == true)
        {
            hasTextureStorage = true;
            hasTextureSwizzle = true;
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

bool CentralVideoSettings::needRHWorkaround() const
{
    return m_need_rh_workaround;
}

bool CentralVideoSettings::needsRGBBindlessWorkaround() const
{
    return m_need_srgb_workaround;
}

bool CentralVideoSettings::needsSRGBCapableVisualWorkaround() const
{
    return m_need_srgb_visual_workaround;
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
    return hasUBO;
}

bool CentralVideoSettings::isARBExplicitAttribLocationUsable() const
{
    return hasExplicitAttribLocation;
}

bool CentralVideoSettings::isEXTTextureCompressionS3TCUsable() const
{
    return hasTextureCompression;
}

bool CentralVideoSettings::isARBBaseInstanceUsable() const
{
    return hasBaseInstance;
}

bool CentralVideoSettings::isARBDrawIndirectUsable() const
{
    return hasDrawIndirect;
}

bool CentralVideoSettings::isAMDVertexShaderLayerUsable() const
{
    return hasVSLayer;
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

bool CentralVideoSettings::isARBBindlessTextureUsable() const
{
    return hasBindlessTexture;
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

bool CentralVideoSettings::isARBMultiDrawIndirectUsable() const
{
    return hasMultiDrawIndirect;
}

bool CentralVideoSettings::isEXTTextureFilterAnisotropicUsable() const
{
    return hasTextureFilterAnisotropic;
}

bool CentralVideoSettings::isARBSRGBFramebufferUsable() const
{
    return hasSRGBFramebuffer;
}

#if defined(USE_GLES2)
bool CentralVideoSettings::isEXTTextureFormatBGRA8888Usable() const
{
    return hasBGRA;
}

bool CentralVideoSettings::isEXTColorBufferFloatUsable() const
{
    return hasColorBufferFloat;
}
#endif

bool CentralVideoSettings::supportsShadows() const
{
    return isARBGeometryShadersUsable() && isARBUniformBufferObjectUsable() && isARBExplicitAttribLocationUsable();
}

bool CentralVideoSettings::supportsGlobalIllumination() const
{
    return isARBGeometryShadersUsable() && isARBUniformBufferObjectUsable() && isARBExplicitAttribLocationUsable() && !m_GI_has_artifact;
}

bool CentralVideoSettings::supportsIndirectInstancingRendering() const
{
    return isARBBaseInstanceUsable() && isARBDrawIndirectUsable();
}

bool CentralVideoSettings::supportsComputeShadersFiltering() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable() && isARBComputeShaderUsable() && isARBArraysOfArraysUsable();
}

bool CentralVideoSettings::supportsAsyncInstanceUpload() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable();
}

bool CentralVideoSettings::supportsTextureCompression() const
{
    return isEXTTextureCompressionS3TCUsable() && isARBSRGBFramebufferUsable();
}

bool CentralVideoSettings::isShadowEnabled() const
{
    return supportsShadows() && (UserConfigParams::m_shadows_resolution > 0);
}

bool CentralVideoSettings::isGlobalIlluminationEnabled() const
{
    return supportsGlobalIllumination() && UserConfigParams::m_gi;
}

bool CentralVideoSettings::isTextureCompressionEnabled() const
{
    return supportsTextureCompression() && UserConfigParams::m_texture_compression;
}

// See http://visual-computing.intel-research.net/art/publications/sdsm/
bool CentralVideoSettings::isSDSMEnabled() const
{
    return isShadowEnabled() && isARBShaderAtomicCountersUsable() && isARBShaderStorageBufferObjectUsable() && isARBComputeShaderUsable() && isARBImageLoadStoreUsable() && UserConfigParams::m_sdsm;
}

// See http://fr.slideshare.net/CassEveritt/approaching-zero-driver-overhead
bool CentralVideoSettings::isAZDOEnabled() const
{
    return supportsIndirectInstancingRendering() && isARBBindlessTextureUsable() && isARBMultiDrawIndirectUsable() && UserConfigParams::m_azdo;
}

// Switch between Exponential Shadow Map (better but slower filtering) and Percentage Closer Filtering (faster but with some stability issue)
bool CentralVideoSettings::isESMEnabled() const
{
    return UserConfigParams::m_esm;
}

bool CentralVideoSettings::isDefferedEnabled() const
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

bool CentralVideoSettings::supportsThreadedTextureLoading() const
{
    return isARBPixelBufferObjectUsable() && isARBBufferStorageUsable() && isARBTextureStorageUsable();
}

#endif   // !SERVER_ONLY
