#include "graphics/stkanimatedmesh.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include <ISkinnedMesh.h>
#include "graphics/irr_driver.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "graphics/camera.hpp"
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
        if (mesh.vao)
            glDeleteVertexArrays(1, &(mesh.vao));
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
                TransparentMaterial TranspMat = MaterialTypeToTransparentMaterial(type, MaterialTypeParam, false);
                TransparentMesh[TranspMat].push_back(&mesh);
            }
            else
            {
                GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type, mb->getVertexType());
                ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures, mb->getVertexType());
                GeometricMesh[GeometricType].push_back(&mesh);
                ShadedMesh[ShadedType].push_back(&mesh);
            }
            std::pair<unsigned, unsigned> p = getVAOOffsetAndBase(mb);
            mesh.vaoBaseVertex = p.first;
            mesh.vaoOffset = p.second;
            mesh.VAOType = mb->getVertexType();
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
               glBindBuffer(GL_ARRAY_BUFFER, getVBO(mb->getVertexType()));
               glBufferSubData(GL_ARRAY_BUFFER, GLmeshes[i].vaoBaseVertex * GLmeshes[i].Stride, mb->getVertexCount() * GLmeshes[i].Stride, mb->getVertices());
               glBindBuffer(GL_ARRAY_BUFFER, 0);
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
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        GLMesh* mesh;
        for_in(mesh, GeometricMesh[FPSM_DEFAULT_STANDARD])
            ListDefaultStandardG::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, invmodel));

        for_in(mesh, GeometricMesh[FPSM_DEFAULT_2TCOORD])
            ListDefault2TCoordG::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, invmodel));

        for_in(mesh, GeometricMesh[FPSM_ALPHA_REF_TEXTURE])
            ListAlphaRefG::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, invmodel, mesh->TextureMatrix));

        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        GLMesh* mesh;
        for_in(mesh, ShadedMesh[SM_DEFAULT_STANDARD])
            ListDefaultStandardSM::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix, irr_driver->getSceneManager()->getAmbientLight()));

        for_in(mesh, ShadedMesh[SM_DEFAULT_TANGENT])
            ListDefaultTangentSM::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix, irr_driver->getSceneManager()->getAmbientLight()));

        for_in(mesh, ShadedMesh[SM_ALPHA_REF_TEXTURE])
            ListAlphaRefSM::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix, irr_driver->getSceneManager()->getAmbientLight()));

        for_in (mesh, ShadedMesh[SM_UNLIT])
            ListUnlitSM::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation));

        for_in(mesh, ShadedMesh[SM_DETAILS])
            ListDetailSM::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, irr_driver->getSceneManager()->getAmbientLight()));

        return;
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);

        if (!TransparentMesh[TM_BUBBLE].empty())
            glUseProgram(MeshShader::BubbleShader::Program);

        GLMesh* mesh;
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

            for_in(mesh, TransparentMesh[TM_DEFAULT])
                ListBlendTransparentFog::Arguments.push_back(
                    std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                    fogmax, startH, endH, start, end, col));
            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                ListAdditiveTransparentFog::Arguments.push_back(
                    std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                    fogmax, startH, endH, start, end, col));
        }
        else
        {
            for_in(mesh, TransparentMesh[TM_DEFAULT])
                ListBlendTransparent::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix));

            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                ListAdditiveTransparent::Arguments.push_back(std::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix));
        }
        return;
    }
}
