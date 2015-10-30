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

/*
struct MaterialType
{
    Shader *m_first_pass_shader;
    Shader *m_second_pass_shader;
    Shader *m_shadow_shader;
    Shader *m_reflective_shadow_map_shader;
    
    Material::ShaderType m_shader_type;
};*/

/*
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

const STK::Tuple<size_t> DefaultMaterial::FirstPassTextures
    = STK::Tuple<size_t>(1);
const STK::Tuple<size_t, size_t> DefaultMaterial::SecondPassTextures
    = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<> DefaultMaterial::ShadowTextures;
const STK::Tuple<size_t> DefaultMaterial::RSMTextures = STK::Tuple<size_t>(0);
*/

#endif //HEADER_MATERIAL_TYPE_HPP
