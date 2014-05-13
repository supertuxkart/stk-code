#include "graphics/stkanimatedmesh.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include <ISkinnedMesh.h>
#include "graphics/irr_driver.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    firstTime = true;
}

void STKAnimatedMesh::cleanGLMeshes()
{
    for (u32 i = 0; i < GLmeshes.size(); ++i)
    {
        GLMesh mesh = GLmeshes[i];
        if (!mesh.vertex_buffer)
            continue;
        if (mesh.vao_first_pass)
            glDeleteVertexArrays(1, &(mesh.vao_first_pass));
        if (mesh.vao_second_pass)
            glDeleteVertexArrays(1, &(mesh.vao_second_pass));
        if (mesh.vao_glow_pass)
            glDeleteVertexArrays(1, &(mesh.vao_glow_pass));
        if (mesh.vao_displace_pass)
            glDeleteVertexArrays(1, &(mesh.vao_displace_pass));
        if (mesh.vao_displace_mask_pass)
            glDeleteVertexArrays(1, &(mesh.vao_displace_mask_pass));
        if (mesh.vao_shadow_pass)
            glDeleteVertexArrays(1, &(mesh.vao_shadow_pass));
        glDeleteBuffers(1, &(mesh.vertex_buffer));
        glDeleteBuffers(1, &(mesh.index_buffer));
    }
}

void STKAnimatedMesh::setMesh(scene::IAnimatedMesh* mesh)
{
    firstTime = true;
    GLmeshes.clear();
    for (unsigned i = 0; i < FPSM_COUNT; i++)
        GeometricMesh[i].clearWithoutDeleting();
    for (unsigned i = 0; i < SM_COUNT; i++)
        ShadedMesh[i].clearWithoutDeleting();
    CAnimatedMeshSceneNode::setMesh(mesh);
}

void STKAnimatedMesh::render()
{
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    bool isTransparentPass =
        SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

    ++PassCount;

    scene::IMesh* m = getMeshForCurrentFrame();

    if (m)
    {
        Box = m->getBoundingBox();
    }
    else
    {
        Log::error("animated mesh", "Animated Mesh returned no mesh to render.");
        return;
    }

    if (firstTime)
    {
        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            GLmeshes.push_back(allocateMeshBuffer(mb));
        }

        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            f32 MaterialTypeParam = mb->getMaterial().MaterialTypeParam;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            if (!isObject(type))
            {
#ifdef DEBUG
                Log::warn("material", "Unhandled (animated) material type : %d", type);
#endif
                continue;
            }
            GLMesh &mesh = GLmeshes[i];
            if (rnd->isTransparent())
            {
                TransparentMaterial TranspMat = MaterialTypeToTransparentMaterial(type, MaterialTypeParam);
                initvaostate(mesh, TranspMat);
                TransparentMesh[TranspMat].push_back(&mesh);
            }
            else
            {
                GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type);
                ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures);
                initvaostate(mesh, GeometricType, ShadedType);
                GeometricMesh[GeometricType].push_back(&mesh);
                ShadedMesh[ShadedType].push_back(&mesh);
            }
        }
    }
    firstTime = false;

    for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = m->getMeshBuffer(i);
        const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
        if (isObject(material.MaterialType))
        {
           if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
           {
               glBindVertexArray(0);
               glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
               glBufferSubData(GL_ARRAY_BUFFER, 0, mb->getVertexCount() * GLmeshes[i].Stride, mb->getVertices());
           }
        }
        if (mb)
            GLmeshes[i].TextureMatrix = getMaterial(i).getTextureMatrix(0);

        video::IMaterialRenderer* rnd = driver->getMaterialRenderer(Materials[i].MaterialType);
        bool transparent = (rnd && rnd->isTransparent());

       // only render transparent buffer if this is the transparent render pass
       // and solid only in solid pass
       if (transparent != isTransparentPass)
          continue;
    }

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);
        TransposeInverseModelView = computeTIMV(AbsoluteTransformation);
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        PROFILER_PUSH_CPU_MARKER("GATHER SOLID MESHES", 0xFC, 0xFA, 0x68);

        GLMesh* mesh;
        for_in(mesh, GeometricMesh[FPSM_DEFAULT])
        {
            GroupedFPSM<FPSM_DEFAULT>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_DEFAULT>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_DEFAULT>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, GeometricMesh[FPSM_ALPHA_REF_TEXTURE])
        {
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::TIMVSet.push_back(invmodel);
        }

        PROFILER_POP_CPU_MARKER();
        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        PROFILER_PUSH_CPU_MARKER("GATHER TRANSPARENT MESHES", 0xFC, 0xFA, 0x68);

        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        GLMesh* mesh;
        for_in(mesh, ShadedMesh[SM_DEFAULT])
        {
            GroupedSM<SM_DEFAULT>::MeshSet.push_back(mesh);
            GroupedSM<SM_DEFAULT>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_DEFAULT>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_ALPHA_REF_TEXTURE])
        {
            GroupedSM<SM_ALPHA_REF_TEXTURE>::MeshSet.push_back(mesh);
            GroupedSM<SM_ALPHA_REF_TEXTURE>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_ALPHA_REF_TEXTURE>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_RIMLIT])
        {
            GroupedSM<SM_RIMLIT>::MeshSet.push_back(mesh);
            GroupedSM<SM_RIMLIT>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_RIMLIT>::TIMVSet.push_back(invmodel);
        }

        for_in (mesh, ShadedMesh[SM_UNLIT])
        {
            GroupedSM<SM_UNLIT>::MeshSet.push_back(mesh);
            GroupedSM<SM_UNLIT>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_UNLIT>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_DETAILS])
        {
            GroupedSM<SM_DETAILS>::MeshSet.push_back(mesh);
            GroupedSM<SM_DETAILS>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_DETAILS>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_UNTEXTURED])
        {
            GroupedSM<SM_UNTEXTURED>::MeshSet.push_back(mesh);
            GroupedSM<SM_UNTEXTURED>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_UNTEXTURED>::TIMVSet.push_back(invmodel);
        }

        PROFILER_POP_CPU_MARKER();
        return;
    }

    if (irr_driver->getPhase() == SHADOW_PASS)
    {
        if (!GeometricMesh[FPSM_DEFAULT].empty())
            glUseProgram(MeshShader::ShadowShader::Program);

        GLMesh* mesh;
        for_in(mesh, GeometricMesh[FPSM_DEFAULT])
            drawShadow(*mesh, AbsoluteTransformation);

        if (!GeometricMesh[FPSM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::RefShadowShader::Program);

        for_in(mesh, GeometricMesh[FPSM_ALPHA_REF_TEXTURE])
            drawShadowRef(*mesh, AbsoluteTransformation);
        return;
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);

        if (!TransparentMesh[TM_BUBBLE].empty())
            glUseProgram(MeshShader::BubbleShader::Program);

        GLMesh* mesh;
        for_in(mesh, TransparentMesh[TM_BUBBLE])
            drawBubble(*mesh, ModelViewProjectionMatrix);

        if (World::getWorld()->isFogEnabled())
        {
            if (!TransparentMesh[TM_DEFAULT].empty() || !TransparentMesh[TM_ADDITIVE].empty())
                glUseProgram(MeshShader::TransparentFogShader::Program);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            for_in(mesh, TransparentMesh[TM_DEFAULT])
                drawTransparentFogObject(*mesh, ModelViewProjectionMatrix, mesh->TextureMatrix);
            glBlendFunc(GL_ONE, GL_ONE);
            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                drawTransparentFogObject(*mesh, ModelViewProjectionMatrix, mesh->TextureMatrix);
        }
        else
        {
            if (!TransparentMesh[TM_DEFAULT].empty() || !TransparentMesh[TM_ADDITIVE].empty())
                glUseProgram(MeshShader::TransparentShader::Program);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            for_in(mesh, TransparentMesh[TM_DEFAULT])
                drawTransparentObject(*mesh, ModelViewProjectionMatrix, mesh->TextureMatrix);
            glBlendFunc(GL_ONE, GL_ONE);
            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                drawTransparentObject(*mesh, ModelViewProjectionMatrix, mesh->TextureMatrix);
        }
        return;
    }
}
