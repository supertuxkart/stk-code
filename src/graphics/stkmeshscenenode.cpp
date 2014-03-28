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

static
core::vector3df getWind()
{
    const core::vector3df pos = irr_driver->getVideoDriver()->getTransform(video::ETS_WORLD).getTranslation();
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    GrassShaderProvider *gsp = (GrassShaderProvider *)irr_driver->getCallback(ES_GRASS);
    float m_speed = gsp->getSpeed(), m_amplitude = gsp->getAmplitude();

    float strength = (pos.X + pos.Y + pos.Z) * 1.2f + time * m_speed;
    strength = noise2d(strength / 10.0f) * m_amplitude * 5;
    // * 5 is to work with the existing amplitude values.

    // Pre-multiply on the cpu
    return irr_driver->getWind() * strength;
}

STKMeshSceneNode::STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    reload_each_frame = false;
    createGLMeshes();
}

void STKMeshSceneNode::setReloadEachFrame(bool val)
{
    reload_each_frame = val;
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
          TransparentMaterial TranspMat = MaterialTypeToTransparentMaterial(type);
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
  isMaterialInitialized = true;
}

void STKMeshSceneNode::cleanGLMeshes()
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
    GLmeshes.clear();
    for (unsigned i = 0; i < FPSM_COUNT; i++)
        GeometricMesh[i].clear();
    for (unsigned i = 0; i < SM_COUNT; i++)
        ShadedMesh[i].clear();
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

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    computeMVP(ModelViewProjectionMatrix);
    MeshShader::ColorizeShader::setUniforms(ModelViewProjectionMatrix, cb->getRed(), cb->getGreen(), cb->getBlue());

    assert(mesh.vao_glow_pass);
    glBindVertexArray(mesh.vao_glow_pass);
    glDrawElements(ptype, count, itype, 0);
}

static video::ITexture *displaceTex = 0;

void STKMeshSceneNode::drawDisplace(const GLMesh &mesh)
{
    DisplaceProvider * const cb = (DisplaceProvider *)irr_driver->getCallback(ES_DISPLACE);

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    computeMVP(ModelViewProjectionMatrix);
    core::matrix4 ModelViewMatrix = irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
    ModelViewMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_WORLD);

    // Generate displace mask
    // Use RTT_TMP4 as displace mask
    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_TMP4), false, false);

    glUseProgram(MeshShader::DisplaceMaskShader::Program);
    MeshShader::DisplaceMaskShader::setUniforms(ModelViewProjectionMatrix);

    assert(mesh.vao_displace_mask_pass);
    glBindVertexArray(mesh.vao_displace_mask_pass);
    glDrawElements(ptype, count, itype, 0);

    // Render the effect
    if (!displaceTex)
        displaceTex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");
    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_DISPLACE), false, false);
    setTexture(0, getTextureGLuint(displaceTex), GL_LINEAR, GL_LINEAR, true);
    setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP4)), GL_LINEAR, GL_LINEAR, true);
    setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_COLOR)), GL_LINEAR, GL_LINEAR, true);
    glUseProgram(MeshShader::DisplaceShader::Program);
    MeshShader::DisplaceShader::setUniforms(ModelViewProjectionMatrix, ModelViewMatrix, core::vector2df(cb->getDirX(), cb->getDirY()), core::vector2df(cb->getDir2X(), cb->getDir2Y()), core::vector2df(UserConfigParams::m_width, UserConfigParams::m_height), 0, 1, 2);

    assert(mesh.vao_displace_pass);
    glBindVertexArray(mesh.vao_displace_pass);
    glDrawElements(ptype, count, itype, 0);
}

void STKMeshSceneNode::drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type)
{
    assert(irr_driver->getPhase() == TRANSPARENT_PASS);

    computeMVP(ModelViewProjectionMatrix);

    if (type == irr_driver->getShader(ES_BUBBLES))
        drawBubble(mesh, ModelViewProjectionMatrix);
//    else if (World::getWorld()->getTrack()->isFogEnabled())
//        drawTransparentFogObject(mesh, ModelViewProjectionMatrix, TextureMatrix);
    else
        drawTransparentObject(mesh, ModelViewProjectionMatrix, mesh.TextureMatrix);
    return;
}

void STKMeshSceneNode::drawSolidPass1(const GLMesh &mesh, GeometricMaterial type)
{
    irr_driver->IncreaseObjectCount();
    windDir = getWind();
    switch (type)
    {
    case FPSM_NORMAL_MAP:
        drawNormalPass(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
        break;
    case FPSM_ALPHA_REF_TEXTURE:
        drawObjectRefPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView, mesh.TextureMatrix);
        break;
    case FPSM_GRASS:
        drawGrassPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView, windDir);
        break;
    case FPSM_DEFAULT:
        drawObjectPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
        break;
    default:
        assert(0 && "wrong geometric material");
    }
}

void STKMeshSceneNode::drawSolidPass2(const GLMesh &mesh, ShadedMaterial type)
{
    switch (type)
    {
    case SM_SPHEREMAP:
        drawSphereMap(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
        break;
    case SM_SPLATTING:
        drawSplatting(mesh, ModelViewProjectionMatrix);
        break;
    case SM_ALPHA_REF_TEXTURE:
        drawObjectRefPass2(mesh, ModelViewProjectionMatrix, mesh.TextureMatrix);
        break;
    case SM_GRASS:
        drawGrassPass2(mesh, ModelViewProjectionMatrix, windDir);
        break;
    case SM_RIMLIT:
        drawObjectRimLimit(mesh, ModelViewProjectionMatrix, TransposeInverseModelView, core::matrix4::EM4CONST_IDENTITY);
        break;
    case SM_UNLIT:
        drawObjectUnlit(mesh, ModelViewProjectionMatrix);
        break;
    case SM_CAUSTICS:
    {
        const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
        const float speed = World::getWorld()->getTrack()->getCausticsSpeed();

        float strength = time;
        strength = fabsf(noise2d(strength / 10.0f)) * 0.006f + 0.001f;

        vector3df wind = irr_driver->getWind() * strength * speed;
        caustic_dir.X += wind.X;
        caustic_dir.Y += wind.Z;

        strength = time * 0.56f + sinf(time);
        strength = fabsf(noise2d(0.0, strength / 6.0f)) * 0.0095f + 0.001f;

        wind = irr_driver->getWind() * strength * speed;
        wind.rotateXZBy(cosf(time));
        caustic_dir2.X += wind.X;
        caustic_dir2.Y += wind.Z;
        drawCaustics(mesh, ModelViewProjectionMatrix, caustic_dir, caustic_dir2);
        break;
    }
    case SM_DETAILS:
        drawDetailledObjectPass2(mesh, ModelViewProjectionMatrix);
        break;
    case SM_UNTEXTURED:
        drawUntexturedObject(mesh, ModelViewProjectionMatrix);
        break;
    case SM_DEFAULT:
        drawObjectPass2(mesh, ModelViewProjectionMatrix, mesh.TextureMatrix);
        break;
    default:
        assert(0 && "Wrong shaded material");
        break;
    }
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
        GLenum indexSize;
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

    if (reload_each_frame)
        updatevbo();

    bool isTransparentPass =
        SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

    ++PassCount;

    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
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
        if (reload_each_frame)
            glDisable(GL_CULL_FACE);
        computeMVP(ModelViewProjectionMatrix);
        computeTIMV(TransposeInverseModelView);

        if (!GeometricMesh[FPSM_DEFAULT].empty())
            glUseProgram(MeshShader::ObjectPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_DEFAULT][i], FPSM_DEFAULT);

        if (!GeometricMesh[FPSM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::ObjectRefPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i], FPSM_ALPHA_REF_TEXTURE);

        if (!GeometricMesh[FPSM_NORMAL_MAP].empty())
            glUseProgram(MeshShader::NormalMapShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_NORMAL_MAP].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_NORMAL_MAP][i], FPSM_NORMAL_MAP);

        if (!GeometricMesh[FPSM_GRASS].empty())
            glUseProgram(MeshShader::GrassPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_GRASS].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_GRASS][i], FPSM_GRASS);

        if (reload_each_frame)
            glEnable(GL_CULL_FACE);
        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        if (reload_each_frame)
            glDisable(GL_CULL_FACE);

        if (!ShadedMesh[SM_DEFAULT].empty())
          glUseProgram(MeshShader::ObjectPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_DEFAULT].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_DEFAULT][i], SM_DEFAULT);

        if (!ShadedMesh[SM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::ObjectRefPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_ALPHA_REF_TEXTURE].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_ALPHA_REF_TEXTURE][i], SM_ALPHA_REF_TEXTURE);

        if (!ShadedMesh[SM_RIMLIT].empty())
            glUseProgram(MeshShader::ObjectRimLimitShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_RIMLIT].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_RIMLIT][i], SM_RIMLIT);

        if (!ShadedMesh[SM_SPHEREMAP].empty())
            glUseProgram(MeshShader::SphereMapShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_SPHEREMAP].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_SPHEREMAP][i], SM_SPHEREMAP);

        if (!ShadedMesh[SM_SPLATTING].empty())
            glUseProgram(MeshShader::SplattingShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_SPLATTING].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_SPLATTING][i], SM_SPLATTING);

        if (!ShadedMesh[SM_GRASS].empty())
            glUseProgram(MeshShader::GrassPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_GRASS].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_GRASS][i], SM_GRASS);

        if (!ShadedMesh[SM_UNLIT].empty())
            glUseProgram(MeshShader::ObjectUnlitShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_UNLIT].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_UNLIT][i], SM_UNLIT);

        if (!ShadedMesh[SM_CAUSTICS].empty())
            glUseProgram(MeshShader::CausticsShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_CAUSTICS].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_CAUSTICS][i], SM_CAUSTICS);

        if (!ShadedMesh[SM_DETAILS].empty())
            glUseProgram(MeshShader::DetailledObjectPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_DETAILS].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_DETAILS][i], SM_DETAILS);

        if (!ShadedMesh[SM_UNTEXTURED].empty())
            glUseProgram(MeshShader::UntexturedObjectShader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_UNTEXTURED].size(); i++)
            drawSolidPass2(*ShadedMesh[SM_UNTEXTURED][i], SM_UNTEXTURED);

        if (reload_each_frame)
            glEnable(GL_CULL_FACE);
        return;
    }

    if (irr_driver->getPhase() == SHADOW_PASS)
    {
        if (reload_each_frame)
            glDisable(GL_CULL_FACE);

        if (!GeometricMesh[FPSM_DEFAULT].empty())
            glUseProgram(MeshShader::ShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT].size(); i++)
            drawShadow(*GeometricMesh[FPSM_DEFAULT][i]);

        if (!GeometricMesh[FPSM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::RefShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawShadowRef(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i]);

        if (reload_each_frame)
            glEnable(GL_CULL_FACE);
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
            drawGlow(GLmeshes[i]);
        }
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS)
    {
        computeMVP(ModelViewProjectionMatrix);

        if (!TransparentMesh[TM_BUBBLE].empty())
            glUseProgram(MeshShader::BubbleShader::Program);
        for (unsigned i = 0; i < TransparentMesh[TM_BUBBLE].size(); i++)
            drawBubble(*TransparentMesh[TM_BUBBLE][i], ModelViewProjectionMatrix);

        if (World::getWorld()->getTrack()->isFogEnabled())
        {
            if (!TransparentMesh[TM_DEFAULT].empty())
                glUseProgram(MeshShader::TransparentFogShader::Program);
            for (unsigned i = 0; i < TransparentMesh[TM_DEFAULT].size(); i++)
                drawTransparentFogObject(*TransparentMesh[TM_DEFAULT][i], ModelViewProjectionMatrix, (*TransparentMesh[TM_DEFAULT][i]).TextureMatrix);
        }
        else
        {
            if (!TransparentMesh[TM_DEFAULT].empty())
                glUseProgram(MeshShader::TransparentShader::Program);
            for (unsigned i = 0; i < TransparentMesh[TM_DEFAULT].size(); i++)
                drawTransparentObject(*TransparentMesh[TM_DEFAULT][i], ModelViewProjectionMatrix, (*TransparentMesh[TM_DEFAULT][i]).TextureMatrix);
        }
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
