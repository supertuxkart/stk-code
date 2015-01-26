#ifndef CENTRAL_SETTINGS_HPP
#define CENTRAL_SETTINGS_HPP

class CentralVideoSettings
{
private:
    /** Supports GLSL */
    bool                  m_glsl;

    int m_gl_major_version, m_gl_minor_version;
    bool hasVSLayer;
    bool hasBaseInstance;
    bool hasDrawIndirect;
    bool hasBuffserStorage;
    bool hasComputeShaders;
    bool hasTextureStorage;
    bool hasTextureView;
    bool hasBindlessTexture;
    bool hasUBO;
    bool hasGS;
    bool hasTextureCompression;
    bool hasAtomics;
    bool hasSSBO;
    bool hasImageLoadStore;
    bool hasMultiDrawIndirect;

    bool m_need_rh_workaround;
    bool m_need_srgb_workaround;
public:
    void init();
    bool isGLSL() const;
    unsigned getGLSLVersion() const;

    // Needs special handle ?
    bool needRHWorkaround() const;
    bool needsRGBBindlessWorkaround() const;

    // Extension is available and safe to use
    bool isARBUniformBufferObjectUsable() const;
    bool isEXTTextureCompressionS3TCUsable() const;
    bool isARBTextureViewUsable() const;
    bool isARBGeometryShader4Usable() const;
    bool isARBTextureStorageUsable() const;
    bool isAMDVertexShaderLayerUsable() const;
    bool isARBComputeShaderUsable() const;
    bool isARBBindlessTextureUsable() const;
    bool isARBBufferStorageUsable() const;
    bool isARBBaseInstanceUsable() const;
    bool isARBDrawIndirectUsable() const;
    bool isARBShaderAtomicCountersUsable() const;
    bool isARBShaderStorageBufferObjectUsable() const;
    bool isARBImageLoadStoreUsable() const;
    bool isARBMultiDrawIndirectUsable() const;


    // Are all required extensions available for feature support
    bool supportsShadows() const;
    bool supportsGlobalIllumination() const;
    bool supportsIndirectInstancingRendering() const;
    bool supportsComputeShadersFiltering() const;
    bool supportsAsyncInstanceUpload() const;

    // "Macro" around feature support and user config
    bool isShadowEnabled() const;
    bool isGlobalIlluminationEnabled() const;
    bool isTextureCompressionEnabled() const;
    bool isSDSMEnabled() const;
    bool isAZDOEnabled() const;
    bool isESMEnabled() const;
    bool isDefferedEnabled() const;
};

extern CentralVideoSettings* CVS;

#endif