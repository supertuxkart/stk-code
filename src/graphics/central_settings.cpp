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
    m_GI_has_artifact = false;

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

    if (!ProfileWorld::isNoGraphics())
    {
        std::string driver((char*)(glGetString(GL_VERSION)));
        std::string card((char*)(glGetString(GL_RENDERER)));
        GraphicsRestrictions::init(driver, card);

        if (hasGLExtension("GL_AMD_vertex_shader_layer")) {
            hasVSLayer = true;
            Log::info("GLDriver", "AMD Vertex Shader Layer Present");
        }

        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_BUFFER_STORAGE) &&
            hasGLExtension("GL_ARB_buffer_storage")  )
        {
            hasBuffserStorage = true;
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
            hasGLExtension("GL_EXT_texture_compression_s3tc")) {
            hasTextureCompression = true;
            Log::info("GLDriver", "EXT Texture Compression S3TC Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_UNIFORM_BUFFER_OBJECT) &&
            hasGLExtension("GL_ARB_uniform_buffer_object")) {
            hasUBO = true;
            Log::info("GLDriver", "ARB Uniform Buffer Object Present");
        }
        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_GEOMETRY_SHADER4) &&
            hasGLExtension("GL_ARB_geometry_shader4")) {
            hasGS = true;
            Log::info("GLDriver", "ARB Geometry Shader 4 Present");
        }

        // Only unset the high def textures if they are set as default. If the
        // user has enabled them (bit 1 set), then leave them enabled.
        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_HIGHDEFINITION_TEXTURES) &&
            (UserConfigParams::m_high_definition_textures & 0x02) == 0)
        {
            UserConfigParams::m_high_definition_textures = 0x00;
        }

        if (!GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_GI))
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
    }
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
    return isARBGeometryShader4Usable() && isARBUniformBufferObjectUsable() && !m_GI_has_artifact;
}

bool CentralVideoSettings::supportsIndirectInstancingRendering() const
{
    return isARBBaseInstanceUsable() && isARBDrawIndirectUsable();
}

bool CentralVideoSettings::supportsComputeShadersFiltering() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable() && isARBComputeShaderUsable();
}

bool CentralVideoSettings::supportsAsyncInstanceUpload() const
{
    return isARBBufferStorageUsable() && isARBImageLoadStoreUsable();
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

// Switch between Exponential Shadow Map (better but slower filtering) and Percentage Closer Filtering (faster but with some stability issue)
bool CentralVideoSettings::isESMEnabled() const
{
    return UserConfigParams::m_esm;
}

bool CentralVideoSettings::isDefferedEnabled() const
{
    return UserConfigParams::m_dynamic_lights && !GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_ADVANCED_PIPELINE);
}