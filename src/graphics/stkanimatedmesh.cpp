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
#include "utils/cpp2011.hpp"

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    isGLInitialized = false;
    isMaterialInitialized = false;
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
        if (mesh.instance_buffer)
            glDeleteBuffers(1, &(mesh.instance_buffer));
#ifdef Bindless_Texture_Support
        for (unsigned j = 0; j < 6; j++)
        {
            if (mesh.TextureHandles[j] && glIsTextureHandleResidentARB(mesh.TextureHandles[j]))
                glMakeTextureHandleNonResidentARB(mesh.TextureHandles[j]);
        }
#endif
    }
}

void STKAnimatedMesh::setMesh(scene::IAnimatedMesh* mesh)
{
    isGLInitialized = false;
    isMaterialInitialized = false;
    GLmeshes.clear();
    for (unsigned i = 0; i < MAT_COUNT; i++)
        MeshSolidMaterial[i].clearWithoutDeleting();
    CAnimatedMeshSceneNode::setMesh(mesh);
}

void STKAnimatedMesh::updateNoGL()
{
    scene::IMesh* m = getMeshForCurrentFrame();

    if (m)
        Box = m->getBoundingBox();
    else
    {
        Log::error("animated mesh", "Animated Mesh returned no mesh to render.");
        return;
    }

    if (!isMaterialInitialized)
    {
        video::IVideoDriver* driver = SceneManager->getVideoDriver();
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
                TransparentMesh[TranspMat].push_back(&mesh);
            }
            else
            {
                MeshMaterial MatType = MaterialTypeToMeshMaterial(type, mb->getVertexType());
                MeshSolidMaterial[MatType].push_back(&mesh);
            }
        }
        isMaterialInitialized = true;
    }
}

void STKAnimatedMesh::updateGL()
{

    scene::IMesh* m = getMeshForCurrentFrame();

    if (!isGLInitialized)
    {
        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            video::IVideoDriver* driver = SceneManager->getVideoDriver();
            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            GLMesh &mesh = GLmeshes[i];

            if (!rnd->isTransparent())
            {
                MeshMaterial MatType = MaterialTypeToMeshMaterial(type, mb->getVertexType());
                InitTextures(mesh, MatType);
            }

            if (irr_driver->hasARB_base_instance())
            {
                std::pair<unsigned, unsigned> p = VAOManager::getInstance()->getBase(mb);
                mesh.vaoBaseVertex = p.first;
                mesh.vaoOffset = p.second;
            }
            else
            {
                fillLocalBuffer(mesh, mb);
                mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
                glBindVertexArray(0);
            }
        }
        isGLInitialized = true;
    }

    for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = m->getMeshBuffer(i);
        const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
        if (isObject(material.MaterialType))
        {

            size_t size = mb->getVertexCount() * GLmeshes[i].Stride, offset = GLmeshes[i].vaoBaseVertex * GLmeshes[i].Stride;
            void *buf;
            if (irr_driver->hasBufferStorageExtension())
            {
                buf = VAOManager::getInstance()->getVBOPtr(mb->getVertexType());
                buf = (char *)buf + offset;
            }
            else
            {
                glBindVertexArray(0);
                if (irr_driver->hasARB_base_instance())
                    glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getVBO(mb->getVertexType()));
                else
                    glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
                GLbitfield bitfield = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
                buf = glMapBufferRange(GL_ARRAY_BUFFER, offset, size, bitfield);
            }
            memcpy(buf, mb->getVertices(), size);
            if (!irr_driver->hasBufferStorageExtension())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
        if (mb)
            GLmeshes[i].TextureMatrix = getMaterial(i).getTextureMatrix(0);
    }

}

void STKAnimatedMesh::render()
{
    bool isTransparentPass =
        SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

    ++PassCount;

    updateNoGL();
    updateGL();

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
                ListBlendTransparentFog::getInstance()->push_back(
                    STK::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                    fogmax, startH, endH, start, end, col));
            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                ListAdditiveTransparentFog::getInstance()->push_back(
                STK::make_tuple(mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                    fogmax, startH, endH, start, end, col));
        }
        else
        {
            for_in(mesh, TransparentMesh[TM_DEFAULT])
                pushVector(ListBlendTransparent::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix);

            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                pushVector(ListAdditiveTransparent::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix);
        }
        return;
    }
}
