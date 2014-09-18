#include "graphics/glwrap.hpp"
#include "graphics/stkscenemanager.hpp"
#include "graphics/stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include "stkanimatedmesh.hpp"
#include "stkmeshscenenode.hpp"
#include "utils/ptr_vector.hpp"
#include <ICameraSceneNode.h>
#include <SViewFrustum.h>
#include "callbacks.hpp"
#include "utils/cpp2011.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "lod_node.hpp"
#include "utils/profiler.hpp"

#include <ISceneManager.h>
#include <ISceneNode.h>

#include <unordered_map>
#include <SViewFrustum.h>
#include <functional>

template<typename T>
struct InstanceFiller
{
    static void add(GLMesh *, scene::ISceneNode *, T &);
};

template<>
void InstanceFiller<InstanceData>::add(GLMesh *mesh, scene::ISceneNode *node, InstanceData &Instance)
{
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

template<>
void InstanceFiller<GlowInstanceData>::add(GLMesh *mesh, scene::ISceneNode *node, GlowInstanceData &Instance)
{
    STKMeshSceneNode *nd = dynamic_cast<STKMeshSceneNode*>(node);
    const core::matrix4 &mat = node->getAbsoluteTransformation();
    const core::vector3df &Origin = mat.getTranslation();
    const core::vector3df &Orientation = mat.getRotationDegrees();
    const core::vector3df &Scale = mat.getScale();
    Instance.Color = nd->getGlowColor().color;
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

template<typename T>
static void
FillInstances_impl(std::vector<std::pair<GLMesh *, scene::ISceneNode *> > InstanceList, T * InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer,
    size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &PolyCount, std::function<bool (const scene::ISceneNode *)> cull_func)
{
    // Should never be empty
    GLMesh *mesh = InstanceList.front().first;
    size_t InitialOffset = InstanceBufferOffset;

    for (unsigned i = 0; i < InstanceList.size(); i++)
    {
        auto &Tp = InstanceList[i];
        scene::ISceneNode *node = Tp.second;
        if (cull_func(node))
            continue;
        InstanceFiller<T>::add(mesh, node, InstanceBuffer[InstanceBufferOffset++]);
    }

    DrawElementsIndirectCommand &CurrentCommand = CommandBuffer[CommandBufferOffset++];
    CurrentCommand.baseVertex = mesh->vaoBaseVertex;
    CurrentCommand.count = mesh->IndexCount;
    CurrentCommand.firstIndex = mesh->vaoOffset / 2;
    CurrentCommand.baseInstance = InitialOffset;
    CurrentCommand.instanceCount = InstanceBufferOffset - InitialOffset;

    PolyCount += (InstanceBufferOffset - InitialOffset) * mesh->IndexCount / 3;
}

template<typename T>
static
void FillInstances(const std::unordered_map<scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > &GatheredGLMesh, std::vector<GLMesh *> &InstancedList,
    T *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &Polycount,
    std::function<bool (const scene::ISceneNode *)> cull_func)
{
    auto It = GatheredGLMesh.begin(), E = GatheredGLMesh.end();
    for (; It != E; ++It)
    {
        FillInstances_impl<T>(It->second, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, Polycount, cull_func);
        if (!UserConfigParams::m_azdo)
            InstancedList.push_back(It->second.front().first);
    }
}

static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > MeshForSolidPass[MAT_COUNT];
static std::unordered_map <scene::IMeshBuffer *, std::vector<std::pair<GLMesh *, scene::ISceneNode*> > > MeshForGlowPass;
static std::vector <STKMeshCommon *> DeferredUpdate;

static core::vector3df windDir;

// From irrlicht code
static
bool isBoxInFrontOfPlane(const core::plane3df &plane, const core::vector3df edges[8])
{
    for (u32 j = 0; j<8; ++j)
        if (plane.classifyPointRelation(edges[j]) != core::ISREL3D_FRONT)
            return false;
    return true;
}

static
bool isCulledPrecise(const scene::ICameraSceneNode *cam, const scene::ISceneNode *node)
{
    if (!node->getAutomaticCulling())
        return false;

    scene::SViewFrustum frust = *cam->getViewFrustum();

    //transform the frustum to the node's current absolute transformation
    core::matrix4 invTrans(node->getAbsoluteTransformation(), core::matrix4::EM4CONST_INVERSE);
    //invTrans.makeInverse();
    frust.transform(invTrans);

    core::vector3df edges[8];
    node->getBoundingBox().getEdges(edges);

    for (s32 i = 0; i < scene::SViewFrustum::VF_PLANE_COUNT; ++i)
        if (isBoxInFrontOfPlane(frust.planes[i], edges))
            return true;
    return false;
}

static
bool isCulledFast(const scene::ICameraSceneNode *cam, const scene::ISceneNode *node)
{
    if (!node->getAutomaticCulling())
        return false;

    core::aabbox3d<f32> tbox = node->getBoundingBox();
    node->getAbsoluteTransformation().transformBoxEx(tbox);
    return !(tbox.intersectsWithBox(cam->getViewFrustum()->getBoundingBox()));
}

static void
handleSTKCommon(scene::ISceneNode *Node, std::vector<scene::ISceneNode *> *ImmediateDraw,
    const scene::ICameraSceneNode *cam, scene::ICameraSceneNode *shadowcam[4], const scene::ICameraSceneNode *rsmcam)
{
    STKMeshCommon *node = dynamic_cast<STKMeshCommon*>(Node);
    if (!node)
        return;
    node->updateNoGL();
    DeferredUpdate.push_back(node);

    if (node->isImmediateDraw())
    {
        ImmediateDraw->push_back(Node);
        return;
    }

    // Transparent
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

    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
    {
        if (irr_driver->hasARB_draw_indirect())
        {
            for_in(mesh, node->MeshSolidMaterial[Mat])
            {
                if (node->glow())
                    MeshForGlowPass[mesh->mb].emplace_back(mesh, Node);

                if (Mat != MAT_SPLATTING && mesh->TextureMatrix.isIdentity())
                    MeshForSolidPass[Mat][mesh->mb].emplace_back(mesh, Node);
                else
                {
                    core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                    ModelMatrix.getInverse(InvModelMatrix);
                    switch (Mat)
                    {
                    case MAT_DEFAULT:
                        ListMatDefault::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_ALPHA_REF:
                        ListMatAlphaRef::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_UNLIT:
                        ListMatUnlit::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_SPLATTING:
                        ListMatSplatting::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                        break;
                    }
                }
            }
        }
        else
        {
            if (isCulledFast(cam, Node))
                continue;
            core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
            ModelMatrix.getInverse(InvModelMatrix);

            for_in(mesh, node->MeshSolidMaterial[Mat])
            {
                switch (Mat)
                {
                case MAT_DEFAULT:
                    ListMatDefault::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_ALPHA_REF:
                    ListMatAlphaRef::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_NORMAL_MAP:
                    ListMatNormalMap::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_DETAIL:
                    ListMatDetails::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_UNLIT:
                    ListMatUnlit::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_SPHEREMAP:
                    ListMatSphereMap::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_SPLATTING:
                    ListMatSplatting::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                    break;
                case MAT_GRASS:
                    ListMatGrass::getInstance()->SolidPass.emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                    break;
                }
            }
        }
    }
    if (!UserConfigParams::m_shadows)
        return;
    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
    {
        for (unsigned cascade = 0; cascade < 4; ++cascade)
        {
            if (!irr_driver->hasARB_draw_indirect())
            {
                if (isCulledFast(shadowcam[cascade], Node))
                    continue;
                core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                ModelMatrix.getInverse(InvModelMatrix);

                for_in(mesh, node->MeshSolidMaterial[Mat])
                {
                    switch (Mat)
                    {
                    case MAT_DEFAULT:
                        ListMatDefault::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_ALPHA_REF:
                        ListMatAlphaRef::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_NORMAL_MAP:
                        ListMatNormalMap::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_DETAIL:
                        ListMatDetails::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_UNLIT:
                        ListMatUnlit::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_SPHEREMAP:
                        ListMatSphereMap::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                        break;
                    case MAT_SPLATTING:
                        ListMatSplatting::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix);
                        break;
                    case MAT_GRASS:
                        ListMatGrass::getInstance()->Shadows[cascade].emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                    }
                }
            }
        }
    }
    if (!UserConfigParams::m_gi)
        return;
    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
    {
        if (irr_driver->hasARB_draw_indirect())
        {
            if (Mat == MAT_SPLATTING)
                for_in(mesh, node->MeshSolidMaterial[Mat])
                {
                    core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
                    ModelMatrix.getInverse(InvModelMatrix);
                    ListMatSplatting::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                }
        }
        else
        {
            if (isCulledFast(rsmcam, Node))
                continue;
            core::matrix4 ModelMatrix = Node->getAbsoluteTransformation(), InvModelMatrix;
            ModelMatrix.getInverse(InvModelMatrix);

            for_in(mesh, node->MeshSolidMaterial[Mat])
            {
                switch (Mat)
                {
                case MAT_DEFAULT:
                    ListMatDefault::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_ALPHA_REF:
                    ListMatAlphaRef::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_NORMAL_MAP:
                    ListMatNormalMap::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_DETAIL:
                    ListMatDetails::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_UNLIT:
                    ListMatUnlit::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_SPHEREMAP:
                    ListMatSphereMap::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, mesh->TextureMatrix);
                    break;
                case MAT_SPLATTING:
                    ListMatSplatting::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix);
                    break;
                case MAT_GRASS:
                    ListMatGrass::getInstance()->RSM.emplace_back(mesh, ModelMatrix, InvModelMatrix, windDir);
                    break;
                }
            }
        }
    }
}

static void
parseSceneManager(core::list<scene::ISceneNode*> List, std::vector<scene::ISceneNode *> *ImmediateDraw,
    const scene::ICameraSceneNode* cam, scene::ICameraSceneNode *shadow_cam[4], const scene::ICameraSceneNode *rsmcam)
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
            if (!isCulledPrecise(cam, *I) && node->update())
                ParticlesList::getInstance()->push_back(node);
            continue;
        }

        if (STKBillboard *node = dynamic_cast<STKBillboard *>(*I))
        {
            if (!isCulledPrecise(cam, *I))
                BillBoardList::getInstance()->push_back(node);
            continue;
        }

        handleSTKCommon(*I, ImmediateDraw, cam, shadow_cam, rsmcam);

        parseSceneManager((*I)->getChildren(), ImmediateDraw, cam, shadow_cam, rsmcam);
    }
}

template<MeshMaterial Mat, typename T> static void
GenDrawCalls(unsigned cascade, std::vector<GLMesh *> &InstancedList,
    T *InstanceBuffer, DrawElementsIndirectCommand *CommandBuffer, size_t &InstanceBufferOffset, size_t &CommandBufferOffset, size_t &PolyCount)
{
    std::function<bool(const scene::ISceneNode *)> shadowculling = [&](const scene::ISceneNode *nd) {return dynamic_cast<const STKMeshCommon*>(nd)->isCulledForShadowCam(cascade); };
    if (irr_driver->hasARB_draw_indirect())
        ShadowPassCmd::getInstance()->Offset[cascade][Mat] = CommandBufferOffset; // Store command buffer offset
    FillInstances<T>(MeshForSolidPass[Mat], InstancedList, InstanceBuffer, CommandBuffer, InstanceBufferOffset, CommandBufferOffset, PolyCount, shadowculling);
    if (UserConfigParams::m_azdo)
        ShadowPassCmd::getInstance()->Size[cascade][Mat] = CommandBufferOffset - ShadowPassCmd::getInstance()->Offset[cascade][Mat];
}

int enableOpenMP;

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
    BillBoardList::getInstance()->clear();
    ParticlesList::getInstance()->clear();
    ListInstancedGlow::getInstance()->clear();

    for (unsigned Mat = 0; Mat < MAT_COUNT; ++Mat)
        MeshForSolidPass[Mat].clear();
    MeshForGlowPass.clear();
    DeferredUpdate.clear();
    core::list<scene::ISceneNode*> List = m_scene_manager->getRootSceneNode()->getChildren();

    parseSceneManager(List, ImmediateDrawList::getInstance(), camnode, m_shadow_camnodes, m_suncam);

#pragma omp parallel for
    for (int i = 0; i < (int)DeferredUpdate.size(); i++)
    {
        scene::ISceneNode *node = dynamic_cast<scene::ISceneNode *>(DeferredUpdate[i]);
        DeferredUpdate[i]->setCulledForPlayerCam(isCulledPrecise(camnode, node));
        DeferredUpdate[i]->setCulledForRSMCam(isCulledPrecise(m_suncam, node));
        DeferredUpdate[i]->setCulledForShadowCam(0, isCulledPrecise(m_shadow_camnodes[0], node));
        DeferredUpdate[i]->setCulledForShadowCam(1, isCulledPrecise(m_shadow_camnodes[1], node));
        DeferredUpdate[i]->setCulledForShadowCam(2, isCulledPrecise(m_shadow_camnodes[2], node));
        DeferredUpdate[i]->setCulledForShadowCam(3, isCulledPrecise(m_shadow_camnodes[3], node));
    }

    // Add a 1 s timeout
    if (!m_sync)
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    PROFILER_PUSH_CPU_MARKER("- Sync Stall", 0xFF, 0x0, 0x0);
    GLenum reason;
    do { reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0); if (reason == GL_WAIT_FAILED) break; } while (reason != GL_ALREADY_SIGNALED);
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
    for (unsigned i = 0; i < DeferredUpdate.size(); i++)
        DeferredUpdate[i]->updateGL();
    PROFILER_POP_CPU_MARKER();

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
        enableOpenMP = 1;
    }
    else
        enableOpenMP = 0;

    ListInstancedMatDefault::getInstance()->clear();
    ListInstancedMatAlphaRef::getInstance()->clear();
    ListInstancedMatGrass::getInstance()->clear();
    ListInstancedMatNormalMap::getInstance()->clear();
    ListInstancedMatSphereMap::getInstance()->clear();
    ListInstancedMatDetails::getInstance()->clear();
    ListInstancedMatUnlit::getInstance()->clear();

    size_t SolidPoly = 0, ShadowPoly = 0, MiscPoly = 0;

    PROFILER_PUSH_CPU_MARKER("- Draw Command upload", 0xFF, 0x0, 0xFF);

    auto playercamculling = [](const scene::ISceneNode *nd) {return dynamic_cast<const STKMeshCommon*>(nd)->isCulledForPlayerCam(); };
#pragma omp parallel sections if(enableOpenMP)
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
            FillInstances(MeshForSolidPass[MAT_DEFAULT], ListInstancedMatDefault::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_DEFAULT] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_DEFAULT];
            // Alpha Ref
            SolidPassCmd::getInstance()->Offset[MAT_ALPHA_REF] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_ALPHA_REF], ListInstancedMatAlphaRef::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_ALPHA_REF] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_ALPHA_REF];
            // Unlit
            SolidPassCmd::getInstance()->Offset[MAT_UNLIT] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_UNLIT], ListInstancedMatUnlit::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_UNLIT] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_UNLIT];
            // Spheremap
            SolidPassCmd::getInstance()->Offset[MAT_SPHEREMAP] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_SPHEREMAP], ListInstancedMatSphereMap::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_SPHEREMAP] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_SPHEREMAP];
            // Detail
            SolidPassCmd::getInstance()->Offset[MAT_DETAIL] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_DETAIL], ListInstancedMatDetails::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_DETAIL] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_DETAIL];
            // Normal Map
            SolidPassCmd::getInstance()->Offset[MAT_NORMAL_MAP] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_NORMAL_MAP], ListInstancedMatNormalMap::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
            SolidPassCmd::getInstance()->Size[MAT_NORMAL_MAP] = current_cmd - SolidPassCmd::getInstance()->Offset[MAT_NORMAL_MAP];

            // Grass
            SolidPassCmd::getInstance()->Offset[MAT_GRASS] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_GRASS], ListInstancedMatGrass::getInstance()->SolidPass, InstanceBuffer, CmdBuffer, offset, current_cmd, SolidPoly, playercamculling);
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
                size_t Polycnt = 0;
                FillInstances_impl<GlowInstanceData>(It->second, GlowInstanceBuffer, GlowCmdBuffer, offset, current_cmd, Polycnt, playercamculling);
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
                GenDrawCalls<MAT_GRASS>(i, ListInstancedMatGrass::getInstance()->Shadows[i], ShadowInstanceBuffer, ShadowCmdBuffer, offset, current_cmd, ShadowPoly);
            }
            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
#pragma omp section
        {
            auto rsmcamculling = [](const scene::ISceneNode *nd) {return dynamic_cast<const STKMeshCommon*>(nd)->isCulledForRSMCam(); };
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
            FillInstances(MeshForSolidPass[MAT_DEFAULT], ListInstancedMatDefault::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly, rsmcamculling);
            RSMPassCmd::getInstance()->Size[MAT_DEFAULT] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_DEFAULT];
            // Alpha Ref
            RSMPassCmd::getInstance()->Offset[MAT_ALPHA_REF] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_ALPHA_REF], ListInstancedMatAlphaRef::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly, rsmcamculling);
            RSMPassCmd::getInstance()->Size[MAT_ALPHA_REF] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_ALPHA_REF];
            // Unlit
            RSMPassCmd::getInstance()->Offset[MAT_UNLIT] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_UNLIT], ListInstancedMatUnlit::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly, rsmcamculling);
            RSMPassCmd::getInstance()->Size[MAT_UNLIT] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_UNLIT];
            // Detail
            RSMPassCmd::getInstance()->Offset[MAT_DETAIL] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_DETAIL], ListInstancedMatDetails::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly, rsmcamculling);
            RSMPassCmd::getInstance()->Size[MAT_DETAIL] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_DETAIL];
            // Normal Map
            RSMPassCmd::getInstance()->Offset[MAT_NORMAL_MAP] = current_cmd;
            FillInstances(MeshForSolidPass[MAT_NORMAL_MAP], ListInstancedMatNormalMap::getInstance()->RSM, RSMInstanceBuffer, RSMCmdBuffer, offset, current_cmd, MiscPoly, rsmcamculling);
            RSMPassCmd::getInstance()->Size[MAT_NORMAL_MAP] = current_cmd - RSMPassCmd::getInstance()->Offset[MAT_NORMAL_MAP];

            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
            }
        }
    }
    PROFILER_POP_CPU_MARKER();
    poly_count[SOLID_NORMAL_AND_DEPTH_PASS] += SolidPoly;
    poly_count[SHADOW_PASS] += ShadowPoly;

    if (irr_driver->hasBufferStorageExtension())
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
}