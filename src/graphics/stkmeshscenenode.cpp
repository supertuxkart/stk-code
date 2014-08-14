#include "stkmeshscenenode.hpp"
#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include "tracks/track.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "modes/world.hpp"
#include "utils/helpers.hpp"
#include "utils/tuple.hpp"
#include "utils/cpp2011.hpp"

STKMeshSceneNode::STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale, bool createGLMeshes) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    isDisplacement = false;
    immediate_draw = false;
    update_each_frame = false;

    if (createGLMeshes)
        this->createGLMeshes();
}

void STKMeshSceneNode::setReloadEachFrame(bool val)
{
    update_each_frame = val;
    if (val)
        immediate_draw = true;
}

void STKMeshSceneNode::createGLMeshes()
{
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        GLmeshes.push_back(allocateMeshBuffer(mb));
    }
    isMaterialInitialized = false;
}

void STKMeshSceneNode::setFirstTimeMaterial()
{
  if (isMaterialInitialized)
      return;
  irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();
  for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
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
          Log::warn("material", "Unhandled (static) material type : %d", type);
#endif
          continue;
      }


      GLMesh &mesh = GLmeshes[i];
      if (rnd->isTransparent())
      {
          TransparentMaterial TranspMat = MaterialTypeToTransparentMaterial(type, MaterialTypeParam);
          if (immediate_draw)
          {
              fillLocalBuffer(mesh, mb);
              mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
              glBindVertexArray(0);
          }
          else
              TransparentMesh[TranspMat].push_back(&mesh);
      }
      else
      {
          assert(!isDisplacement);
          MeshMaterial MatType = MaterialTypeToMeshMaterial(type, mb->getVertexType());
          if (immediate_draw)
          {
              fillLocalBuffer(mesh, mb);
              mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
              glBindVertexArray(0);
          }
          else
              MeshSolidMaterials[MatType].push_back(&mesh);
      }

      if (!immediate_draw)
      {
          std::pair<unsigned, unsigned> p = getVAOOffsetAndBase(mb);
          mesh.vaoBaseVertex = p.first;
          mesh.vaoOffset = p.second;
          mesh.VAOType = mb->getVertexType();
      }
  }
  isMaterialInitialized = true;
}

void STKMeshSceneNode::cleanGLMeshes()
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
    GLmeshes.clear();
    for (unsigned i = 0; i < MAT_COUNT; i++)
        MeshSolidMaterials[i].clearWithoutDeleting();
}

void STKMeshSceneNode::setMesh(irr::scene::IMesh* mesh)
{
    CMeshSceneNode::setMesh(mesh);
    cleanGLMeshes();
    createGLMeshes();
}

STKMeshSceneNode::~STKMeshSceneNode()
{
    cleanGLMeshes();
}

void STKMeshSceneNode::drawGlow(const GLMesh &mesh)
{
    ColorizeProvider * const cb = (ColorizeProvider *)irr_driver->getCallback(ES_COLORIZE);
    assert(mesh.VAOType == video::EVT_STANDARD);

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    MeshShader::ColorizeShader::getInstance()->setUniforms(AbsoluteTransformation, video::SColorf(cb->getRed(), cb->getGreen(), cb->getBlue()));
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
}

void STKMeshSceneNode::updatevbo()
{
    for (unsigned i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        GLMesh &mesh = GLmeshes[i];
        glDeleteBuffers(1, &(mesh.vertex_buffer));
        glDeleteBuffers(1, &(mesh.index_buffer));
        glDeleteVertexArrays(1, &(mesh.vao));

        fillLocalBuffer(mesh, mb);
        mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
    }
}

static video::ITexture *spareWhiteTex = 0;


void STKMeshSceneNode::OnRegisterSceneNode()
{
    if (isDisplacement)
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
    else
        CMeshSceneNode::OnRegisterSceneNode();
}

void STKMeshSceneNode::render()
{
    irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();

    if (!Mesh || !driver)
        return;

    ++PassCount;

    Box = Mesh->getBoundingBox();

    setFirstTimeMaterial();

    for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        GLmeshes[i].TextureMatrix = getMaterial(i).getTextureMatrix(0);
    }

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS && immediate_draw)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);


        glDisable(GL_CULL_FACE);
        if (update_each_frame)
            updatevbo();
        glUseProgram(MeshShader::ObjectPass1Shader::getInstance()->Program);
        // Only untextured
        for (unsigned i = 0; i < GLmeshes.size(); i++)
        {
            irr_driver->IncreaseObjectCount();
            GLMesh &mesh = GLmeshes[i];
            GLenum ptype = mesh.PrimitiveType;
            GLenum itype = mesh.IndexType;
            size_t count = mesh.IndexCount;

            MeshShader::ObjectPass1Shader::getInstance()->setUniforms(AbsoluteTransformation, invmodel);
            assert(mesh.vao);
            glBindVertexArray(mesh.vao);
            glDrawElements(ptype, count, itype, 0);
            glBindVertexArray(0);
        }
        glEnable(GL_CULL_FACE);
        return;
    }

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS || irr_driver->getPhase() == SHADOW_PASS)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        GLMesh* mesh;
        for_in(mesh, MeshSolidMaterials[MAT_DEFAULT])
            pushVector(ListMatDefault::getInstance(), mesh, AbsoluteTransformation, invmodel, mesh->TextureMatrix);

        for_in(mesh, MeshSolidMaterials[MAT_ALPHA_REF])
            pushVector(ListMatAlphaRef::getInstance(), mesh, AbsoluteTransformation, invmodel, mesh->TextureMatrix);

        for_in(mesh, MeshSolidMaterials[MAT_SPHEREMAP])
            pushVector(ListMatSphereMap::getInstance(), mesh, AbsoluteTransformation, invmodel, mesh->TextureMatrix);

        for_in(mesh, MeshSolidMaterials[MAT_DETAIL])
            pushVector(ListMatDetails::getInstance(), mesh, AbsoluteTransformation, invmodel, mesh->TextureMatrix);

        windDir = getWind();
        for_in(mesh, MeshSolidMaterials[MAT_GRASS])
            pushVector(ListMatGrass::getInstance(), mesh, AbsoluteTransformation, invmodel, windDir);

        for_in(mesh, MeshSolidMaterials[MAT_UNLIT])
            pushVector(ListMatUnlit::getInstance(), mesh, AbsoluteTransformation, core::matrix4::EM4CONST_IDENTITY, mesh->TextureMatrix);

        for_in(mesh, MeshSolidMaterials[MAT_SPLATTING])
            pushVector(ListMatSplatting::getInstance(), mesh, AbsoluteTransformation, invmodel);

        for_in(mesh, MeshSolidMaterials[MAT_NORMAL_MAP])
            pushVector(ListMatNormalMap::getInstance(), mesh, AbsoluteTransformation, invmodel, core::matrix4::EM4CONST_IDENTITY);

        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        if (immediate_draw)
        {
            glDisable(GL_CULL_FACE);
            if (!spareWhiteTex)
                spareWhiteTex = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            glUseProgram(MeshShader::ObjectPass2Shader::getInstance()->Program);
            // Only untextured
            for (unsigned i = 0; i < GLmeshes.size(); i++)
            {
                irr_driver->IncreaseObjectCount();
                GLMesh &mesh = GLmeshes[i];
                GLenum ptype = mesh.PrimitiveType;
                GLenum itype = mesh.IndexType;
                size_t count = mesh.IndexCount;

                setTexture(MeshShader::ObjectPass2Shader::getInstance()->TU_Albedo, getTextureGLuint(spareWhiteTex), GL_NEAREST, GL_NEAREST, false);
                MeshShader::ObjectPass2Shader::getInstance()->setUniforms(AbsoluteTransformation, mesh.TextureMatrix);
                assert(mesh.vao);
                glBindVertexArray(mesh.vao);
                glDrawElements(ptype, count, itype, 0);
                glBindVertexArray(0);
            }
            glEnable(GL_CULL_FACE);
            return;
        }

        return;
    }

    if (irr_driver->getPhase() == GLOW_PASS)
    {
        glUseProgram(MeshShader::ColorizeShader::getInstance()->Program);
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            glBindVertexArray(getVAO(video::EVT_STANDARD));
            drawGlow(GLmeshes[i]);
        }
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);

        if (immediate_draw)
        {
            if (update_each_frame)
                updatevbo();
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            if (World::getWorld() && World::getWorld()->isFogEnabled())
            {
                glUseProgram(MeshShader::TransparentFogShader::getInstance()->Program);
                for (unsigned i = 0; i < GLmeshes.size(); i++)
                {
                    GLMesh &mesh = GLmeshes[i];
                    irr_driver->IncreaseObjectCount();
                    GLenum ptype = mesh.PrimitiveType;
                    GLenum itype = mesh.IndexType;
                    size_t count = mesh.IndexCount;

                    const Track * const track = World::getWorld()->getTrack();

                    // This function is only called once per frame - thus no need for setters.
                    const float fogmax = track->getFogMax();
                    const float startH = track->getFogStartHeight();
                    const float endH = track->getFogEndHeight();
                    const float start = track->getFogStart();
                    const float end = track->getFogEnd();
                    const video::SColor tmpcol = track->getFogColor();

                    video::SColorf col(tmpcol.getRed() / 255.0f,
                        tmpcol.getGreen() / 255.0f,
                        tmpcol.getBlue() / 255.0f);

                    compressTexture(mesh.textures[0], true);
                    setTexture(MeshShader::TransparentFogShader::getInstance()->TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
                    MeshShader::TransparentFogShader::getInstance()->setUniforms(AbsoluteTransformation, mesh.TextureMatrix, fogmax, startH, endH, start, end, col);

                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            else
            {
                glUseProgram(MeshShader::TransparentShader::getInstance()->Program);
                for (unsigned i = 0; i < GLmeshes.size(); i++)
                {
                    irr_driver->IncreaseObjectCount();
                    GLMesh &mesh = GLmeshes[i];
                    GLenum ptype = mesh.PrimitiveType;
                    GLenum itype = mesh.IndexType;
                    size_t count = mesh.IndexCount;

                    compressTexture(mesh.textures[0], true);
                    setTexture(MeshShader::TransparentShader::getInstance()->TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

                    MeshShader::TransparentShader::getInstance()->setUniforms(AbsoluteTransformation, mesh.TextureMatrix);
                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            return;
        }

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
                pushVector(ListBlendTransparentFog::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                                                fogmax, startH, endH, start, end, col);
            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                pushVector(ListAdditiveTransparentFog::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix,
                                                                   fogmax, startH, endH, start, end, col);
        }
        else
        {
            for_in(mesh, TransparentMesh[TM_DEFAULT])
                pushVector(ListBlendTransparent::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix);

            for_in(mesh, TransparentMesh[TM_ADDITIVE])
                pushVector(ListAdditiveTransparent::getInstance(), mesh, AbsoluteTransformation, mesh->TextureMatrix);
        }

        for_in(mesh, TransparentMesh[TM_DISPLACEMENT])
            pushVector(ListDisplacement::getInstance(), mesh, AbsoluteTransformation);

        if (!TransparentMesh[TM_BUBBLE].empty())
            glUseProgram(MeshShader::BubbleShader::Program);
        glBindVertexArray(getVAO(video::EVT_STANDARD));
        for_in(mesh, TransparentMesh[TM_BUBBLE])
            drawBubble(*mesh, ModelViewProjectionMatrix);
        return;
    }
}
