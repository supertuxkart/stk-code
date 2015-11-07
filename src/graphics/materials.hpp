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

#include "graphics/shader.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_mesh.hpp"
#include "graphics/vao_manager.hpp"

class InstancedObjectPass1Shader;
class InstancedObjectPass2Shader;
class InstancedShadowShader;
class CInstancedRSMShader;
class ListInstancedMatDefault;
class ShadowShader;
class CRSMShader;
class InstancedObjectRefPass1Shader;
class InstancedObjectRefPass2Shader;
class InstancedRefShadowShader;
class ObjectRefPass1Shader;
class ObjectRefPass2Shader;
class RefShadowShader;
class InstancedSphereMapShader;
class SphereMapShader;
class InstancedObjectUnlitShader;
class ObjectUnlitShader;
class InstancedGrassPass1Shader;
class InstancedGrassPass2Shader;
class InstancedGrassShadowShader;
class GrassPass1Shader;
class GrassPass2Shader;
class GrassShadowShader;
class InstancedNormalMapShader;
class NormalMapShader;
class InstancedDetailedObjectPass2Shader;
class DetailedObjectPass2Shader;
class SplattingShader;
class SplattingRSMShader;


// ============================================================================
struct DefaultMaterial
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatDefault InstancedList;
    typedef Shaders::ObjectPass1Shader FirstPassShader;
    typedef Shaders::ObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatDefault List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType
                                               = Material::SHADERTYPE_SOLID;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // struct DefaultMaterial

// ----------------------------------------------------------------------------
struct AlphaRef
{
    typedef InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectRefPass2Shader InstancedSecondPassShader;
    typedef InstancedRefShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatAlphaRef InstancedList;
    typedef ObjectRefPass1Shader FirstPassShader;
    typedef ObjectRefPass2Shader SecondPassShader;
    typedef RefShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatAlphaRef List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_ALPHA_TEST;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // struct AlphaRef

// ----------------------------------------------------------------------------
struct SphereMap
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedSphereMapShader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatSphereMap InstancedList;
    typedef Shaders::ObjectPass1Shader FirstPassShader;
    typedef SphereMapShader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatSphereMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType 
                                          = Material::SHADERTYPE_SPHERE_MAP;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // struct SphereMap

// ----------------------------------------------------------------------------
struct UnlitMat
{
    typedef InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef InstancedObjectUnlitShader InstancedSecondPassShader;
    typedef InstancedRefShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatUnlit InstancedList;
    typedef ObjectRefPass1Shader FirstPassShader;
    typedef ObjectUnlitShader SecondPassShader;
    typedef RefShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatUnlit List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType =
                                           Material::SHADERTYPE_SOLID_UNLIT;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // struct UnlitMat

// ----------------------------------------------------------------------------
struct GrassMat
{
    typedef InstancedGrassPass1Shader InstancedFirstPassShader;
    typedef InstancedGrassPass2Shader InstancedSecondPassShader;
    typedef InstancedGrassShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatGrass InstancedList;
    typedef GrassPass1Shader FirstPassShader;
    typedef GrassPass2Shader SecondPassShader;
    typedef GrassShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatGrass List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType 
        = Material::SHADERTYPE_VEGETATION;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // GrassMat

// ----------------------------------------------------------------------------
struct NormalMat
{
    typedef InstancedNormalMapShader InstancedFirstPassShader;
    typedef InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatNormalMap InstancedList;
    typedef NormalMapShader FirstPassShader;
    typedef Shaders::ObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatNormalMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_TANGENTS;
    static const enum Material::ShaderType MaterialType
                                          = Material::SHADERTYPE_NORMAL_MAP;
    static const enum InstanceType Instance = InstanceTypeThreeTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // NormalMat

// ----------------------------------------------------------------------------
struct DetailMat
{
    typedef InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef InstancedDetailedObjectPass2Shader InstancedSecondPassShader;
    typedef InstancedShadowShader InstancedShadowPassShader;
    typedef CInstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatDetails InstancedList;
    typedef Shaders::ObjectPass1Shader FirstPassShader;
    typedef DetailedObjectPass2Shader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef CRSMShader RSMShader;
    typedef ListMatDetails List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const enum Material::ShaderType MaterialType
                                          = Material::SHADERTYPE_DETAIL_MAP;
    static const enum InstanceType Instance = InstanceTypeThreeTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};   // DetailMat

// ----------------------------------------------------------------------------
struct SplattingMat
{
    typedef Shaders::ObjectPass1Shader FirstPassShader;
    typedef SplattingShader SecondPassShader;
    typedef ShadowShader ShadowPassShader;
    typedef SplattingRSMShader RSMShader;
    typedef ListMatSplatting List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t, size_t, size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t, size_t, size_t, size_t, size_t> RSMTextures;
};   // SplattingMat



#endif //HEADER_MATERIAL_TYPE_HPP
