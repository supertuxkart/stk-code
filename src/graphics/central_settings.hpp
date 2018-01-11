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

#include <string>

class CentralVideoSettings
{
private:
    /** Supports GLSL */
    bool                  m_glsl;

    int m_gl_major_version, m_gl_minor_version;
    std::string m_vs_layer_extension;
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
    bool hasTextureFilterAnisotropic;
    bool hasTextureSwizzle;
    bool hasPixelBufferObject;
    bool hasSRGBFramebuffer;
    bool hasSamplerObjects;
    bool hasVertexType2101010Rev;
    bool hasInstancedArrays;

#if defined(USE_GLES2)
    bool hasBGRA;
    bool hasColorBufferFloat;
#endif

    bool m_need_rh_workaround;
    bool m_need_vertex_id_workaround;
    bool m_GI_has_artifact;
public:
    static bool m_supports_sp;

    void init();
    bool isGLSL() const;
    unsigned getGLSLVersion() const;

    // Needs special handle ?
    bool needRHWorkaround() const;
    bool needsVertexIdWorkaround() const;

    // Extension is available and safe to use
    bool isARBUniformBufferObjectUsable() const;
    bool isEXTTextureCompressionS3TCUsable() const;
    bool isARBTextureViewUsable() const;
    bool isARBGeometryShadersUsable() const;
    bool isARBTextureStorageUsable() const;
    bool isARBComputeShaderUsable() const;
    bool isARBArraysOfArraysUsable() const;
    bool isARBBindlessTextureUsable() const;
    bool isARBBufferStorageUsable() const;
    bool isARBShaderAtomicCountersUsable() const;
    bool isARBShaderStorageBufferObjectUsable() const;
    bool isARBImageLoadStoreUsable() const;
    bool isARBExplicitAttribLocationUsable() const;
    bool isEXTTextureFilterAnisotropicUsable() const;
    bool isARBTextureSwizzleUsable() const;
    bool isARBPixelBufferObjectUsable() const;
    bool isARBSRGBFramebufferUsable() const;
    bool isARBSamplerObjectsUsable() const;
    bool isARBVertexType2101010RevUsable() const;
    bool useArrayTextures() const;
    bool isARBInstancedArraysUsable() const;

    const std::string& getVSLayerExtension() const { return m_vs_layer_extension; }

#if defined(USE_GLES2)
    bool isEXTTextureFormatBGRA8888Usable() const;
    bool isEXTColorBufferFloatUsable() const;
#endif


    // Are all required extensions available for feature support
    bool supportsShadows() const;
    bool supportsGlobalIllumination() const;
    bool supportsComputeShadersFiltering() const;
    bool supportsHardwareSkinning() const;
    bool supportsTextureCompression() const;

    bool supportsGLLayerInVertexShader() const;

    // "Macro" around feature support and user config
    bool isShadowEnabled() const;
    bool isGlobalIlluminationEnabled() const;
    bool isTextureCompressionEnabled() const;
    bool isSDSMEnabled() const;
    bool isESMEnabled() const;
    bool isDefferedEnabled() const;
};

extern CentralVideoSettings* CVS;

#endif
