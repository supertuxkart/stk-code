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

#ifndef SERVER_ONLY

#include "graphics/command_buffer.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "utils/cpp2011.hpp"

// ----------------------------------------------------------------------------
template<>
void InstanceFiller<InstanceDataSingleTex>::add(GLMesh* mesh,
                                                const InstanceSettings& is,
                                                InstanceDataSingleTex& instance)
{
    fillOriginOrientationScale<InstanceDataSingleTex>(std::get<0>(is), instance);
    instance.Texture = mesh->TextureHandles[0];
    instance.skinning_offset = std::get<3>(is);
}

// ----------------------------------------------------------------------------
template<>
void InstanceFiller<InstanceDataThreeTex>::add(GLMesh* mesh,
                                               const InstanceSettings& is,
                                               InstanceDataThreeTex& instance)
{
    fillOriginOrientationScale<InstanceDataThreeTex>(std::get<0>(is), instance);
    instance.MiscData.X = std::get<1>(is).X;
    instance.MiscData.Y = std::get<1>(is).Y;
    instance.MiscData.Z = std::get<2>(is).X;
    instance.MiscData.W = std::get<2>(is).Y;
    instance.Texture = mesh->TextureHandles[0];
    instance.SecondTexture = mesh->TextureHandles[1];
    instance.ThirdTexture = mesh->TextureHandles[2];
    instance.skinning_offset = std::get<3>(is);
}

// ----------------------------------------------------------------------------
template<>
void InstanceFiller<InstanceDataFourTex>::add(GLMesh* mesh,
                                              const InstanceSettings& is,
                                              InstanceDataFourTex& instance)
{
    fillOriginOrientationScale<InstanceDataFourTex>(std::get<0>(is), instance);
    instance.MiscData.X = std::get<1>(is).X;
    instance.MiscData.Y = std::get<1>(is).Y;
    instance.MiscData.Z = std::get<2>(is).X;
    instance.MiscData.W = std::get<2>(is).Y;
    instance.Texture = mesh->TextureHandles[0];
    instance.SecondTexture = mesh->TextureHandles[1];
    instance.ThirdTexture = mesh->TextureHandles[2];
    instance.FourthTexture = mesh->TextureHandles[3];
    instance.skinning_offset = std::get<3>(is);
}

// ----------------------------------------------------------------------------
template<>
void InstanceFiller<GlowInstanceData>::add(GLMesh* mesh,
                                           const InstanceSettings& is,
                                           GlowInstanceData& instance)
{
    scene::ISceneNode* node = std::get<0>(is);
    fillOriginOrientationScale<GlowInstanceData>(node, instance);
    STKMeshSceneNode *nd = dynamic_cast<STKMeshSceneNode*>(node);
    instance.Color = nd->getGlowColor().color;
}

#if !defined(USE_GLES2)
// ----------------------------------------------------------------------------
template<int N>
void CommandBuffer<N>::clearMeshes()
{
    m_instance_buffer_offset = 0;
    m_command_buffer_offset = 0;
    m_poly_count = 0;    
    
    for(int i=0;i<N;i++)
    {
        m_meshes[i].clear();
    }   
}

// ----------------------------------------------------------------------------
template<int N>
void CommandBuffer<N>::mapIndirectBuffer()
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
        m_draw_indirect_cmd_id);
        
    m_draw_indirect_cmd = (DrawElementsIndirectCommand*)
        glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0,
                         10000 * sizeof(DrawElementsIndirectCommand),
                         GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);    
}

// ----------------------------------------------------------------------------
template<int N>
CommandBuffer<N>::CommandBuffer():
m_poly_count(0)
{
    glGenBuffers(1, &m_draw_indirect_cmd_id);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_draw_indirect_cmd_id);
    if (CVS->supportsAsyncInstanceUpload())
    {
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER,
                        10000 * sizeof(DrawElementsIndirectCommand),
                        0, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
        m_draw_indirect_cmd = (DrawElementsIndirectCommand *)
            glMapBufferRange(GL_DRAW_INDIRECT_BUFFER,
                             0, 10000 * sizeof(DrawElementsIndirectCommand),
                             GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
    }
    else
    {
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
                     10000 * sizeof(DrawElementsIndirectCommand),
                     0, GL_STREAM_DRAW);
    }    
}

// ----------------------------------------------------------------------------
SolidCommandBuffer::SolidCommandBuffer(): CommandBuffer()
{
}

// ----------------------------------------------------------------------------
void SolidCommandBuffer::fill(MeshMap *mesh_map)
{
    clearMeshes();
    
    if(!CVS->supportsAsyncInstanceUpload())
        mapIndirectBuffer();

    std::vector<int> three_tex_material_list =
        createVector<int>(Material::SHADERTYPE_SOLID,
                          Material::SHADERTYPE_ALPHA_TEST,
                          Material::SHADERTYPE_SOLID_UNLIT,
                          Material::SHADERTYPE_SPHERE_MAP,
                          Material::SHADERTYPE_VEGETATION,
                          Material::SHADERTYPE_SOLID_SKINNED_MESH,
                          Material::SHADERTYPE_ALPHA_TEST_SKINNED_MESH,
                          Material::SHADERTYPE_SOLID_UNLIT_SKINNED_MESH);

    fillInstanceData<InstanceDataThreeTex, MeshMap>
        (mesh_map, three_tex_material_list, InstanceTypeThreeTex);

    std::vector<int> four_tex_material_list =
        createVector<int>(Material::SHADERTYPE_DETAIL_MAP,
                          Material::SHADERTYPE_NORMAL_MAP,
                          Material::SHADERTYPE_NORMAL_MAP_SKINNED_MESH);

    fillInstanceData<InstanceDataFourTex, MeshMap>
        (mesh_map, four_tex_material_list, InstanceTypeFourTex);

    if (!CVS->supportsAsyncInstanceUpload())
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
} //SolidCommandBuffer::fill

// ----------------------------------------------------------------------------
ShadowCommandBuffer::ShadowCommandBuffer(): CommandBuffer()
{
}

// ----------------------------------------------------------------------------
void ShadowCommandBuffer::fill(MeshMap *mesh_map)
{
    clearMeshes();
    
    if(!CVS->supportsAsyncInstanceUpload())
        mapIndirectBuffer();
    
    std::vector<int> shadow_tex_material_list;
    for(int cascade=0; cascade<4; cascade++)
    {
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SOLID);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_ALPHA_TEST);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SOLID_UNLIT);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_NORMAL_MAP);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SPHERE_MAP);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_DETAIL_MAP);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_VEGETATION);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SPLATTING);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SOLID_SKINNED_MESH);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_ALPHA_TEST_SKINNED_MESH);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_SOLID_UNLIT_SKINNED_MESH);
        shadow_tex_material_list.push_back(cascade * Material::SHADERTYPE_COUNT
                                           + Material::SHADERTYPE_NORMAL_MAP_SKINNED_MESH);
    }
    
    fillInstanceData<InstanceDataSingleTex, MeshMap>
        (mesh_map, shadow_tex_material_list, InstanceTypeShadow);
    
    if (!CVS->supportsAsyncInstanceUpload())
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    
} //ShadowCommandBuffer::fill

// ----------------------------------------------------------------------------
ReflectiveShadowMapCommandBuffer::ReflectiveShadowMapCommandBuffer()
{
}

// ----------------------------------------------------------------------------
void ReflectiveShadowMapCommandBuffer::fill(MeshMap *mesh_map)
{
    clearMeshes();
    
    if(!CVS->supportsAsyncInstanceUpload())
        mapIndirectBuffer();
    
    std::vector<int> rsm_material_list =
        createVector<int>(Material::SHADERTYPE_SOLID,
                          Material::SHADERTYPE_ALPHA_TEST,
                          Material::SHADERTYPE_SOLID_UNLIT,
                          Material::SHADERTYPE_DETAIL_MAP,
                          Material::SHADERTYPE_NORMAL_MAP);
                          
    fillInstanceData<InstanceDataSingleTex, MeshMap>
        (mesh_map, rsm_material_list, InstanceTypeRSM);

    if (!CVS->supportsAsyncInstanceUpload())
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
        
} //ReflectiveShadowMapCommandBuffer::fill

// ----------------------------------------------------------------------------
GlowCommandBuffer::GlowCommandBuffer()
{   
}

// ----------------------------------------------------------------------------
void GlowCommandBuffer::fill(MeshMap *mesh_map)
{
    clearMeshes();
    
    if(!CVS->supportsAsyncInstanceUpload())
        mapIndirectBuffer();
    
    fillInstanceData<GlowInstanceData, MeshMap>
        (mesh_map, createVector<int>(0), InstanceTypeGlow);
    if (!CVS->supportsAsyncInstanceUpload())
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    
} //GlowCommandBuffer::fill
#endif   // !defined(USE_GLES2)

#endif   // !SERVER_ONLY