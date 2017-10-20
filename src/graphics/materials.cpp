//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#include "graphics/materials.hpp"
#include "graphics/vao_manager.hpp"

#ifndef SERVER_ONLY

// ============================================================================
// Solid Normal and depth pass shaders
ObjectPass1Shader::ObjectPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "object_pass1.frag");
    assignUniforms("ModelMatrix", "InverseModelMatrix");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // ObjectPass1Shader

// ============================================================================
// Solid Lit pass shaders
ObjectPass2Shader::ObjectPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "object_pass2.frag");
    assignUniforms("ModelMatrix", "texture_trans", "color_change");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // ObjectPass2Shader

// ============================================================================
InstancedObjectPass1Shader::InstancedObjectPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_object_pass1.frag");
    assignUniforms();
    assignSamplerNames(0, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedObjectPass1Shader

// ============================================================================
InstancedObjectRefPass1Shader::InstancedObjectRefPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_objectref_pass1.frag");
    assignUniforms();
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedObjectRefPass1Shader

// ============================================================================
ObjectRefPass2Shader::ObjectRefPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "objectref_pass2.frag");
    assignUniforms("ModelMatrix", "texture_trans", "color_change");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // ObjectRefPass2Shader

// ============================================================================
InstancedObjectPass2Shader::InstancedObjectPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_object_pass2.frag");
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedObjectPass2Shader

// ============================================================================
InstancedObjectRefPass2Shader::InstancedObjectRefPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_objectref_pass2.frag");
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}    // InstancedObjectRefPass2Shader

// ============================================================================
ShadowShader::ShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow.vert",
                            GL_FRAGMENT_SHADER, "shadow.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow.vert",
                            GL_GEOMETRY_SHADER, "shadow.geom",
                            GL_FRAGMENT_SHADER, "shadow.frag");
    }
    assignUniforms("ModelMatrix", "layer");
#endif
}   // ShadowShader

// ============================================================================
InstancedShadowShader::InstancedShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_shadow.vert",
                            GL_FRAGMENT_SHADER, "shadow.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_shadow.vert",
                            GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                            GL_FRAGMENT_SHADER, "shadow.frag");
    }
    assignUniforms("layer");
#endif
}   // InstancedShadowShader

// ============================================================================
CRSMShader::CRSMShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "rsm.vert",
                        GL_FRAGMENT_SHADER, "rsm.frag");
    assignUniforms("ModelMatrix", "texture_trans", "RSMMatrix");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // CRSMShader

// ============================================================================
SplattingRSMShader::SplattingRSMShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "rsm.vert",
                        GL_FRAGMENT_SHADER, "splatting_rsm.frag");
    assignUniforms("ModelMatrix", "RSMMatrix");
    assignSamplerNames(0, "tex_layout", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "tex_detail0", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       2, "tex_detail1", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       3, "tex_detail2", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "tex_detail3", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SplattingRSMShader

// ============================================================================
CInstancedRSMShader::CInstancedRSMShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_rsm.vert",
                        GL_FRAGMENT_SHADER, "instanced_rsm.frag");
    assignUniforms("RSMMatrix");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // CInstancedRSMShader

// ============================================================================
SphereMapShader::SphereMapShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "objectpass_spheremap.frag");
    assignUniforms("ModelMatrix", "InverseModelMatrix");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SphereMapShader

// ============================================================================
InstancedSphereMapShader::InstancedSphereMapShader()
{
    loadProgram(OBJECT,
                GL_VERTEX_SHADER, "instanced_object_pass.vert",
                GL_FRAGMENT_SHADER, "instanced_objectpass_spheremap.frag");
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedSphereMapShader

// ============================================================================
SplattingShader::SplattingShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "splatting.frag");
    assignUniforms("ModelMatrix");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex_layout", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "tex_detail0", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "tex_detail1", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       6, "tex_detail2", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       7, "tex_detail3", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SplattingShader

// ============================================================================
ObjectRefPass1Shader::ObjectRefPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "objectref_pass1.frag");
    assignUniforms("ModelMatrix", "InverseModelMatrix", "texture_trans");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // ObjectRefPass1Shader

// ============================================================================
NormalMapShader::NormalMapShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "normalmap.frag");
    assignUniforms("ModelMatrix", "InverseModelMatrix");
    assignSamplerNames(0, "normalMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glossMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // NormalMapShader

// ============================================================================
InstancedNormalMapShader::InstancedNormalMapShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_normalmap.frag");
    assignUniforms();
    assignSamplerNames(0, "normalMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glossMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedNormalMapShader

// ============================================================================
ObjectUnlitShader::ObjectUnlitShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "object_unlit.frag");
    assignUniforms("ModelMatrix", "texture_trans");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // ObjectUnlitShader

// ============================================================================
InstancedObjectUnlitShader::InstancedObjectUnlitShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_FRAGMENT_SHADER, "instanced_object_unlit.frag");
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedObjectUnlitShader

// ============================================================================
RefShadowShader::RefShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow.vert",
                            GL_FRAGMENT_SHADER, "shadowref.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow.vert",
                            GL_GEOMETRY_SHADER, "shadow.geom",
                            GL_FRAGMENT_SHADER, "shadowref.frag");
    }
    assignUniforms("ModelMatrix", "layer");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
#endif
}   // RefShadowShader

// ============================================================================
InstancedRefShadowShader::InstancedRefShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_shadow.vert",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_shadow.vert",
                            GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    assignUniforms("layer");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
#endif
}   // InstancedRefShadowShader

// ============================================================================
DisplaceMaskShader::DisplaceMaskShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "white.frag");
    assignUniforms("ModelMatrix");
}   // DisplaceMaskShader

// ============================================================================
DisplaceShader::DisplaceShader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "displace.frag");
    assignUniforms("ModelMatrix", "texture_trans", "dir", "dir2");
    assignSamplerNames(0, "displacement_tex", ST_BILINEAR_FILTERED,
                       1, "color_tex", ST_BILINEAR_FILTERED,
                       2, "mask_tex", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // DisplaceShader

// ============================================================================
NormalVisualizer::NormalVisualizer()
{
#if !defined(USE_GLES2)
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_object_pass.vert",
                        GL_GEOMETRY_SHADER, "normal_visualizer.geom",
                        GL_FRAGMENT_SHADER, "coloredquad.frag");
    assignUniforms("color");
#endif
}   // NormalVisualizer

// ============================================================================
GrassPass1Shader::GrassPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "grass_pass.vert",
                        GL_FRAGMENT_SHADER, "objectref_pass1.frag");
    assignUniforms("ModelMatrix", "InverseModelMatrix", "windDir");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // GrassPass1Shader

// ============================================================================
InstancedGrassPass1Shader::InstancedGrassPass1Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_grass.vert",
                        GL_FRAGMENT_SHADER, "instanced_objectref_pass1.frag");
    assignUniforms("windDir");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedGrassPass1Shader

// ============================================================================
GrassShadowShader::GrassShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow_grass.vert",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "shadow_grass.vert",
                            GL_GEOMETRY_SHADER, "shadow.geom",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    assignUniforms("ModelMatrix", "windDir", "layer");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
#endif
}   // GrassShadowShader

// ============================================================================
InstancedGrassShadowShader::InstancedGrassShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150)
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_grassshadow.vert",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    else
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_grassshadow.vert",
                            GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                            GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
    assignUniforms("layer", "windDir");
#endif
}   // InstancedGrassShadowShader

// ============================================================================
GrassPass2Shader::GrassPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "grass_pass.vert",
                        GL_FRAGMENT_SHADER, "grass_pass2.frag");
    assignUniforms("ModelMatrix", "windDir", "color_change");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // GrassPass2Shader

// ============================================================================
InstancedGrassPass2Shader::InstancedGrassPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "instanced_grass.vert",
                        GL_FRAGMENT_SHADER, "instanced_grass_pass2.frag");
    assignUniforms("windDir");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedGrassPass2Shader

// ============================================================================
DetailedObjectPass2Shader::DetailedObjectPass2Shader()
{
    loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                        GL_FRAGMENT_SHADER, "detailed_object_pass2.frag");
    assignUniforms("ModelMatrix");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "Detail", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}  // DetailedObjectPass2Shader

// ============================================================================
InstancedDetailedObjectPass2Shader::InstancedDetailedObjectPass2Shader()
{
    loadProgram(OBJECT,
                GL_VERTEX_SHADER, "instanced_object_pass.vert",
                GL_FRAGMENT_SHADER, "instanced_detailed_object_pass2.frag");
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "Detail", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedDetailedObjectPass2Shader

// ============================================================================
SkinnedPass1Shader::SkinnedPass1Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "object_pass1.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "InverseModelMatrix", "skinning_offset");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedPass1Shader

// ============================================================================
InstancedSkinnedPass1Shader::InstancedSkinnedPass1Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH,
                GL_VERTEX_SHADER, "instanced_skinning.vert",
                GL_FRAGMENT_SHADER, "instanced_object_pass1.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedSkinnedPass1Shader

// ============================================================================
SkinnedPass2Shader::SkinnedPass2Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "object_pass2.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "texture_trans", "color_change",
                   "skinning_offset");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedPass2Shader

// ============================================================================
InstancedSkinnedPass2Shader::InstancedSkinnedPass2Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH,
                GL_VERTEX_SHADER, "instanced_skinning.vert",
                GL_FRAGMENT_SHADER, "instanced_object_pass2.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedSkinnedPass2Shader

// ============================================================================
SkinnedRefPass1Shader::SkinnedRefPass1Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "objectref_pass1.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "InverseModelMatrix", "texture_trans",
                   "skinning_offset");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedRefPass1Shader

// ============================================================================
InstancedSkinnedRefPass1Shader::InstancedSkinnedRefPass1Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH,
                GL_VERTEX_SHADER, "instanced_skinning.vert",
                GL_FRAGMENT_SHADER, "instanced_objectref_pass1.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glosstex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}    // InstancedSkinnedRefPass1Shader

// ============================================================================
SkinnedRefPass2Shader::SkinnedRefPass2Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "objectref_pass2.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "texture_trans", "color_change",
                   "skinning_offset");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedRefPass2Shader

// ============================================================================
InstancedSkinnedRefPass2Shader::InstancedSkinnedRefPass2Shader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH,
                GL_VERTEX_SHADER, "instanced_skinning.vert",
                GL_FRAGMENT_SHADER, "instanced_objectref_pass2.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "Albedo", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       4, "SpecMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       5, "colorization_mask",
                       ST_TRILINEAR_ANISOTROPIC_FILTERED);
}    // InstancedSkinnedRefPass2Shader

// ============================================================================
SkinnedUnlitShader::SkinnedUnlitShader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "object_unlit.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "texture_trans", "skinning_offset");
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedUnlitShader

// ============================================================================
InstancedSkinnedUnlitShader::InstancedSkinnedUnlitShader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH,
                GL_VERTEX_SHADER, "instanced_skinning.vert",
                GL_FRAGMENT_SHADER, "instanced_object_unlit.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "DiffuseMap", ST_NEAREST_FILTERED,
                       1, "SpecularMap", ST_NEAREST_FILTERED,
                       2, "SSAO", ST_BILINEAR_FILTERED,
                       3, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedSkinnedUnlitShader

// ============================================================================
SkinnedNormalMapShader::SkinnedNormalMapShader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning.vert",
                              GL_FRAGMENT_SHADER, "normalmap.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "InverseModelMatrix", "skinning_offset");
    assignSamplerNames(0, "normalMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glossMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // SkinnedNormalMapShader

// ============================================================================
InstancedSkinnedNormalMapShader::InstancedSkinnedNormalMapShader()
{
    if (!CVS->supportsHardwareSkinning()) return;
    loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "instanced_skinning.vert",
                              GL_FRAGMENT_SHADER, "instanced_normalmap.frag");
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms();
    assignSamplerNames(0, "normalMap", ST_TRILINEAR_ANISOTROPIC_FILTERED,
                       1, "glossMap", ST_TRILINEAR_ANISOTROPIC_FILTERED);
}   // InstancedSkinnedNormalMapShader

// ============================================================================
SkinnedShadowShader::SkinnedShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150 || !CVS->supportsHardwareSkinning())
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning_shadow.vert",
                                  GL_FRAGMENT_SHADER, "shadow.frag");
    }
    else
    {
        loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning_shadow.vert",
                                  GL_GEOMETRY_SHADER, "shadow.geom",
                                  GL_FRAGMENT_SHADER, "shadow.frag");
    }
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "skinning_offset", "layer");
#endif
}   // SkinnedShadowShader

// ============================================================================
InstancedSkinnedShadowShader::InstancedSkinnedShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150 || !CVS->supportsHardwareSkinning())
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(SKINNED_MESH,
                    GL_VERTEX_SHADER, "instanced_skinning_shadow.vert",
                    GL_FRAGMENT_SHADER, "shadow.frag");
    }
    else
    {
        loadProgram(SKINNED_MESH,
                    GL_VERTEX_SHADER, "instanced_skinning_shadow.vert",
                    GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                    GL_FRAGMENT_SHADER, "shadow.frag");
    }
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("layer");
#endif
}   // InstancedSkinnedShadowShader

// ============================================================================
SkinnedRefShadowShader::SkinnedRefShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150 || !CVS->supportsHardwareSkinning())
         return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning_shadow.vert",
                                  GL_FRAGMENT_SHADER, "shadowref.frag");
    }
    else
    {
        loadProgram(SKINNED_MESH, GL_VERTEX_SHADER, "skinning_shadow.vert",
                                  GL_GEOMETRY_SHADER, "shadow.geom",
                                  GL_FRAGMENT_SHADER, "shadowref.frag");
    }
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("ModelMatrix", "skinning_offset", "layer");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
#endif
}   // SkinnedRefShadowShader

// ============================================================================
InstancedSkinnedRefShadowShader::InstancedSkinnedRefShadowShader()
{
#if !defined(USE_GLES2)
    // Geometry shader needed
    if (CVS->getGLSLVersion() < 150 || !CVS->supportsHardwareSkinning())
        return;
    if (CVS->isAMDVertexShaderLayerUsable())
    {
        loadProgram(SKINNED_MESH,
                    GL_VERTEX_SHADER, "instanced_skinning_shadow.vert",
                    GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    else
    {
        loadProgram(SKINNED_MESH,
                    GL_VERTEX_SHADER, "instanced_skinning_shadow.vert",
                    GL_GEOMETRY_SHADER, "instanced_shadow.geom",
                    GL_FRAGMENT_SHADER, "instanced_shadowref.frag");
    }
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(this))
    {
        sms->init(this);
    }
    assignUniforms("layer");
    assignSamplerNames(0, "tex", ST_TRILINEAR_ANISOTROPIC_FILTERED);
#endif
}   // InstancedSkinnedRefShadowShader

// ============================================================================
const InstanceType SkinnedSolid::Instance = InstanceTypeThreeTex;
const std::tuple<size_t> SkinnedSolid::FirstPassTextures
    = std::tuple<size_t>(1);
const std::tuple<size_t, size_t, size_t> SkinnedSolid::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
std::tuple<> SkinnedSolid::ShadowTextures;
const std::tuple<size_t> SkinnedSolid::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType SkinnedAlphaRef::Instance = InstanceTypeThreeTex;
const std::tuple<size_t, size_t> SkinnedAlphaRef::FirstPassTextures
    = std::tuple<size_t, size_t>(0, 1);
const std::tuple<size_t, size_t, size_t> SkinnedAlphaRef::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
const std::tuple<size_t> SkinnedAlphaRef::ShadowTextures
    = std::tuple<size_t>(0);
const std::tuple<size_t> SkinnedAlphaRef::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType SkinnedUnlitMat::Instance = InstanceTypeThreeTex;
const std::tuple<size_t, size_t> SkinnedUnlitMat::FirstPassTextures
    = std::tuple<size_t, size_t>(0, 1);
const std::tuple<size_t> SkinnedUnlitMat::SecondPassTextures
    = std::tuple<size_t>(0);
const std::tuple<size_t> SkinnedUnlitMat::ShadowTextures
    = std::tuple<size_t>(0);
const std::tuple<size_t> SkinnedUnlitMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType SkinnedNormalMat::Instance = InstanceTypeFourTex;
const std::tuple<size_t, size_t> SkinnedNormalMat::FirstPassTextures
    = std::tuple<size_t, size_t>(3, 1);
const std::tuple<size_t, size_t, size_t> SkinnedNormalMat::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
std::tuple<> SkinnedNormalMat::ShadowTextures;
const std::tuple<size_t> SkinnedNormalMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType DefaultMaterial::Instance = InstanceTypeThreeTex;
const std::tuple<size_t> DefaultMaterial::FirstPassTextures
    = std::tuple<size_t>(1);
const std::tuple<size_t, size_t, size_t> DefaultMaterial::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
std::tuple<> DefaultMaterial::ShadowTextures;
const std::tuple<size_t> DefaultMaterial::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType AlphaRef::Instance = InstanceTypeThreeTex;
const std::tuple<size_t, size_t> AlphaRef::FirstPassTextures
    = std::tuple<size_t, size_t>(0, 1);
const std::tuple<size_t, size_t, size_t> AlphaRef::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
const std::tuple<size_t> AlphaRef::ShadowTextures = std::tuple<size_t>(0);
const std::tuple<size_t> AlphaRef::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType SphereMap::Instance = InstanceTypeThreeTex;
const std::tuple<size_t> SphereMap::FirstPassTextures = std::tuple<size_t>(1);
const std::tuple<size_t> SphereMap::SecondPassTextures = std::tuple<size_t>(0);
std::tuple<> SphereMap::ShadowTextures;
const std::tuple<size_t> SphereMap::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType UnlitMat::Instance = InstanceTypeThreeTex;
const std::tuple<size_t, size_t> UnlitMat::FirstPassTextures
    = std::tuple<size_t, size_t>(0, 1);
const std::tuple<size_t> UnlitMat::SecondPassTextures = std::tuple<size_t>(0);
const std::tuple<size_t> UnlitMat::ShadowTextures = std::tuple<size_t>(0);
const std::tuple<size_t> UnlitMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType GrassMat::Instance = InstanceTypeThreeTex;
const std::tuple<size_t, size_t> GrassMat::FirstPassTextures
    = std::tuple<size_t, size_t>(0, 1);
const std::tuple<size_t, size_t, size_t> GrassMat::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
const std::tuple<size_t> GrassMat::ShadowTextures = std::tuple<size_t>(0);
const std::tuple<size_t> GrassMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType NormalMat::Instance = InstanceTypeFourTex;
const std::tuple<size_t, size_t> NormalMat::FirstPassTextures
    = std::tuple<size_t, size_t>(3, 1);
const std::tuple<size_t, size_t, size_t> NormalMat::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 1, 2);
std::tuple<> NormalMat::ShadowTextures;
const std::tuple<size_t> NormalMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const InstanceType DetailMat::Instance = InstanceTypeFourTex;
const std::tuple<size_t> DetailMat::FirstPassTextures = std::tuple<size_t>(1);
const std::tuple<size_t, size_t, size_t> DetailMat::SecondPassTextures
    = std::tuple<size_t, size_t, size_t>(0, 3, 1);
std::tuple<> DetailMat::ShadowTextures;
const std::tuple<size_t> DetailMat::RSMTextures = std::tuple<size_t>(0);

// ----------------------------------------------------------------------------
const std::tuple<size_t> SplattingMat::FirstPassTextures
    = std::tuple<size_t>(7);
const std::tuple<size_t, size_t, size_t, size_t, size_t>
    SplattingMat::SecondPassTextures
    = std::tuple<size_t, size_t, size_t, size_t, size_t>(1, 3, 4, 5, 6);
std::tuple<> SplattingMat::ShadowTextures;
const std::tuple<size_t, size_t, size_t, size_t, size_t>
    SplattingMat::RSMTextures
    = std::tuple<size_t, size_t, size_t, size_t, size_t>(1, 3, 4, 5, 6);

// ----------------------------------------------------------------------------
#endif
