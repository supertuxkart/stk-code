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

#include "graphics/draw_calls.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/materials.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "graphics/vao_manager.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

using namespace irr;

namespace
{
    void FixBoundingBoxes(scene::ISceneNode* node)
    {
        for (scene::ISceneNode *child : node->getChildren())
        {
            FixBoundingBoxes(child);
            const_cast<core::aabbox3df&>(node->getBoundingBox()).addInternalBox(child->getBoundingBox());
        }
    }
    

} //namespace



void DrawCalls::clearLists()
{
    ListBlendTransparent::getInstance()->clear();
    ListAdditiveTransparent::getInstance()->clear();
    ListBlendTransparentFog::getInstance()->clear();
    ListAdditiveTransparentFog::getInstance()->clear();
    ListDisplacement::getInstance()->clear();

    ListMatDefault::getInstance()->clear();
    ListMatAlphaRef::getInstance()->clear();
    ListMatSphereMap::getInstance()->clear();
    ListMatDetails::getInstance()->clear();
    ListMatUnlit::getInstance()->clear();
    ListMatNormalMap::getInstance()->clear();
    ListMatGrass::getInstance()->clear();
    ListMatSplatting::getInstance()->clear();

    m_immediate_draw_list.clear();
    m_billboard_list.clear();
    m_particles_list.clear();
    ListInstancedGlow::getInstance()->clear();    
}

//TODO: rename (shadow pass only)
template<Material::ShaderType Mat, typename T>
void DrawCalls::genDrawCalls(unsigned cascade,
                            std::vector<GLMesh *> &InstancedList,
                            T *InstanceBuffer,
                             DrawElementsIndirectCommand *CommandBuffer,
                             size_t &InstanceBufferOffset,
                             size_t &CommandBufferOffset,
                             size_t &PolyCount)
{
    if (CVS->supportsIndirectInstancingRendering())
        ShadowPassCmd::getInstance()->Offset[cascade][Mat] = CommandBufferOffset; // Store command buffer offset
        
    FillInstances<T>(m_shadow_pass_mesh[Mat][cascade],
                     InstancedList,
                     InstanceBuffer,
                     CommandBuffer,
                     InstanceBufferOffset,
                     CommandBufferOffset,
                     PolyCount);
    
    if (CVS->isAZDOEnabled())
        ShadowPassCmd::getInstance()->Size[cascade][Mat] = CommandBufferOffset - ShadowPassCmd::getInstance()->Offset[cascade][Mat];
}


void DrawCalls::handleSTKCommon(scene::ISceneNode *Node,
                            std::vector<scene::ISceneNode *> *ImmediateDraw,
                            const scene::ICameraSceneNode *cam,
                            scene::ICameraSceneNode *shadowcam[4],
                            const scene::ICameraSceneNode *rsmcam,
                            bool &culledforcam,
                            bool culledforshadowcam[4],
                            bool &culledforrsm,
                            bool drawRSM)
{
    STKMeshCommon *node = dynamic_cast<STKMeshCommon*>(Node);
    if (!node)
        return;
    node->updateNoGL();
    m_deferred_update.push_back(node);


    const core::matrix4 &trans = Node->getAbsoluteTransformation();

    core::vector3df edges[8];
    Node->getBoundingBox().getEdges(edges);
    for (unsigned i = 0; i < 8; i++)
        trans.transformVect(edges[i]);

    /* From irrlicht
       /3--------/7
      / |       / |
     /  |      /  |
    1---------5   |
    |  /2- - -|- -6
    | /       |  /
    |/        | /
    0---------4/
    */

    if (irr_driver->getBoundingBoxesViz())
    {
        addEdge(edges[0], edges[1]);
        addEdge(edges[1], edges[5]);
        addEdge(edges[5], edges[4]);
        addEdge(edges[4], edges[0]);
        addEdge(edges[2], edges[3]);
        addEdge(edges[3], edges[7]);
        addEdge(edges[7], edges[6]);
        addEdge(edges[6], edges[2]);
        addEdge(edges[0], edges[2]);
        addEdge(edges[1], edges[3]);
        addEdge(edges[5], edges[7]);
        addEdge(edges[4], edges[6]);
    }

    if (node->isImmediateDraw())
    {
        ImmediateDraw->push_back(Node);
        return;
    }

    culledforcam = culledforcam || isCulledPrecise(cam, Node);
    culledforrsm = culledforrsm || isCulledPrecise(rsmcam, Node);
    for (unsigned i = 0; i < 4; i++)
        culledforshadowcam[i] = culledforshadowcam[i] || isCulledPrecise(shadowcam[i], Node);

    // Transparent

    if (World::getWorld() && World::getWorld()->isFogEnabled())
    {
        const Track * const track = World::getWorld()->getTrack();

        // Todo : put everything in a ubo
        const float fogmax = track->getFogMax();
        const float startH = track->getFogStartHeight();
        const float endH = track->getFogEndHeight();
        const float start = track->getFogStart();
        const float end = track->getFogEnd();
        const video::SColor tmpcol = track->getFogColor();

        video::SColorf col(tmpcol.getRed() / 255.0f,
            tmpcol.getGreen() / 255.0f,
            tmpcol.getBlue() / 255.0f);

        for (GLMesh *mesh : node->TransparentMesh[TM_DEFAULT])
            pushVector(ListBlendTransparentFog::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix,
            fogmax, startH, endH, start, end, col);
        for (GLMesh *mesh : node->TransparentMesh[TM_ADDITIVE])
            pushVector(ListAdditiveTransparentFog::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix,
            fogmax, startH, endH, start, end, col);
    }
    else
    {
        for (GLMesh *mesh : node->TransparentMesh[TM_DEFAULT])
            pushVector(ListBlendTransparent::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix);
        for (GLMesh *mesh : node->TransparentMesh[TM_ADDITIVE])
            pushVector(ListAdditiveTransparent::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix);
    }
    for (GLMesh *mesh : node->TransparentMesh[TM_DISPLACEMENT])
        pushVector(ListDisplacement::getInstance(), mesh, Node->getAbsoluteTransformation());

    if (!culledforcam)
    {
        for (unsigned Mat = 0; Mat < Material::SHADERTYPE_COUNT; ++Mat)
        {
            if (CVS->supportsIndirectInstancingRendering())
            {
                for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                {
                    if (node->glow())
                        m_glow_pass_mesh[mesh->mb].emplace_back(mesh, Node);

                    if (Mat != Material::SHADERTYPE_SPLATTING && mesh->TextureMatrix.isIdentity())
                        m_solid_pass_mesh[Mat][mesh->mb].emplace_back(mesh, Node);
                    else
                    {
                        core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                        ModelMatrix.getInverse(InvModelMatrix);
                        switch (Mat)
                        {
                        case Material::SHADERTYPE_SOLID:
                            ListMatDefault::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                            break;
                        case Material::SHADERTYPE_ALPHA_TEST:
                            ListMatAlphaRef::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                            break;
                        case Material::SHADERTYPE_SOLID_UNLIT:
                            ListMatUnlit::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                            break;
                        case Material::SHADERTYPE_SPLATTING:
                            ListMatSplatting::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                            break;
                        }
                    }
                }
            }
            else
            {
                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case Material::SHADERTYPE_SOLID:
                        ListMatDefault::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_ALPHA_TEST:
                        ListMatAlphaRef::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_DETAIL_MAP:
                        ListMatDetails::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SOLID_UNLIT:
                        ListMatUnlit::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPHERE_MAP:
                        ListMatSphereMap::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPLATTING:
                        ListMatSplatting::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                        break;
                    case Material::SHADERTYPE_VEGETATION:
                        ListMatGrass::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                        break;
                    }
                }
            }
        }
    }
    if (!CVS->isShadowEnabled())
        return;
    for (unsigned cascade = 0; cascade < 4; ++cascade)
    {
        if (culledforshadowcam[cascade])
            continue;
        for (unsigned Mat = 0; Mat < Material::SHADERTYPE_COUNT; ++Mat)
        {
            if (CVS->supportsIndirectInstancingRendering())
            {
                for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                {
                    if (Mat != Material::SHADERTYPE_SPLATTING)
                        m_shadow_pass_mesh[Mat][cascade][mesh->mb].emplace_back(mesh, Node);
                    else
                    {
                        core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                        ModelMatrix.getInverse(InvModelMatrix);
                        ListMatSplatting::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix);
                    }
                }
            }
            else
            {
                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case Material::SHADERTYPE_SOLID:
                        ListMatDefault::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_ALPHA_TEST:
                        ListMatAlphaRef::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_DETAIL_MAP:
                        ListMatDetails::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SOLID_UNLIT:
                        ListMatUnlit::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPHERE_MAP:
                        ListMatSphereMap::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPLATTING:
                        ListMatSplatting::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix);
                        break;
                    case Material::SHADERTYPE_VEGETATION:
                        ListMatGrass::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                    }
                }
            }
        }
    }
    if (!UserConfigParams::m_gi || !drawRSM)
        return;
    if (!culledforrsm)
    {
        for (unsigned Mat = 0; Mat < Material::SHADERTYPE_COUNT; ++Mat)
        {
            if (CVS->supportsIndirectInstancingRendering())
            {
                if (Mat == Material::SHADERTYPE_SPLATTING)
                    for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                    {
                        core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                        ModelMatrix.getInverse(InvModelMatrix);
                       ListMatSplatting::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                     }
                else
                {
                    for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                        m_reflective_shadow_map_mesh[Mat][mesh->mb].emplace_back(mesh, Node);
                }
            }
            else
            {

                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for (GLMesh *mesh : node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case Material::SHADERTYPE_SOLID:
                        ListMatDefault::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_ALPHA_TEST:
                        ListMatAlphaRef::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_DETAIL_MAP:
                        ListMatDetails::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SOLID_UNLIT:
                        ListMatUnlit::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPHERE_MAP:
                        ListMatSphereMap::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case Material::SHADERTYPE_SPLATTING:
                        ListMatSplatting::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                        break;
                    case Material::SHADERTYPE_VEGETATION:
                        ListMatGrass::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                        break;
                    }
                }
            }
        }
    }
}


void DrawCalls::parseSceneManager(core::list<scene::ISceneNode*> &List,
                                  std::vector<scene::ISceneNode *> *ImmediateDraw,
                                  const scene::ICameraSceneNode* cam,
                                  scene::ICameraSceneNode *shadow_cam[4],
                                  const scene::ICameraSceneNode *rsmcam,
                                  bool culledforcam,
                                  bool culledforshadowcam[4],
                                  bool culledforrsm,
                                  bool drawRSM)
{
    core::list<scene::ISceneNode*>::Iterator I = List.begin(), E = List.end();
    for (; I != E; ++I)
    {
        if (LODNode *node = dynamic_cast<LODNode *>(*I))
            node->updateVisibility();
        (*I)->updateAbsolutePosition();
        if (!(*I)->isVisible())
            continue;

        if (ParticleSystemProxy *node = dynamic_cast<ParticleSystemProxy *>(*I))
        {
            if (!isCulledPrecise(cam, *I))
                m_particles_list.push_back(node);
            continue;
        }

        if (STKBillboard *node = dynamic_cast<STKBillboard *>(*I))
        {
            if (!isCulledPrecise(cam, *I))
                m_billboard_list.push_back(node);
            continue;
        }

        bool newculledforcam = culledforcam;
        bool newculledforrsm = culledforrsm;
        bool newculledforshadowcam[4] = { culledforshadowcam[0], culledforshadowcam[1], culledforshadowcam[2], culledforshadowcam[3] };

        handleSTKCommon(*I, ImmediateDraw, cam, shadow_cam, rsmcam, newculledforcam, newculledforshadowcam, newculledforrsm, drawRSM);

        parseSceneManager(const_cast<core::list<scene::ISceneNode*>& >((*I)->getChildren()), ImmediateDraw, cam, shadow_cam, rsmcam, newculledforcam, newculledforshadowcam, newculledforrsm, drawRSM);
    }
}


void DrawCalls::prepareDrawCalls( ShadowMatrices& shadow_matrices, scene::ICameraSceneNode *camnode)
{
    windDir = getWindDir();
    clearLists();
    
    for (unsigned Mat = 0; Mat < Material::SHADERTYPE_COUNT; ++Mat)
    {
        m_solid_pass_mesh[Mat].clear();
        m_reflective_shadow_map_mesh[Mat].clear();
        for (unsigned i = 0; i < 4; i++)
            m_shadow_pass_mesh[Mat][i].clear();
    }

    m_glow_pass_mesh.clear();
    m_deferred_update.clear();
    
    core::list<scene::ISceneNode*> List = irr_driver->getSceneManager()->getRootSceneNode()->getChildren();


    PROFILER_PUSH_CPU_MARKER("- culling", 0xFF, 0xFF, 0x0);
    for (scene::ISceneNode *child : List)
        FixBoundingBoxes(child);

    bool cam = false, rsmcam = false;
    bool shadowcam[4] = { false, false, false, false };
    parseSceneManager(List,
                      &m_immediate_draw_list,
                      camnode, 
                      shadow_matrices.getShadowCamNodes(),
                      shadow_matrices.getSunCam(),
                      cam, shadowcam, rsmcam,
                      !shadow_matrices.isRSMMapAvail());
    PROFILER_POP_CPU_MARKER();

    // Add a 1 s timeout
    if (!m_sync)
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    PROFILER_PUSH_CPU_MARKER("- Sync Stall", 0xFF, 0x0, 0x0);
    GLenum reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    while (reason != GL_ALREADY_SIGNALED)
    {
        if (reason == GL_WAIT_FAILED) break;
        StkTime::sleep(1);
        reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    }
    glDeleteSync(m_sync);
    PROFILER_POP_CPU_MARKER();

    /*    switch (reason)
    {
    case GL_ALREADY_SIGNALED:
    printf("Already Signaled\n");
    break;
    case GL_TIMEOUT_EXPIRED:
    printf("Timeout Expired\n");
    break;
    case GL_CONDITION_SATISFIED:
    printf("Condition Satisfied\n");
    break;
    case GL_WAIT_FAILED:
    printf("Wait Failed\n");
    break;
    }*/

    PROFILER_PUSH_CPU_MARKER("- Animations/Buffer upload", 0x0, 0x0, 0x0);
    for (unsigned i = 0; i < m_deferred_update.size(); i++)
        m_deferred_update[i]->updateGL();
    PROFILER_POP_CPU_MARKER();

    if (!CVS->supportsIndirectInstancingRendering())
        return;

    InstanceDataSingleTex *ShadowInstanceBuffer;
    InstanceDataSingleTex *RSMInstanceBuffer;
    GlowInstanceData *GlowInstanceBuffer;
    DrawElementsIndirectCommand *ShadowCmdBuffer;
    DrawElementsIndirectCommand *RSMCmdBuffer;
    DrawElementsIndirectCommand *GlowCmdBuffer;

    int enableOpenMP = 0;
    
    if (CVS->supportsAsyncInstanceUpload())
    {
        ShadowInstanceBuffer = (InstanceDataSingleTex*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeShadow);
        RSMInstanceBuffer = (InstanceDataSingleTex*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeRSM);
        GlowInstanceBuffer = (GlowInstanceData*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeGlow);
        ShadowCmdBuffer = ShadowPassCmd::getInstance()->Ptr;
        GlowCmdBuffer = GlowPassCmd::getInstance()->Ptr;
        RSMCmdBuffer = RSMPassCmd::getInstance()->Ptr;
        enableOpenMP = 1;
    }

    ListInstancedMatDefault::getInstance()->clear();
    ListInstancedMatAlphaRef::getInstance()->clear();
    ListInstancedMatGrass::getInstance()->clear();
    ListInstancedMatNormalMap::getInstance()->clear();
    ListInstancedMatSphereMap::getInstance()->clear();
    ListInstancedMatDetails::getInstance()->clear();
    ListInstancedMatUnlit::getInstance()->clear();


    
    //TODO: split command upload in separate functions
    size_t SolidPoly = 0, ShadowPoly = 0, MiscPoly = 0;

    PROFILER_PUSH_CPU_MARKER("- Draw Command upload", 0xFF, 0x0, 0xFF);

#pragma omp parallel sections if(enableOpenMP)
    {
#pragma omp section
        {
            m_solid_cmd_buffer.fill(m_solid_pass_mesh);
        }
#pragma omp section
        {
            size_t offset = 0, current_cmd = 0;

            if (!CVS->supportsAsyncInstanceUpload())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeGlow));
                GlowInstanceBuffer = (GlowInstanceData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceDataDualTex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GlowPassCmd::getInstance()->drawindirectcmd);
                GlowCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            // Glow
            if (CVS->supportsIndirectInstancingRendering())
                GlowPassCmd::getInstance()->Offset = offset; // Store command buffer offset

            auto It = m_glow_pass_mesh.begin(), E = m_glow_pass_mesh.end();//TODO
            for (; It != E; ++It)
            {
                size_t Polycnt = 0;
                FillInstances_impl<GlowInstanceData>(It->second, GlowInstanceBuffer, GlowCmdBuffer, offset, current_cmd, Polycnt);
                if (!CVS->isAZDOEnabled())
                    ListInstancedGlow::getInstance()->push_back(It->second.front().first);
            }

            if (CVS->isAZDOEnabled())
                GlowPassCmd::getInstance()->Size = current_cmd - GlowPassCmd::getInstance()->Offset;

            if (!CVS->supportsAsyncInstanceUpload())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        {
            irr_driver->setPhase(SHADOW_PASS);

            size_t offset = 0, current_cmd = 0;
            if (!CVS->supportsAsyncInstanceUpload())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeShadow));
                ShadowInstanceBuffer = (InstanceDataSingleTex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceDataDualTex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ShadowPassCmd::getInstance()->drawindirectcmd);
                ShadowCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            for (unsigned i = 0; i < 4; i++)
            {
                // Mat default
                genDrawCalls<Material::SHADERTYPE_SOLID>(i, ListInstancedMatDefault::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat AlphaRef
                genDrawCalls<Material::SHADERTYPE_ALPHA_TEST>(i, ListInstancedMatAlphaRef::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Unlit
                genDrawCalls<Material::SHADERTYPE_SOLID_UNLIT>(i, ListInstancedMatUnlit::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat NormalMap
                genDrawCalls<Material::SHADERTYPE_NORMAL_MAP>(i, ListInstancedMatNormalMap::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Spheremap
                genDrawCalls<Material::SHADERTYPE_SPHERE_MAP>(i, ListInstancedMatSphereMap::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Detail
                genDrawCalls<Material::SHADERTYPE_DETAIL_MAP>(i, ListInstancedMatDetails::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Grass
                genDrawCalls<Material::SHADERTYPE_VEGETATION>(i, ListInstancedMatGrass::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
            }
            if (!CVS->supportsAsyncInstanceUpload())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        if (!shadow_matrices.isRSMMapAvail())
        {
            size_t offset = 0, current_cmd = 0;
            if (!CVS->supportsAsyncInstanceUpload())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeRSM));
                RSMInstanceBuffer = (InstanceDataSingleTex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceDataDualTex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RSMPassCmd::getInstance()->drawindirectcmd);
                RSMCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            // Default Material
            RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_SOLID] = current_cmd;
            FillInstances(m_reflective_shadow_map_mesh[Material::SHADERTYPE_SOLID], ListInstancedMatDefault::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[Material::SHADERTYPE_SOLID] = current_cmd - RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_SOLID];
            // Alpha Ref
            RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_ALPHA_TEST] = current_cmd;
            FillInstances(m_reflective_shadow_map_mesh[Material::SHADERTYPE_ALPHA_TEST], ListInstancedMatAlphaRef::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[Material::SHADERTYPE_ALPHA_TEST] = current_cmd - RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_ALPHA_TEST];
            // Unlit
            RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_SOLID_UNLIT] = current_cmd;
            FillInstances(m_reflective_shadow_map_mesh[Material::SHADERTYPE_SOLID_UNLIT], ListInstancedMatUnlit::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[Material::SHADERTYPE_SOLID_UNLIT] = current_cmd - RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_SOLID_UNLIT];
            // Detail
            RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_DETAIL_MAP] = current_cmd;
            FillInstances(m_reflective_shadow_map_mesh[Material::SHADERTYPE_DETAIL_MAP], ListInstancedMatDetails::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[Material::SHADERTYPE_DETAIL_MAP] = current_cmd - RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_DETAIL_MAP];
            // Normal Map
            RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_NORMAL_MAP] = current_cmd;
            FillInstances(m_reflective_shadow_map_mesh[Material::SHADERTYPE_NORMAL_MAP], ListInstancedMatNormalMap::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[Material::SHADERTYPE_NORMAL_MAP] = current_cmd - RSMPassCmd::getInstance()->Offset[Material::SHADERTYPE_NORMAL_MAP];

            if (!CVS->supportsAsyncInstanceUpload())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
    }
    PROFILER_POP_CPU_MARKER();
    //poly_count[SOLID_NORMAL_AND_DEPTH_PASS] += SolidPoly;
    //poly_count[SHADOW_PASS] += ShadowPoly;
    //TODO: return polycount and update it in renderer class
    
    
    if (CVS->supportsAsyncInstanceUpload())
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
}

void DrawCalls::renderImmediateDrawList() const
{
    for(auto node: m_immediate_draw_list)
        node->render();
}

void DrawCalls::renderBillboardList() const
{
    for(auto billboard: m_billboard_list)
        billboard->render();
}

void DrawCalls::renderParticlesList() const
{
    for(auto particles: m_particles_list)
        particles->render();
}


 /** Draw the i-th mesh with the specified material for the solid pass
 * (require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 */ 
void DrawCalls::drawIndirectSolidCmd(Material::ShaderType shader_type, int i) const
{
    m_solid_cmd_buffer.drawIndirect(static_cast<int>(shader_type), i);
}

/** Draw the meshes with the specified material
 * (require at least OpenGL 4.3 or AZDO extensions)
 */ 
void DrawCalls::multidrawIndirectSolidCmd(Material::ShaderType shader_type) const
{
    m_solid_cmd_buffer.multidrawIndirect(static_cast<int>(shader_type));
}

void DrawCalls::drawIndirectSolidFirstPass() const
{
    m_solid_cmd_buffer.drawIndirect<DefaultMaterial>();
    m_solid_cmd_buffer.drawIndirect<AlphaRef>();
    m_solid_cmd_buffer.drawIndirect<UnlitMat>();
    m_solid_cmd_buffer.drawIndirect<SphereMap>();
    m_solid_cmd_buffer.drawIndirect<GrassMat>(windDir);
    m_solid_cmd_buffer.drawIndirect<DetailMat>();
    m_solid_cmd_buffer.drawIndirect<NormalMat>();  
}


void DrawCalls::drawIndirectSolidSecondPass(const std::vector<GLuint> &prefilled_tex) const
{

    m_solid_cmd_buffer.drawIndirect2ndPass<DefaultMaterial>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirect2ndPass<AlphaRef>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirect2ndPass<UnlitMat>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirect2ndPass<SphereMap>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirect2ndPass<GrassMat>(prefilled_tex, windDir, irr_driver->getSunDirection());
    m_solid_cmd_buffer.drawIndirect2ndPass<DetailMat>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirect2ndPass<NormalMat>(prefilled_tex);
}


