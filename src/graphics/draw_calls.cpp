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

// ----------------------------------------------------------------------------
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
}

// ----------------------------------------------------------------------------
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
                    {
                        m_glow_pass_mesh[mesh->mb].m_mesh = mesh;
                        m_glow_pass_mesh[mesh->mb].m_scene_nodes.emplace_back(Node);
                    }

                    if (Mat != Material::SHADERTYPE_SPLATTING && mesh->TextureMatrix.isIdentity())
                    {
                        m_solid_pass_mesh[Mat][mesh->mb].m_mesh = mesh;
                        m_solid_pass_mesh[Mat][mesh->mb].m_scene_nodes.emplace_back(Node);
                    }
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
                        ListMatGrass::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, m_wind_dir);
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
                    m_shadow_pass_mesh[cascade * Material::SHADERTYPE_COUNT + Mat][mesh->mb].m_mesh = mesh;
                    m_shadow_pass_mesh[cascade * Material::SHADERTYPE_COUNT + Mat][mesh->mb].m_scene_nodes.emplace_back(Node);
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
                        ListMatGrass::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, m_wind_dir);
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
                    {
                        m_reflective_shadow_map_mesh[Mat][mesh->mb].m_mesh = mesh;
                        m_reflective_shadow_map_mesh[Mat][mesh->mb].m_scene_nodes.emplace_back(Node);
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
                        ListMatGrass::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, m_wind_dir);
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

// ----------------------------------------------------------------------------
 /** Prepare draw calls before scene rendering
 * \param[out] solid_poly_count Total number of polygons in objects 
 *                              that will be rendered in this frame
 * \param[out] shadow_poly_count Total number of polygons for shadow
 *                               (rendered this frame)
 */ 
void DrawCalls::prepareDrawCalls( ShadowMatrices& shadow_matrices,
                                  scene::ICameraSceneNode *camnode,
                                  unsigned &solid_poly_count,
                                  unsigned &shadow_poly_count)
{
    m_wind_dir = getWindDir();
    clearLists();
    
    for (unsigned Mat = 0; Mat < Material::SHADERTYPE_COUNT; ++Mat)
    {
        m_solid_pass_mesh[Mat].clear();
        m_reflective_shadow_map_mesh[Mat].clear();
        for (unsigned i = 0; i < 4; i++)
            m_shadow_pass_mesh[i * Material::SHADERTYPE_COUNT + Mat].clear();
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

    int enableOpenMP = 0;
    
    if (CVS->supportsAsyncInstanceUpload())
    {
        enableOpenMP = 1;
    }
    
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
            m_glow_cmd_buffer.fill(&m_glow_pass_mesh);
        }
#pragma omp section
        {
            irr_driver->setPhase(SHADOW_PASS);
            m_shadow_cmd_buffer.fill(m_shadow_pass_mesh);
        }
#pragma omp section
        if (!shadow_matrices.isRSMMapAvail())
        {
            m_reflective_shadow_map_cmd_buffer.fill(m_reflective_shadow_map_mesh);
        }
    }
    PROFILER_POP_CPU_MARKER();
    solid_poly_count = m_solid_cmd_buffer.getPolyCount();
    shadow_poly_count = m_shadow_cmd_buffer.getPolyCount();    
    
    if (CVS->supportsAsyncInstanceUpload())
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
}

// ----------------------------------------------------------------------------
void DrawCalls::renderImmediateDrawList() const
{
    for(auto node: m_immediate_draw_list)
        node->render();
}

// ----------------------------------------------------------------------------
void DrawCalls::renderBillboardList() const
{
    for(auto billboard: m_billboard_list)
        billboard->render();
}

// ----------------------------------------------------------------------------
void DrawCalls::renderParticlesList() const
{
    for(auto particles: m_particles_list)
        particles->render();
}

// ----------------------------------------------------------------------------
 /** Render the solid first pass (depth and normals)
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 */ 
void DrawCalls::drawIndirectSolidFirstPass() const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.drawIndirectFirstPass<DefaultMaterial>();
    m_solid_cmd_buffer.drawIndirectFirstPass<AlphaRef>();
    m_solid_cmd_buffer.drawIndirectFirstPass<UnlitMat>();
    m_solid_cmd_buffer.drawIndirectFirstPass<SphereMap>();
    m_solid_cmd_buffer.drawIndirectFirstPass<GrassMat>(m_wind_dir);
    m_solid_cmd_buffer.drawIndirectFirstPass<DetailMat>();
    m_solid_cmd_buffer.drawIndirectFirstPass<NormalMat>();  
}

// ----------------------------------------------------------------------------
 /** Render the solid first pass (depth and normals)
 * Require OpenGL AZDO extensions. Faster than drawIndirectSolidFirstPass.
 */ 
void DrawCalls::multidrawSolidFirstPass() const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.multidrawFirstPass<DefaultMaterial>();
    m_solid_cmd_buffer.multidrawFirstPass<AlphaRef>();
    m_solid_cmd_buffer.multidrawFirstPass<SphereMap>();
    m_solid_cmd_buffer.multidrawFirstPass<UnlitMat>();
    m_solid_cmd_buffer.multidrawFirstPass<GrassMat>(m_wind_dir);
    m_solid_cmd_buffer.multidrawFirstPass<NormalMat>();
    m_solid_cmd_buffer.multidrawFirstPass<DetailMat>();  
}

// ----------------------------------------------------------------------------
 /** Render the solid second pass (apply lighting on materials)
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 * \param prefilled_tex The textures which have been drawn 
 *                      during previous rendering passes.
 */ 
void DrawCalls::drawIndirectSolidSecondPass(const std::vector<GLuint> &prefilled_tex) const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.drawIndirectSecondPass<DefaultMaterial>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirectSecondPass<AlphaRef>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirectSecondPass<UnlitMat>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirectSecondPass<SphereMap>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirectSecondPass<GrassMat>(prefilled_tex, m_wind_dir);
    m_solid_cmd_buffer.drawIndirectSecondPass<DetailMat>(prefilled_tex);
    m_solid_cmd_buffer.drawIndirectSecondPass<NormalMat>(prefilled_tex);
}

// ----------------------------------------------------------------------------
 /** Render the solid second pass (apply lighting on materials)
 * Require OpenGL AZDO extensions. Faster than drawIndirectSolidSecondPass.
 * \param handles The handles to textures which have been drawn 
 *                during previous rendering passes.
 */
void DrawCalls::multidrawSolidSecondPass(const std::vector<uint64_t> &handles) const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.multidraw2ndPass<DefaultMaterial>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<AlphaRef>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<SphereMap>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<UnlitMat>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<NormalMat>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<DetailMat>(handles);
    m_solid_cmd_buffer.multidraw2ndPass<GrassMat>(handles, m_wind_dir);
}

// ----------------------------------------------------------------------------
 /** Render normals (for debug)
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 */ 
void DrawCalls::drawIndirectNormals() const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.drawIndirectNormals<DefaultMaterial>();
    m_solid_cmd_buffer.drawIndirectNormals<AlphaRef>();
    m_solid_cmd_buffer.drawIndirectNormals<UnlitMat>();
    m_solid_cmd_buffer.drawIndirectNormals<SphereMap>();
    m_solid_cmd_buffer.drawIndirectNormals<DetailMat>();
    m_solid_cmd_buffer.drawIndirectNormals<NormalMat>();
}

// ----------------------------------------------------------------------------
 /** Render normals (for debug)
 * Require OpenGL AZDO extensions. Faster than drawIndirectNormals.
 */ 
void DrawCalls::multidrawNormals() const
{
    m_solid_cmd_buffer.bind();
    m_solid_cmd_buffer.multidrawNormals<DefaultMaterial>();
    m_solid_cmd_buffer.multidrawNormals<AlphaRef>();
    m_solid_cmd_buffer.multidrawNormals<UnlitMat>();
    m_solid_cmd_buffer.multidrawNormals<SphereMap>();
    m_solid_cmd_buffer.multidrawNormals<DetailMat>();
    m_solid_cmd_buffer.multidrawNormals<NormalMat>();
}

// ----------------------------------------------------------------------------
 /** Render shadow map
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 * \param cascade The id of the cascading shadow map.
 */
void DrawCalls::drawIndirectShadows(unsigned cascade) const
{
    m_shadow_cmd_buffer.bind();
    m_shadow_cmd_buffer.drawIndirect<DefaultMaterial>(cascade);
    m_shadow_cmd_buffer.drawIndirect<DetailMat>(cascade);
    m_shadow_cmd_buffer.drawIndirect<AlphaRef>(cascade);
    m_shadow_cmd_buffer.drawIndirect<UnlitMat>(cascade);
    m_shadow_cmd_buffer.drawIndirect<GrassMat,irr::core::vector3df>(m_wind_dir, cascade);
    m_shadow_cmd_buffer.drawIndirect<NormalMat>(cascade);
    m_shadow_cmd_buffer.drawIndirect<SplattingMat>(cascade);
    m_shadow_cmd_buffer.drawIndirect<SphereMap>(cascade);
}

// ----------------------------------------------------------------------------
 /** Render shadow map
 * Require OpenGL AZDO extensions. Faster than drawIndirectShadows.
 * \param cascade The id of the cascading shadow map.
 */
void DrawCalls::multidrawShadows(unsigned cascade) const
{
    m_shadow_cmd_buffer.bind();
    m_shadow_cmd_buffer.multidrawShadow<DefaultMaterial>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<DetailMat>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<NormalMat>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<AlphaRef>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<UnlitMat>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<GrassMat,irr::core::vector3df>(m_wind_dir, cascade); 
    m_shadow_cmd_buffer.multidrawShadow<SplattingMat>(cascade);
    m_shadow_cmd_buffer.multidrawShadow<SphereMap>(cascade);
}

// ----------------------------------------------------------------------------
 /** Render reflective shadow map (need to be called only once per track)
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 * \param rsm_matrix The reflective shadow map matrix
 */
void DrawCalls::drawIndirectReflectiveShadowMaps(const core::matrix4 &rsm_matrix) const
{
    m_reflective_shadow_map_cmd_buffer.bind();
    m_reflective_shadow_map_cmd_buffer.drawIndirect<DefaultMaterial>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.drawIndirect<AlphaRef>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.drawIndirect<UnlitMat>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.drawIndirect<NormalMat>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.drawIndirect<DetailMat>(rsm_matrix);   
}

// ----------------------------------------------------------------------------
 /** Render reflective shadow map (need to be called only once per track)
 * Require OpenGL AZDO extensions. Faster than drawIndirectReflectiveShadowMaps.
 * \param rsm_matrix The reflective shadow map matrix
 */
void DrawCalls::multidrawReflectiveShadowMaps(const core::matrix4 &rsm_matrix) const
{
    m_reflective_shadow_map_cmd_buffer.bind();
    m_reflective_shadow_map_cmd_buffer.multidraw<DefaultMaterial>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.multidraw<NormalMat>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.multidraw<AlphaRef>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.multidraw<UnlitMat>(rsm_matrix);
    m_reflective_shadow_map_cmd_buffer.multidraw<DetailMat>(rsm_matrix);
}

// ----------------------------------------------------------------------------
 /** Render glowing objects.
 * Require at least OpenGL 4.0
 * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
 */
void DrawCalls::drawIndirectGlow() const
{
    m_glow_cmd_buffer.bind();
    m_glow_cmd_buffer.drawIndirect();
}

// ----------------------------------------------------------------------------
 /** Render glowing objects.
 * Require OpenGL AZDO extensions. Faster than drawIndirectGlow.
 */
void DrawCalls::multidrawGlow() const
{
    m_glow_cmd_buffer.bind();
    m_glow_cmd_buffer.multidraw();
}
