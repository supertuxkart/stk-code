//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "graphics/irr_driver.hpp"
#include "graphics/hardware_skinning.hpp"
#include <IAnimatedMeshSceneNode.h>
#include <IAnimatedMesh.h>
#include <ISkinnedMesh.h>
#include <IGPUProgrammingServices.h>
#include <IMaterialRendererServices.h>
#include <assert.h>

void HardwareSkinning::prepareNode(scene::IAnimatedMeshSceneNode *node)
{
    return; // BOUM
    
    scene::IAnimatedMesh* mesh = node->getMesh();
    video::IVideoDriver* driver = irr_driver->getVideoDriver();
    video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
    assert(gpu);    // TODO
                
    // Create the callback
    HWSkinningCallback* callback = new HWSkinningCallback(node);
    
    // Compile the shaders and associate the callback
    s32 material_type = gpu->addHighLevelShaderMaterialFromFiles(
                                        "../data/shaders/skinning.vert", "main", video::EVST_VS_2_0,
                                        "",                              "main", video::EPST_PS_2_0,
                                        callback, video::EMT_SOLID);
    
    /*s32 material_type = gpu->addHighLevelShaderMaterialFromFiles(
                                        "../data/shaders/skinning.vert", "main", video::EVST_VS_2_0,
                                        "../data/shaders/skinning.frag", "main", video::EPST_PS_2_0,
                                        callback, video::EMT_SOLID);
    */
    
    // Drop
    callback->drop();
    
    // Assign the hardware skinning material type
    node->setMaterialType((video::E_MATERIAL_TYPE)material_type);
    
    // Use VBOs and avoid streaming the vertex data
    //mesh->setHardwareMappingHint(scene::EHM_STATIC);
    mesh->setHardwareMappingHint(scene::EHM_DYNAMIC);    // BOUM
    
    // Hardware skinning is not implemented in Irrlicht (as of version 1.7.2) so "enabling" it
    // results in the data not being sent and the CPU not computing software skinning.
    scene::ISkinnedMesh* skin_mesh = (scene::ISkinnedMesh*)mesh;
    skin_mesh->setHardwareSkinning(true);
    
    // TODO: use custom vertex attributes?
    // !FUNTO! initialize all vertex colors to 0 (used as indexes)
    for(u32 i = 0;i < skin_mesh->getMeshBuffers().size();++i)
    {
        for(u32 g = 0;g < skin_mesh->getMeshBuffers()[i]->getVertexCount();++g)
        {
            skin_mesh->getMeshBuffers()[i]->getVertex(g)->Color = video::SColor(0,0,0,0);
        }
    }
    
    // !FUNTO! set all vertex colors to indexes
    for(u32 z = 0;z < skin_mesh->getAllJoints().size();++z)
    {
        for(u32 j = 0;j < skin_mesh->getAllJoints()[z]->Weights.size();++j)
        {
            int buffId = skin_mesh->getAllJoints()[z]->Weights[j].buffer_id;

            int vertexId = skin_mesh->getAllJoints()[z]->Weights[j].vertex_id;
            video::SColor* vColor = &skin_mesh->getMeshBuffers()[buffId]->getVertex(vertexId)->Color;
            
            if(vColor->getRed() == 0)
                vColor->setRed(z + 1);
            else if(vColor->getGreen() == 0)
                vColor->setGreen(z + 1);
            else if(vColor->getBlue() == 0)
                vColor->setBlue(z + 1);
            else if(vColor->getAlpha() == 0)
                vColor->setAlpha(z + 1);
        }
    }
}

//  ------------------------------------------------------------------
HWSkinningCallback::HWSkinningCallback(scene::IAnimatedMeshSceneNode* node)
{
    m_node = node;
}

HWSkinningCallback::~HWSkinningCallback()
{
}

void HWSkinningCallback::OnSetConstants(video::IMaterialRendererServices *services, s32 userData)
{
/*    static const float colors[] = {
        1.0, 0.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 1.0,
        0.0, 0.0, 1.0, 1.0,
        1.0, 1.0, 0.0, 1.0,
        0.0, 1.0, 1.0, 1.0,
        1.0, 0.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
    };
    static const int nb_colors = 8;
    //int index = m_used_material
    
    int index = 4;
    //int index = *reinterpret_cast<const int*>(&m_used_material->MaterialTypeParam);
    //printf("OnSetConstants: index: %d\n", index);
    //index = 4 * (index % nb_colors);
    
    services->setPixelShaderConstant("debug_color", &colors[index], 4);
*/
    //! Sets a constant for the pixel shader based on a name.
 /** This can be used if you used a high level shader language like GLSL
 or HLSL to create a shader. See setVertexShaderConstant() for an
 example on how to use this.
 \param name Name of the variable
 \param floats Pointer to array of floats
 \param count Amount of floats in array.
 \return True if successful. */
 /*virtual bool setPixelShaderConstant(const c8* name, const f32* floats, int count) = 0;*/
    
    // TODO: cleanup
    // - joints
    scene::ISkinnedMesh* mesh = (scene::ISkinnedMesh*)m_node->getMesh();
    f32 JointArray[55 * 16];
    int copyIncrement = 0;

    for(u32 i = 0;i < mesh->getAllJoints().size();++i)
    {
        core::matrix4 JointVertexPull(core::matrix4::EM4CONST_NOTHING);
        JointVertexPull.setbyproduct(
        mesh->getAllJoints()[i]->GlobalAnimatedMatrix, 
        mesh->getAllJoints()[i]->GlobalInversedMatrix);
    
        f32* pointer = JointArray + copyIncrement;
        for(int i = 0;i < 16;++i)
            *pointer++ = JointVertexPull[i];
        
        copyIncrement += 16;
    }
    
    services->setVertexShaderConstant("JointTransform", JointArray, mesh->getAllJoints().size() * 16);
    
    // - mWorldViewProj
    // set clip matrix at register 4
/*    core::matrix4 worldViewProj(driver->getTransform(video::ETS_PROJECTION));
    worldViewProj *= driver->getTransform(video::ETS_VIEW);
    worldViewProj *= driver->getTransform(video::ETS_WORLD);
    services->setVertexShaderConstant(&worldViewProj.M[0], 4, 4);
*/    // for high level shading languages, this would be another solution:
    //services->setVertexShaderConstant("mWorldViewProj", worldViewProj.M, 16);
    
    // - 
}
