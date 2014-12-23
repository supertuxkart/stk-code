#include "central_settings.hpp"
#include "modes/profile_world.hpp"
#include "gl_headers.hpp"
#include "glwrap.hpp"
#include "graphics_restrictions.hpp"

CentralVideoSettings *CVS = new CentralVideoSettings();

void CentralVideoSettings::init()
{
    m_gl_major_version = 2;
    m_gl_minor_version = 1;

    // Parse extensions
    hasVSLayer = false;
    hasBaseInstance = false;
    hasBuffserStorage = false;
    hasDrawIndirect = false;
    hasComputeShaders = false;
    hasTextureStorage = false;
    hasTextureView = false;
    hasBindlessTexture = false;
    hasAtomics = false;
    hasSSBO = false;
    hasImageLoadStore = false;
    hasMultiDrawIndirect = false;
    hasTextureCompression = false;
    hasUBO = false;
    hasGS = false;

    m_need_rh_workaround = false;
    m_need_srgb_workaround = false;

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
    m_glsl = (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version >= 1));
    if (!ProfileWorld::isNoGraphics())
        initGL();

#if !defined(__APPLE__)
    if (!ProfileWorld::isNoGraphics())
    {
        std::string driver((char*)(glGetString(GL_VERSION)));
        std::string card((char*)(glGetString(GL_RENDERER)));
        std::vector<std::string> restrictions =
            GraphicsRestrictions::getRestrictions(driver, card);

        if (hasGLExtension("GL_AMD_vertex_shader_layer")) {
            hasVSLayer = true;
            Log::info("GLDriver", "AMD Vertex Shader Layer Present");
        }
        if (hasGLExtension("GL_ARB_buffer_storage")) {
            hasBuffserStorage = true;
            Log::info("GLDriver", "ARB Buffer Storage Present");
        }
        if (hasGLExtension("GL_ARB_base_instance")) {
            hasBaseInstance = true;
            Log::info("GLDriver", "ARB Base Instance Present");
        }
        if (hasGLExtension("GL_ARB_draw_indirect")) {
            hasDrawIndirect = true;
            Log::info("GLDriver", "ARB Draw Indirect Present");
        }
        if (hasGLExtension("GL_ARB_compute_shader")) {
            hasComputeShaders = true;
            Log::info("GLDriver", "ARB Compute Shader Present");
        }
        if (hasGLExtension("GL_ARB_texture_storage")) {
            hasTextureStorage = true;
            Log::info("GLDriver", "ARB Texture Storage Present");
        }
        if (hasGLExtension("GL_ARB_texture_view")) {
            hasTextureView = true;
            Log::info("GLDriver", "ARB Texture View Present");
        }
        if (hasGLExtension("GL_ARB_bindless_texture")) {
            hasBindlessTexture = true;
            Log::info("GLDriver", "ARB Bindless Texture Present");
        }
        if (hasGLExtension("GL_ARB_shader_image_load_store")) {
            hasImageLoadStore = true;
            Log::info("GLDriver", "ARB Image Load Store Present");
        }
        if (hasGLExtension("GL_ARB_shader_atomic_counters")) {
            hasAtomics = true;
            Log::info("GLDriver", "ARB Shader Atomic Counters Present");
        }
        if (hasGLExtension("GL_ARB_shader_storage_buffer_object")) {
            hasSSBO = true;
            Log::info("GLDriver", "ARB Shader Storage Buffer Object Present");
        }
        if (hasGLExtension("GL_ARB_multi_draw_indirect")) {
            hasMultiDrawIndirect = true;
            Log::info("GLDriver", "ARB Multi Draw Indirect Present");
        }
        if (hasGLExtension("GL_EXT_texture_compression_s3tc")) {
            hasTextureCompression = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC Present");
        }
        if (hasGLExtension("GL_ARB_uniform_buffer_object")) {
            hasUBO = true;
            Log::info("GLDriver", "ARB Uniform Buffer Object Present");
        }
        if (hasGLExtension("GL_ARB_geometry_shader4")) {
            hasGS = true;
            Log::info("GLDriver", "ARB Geometry Shader 4 Present");
        }
        

        // Specific disablement
        // (should use graphic restriction system)
        if (strstr((const char *)glGetString(GL_VENDOR), "Intel") != NULL)
        {
            // Intel doesnt support sRGB compressed textures on some chip/OS
            // TODO: Have a more precise list
            // Sandy Bridge on Windows
            hasTextureCompression = false;
#ifdef WIN32
            // Fix for Intel Sandy Bridge on Windows which supports GL up to 3.1 only
            // Works with Haswell and latest drivers
            // Status unknown on Ivy Bridge
            // Status unknown on older driver for Haswell
            if (m_gl_major_version == 3 && m_gl_minor_version == 1)
                hasUBO = false;
#endif
        }

        if (strstr((const char *)glGetString(GL_VENDOR), "NVIDIA") != NULL)
        {
            // Fix for Nvidia and instanced RH
            // Compiler crashes with a big loop in RH or GI shaders
            m_need_rh_workaround = true;
            // Atomic counters make the driver crash on windows and linux
            hasAtomics = false;
        }

        if (strstr((const char *)glGetString(GL_VENDOR), "ATI") != NULL)
        {
            // Bindless textures are all treated RGB even sRGB one
            m_need_srgb_workaround = true;
        }

        // Mesa
        if (strstr((const char *)glGetString(GL_VENDOR), "Intel Open Source Technology Center") != NULL ||
            strstr((const char *)glGetString(GL_VENDOR), "Gallium") != NULL)
        {
            // Needs a patched Mesa (current 10.4) to use array texture fbo
            // Technically GS works but array texture fbo interacts with GS.
            hasGS = false;
        }
    }
#endif
}

unsigned CentralVideoSettings::getGLSLVersion() const
{
    if (m_gl_major_version > 3 || (m_gl_major_version == 3 && m_gl_minor_version == 3))
        return m_gl_major_version * 100 + m_gl_minor_version * 10;
    else if (m_gl_major_version == 3)
        return 100 + (m_gl_minor_version + 3) * 10;
    else
        return 120;
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

bool CentralVideoSettings::isARBGeometryShader4Usable() const
{
    return hasGS;
}

bool CentralVideoSettings::isARBUniformBufferObjectUsable() const
{
    return hasUBO;
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
    return hasBuffserStorage;
}

bool CentralVideoSettings::isARBComputeShaderUsable() const
{
    return hasComputeShaders;
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
    return hasComputeShaders;
}

bool CentralVideoSettings::isARBMultiDrawIndirectUsable() const
{
    return hasMultiDrawIndirect;
}

bool CentralVideoSettings::supportsShadows() const
{
    return isARBGeometryShader4Usable() && isARBUniformBufferObjectUsable();
}

bool CentralVideoSettings::supportsGlobalIllumination() const
{
    return isARBGeometryShader4Usable() && isARBUniformBufferObjectUsable();
}

bool CentralVideoSettings::supportsIndirectInstancingRendering() const
{
    return isARBBaseInstanceUsable() && isARBDrawIndirectUsable();
}

bool CentralVideoSettings::supportsComputeShadersFiltering() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable();
}

bool CentralVideoSettings::isShadowEnabled() const
{
    return supportsShadows() && UserConfigParams::m_shadows;
}

bool CentralVideoSettings::isGlobalIlluminationEnabled() const
{
    return supportsGlobalIllumination() && UserConfigParams::m_gi;
}

bool CentralVideoSettings::isTextureCompressionEnabled() const
{
    return isEXTTextureCompressionS3TCUsable() && UserConfigParams::m_texture_compression;
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