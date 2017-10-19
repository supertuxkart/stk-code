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

#ifndef HEADER_MATERIAL_TYPE_HPP
#define HEADER_MATERIAL_TYPE_HPP

#ifndef SERVER_ONLY

#include "graphics/stk_mesh.hpp"
#include "graphics/texture_shader.hpp"

enum InstanceType : unsigned int;

// ============================================================================
class ObjectPass1Shader : public TextureShader<ObjectPass1Shader, 1,
                                               core::matrix4, core::matrix4>
{
public:
    ObjectPass1Shader();
};   // ObjectPass1Shader

// ============================================================================
class ObjectPass2Shader : public TextureShader<ObjectPass2Shader, 6,
                                               core::matrix4, core::vector2df,
                                               core::vector2df >
{
public:
    ObjectPass2Shader();
};   // ObjectPass2Shader

// ============================================================================
class InstancedObjectPass1Shader
                          : public TextureShader<InstancedObjectPass1Shader, 1>
{
public:
    InstancedObjectPass1Shader();
};   // InstancedObjectPass1Shader

// ============================================================================
class InstancedObjectRefPass1Shader
                       : public TextureShader<InstancedObjectRefPass1Shader, 2>
{
public:
    InstancedObjectRefPass1Shader();
};   // InstancedObjectRefPass1Shader

// ============================================================================
class ObjectRefPass2Shader: public TextureShader<ObjectRefPass2Shader, 6,
                                                 core::matrix4,
                                                 core::vector2df,
                                                 core::vector2df>
{
public:
    ObjectRefPass2Shader();
};   // ObjectRefPass2Shader

// ============================================================================
class InstancedObjectPass2Shader
                          : public TextureShader<InstancedObjectPass2Shader, 6>
{
public:
    InstancedObjectPass2Shader();
};   // InstancedObjectPass2Shader

// ============================================================================
class InstancedObjectRefPass2Shader
                       : public TextureShader<InstancedObjectRefPass2Shader, 6>
{
public:
    InstancedObjectRefPass2Shader();
};   // InstancedObjectRefPass2Shader

// ============================================================================
class ShadowShader : public TextureShader<ShadowShader, 0, core::matrix4, int>
{
public:
    ShadowShader();
};   // ShadowShader

// ============================================================================
class InstancedShadowShader : public TextureShader<InstancedShadowShader, 0,
                                                   int>
{
public:
    InstancedShadowShader();
};   // InstancedShadowShader

// ============================================================================
class CRSMShader : public TextureShader<CRSMShader, 1, core::matrix4,
                                        core::vector2df, core::matrix4>
{
public:
    CRSMShader();
};   // CRSMShader

// ============================================================================
class SplattingRSMShader : public TextureShader<SplattingRSMShader, 5,
                                                core::matrix4, core::matrix4>
{
public:
    SplattingRSMShader();
};  // SplattingRSMShader

// ============================================================================
class CInstancedRSMShader : public TextureShader<CInstancedRSMShader, 1,
                                                 core::matrix4>
{
public:
    CInstancedRSMShader();
};   // CInstancedRSMShader

// ============================================================================
class SphereMapShader : public TextureShader<SphereMapShader, 4, core::matrix4,
                                             core::matrix4>
{
public:
    SphereMapShader();
};   // SphereMapShader

// ============================================================================
class InstancedSphereMapShader : public TextureShader<InstancedSphereMapShader,
                                                      4>
{
public:
    InstancedSphereMapShader();
};   // InstancedSphereMapShader

// ============================================================================
class SplattingShader : public TextureShader<SplattingShader, 8, core::matrix4>
{
public:
    SplattingShader();
};   // SplattingShader

// ============================================================================
class ObjectRefPass1Shader : public TextureShader<ObjectRefPass1Shader, 2,
                                                  core::matrix4, core::matrix4,
                                                  core::vector2df>
{
public:
    ObjectRefPass1Shader();
};  // ObjectRefPass1Shader

// ============================================================================
class NormalMapShader : public TextureShader<NormalMapShader, 2, core::matrix4,
                                             core::matrix4>
{
public:
    NormalMapShader();
};   // NormalMapShader

// ============================================================================
class InstancedNormalMapShader : public TextureShader<InstancedNormalMapShader,
                                                      2>
{
public:
    InstancedNormalMapShader();
};   // InstancedNormalMapShader

// ============================================================================
class ObjectUnlitShader : public TextureShader<ObjectUnlitShader, 4,
                                               core::matrix4, core::vector2df>
{
public:
    ObjectUnlitShader();
};   // ObjectUnlitShader

// ============================================================================
class InstancedObjectUnlitShader
                          : public TextureShader<InstancedObjectUnlitShader, 4>
{
public:
    InstancedObjectUnlitShader();
};   // InstancedObjectUnlitShader

// ============================================================================
class RefShadowShader : public TextureShader<RefShadowShader, 1,
                                             core::matrix4, int>
{
public:
    RefShadowShader();
};   // RefShadowShader

// ============================================================================
class InstancedRefShadowShader : public TextureShader<InstancedRefShadowShader,
                                                      1, int>
{
public:
    InstancedRefShadowShader();
};   // InstancedRefShadowShader

// ============================================================================
class DisplaceMaskShader : public Shader<DisplaceMaskShader, core::matrix4>
{
public:
    DisplaceMaskShader();
};   // DisplaceMaskShader

// ============================================================================
class DisplaceShader : public TextureShader<DisplaceShader, 4, core::matrix4,
                                            core::vector2df, core::vector2df,
                                            core::vector2df>
{
public:
    DisplaceShader();
};   // DisplaceShader

// ============================================================================
class NormalVisualizer : public Shader<NormalVisualizer, video::SColor>
{
public:
    NormalVisualizer();
};   // NormalVisualizer

// ============================================================================
class GrassPass1Shader : public TextureShader<GrassPass1Shader, 2,
                                              core::matrix4, core::matrix4,
                                              core::vector3df>
{
public:
    GrassPass1Shader();
};   // GrassPass1Shader

// ============================================================================
class InstancedGrassPass1Shader
          : public TextureShader<InstancedGrassPass1Shader, 2, core::vector3df>
{
public:
    InstancedGrassPass1Shader();
};   // InstancedGrassPass1Shader

// ============================================================================
class GrassShadowShader : public TextureShader<GrassShadowShader, 1,
                                               core::matrix4, core::vector3df,
                                               int>
{
public:
    GrassShadowShader();
};   // GrassShadowShader

// ============================================================================
class InstancedGrassShadowShader
    : public TextureShader<InstancedGrassShadowShader, 1, int, core::vector3df>
{
public:
    InstancedGrassShadowShader();
};   // InstancedGrassShadowShader

// ============================================================================
class GrassPass2Shader : public TextureShader<GrassPass2Shader, 6,
                                              core::matrix4, core::vector3df,
                                              core::vector2df>
{
public:
    GrassPass2Shader();
};   // GrassPass2Shader

// ============================================================================
class InstancedGrassPass2Shader
          : public TextureShader<InstancedGrassPass2Shader, 6, core::vector3df>
{
public:
    InstancedGrassPass2Shader();
};   // InstancedGrassPass2Shader

// ============================================================================
class DetailedObjectPass2Shader
            : public TextureShader<DetailedObjectPass2Shader, 6, core::matrix4>
{
public:
    DetailedObjectPass2Shader();
};   // DetailedObjectPass2Shader

// ============================================================================
class InstancedDetailedObjectPass2Shader
                  : public TextureShader<InstancedDetailedObjectPass2Shader, 6>
{
public:
    InstancedDetailedObjectPass2Shader();
};   // InstancedDetailedObjectPass2Shader

// ============================================================================
class SkinnedPass1Shader : public TextureShader<SkinnedPass1Shader, 1,
                                                core::matrix4, core::matrix4,
                                                int>,
                          public SkinnedMeshShader
{
public:
    SkinnedPass1Shader();
};   // SkinnedPass1Shader

// ============================================================================
class InstancedSkinnedPass1Shader
                        : public TextureShader<InstancedSkinnedPass1Shader, 1>,
                          public SkinnedMeshShader
{
public:
    InstancedSkinnedPass1Shader();
};   // InstancedSkinnedPass1Shader

// ============================================================================
class SkinnedPass2Shader : public TextureShader<SkinnedPass2Shader, 6,
                                                core::matrix4, core::vector2df,
                                                core::vector2df, int >,
                           public SkinnedMeshShader
{
public:
    SkinnedPass2Shader();
};   // SkinnedPass2Shader

// ============================================================================
class InstancedSkinnedPass2Shader
                        : public TextureShader<InstancedSkinnedPass2Shader, 6>,
                          public SkinnedMeshShader
{
public:
    InstancedSkinnedPass2Shader();
};   // InstancedSkinnedPass2Shader

// ============================================================================
class SkinnedRefPass1Shader : public TextureShader<SkinnedRefPass1Shader, 2,
                                                   core::matrix4,
                                                   core::matrix4,
                                                   core::vector2df, int>,
                              public SkinnedMeshShader
{
public:
    SkinnedRefPass1Shader();
};  // SkinnedRefPass1Shader

// ============================================================================
class InstancedSkinnedRefPass1Shader
                     : public TextureShader<InstancedSkinnedRefPass1Shader, 2>,
                       public SkinnedMeshShader
{
public:
    InstancedSkinnedRefPass1Shader();
};   // InstancedSkinnedRefPass1Shader

// ============================================================================
class SkinnedRefPass2Shader : public TextureShader<SkinnedRefPass2Shader, 6,
                                                   core::matrix4,
                                                   core::vector2df,
                                                   core::vector2df, int>,
                              public SkinnedMeshShader
{
public:
    SkinnedRefPass2Shader();
};   // SkinnedRefPass2Shader

// ============================================================================
class InstancedSkinnedRefPass2Shader
                     : public TextureShader<InstancedSkinnedRefPass2Shader, 6>,
                       public SkinnedMeshShader
{
public:
    InstancedSkinnedRefPass2Shader();
};   // InstancedSkinnedRefPass2Shader

// ============================================================================
class SkinnedUnlitShader : public TextureShader<SkinnedUnlitShader, 4,
                                                core::matrix4, core::vector2df,
                                                int>,
                           public SkinnedMeshShader
{
public:
    SkinnedUnlitShader();
};   // SkinnedUnlitShader

// ============================================================================
class InstancedSkinnedUnlitShader
                        : public TextureShader<InstancedSkinnedUnlitShader, 4>,
                          public SkinnedMeshShader
{
public:
    InstancedSkinnedUnlitShader();
};   // InstancedSkinnedUnlitShader

// ============================================================================
class SkinnedNormalMapShader : public TextureShader<SkinnedNormalMapShader, 2,
                                                    core::matrix4,
                                                    core::matrix4, int>,
                               public SkinnedMeshShader
{
public:
    SkinnedNormalMapShader();
};   // SkinnedNormalMapShader

// ============================================================================
class InstancedSkinnedNormalMapShader
                    : public TextureShader<InstancedSkinnedNormalMapShader, 2>,
                      public SkinnedMeshShader
{
public:
    InstancedSkinnedNormalMapShader();
};   // InstancedSkinnedNormalMapShader

// ============================================================================
class SkinnedShadowShader : public TextureShader<SkinnedShadowShader, 0,
                                                 core::matrix4, int, int>,
                            public SkinnedMeshShader
{
public:
    SkinnedShadowShader();
};   // SkinnedShadowShader

// ============================================================================
class InstancedSkinnedShadowShader
                  : public TextureShader<InstancedSkinnedShadowShader, 0, int>,
                    public SkinnedMeshShader
{
public:
    InstancedSkinnedShadowShader();
};   // InstancedSkinnedShadowShader

// ============================================================================
class SkinnedRefShadowShader : public TextureShader<SkinnedRefShadowShader, 1,
                                                    core::matrix4, int, int>,
                               public SkinnedMeshShader
{
public:
    SkinnedRefShadowShader();
};   // SkinnedRefShadowShader

// ============================================================================
class InstancedSkinnedRefShadowShader
               : public TextureShader<InstancedSkinnedRefShadowShader, 1, int>,
                 public SkinnedMeshShader
{
public:
    InstancedSkinnedRefShadowShader();
};   // InstancedSkinnedRefShadowShader

// ============================================================================
struct SkinnedSolid
{
    typedef InstancedSkinnedPass1Shader InstancedFirstPassShader;
    typedef InstancedSkinnedPass2Shader InstancedSecondPassShader;
    typedef InstancedSkinnedShadowShader InstancedShadowPassShader;
    typedef SkinnedPass1Shader FirstPassShader;
    typedef SkinnedPass2Shader SecondPassShader;
    typedef SkinnedShadowShader ShadowPassShader;
    // Todo: RSMs
    typedef ListSkinnedSolid List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_SKINNED_MESH;
    static const enum Material::ShaderType MaterialType
                                      = Material::SHADERTYPE_SOLID_SKINNED_MESH;
    static const enum InstanceType Instance;
    static const std::tuple<size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // SkinnedSolid

// ----------------------------------------------------------------------------
struct SkinnedAlphaRef
{
    typedef InstancedSkinnedRefPass1Shader InstancedFirstPassShader;
    typedef InstancedSkinnedRefPass2Shader InstancedSecondPassShader;
    typedef InstancedSkinnedRefShadowShader InstancedShadowPassShader;
    typedef SkinnedRefPass1Shader FirstPassShader;
    typedef SkinnedRefPass2Shader SecondPassShader;
    typedef SkinnedRefShadowShader ShadowPassShader;
    // Todo: RSMs
    typedef ListSkinnedAlphaRef List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_SKINNED_MESH;
    static const enum Material::ShaderType MaterialType =
                                   Material::SHADERTYPE_ALPHA_TEST_SKINNED_MESH;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static const std::tuple<size_t> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // SkinnedAlphaRef

// ----------------------------------------------------------------------------
struct SkinnedNormalMat
{
    typedef InstancedSkinnedNormalMapShader InstancedFirstPassShader;
    typedef InstancedSkinnedPass2Shader InstancedSecondPassShader;
    typedef InstancedSkinnedShadowShader InstancedShadowPassShader;
    typedef SkinnedNormalMapShader FirstPassShader;
    typedef SkinnedPass2Shader SecondPassShader;
    typedef SkinnedShadowShader ShadowPassShader;
    // Todo: RSMs
    typedef ListSkinnedNormalMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_SKINNED_MESH;
    static const enum Material::ShaderType MaterialType =
                                   Material::SHADERTYPE_NORMAL_MAP_SKINNED_MESH;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // SkinnedNormalMat

// ----------------------------------------------------------------------------
struct SkinnedUnlitMat
{
    typedef InstancedSkinnedRefPass1Shader InstancedFirstPassShader;
    typedef InstancedSkinnedUnlitShader InstancedSecondPassShader;
    typedef InstancedSkinnedRefShadowShader InstancedShadowPassShader;
    typedef SkinnedRefPass1Shader FirstPassShader;
    typedef SkinnedUnlitShader SecondPassShader;
    typedef SkinnedRefShadowShader ShadowPassShader;
    // Todo: RSMs
    typedef ListSkinnedUnlit List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_SKINNED_MESH;
    static const enum Material::ShaderType MaterialType =
                                  Material::SHADERTYPE_SOLID_UNLIT_SKINNED_MESH;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t> SecondPassTextures;
    static const std::tuple<size_t> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // SkinnedUnlitMat

// ----------------------------------------------------------------------------
struct DefaultMaterial
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ObjectPass1Shader FirstPassShader;
    typedef ObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatDefault List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType
                                               = Material::SHADERTYPE_SOLID;
    static const enum InstanceType Instance;
    static const std::tuple<size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // DefaultMaterial

// ----------------------------------------------------------------------------
struct AlphaRef
{
    typedef InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectRefPass2Shader InstancedSecondPassShader;
    typedef InstancedRefShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ObjectRefPass1Shader FirstPassShader;
    typedef ObjectRefPass2Shader SecondPassShader;
    typedef RefShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatAlphaRef List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_ALPHA_TEST;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static const std::tuple<size_t> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // AlphaRef

// ----------------------------------------------------------------------------
struct SphereMap
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedSphereMapShader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ObjectPass1Shader FirstPassShader;
    typedef SphereMapShader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatSphereMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType
                                          = Material::SHADERTYPE_SPHERE_MAP;
    static const enum InstanceType Instance;
    static const std::tuple<size_t> FirstPassTextures;
    static const std::tuple<size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // SphereMap

// ----------------------------------------------------------------------------
struct UnlitMat
{
    typedef InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectUnlitShader InstancedSecondPassShader;
    typedef InstancedRefShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ObjectRefPass1Shader FirstPassShader;
    typedef ObjectUnlitShader SecondPassShader;
    typedef RefShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatUnlit List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType =
                                           Material::SHADERTYPE_SOLID_UNLIT;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t> SecondPassTextures;
    static const std::tuple<size_t> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // UnlitMat

// ----------------------------------------------------------------------------
struct GrassMat
{
    typedef InstancedGrassPass1Shader InstancedFirstPassShader;
    typedef InstancedGrassPass2Shader InstancedSecondPassShader;
    typedef InstancedGrassShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef GrassPass1Shader FirstPassShader;
    typedef GrassPass2Shader SecondPassShader;
    typedef GrassShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatGrass List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType
        = Material::SHADERTYPE_VEGETATION;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static const std::tuple<size_t> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // GrassMat

// ----------------------------------------------------------------------------
struct NormalMat
{
    typedef InstancedNormalMapShader InstancedFirstPassShader;
    typedef InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef NormalMapShader FirstPassShader;
    typedef ObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatNormalMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_TANGENTS;
    static const enum Material::ShaderType MaterialType
                                          = Material::SHADERTYPE_NORMAL_MAP;
    static const enum InstanceType Instance;
    static const std::tuple<size_t, size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // NormalMat

// ----------------------------------------------------------------------------
struct DetailMat
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedDetailedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ObjectPass1Shader FirstPassShader;
    typedef DetailedObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatDetails List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const enum Material::ShaderType MaterialType
                                          = Material::SHADERTYPE_DETAIL_MAP;
    static const enum InstanceType Instance;
    static const std::tuple<size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t> SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t> RSMTextures;
};   // DetailMat

// ----------------------------------------------------------------------------
struct SplattingMat
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef ObjectPass1Shader FirstPassShader;
    typedef SplattingShader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef SplattingRSMShader RSMShader;
    typedef ListMatSplatting List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const enum Material::ShaderType MaterialType
                                           = Material::SHADERTYPE_SPLATTING;
    static const std::tuple<size_t> FirstPassTextures;
    static const std::tuple<size_t, size_t, size_t, size_t, size_t>
                                                         SecondPassTextures;
    static std::tuple<> ShadowTextures;
    static const std::tuple<size_t, size_t, size_t, size_t, size_t>
                                                                RSMTextures;
};   // SplattingMat

#endif  // !SERVER_ONLY
#endif  // HEADER_MATERIAL_TYPE_HPP
