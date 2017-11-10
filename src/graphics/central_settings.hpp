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
    bool hasBufferStorage;
    bool hasComputeShaders;
    bool hasArraysOfArrays;
    bool hasTextureStorage;
    bool hasTextureView;
    bool hasBindlessTexture;
    bool hasUBO;
    bool hasExplicitAttribLocation;
    bool hasGS;
    bool hasTextureCompression;
    bool hasAtomics;
    bool hasSSBO;
    bool hasImageLoadStore;
    bool hasMultiDrawIndirect;
    bool hasTextureFilterAnisotropic;
    bool hasTextureSwizzle;
    bool hasPixelBufferObject;
    bool hasSRGBFramebuffer;

#if defined(USE_GLES2)
    bool hasBGRA;
    bool hasColorBufferFloat;
#endif

    bool m_need_rh_workaround;
    bool m_need_srgb_workaround;
    bool m_need_srgb_visual_workaround;
    bool m_need_vertex_id_workaround;
    bool m_GI_has_artifact;
public:
    void init();
    bool isGLSL() const;
    unsigned getGLSLVersion() const;

    // Needs special handle ?
    bool needRHWorkaround() const;
    bool needsRGBBindlessWorkaround() const;
    bool needsSRGBCapableVisualWorkaround() const;
    bool needsVertexIdWorkaround() const;

    // Extension is available and safe to use
    bool isARBUniformBufferObjectUsable() const;
    bool isEXTTextureCompressionS3TCUsable() const;
    bool isARBTextureViewUsable() const;
    bool isARBGeometryShadersUsable() const;
    bool isARBTextureStorageUsable() const;
    bool isAMDVertexShaderLayerUsable() const;
    bool isARBComputeShaderUsable() const;
    bool isARBArraysOfArraysUsable() const;
    bool isARBBindlessTextureUsable() const;
    bool isARBBufferStorageUsable() const;
    bool isARBBaseInstanceUsable() const;
    bool isARBDrawIndirectUsable() const;
    bool isARBShaderAtomicCountersUsable() const;
    bool isARBShaderStorageBufferObjectUsable() const;
    bool isARBImageLoadStoreUsable() const;
    bool isARBMultiDrawIndirectUsable() const;
    bool isARBExplicitAttribLocationUsable() const;
    bool isEXTTextureFilterAnisotropicUsable() const;
    bool isARBTextureSwizzleUsable() const;
    bool isARBPixelBufferObjectUsable() const;
    bool isARBSRGBFramebufferUsable() const;

#if defined(USE_GLES2)
    bool isEXTTextureFormatBGRA8888Usable() const;
    bool isEXTColorBufferFloatUsable() const;
#endif


    // Are all required extensions available for feature support
    bool supportsShadows() const;
    bool supportsGlobalIllumination() const;
    bool supportsIndirectInstancingRendering() const;
    bool supportsComputeShadersFiltering() const;
    bool supportsAsyncInstanceUpload() const;
    bool supportsHardwareSkinning() const;
    bool supportsThreadedTextureLoading() const;
    bool supportsTextureCompression() const;

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
