#include "stkmeshscenenode.hpp"
#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include "tracks/track.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "utils/helpers.hpp"
#include "graphics/camera.hpp"
#include "modes/world.hpp"

STKMeshSceneNode::STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    immediate_draw = false;
    update_each_frame = false;
    createGLMeshes();
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
              initvaostate(mesh, TranspMat);
              glBindVertexArray(0);
          }
          else
              TransparentMesh[TranspMat].push_back(&mesh);
      }
      else
      {
          GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type, mb->getVertexType());
          ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures, mb->getVertexType());
          if (immediate_draw)
          {
              fillLocalBuffer(mesh, mb);
              initvaostate(mesh, GeometricType, ShadedType);
              glBindVertexArray(0);
          }
          else
          {
              GeometricMesh[GeometricType].push_back(&mesh);
              ShadedMesh[ShadedType].push_back(&mesh);
          }
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
    for (unsigned i = 0; i < FPSM_COUNT; i++)
        GeometricMesh[i].clearWithoutDeleting();
    for (unsigned i = 0; i < SM_COUNT; i++)
        ShadedMesh[i].clearWithoutDeleting();
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

    MeshShader::ColorizeShader::setUniforms(AbsoluteTransformation, cb->getRed(), cb->getGreen(), cb->getBlue());
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
}

static video::ITexture *displaceTex = 0;

void STKMeshSceneNode::drawDisplace(const GLMesh &mesh)
{
    if (mesh.VAOType != video::EVT_2TCOORDS)
    {
        Log::error("Materials", "Displacement has wrong vertex type");
        return;
    }
    glBindVertexArray(getVAO(video::EVT_2TCOORDS));
    DisplaceProvider * const cb = (DisplaceProvider *)irr_driver->getCallback(ES_DISPLACE);

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    // Generate displace mask
    // Use RTT_TMP4 as displace mask
    irr_driver->getFBO(FBO_TMP1_WITH_DS).Bind();

    glUseProgram(MeshShader::DisplaceMaskShader::Program);
    MeshShader::DisplaceMaskShader::setUniforms(AbsoluteTransformation);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);

    // Render the effect
    if (!displaceTex)
        displaceTex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");
    irr_driver->getFBO(FBO_DISPLACE).Bind();
    setTexture(0, getTextureGLuint(displaceTex), GL_LINEAR, GL_LINEAR, true);
    setTexture(1, irr_driver->getRenderTargetTexture(RTT_TMP1), GL_LINEAR, GL_LINEAR, true);
    setTexture(2, irr_driver->getRenderTargetTexture(RTT_COLOR), GL_LINEAR, GL_LINEAR, true);
    setTexture(3, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR, true);
    glUseProgram(MeshShader::DisplaceShader::Program);
    MeshShader::DisplaceShader::setUniforms(AbsoluteTransformation,
                                            core::vector2df(cb->getDirX(), cb->getDirY()),
                                            core::vector2df(cb->getDir2X(), cb->getDir2Y()),
                                            core::vector2df(float(UserConfigParams::m_width),
                                                            float(UserConfigParams::m_height)),
                                            0, 1, 2, 3);

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
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
        const void* vertices = mb->getVertices();
        const u32 vertexCount = mb->getVertexCount();
        const c8* vbuf = static_cast<const c8*>(vertices);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * mesh.Stride, vbuf, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);
        const void* indices = mb->getIndices();
        mesh.IndexCount = mb->getIndexCount();
        size_t indexSize = 0;
        switch (mb->getIndexType())
        {
        case irr::video::EIT_16BIT:
            indexSize = sizeof(u16);
            break;
        case irr::video::EIT_32BIT:
            indexSize = sizeof(u32);
            break;
        default:
            assert(0 && "Wrong index size");
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount * indexSize, indices, GL_STATIC_DRAW);
    }
}

void STKMeshSceneNode::render()
{
    irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();

    if (!Mesh || !driver)
        return;

    bool isTransparentPass =
        SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

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

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);
        TransposeInverseModelView = computeTIMV(AbsoluteTransformation);
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        if (immediate_draw)
        {
            glDisable(GL_CULL_FACE);
            if (update_each_frame)
                updatevbo();
            glUseProgram(MeshShader::ObjectPass1Shader::Program);
            // Only untextured
            for (unsigned i = 0; i < GLmeshes.size(); i++)
            {
                irr_driver->IncreaseObjectCount();
                GLMesh &mesh = GLmeshes[i];
                GLenum ptype = mesh.PrimitiveType;
                GLenum itype = mesh.IndexType;
                size_t count = mesh.IndexCount;

                MeshShader::ObjectPass1Shader::setUniforms(AbsoluteTransformation, invmodel, 0);
                assert(mesh.vao);
                glBindVertexArray(mesh.vao);
                glDrawElements(ptype, count, itype, 0);
                glBindVertexArray(0);
            }
            glEnable(GL_CULL_FACE);
            return;
        }


        GLMesh* mesh;
        for_in(mesh, GeometricMesh[FPSM_DEFAULT_STANDARD])
        {
            GroupedFPSM<FPSM_DEFAULT_STANDARD>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_DEFAULT_STANDARD>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_DEFAULT_STANDARD>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, GeometricMesh[FPSM_DEFAULT_2TCOORD])
        {
            GroupedFPSM<FPSM_DEFAULT_2TCOORD>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_DEFAULT_2TCOORD>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_DEFAULT_2TCOORD>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, GeometricMesh[FPSM_ALPHA_REF_TEXTURE])
        {
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_ALPHA_REF_TEXTURE>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, GeometricMesh[FPSM_NORMAL_MAP])
        {
            GroupedFPSM<FPSM_NORMAL_MAP>::MeshSet.push_back(mesh);
            GroupedFPSM<FPSM_NORMAL_MAP>::MVPSet.push_back(AbsoluteTransformation);
            GroupedFPSM<FPSM_NORMAL_MAP>::TIMVSet.push_back(invmodel);
        }

        if (!GeometricMesh[FPSM_GRASS].empty())
            glUseProgram(MeshShader::GrassPass1Shader::Program);
        windDir = getWind();
        glBindVertexArray(getVAO(video::EVT_STANDARD));
        for_in(mesh, GeometricMesh[FPSM_GRASS])
            drawGrassPass1(*mesh, ModelViewProjectionMatrix, TransposeInverseModelView, windDir);

        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        if (immediate_draw)
        {
            glDisable(GL_CULL_FACE);
            glUseProgram(MeshShader::UntexturedObjectShader::Program);
            // Only untextured
            for (unsigned i = 0; i < GLmeshes.size(); i++)
            {
                irr_driver->IncreaseObjectCount();
                GLMesh &mesh = GLmeshes[i];
                GLenum ptype = mesh.PrimitiveType;
                GLenum itype = mesh.IndexType;
                size_t count = mesh.IndexCount;

                MeshShader::UntexturedObjectShader::setUniforms(AbsoluteTransformation);
                assert(mesh.vao);
                glBindVertexArray(mesh.vao);
                glDrawElements(ptype, count, itype, 0);
                glBindVertexArray(0);
            }
            glEnable(GL_CULL_FACE);
            return;
        }

        GLMesh* mesh;
        for_in(mesh, ShadedMesh[SM_DEFAULT_STANDARD])
        {
            GroupedSM<SM_DEFAULT_STANDARD>::MeshSet.push_back(mesh);
            GroupedSM<SM_DEFAULT_STANDARD>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_DEFAULT_STANDARD>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_DEFAULT_TANGENT])
        {
            GroupedSM<SM_DEFAULT_TANGENT>::MeshSet.push_back(mesh);
            GroupedSM<SM_DEFAULT_TANGENT>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_DEFAULT_TANGENT>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_ALPHA_REF_TEXTURE])
        {
            GroupedSM<SM_ALPHA_REF_TEXTURE>::MeshSet.push_back(mesh);
            GroupedSM<SM_ALPHA_REF_TEXTURE>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_ALPHA_REF_TEXTURE>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_SPHEREMAP])
        {
            GroupedSM<SM_SPHEREMAP>::MeshSet.push_back(mesh);
            GroupedSM<SM_SPHEREMAP>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_SPHEREMAP>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_SPLATTING])
        {
            GroupedSM<SM_SPLATTING>::MeshSet.push_back(mesh);
            GroupedSM<SM_SPLATTING>::MVPSet.push_back(AbsoluteTransformation);
            GroupedSM<SM_SPLATTING>::TIMVSet.push_back(invmodel);
        }

        for_in(mesh, ShadedMesh[SM_UNLIT])
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

        if (!ShadedMesh[SM_GRASS].empty())
            glUseProgram(MeshShader::GrassPass2Shader::Program);
        glBindVertexArray(getVAO(video::EVT_STANDARD));
        for_in(mesh, ShadedMesh[SM_GRASS])
            drawGrassPass2(*mesh, ModelViewProjectionMatrix, windDir);

        return;
    }

    if (irr_driver->getPhase() == GLOW_PASS)
    {
        glUseProgram(MeshShader::ColorizeShader::Program);
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
                glUseProgram(MeshShader::TransparentFogShader::Program);
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

                    core::vector3df col(tmpcol.getRed() / 255.0f,
                        tmpcol.getGreen() / 255.0f,
                        tmpcol.getBlue() / 255.0f);

                    compressTexture(mesh.textures[0], true);
                    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
                    MeshShader::TransparentFogShader::setUniforms(AbsoluteTransformation, mesh.TextureMatrix, fogmax, startH, endH, start, end, col, Camera::getCamera(0)->getCameraSceneNode()->getAbsolutePosition(), 0);

                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            else
            {
                glUseProgram(MeshShader::TransparentShader::Program);
                for (unsigned i = 0; i < GLmeshes.size(); i++)
                {
                    irr_driver->IncreaseObjectCount();
                    GLMesh &mesh = GLmeshes[i];
                    GLenum ptype = mesh.PrimitiveType;
                    GLenum itype = mesh.IndexType;
                    size_t count = mesh.IndexCount;

                    compressTexture(mesh.textures[0], true);
                    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

                    MeshShader::TransparentShader::setUniforms(AbsoluteTransformation, mesh.TextureMatrix, 0);
                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            return;
        }

        GLMesh* mesh;

        for_in(mesh, TransparentMesh[TM_DEFAULT])
        {
            TransparentMeshes<TM_DEFAULT>::MeshSet.push_back(mesh);
            TransparentMeshes<TM_DEFAULT>::MVPSet.push_back(AbsoluteTransformation);
        }

        for_in(mesh, TransparentMesh[TM_ADDITIVE])
        {
            TransparentMeshes<TM_ADDITIVE>::MeshSet.push_back(mesh);
            TransparentMeshes<TM_ADDITIVE>::MVPSet.push_back(AbsoluteTransformation);
        }

        if (!TransparentMesh[TM_BUBBLE].empty())
            glUseProgram(MeshShader::BubbleShader::Program);
        glBindVertexArray(getVAO(video::EVT_STANDARD));
        for_in(mesh, TransparentMesh[TM_BUBBLE])
            drawBubble(*mesh, ModelViewProjectionMatrix);
        return;
    }

    if (irr_driver->getPhase() == DISPLACEMENT_PASS)
    {
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            drawDisplace(GLmeshes[i]);
        }
    }
}
