#include "stkscenemanager.hpp"
#include "stkmesh.hpp"
#include "irr_driver.hpp"
#include <ISceneManager.h>
#include <ISceneNode.h>
#include "stkanimatedmesh.hpp"
#include "stkmeshscenenode.hpp"
#include "utils/ptr_vector.hpp"
#include <ICameraSceneNode.h>
#include <SViewFrustum.h>
#include "callbacks.hpp"
#include "utils/cpp2011.hpp"
#include <omp.h>
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "lod_node.hpp"
#include <unordered_map>

static void
FillInstances_impl(std::vector<std::pair<GLMesh *, scene::ISceneNode *> > InstanceList, InstanceData * InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer,
    size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &PolyCount)
{
    // Should never be empty
    GLMesh *mesh = InstanceList.front().first;

    DrawElementsIndirectCommand &CurrentCommand = CommandBuffer[CommandBufferOffset++];
    CurrentCommand.baseVertex = mesh->vaoBaseVertex;
    CurrentCommand.count = mesh->IndexCount;
    CurrentCommand.firstIndex = mesh->vaoOffset / 2;
    CurrentCommand.baseInstance = InstanceBufferOffset;
    CurrentCommand.instanceCount = InstanceList.size();

    PolyCount += InstanceList.size() * mesh->IndexCount / 3;

    for (unsigned i = 0; i < InstanceList.size(); i++)
    {
        auto &Tp = InstanceList[i];
        InstanceData &Instance = InstanceBuffer[InstanceBufferOffset++];
        scene::ISceneNode *node = Tp.second;
        const core::matrix4 &mat = node->getAbsoluteTransformation();
        const core::vector3df &Origin = mat.getTranslation();
        const core::vector3df &Orientation = mat.getRotationDegrees();
        const core::vector3df &Scale = mat.getScale();
        Instance.Origin.X = Origin.X;
        Instance.Origin.Y = Origin.Y;
        Instance.Origin.Z = Origin.Z;
        Instance.Orientation.X = Orientation.X;
        Instance.Orientation.Y = Orientation.Y;
        Instance.Orientation.Z = Orientation.Z;
        Instance.Scale.X = Scale.X;
        Instance.Scale.Y = Scale.Y;
        Instance.Scale.Z = Scale.Z;
        Instance.Texture = mesh->TextureHandles[0];
        Instance.SecondTexture = mesh->TextureHandles[1];
    }
}


static void
FillInstancesGlow_impl(std::vector<std::pair<GLMesh *, STKMeshCommon *> > InstanceList, GlowInstanceData * InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer,
size_t &InstanceBufferOffset, size_t &CommandBufferOffset)
{
    // Should never be empty
    GLMesh *mesh = InstanceList.front().first;

    DrawElementsIndirectCommand &CurrentCommand = CommandBuffer[CommandBufferOffset++];
    CurrentCommand.baseVertex = mesh->vaoBaseVertex;
    CurrentCommand.count = mesh->IndexCount;
    CurrentCommand.firstIndex = mesh->vaoOffset / 2;
    CurrentCommand.baseInstance = InstanceBufferOffset;
    CurrentCommand.instanceCount = InstanceList.size();

    for (unsigned i = 0; i < InstanceList.size(); i++)
    {
        STKMeshSceneNode *node = dynamic_cast<STKMeshSceneNode*>(InstanceList[i].second);
        GlowInstanceData &Instance = InstanceBuffer[InstanceBufferOffset++];
        const core::matrix4 &mat = node->getAbsoluteTransformation();
        const core::vector3df &Origin = mat.getTranslation();
        const core::vector3df &Orientation = mat.getRotationDegrees();
        const core::vector3df &Scale = mat.getScale();
        Instance.Color = node->getGlowColor().color;
        Instance.Origin.X = Origin.X;
        Instance.Origin.Y = Origin.Y;
        Instance.Origin.Z = Origin.Z;
        Instance.Orientation.X = Orientation.X;
        Instance.Orientation.Y = Orientation.Y;
        Instance.Orientation.Z = Orientation.Z;
        Instance.Scale.X = Scale.X;
        Instance.Scale.Y = Scale.Y;
        Instance.Scale.Z = Scale.Z;
    }
}


static
void FillInstances(const std::unordered_map<scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > &GatheredGLMesh, std::vector<GLMesh *> &InstancedList,
    InstanceData *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &Polycount)
{
    auto It = GatheredGLMesh.begin(), E = GatheredGLMesh.end();
    for (; It != E; ++It)
    {
        FillInstances_impl(It->second, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, Polycount);
        if (!UserConfigParams::m_azdo)
            InstancedList.push_back(It->second.front().first);
    }
}

static
void FillInstancesGrass(const std::unordered_map<scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > &GatheredGLMesh, std::vector<GLMesh *> &InstancedList,
    InstanceData *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, const core::vector3df &dir, size_t &PolyCount)
{
    auto It = GatheredGLMesh.begin(), E = GatheredGLMesh.end();
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
    for (; It != E; ++It)
    {
        FillInstances_impl(It->second, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, PolyCount);
        if (!UserConfigParams::m_azdo)
            InstancedList.push_back(It->second.front().first);
    }
}

static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > MeshForSolidPass[MAT_COUNT];
static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > MeshForShadowPass[4][MAT_COUNT];
static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > MeshForRSMPass[MAT_COUNT];
static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, STKMeshCommon *> > > MeshForGlowPass;

static core::vector3df windDir;

static void
handleSTKCommon(scene::ISceneNode *Node, std::vector<scene::ISceneNode *> *ImmediateDraw, bool IsCulledForSolid, bool IsCulledForShadow[4], bool IsCulledForRSM)
{
    STKMeshCommon *node = dynamic_cast<STKMeshCommon*>(Node);
    if (!node)
        return;
    node->update();

    if (node->isImmediateDraw())
    {
        ImmediateDraw->push_back(Node);
        return;
    }
    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
    {
        GLMesh *mesh;
        if (!IsCulledForSolid)
        {
            if (irr_driver->hasARB_draw_indirect())
            {
                for_in(mesh, node->MeshSolidMaterial[Mat])
                {
                    if (node->glow())
                        MeshForGlowPass[mesh->mb].push_back(std::make_pair(mesh, node));
                    core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                    ModelMatrix.getInverse(InvModelMatrix);

                    if (mesh->TextureMatrix.isIdentity())
                        MeshForSolidPass[Mat][mesh->mb].push_back(std::make_pair(mesh, Node));
                    else
                    {
                        switch (Mat)
                        {
                        case MAT_DEFAULT:
                            ListMatDefault::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_ALPHA_REF:
                            ListMatAlphaRef::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_UNLIT:
                            ListMatUnlit::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        }
                    }
                }
            }
            else
            {
                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for_in(mesh, node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case MAT_DEFAULT:
                        ListMatDefault::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_ALPHA_REF:
                        ListMatAlphaRef::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_DETAIL:
                        ListMatDetails::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_UNLIT:
                        ListMatUnlit::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_SPHEREMAP:
                        ListMatSphereMap::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_SPLATTING:
                        ListMatSplatting::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix));
                        break;
                    case MAT_GRASS:
                        ListMatGrass::getInstance()->SolidPass.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, windDir));
                        break;
                    }
                }
            }
        }
        for (unsigned cascade = 0; cascade < 4; ++cascade)
        {
            if (!IsCulledForShadow[cascade])
            {
                if (irr_driver->hasARB_draw_indirect())
                {
                    for_in(mesh, node->MeshSolidMaterial[Mat])
                        MeshForShadowPass[cascade][Mat][mesh->mb].push_back(std::make_pair(mesh, Node));
                }
                else
                {
                    core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                    ModelMatrix.getInverse(InvModelMatrix);

                    for_in(mesh, node->MeshSolidMaterial[Mat])
                    {
                        switch (Mat)
                        {
                        case MAT_DEFAULT:
                            ListMatDefault::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_ALPHA_REF:
                            ListMatAlphaRef::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_NORMAL_MAP:
                            ListMatNormalMap::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_DETAIL:
                            ListMatDetails::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_UNLIT:
                            ListMatUnlit::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_SPHEREMAP:
                            ListMatSphereMap::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                            break;
                        case MAT_SPLATTING:
                            ListMatSplatting::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix));
                            break;
                        case MAT_GRASS:
                            ListMatGrass::getInstance()->Shadows[cascade].push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, windDir));
                        }
                    }
                }
            }
        }
        if (!IsCulledForRSM)
        {
            if (irr_driver->hasARB_draw_indirect())
            {
                for_in(mesh, node->MeshSolidMaterial[Mat])
                    MeshForRSMPass[Mat][mesh->mb].push_back(std::make_pair(mesh, Node));
            }
            else
            {
                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for_in(mesh, node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case MAT_DEFAULT:
                        ListMatDefault::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_ALPHA_REF:
                        ListMatAlphaRef::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_DETAIL:
                        ListMatDetails::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_UNLIT:
                        ListMatUnlit::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_SPHEREMAP:
                        ListMatSphereMap::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix));
                        break;
                    case MAT_SPLATTING:
                        ListMatSplatting::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix));
                        break;
                    case MAT_GRASS:
                        ListMatGrass::getInstance()->RSM.push_back(STK::make_tuple(mesh, ModelMatrix, InvModelMatrix, windDir));
                        break;
                    }
                }
            }
        }
    }
    // Transparent
    if (!IsCulledForSolid)
    {
        GLMesh *mesh;
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

            for_in(mesh, node->TransparentMesh[TM_DEFAULT])
                pushVector(ListBlendTransparentFog::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix,
                fogmax, startH, endH, start, end, col);
            for_in(mesh, node->TransparentMesh[TM_ADDITIVE])
                pushVector(ListAdditiveTransparentFog::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix,
                fogmax, startH, endH, start, end, col);
        }
        else
        {
            for_in(mesh, node->TransparentMesh[TM_DEFAULT])
                pushVector(ListBlendTransparent::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix);
            for_in(mesh, node->TransparentMesh[TM_ADDITIVE])
                pushVector(ListAdditiveTransparent::getInstance(), mesh, Node->getAbsoluteTransformation(), mesh->TextureMatrix);
        }
        for_in(mesh, node->TransparentMesh[TM_DISPLACEMENT])
            pushVector(ListDisplacement::getInstance(), mesh, Node->getAbsoluteTransformation());
    }
}

static void
parseSceneManager(core::list<scene::ISceneNode*> List, std::vector<scene::ISceneNode *> *ImmediateDraw,
    scene::ICameraSceneNode* cam, scene::ICameraSceneNode *shadowCams[4], scene::ICameraSceneNode *RSM_cam)
{
    core::list<scene::ISceneNode*>::Iterator I = List.begin(), E = List.end();
    for (; I != E; ++I)
    {
        if (LODNode *node = dynamic_cast<LODNode *>(*I))
            node->updateVisibility();
        if (!(*I)->isVisible())
            continue;
        (*I)->updateAbsolutePosition();

        core::aabbox3d<f32> tbox = (*I)->getBoundingBox();
        (*I)->getAbsoluteTransformation().transformBoxEx(tbox);

        bool IsCulledForSolid = !(tbox.intersectsWithBox(cam->getViewFrustum()->getBoundingBox()));

        bool IsCulledForShadow[4];
        for (unsigned i = 0; i < 4; ++i)
            IsCulledForShadow[i] = !(tbox.intersectsWithBox(shadowCams[i]->getViewFrustum()->getBoundingBox()));

        bool IsCulledForRSM = !(tbox.intersectsWithBox(RSM_cam->getViewFrustum()->getBoundingBox()));

        if (!IsCulledForSolid)
        {
            if (ParticleSystemProxy *node = dynamic_cast<ParticleSystemProxy *>(*I))
            {
                if (node->update())
                    ParticlesList::getInstance()->push_back(node);
                continue;
            }
        }

        handleSTKCommon(*I, ImmediateDraw, IsCulledForSolid, IsCulledForShadow, IsCulledForRSM);

        parseSceneManager((*I)->getChildren(), ImmediateDraw, cam, shadowCams, RSM_cam);
    }
}

template<MeshMaterial Mat> static void
GenDrawCalls(unsigned cascade, std::vector<GLMesh *> &InstancedList,
    InstanceData *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &PolyCount)
{
    if (irr_driver->hasARB_draw_indirect())
        ShadowPassCmd::getInstance()->Offset[cascade][Mat] = CommandBufferOffset; // Store command buffer offset
    FillInstances(MeshForShadowPass[cascade][Mat], InstancedList, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, PolyCount);
    if (UserConfigParams::m_azdo)
        ShadowPassCmd::getInstance()->Size[cascade][Mat] = CommandBufferOffset - ShadowPassCmd::getInstance()->Offset[cascade][Mat];
}

template<MeshMaterial Mat> static void
GenDrawCallsGrass(unsigned cascade, std::vector<GLMesh *> &InstancedList,
InstanceData *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, const core::vector3df &dir, size_t &PolyCount)
{
    if (irr_driver->hasARB_draw_indirect())
        ShadowPassCmd::getInstance()->Offset[cascade][Mat] = CommandBufferOffset; // Store command buffer offset
    FillInstancesGrass(MeshForShadowPass[cascade][Mat], InstancedList, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, dir, PolyCount);
    if (UserConfigParams::m_azdo)
        ShadowPassCmd::getInstance()->Size[cascade][Mat] = CommandBufferOffset - ShadowPassCmd::getInstance()->Offset[cascade][Mat];
}

void IrrDriver::PrepareDrawCalls(scene::ICameraSceneNode *camnode)
{
    windDir = getWindDir();
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

    ImmediateDrawList::getInstance()->clear();
    ParticlesList::getInstance()->clear();
    ListInstancedGlow::getInstance()->clear();

    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
    {
        MeshForSolidPass[Mat].clear();
        MeshForRSMPass[Mat].clear();
        for (unsigned cascade = 0; cascade < 4; ++cascade)
            MeshForShadowPass[cascade][Mat].clear();
    }
    MeshForGlowPass.clear();
    core::list<scene::ISceneNode*> List = m_scene_manager->getRootSceneNode()->getChildren();

    bool isCulled[4] = {};

    // Add a 20 ms timeout
    if (!m_sync)
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    GLenum reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
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
    parseSceneManager(List, ImmediateDrawList::getInstance(), camnode, m_shadow_camnodes, m_suncam);
    if (!irr_driver->hasARB_draw_indirect())
        return;

    InstanceData *InstanceBuffer;
    InstanceData *ShadowInstanceBuffer;
    InstanceData *RSMInstanceBuffer;
    GlowInstanceData *GlowInstanceBuffer;
    DrawElementsIndirectCommand *CmdBuffer;
    DrawElementsIndirectCommand *ShadowCmdBuffer;
    DrawElementsIndirectCommand *RSMCmdBuffer;
    DrawElementsIndirectCommand *GlowCmdBuffer;

    if (irr_driver->hasBufferStorageExtension())
    {
        InstanceBuffer = (InstanceData*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeDefault);
        ShadowInstanceBuffer = (InstanceData*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeShadow);
        RSMInstanceBuffer = (InstanceData*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeRSM);
        GlowInstanceBuffer = (GlowInstanceData*)VAOManager::getInstance()->getInstanceBufferPtr(InstanceTypeGlow);
        CmdBuffer = SolidPassCmd::getInstance()->Ptr;
        ShadowCmdBuffer = ShadowPassCmd::getInstance()->Ptr;
        GlowCmdBuffer = GlowPassCmd::getInstance()->Ptr;
        RSMCmdBuffer = RSMPassCmd::getInstance()->Ptr;
    }

    ListInstancedMatDefault::getInstance()->clear();
    ListInstancedMatAlphaRef::getInstance()->clear();
    ListInstancedMatGrass::getInstance()->clear();
    ListInstancedMatNormalMap::getInstance()->clear();
    ListInstancedMatSphereMap::getInstance()->clear();
    ListInstancedMatDetails::getInstance()->clear();
    ListInstancedMatUnlit::getInstance()->clear();

    size_t SolidPoly = 0, ShadowPoly = 0, MiscPoly = 0;

#pragma omp parallel sections
    {
#pragma omp section
        {
            size_t offset = 0, current_cmd = 0;
            if (!irr_driver->hasBufferStorageExtension())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeDefault));
                InstanceBuffer = (InstanceData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, SolidPassCmd::getInstance()->drawindirectcmd);
                CmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            // Default Material
            SolidPassCmd::getInstance()->Offset[MAT_DEFAULT] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_DEFAULT], ListInstancedMatDefault::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_DEFAULT] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_DEFAULT];
            // Alpha Ref
            SolidPassCmd::getInstance()->Offset[MAT_ALPHA_REF] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_ALPHA_REF], ListInstancedMatAlphaRef::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_ALPHA_REF] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_ALPHA_REF];
            // Unlit
            SolidPassCmd::getInstance()->Offset[MAT_UNLIT] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_UNLIT], ListInstancedMatUnlit::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_UNLIT] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_UNLIT];
            // Spheremap
            SolidPassCmd::getInstance()->Offset[MAT_SPHEREMAP] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_SPHEREMAP], ListInstancedMatSphereMap::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_SPHEREMAP] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_SPHEREMAP];
            // Detail
            SolidPassCmd::getInstance()->Offset[MAT_DETAIL] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_DETAIL], ListInstancedMatDetails::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_DETAIL] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_DETAIL];
            // Normal Map
            SolidPassCmd::getInstance()->Offset[MAT_NORMAL_MAP] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_NORMAL_MAP], ListInstancedMatNormalMap::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_NORMAL_MAP] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_NORMAL_MAP];

            // Grass
            SolidPassCmd::getInstance()->Offset[MAT_GRASS] = current_cmd;
            FillInstancesGrass(MeshForSolidPass[MAT_GRASS], ListInstancedMatGrass::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, windDir, SolidPoly);
            SolidPassCmd::getInstance()->Size[MAT_GRASS] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_GRASS];

            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        {
            size_t offset = 0, current_cmd = 0;

            if (!irr_driver->hasBufferStorageExtension())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeGlow));
                GlowInstanceBuffer = (GlowInstanceData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GlowPassCmd::getInstance()->drawindirectcmd);
                GlowCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            // Glow
            if (irr_driver->hasARB_draw_indirect())
                GlowPassCmd::getInstance()->Offset = offset; // Store command buffer offset

            auto It = MeshForGlowPass.begin(), E = MeshForGlowPass.end();
            for (; It != E; ++It)
            {
                FillInstancesGlow_impl(It->second, GlowInstanceBuffer, GlowCmdBuffer, offset, current_cmd);
                if (!UserConfigParams::m_azdo)
                    ListInstancedGlow::getInstance()->push_back(It->second.front().first);
            }

            if (UserConfigParams::m_azdo)
                GlowPassCmd::getInstance()->Size = current_cmd - GlowPassCmd::getInstance()->Offset;

            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        {
            irr_driver->setPhase(SHADOW_PASS);

            size_t offset = 0, current_cmd = 0;
            if (!irr_driver->hasBufferStorageExtension())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeShadow));
                ShadowInstanceBuffer = (InstanceData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ShadowPassCmd::getInstance()->drawindirectcmd);
                ShadowCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            for (unsigned i = 0; i < 4; i++)
            {
                // Mat default
                GenDrawCalls<MAT_DEFAULT>(i, ListInstancedMatDefault::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat AlphaRef
                GenDrawCalls<MAT_ALPHA_REF>(i, ListInstancedMatAlphaRef::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Unlit
                GenDrawCalls<MAT_UNLIT>(i, ListInstancedMatUnlit::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat NormalMap
                GenDrawCalls<MAT_NORMAL_MAP>(i, ListInstancedMatNormalMap::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Spheremap
                GenDrawCalls<MAT_SPHEREMAP>(i, ListInstancedMatSphereMap::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Detail
                GenDrawCalls<MAT_DETAIL>(i, ListInstancedMatDetails::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
                // Mat Grass
                GenDrawCallsGrass<MAT_GRASS>(i, ListInstancedMatGrass::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, windDir, ShadowPoly);
            }
            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        {
            size_t offset = 0, current_cmd = 0;
            if (!irr_driver->hasBufferStorageExtension())
            {
                glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getInstanceBuffer(InstanceTypeRSM));
                RSMInstanceBuffer = (InstanceData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RSMPassCmd::getInstance()->drawindirectcmd);
                RSMCmdBuffer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            }

            // Default Material
            RSMPassCmd::getInstance()->Offset[MAT_DEFAULT] = current_cmd;
            FillInstances(MeshForRSMPass[MAT_DEFAULT], ListInstancedMatDefault::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[MAT_DEFAULT] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_DEFAULT];
            // Alpha Ref
            RSMPassCmd::getInstance()->Offset[MAT_ALPHA_REF] = current_cmd;
            FillInstances(MeshForRSMPass[MAT_ALPHA_REF], ListInstancedMatAlphaRef::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[MAT_ALPHA_REF] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_ALPHA_REF];
            // Unlit
            RSMPassCmd::getInstance()->Offset[MAT_UNLIT] = current_cmd;
            FillInstances(MeshForRSMPass[MAT_UNLIT], ListInstancedMatUnlit::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[MAT_UNLIT] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_UNLIT];
            // Detail
            RSMPassCmd::getInstance()->Offset[MAT_DETAIL] = current_cmd;
            FillInstances(MeshForRSMPass[MAT_DETAIL], ListInstancedMatDetails::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[MAT_DETAIL] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_DETAIL];
            // Normal Map
            RSMPassCmd::getInstance()->Offset[MAT_NORMAL_MAP] = current_cmd;
            FillInstances(MeshForRSMPass[MAT_NORMAL_MAP], ListInstancedMatNormalMap::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly);
            RSMPassCmd::getInstance()->Size[MAT_NORMAL_MAP] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_NORMAL_MAP];

            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
    }
    poly_count[SOLID_NORMAL_AND_DEPTH_PASS] += SolidPoly;
    poly_count[SHADOW_PASS] += ShadowPoly;

    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
}