#include "stkinstancedscenenode.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/callbacks.hpp"

STKInstancedSceneNode::STKInstancedSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    m_ref_count = 0;
    irr_driver->grabAllTextures(mesh);

    if (irr_driver->isGLSL())
    {
        createGLMeshes();
        setAutomaticCulling(0);
    }
}

void STKInstancedSceneNode::cleanGL()
{
    for (u32 i = 0; i < GLmeshes.size(); ++i)
    {
        GLMesh mesh = GLmeshes[i];
        if (!mesh.vertex_buffer)
            continue;
        if (mesh.vao)
            glDeleteVertexArrays(1, &(mesh.vao));
        if (mesh.vao_shadow_pass)
            glDeleteVertexArrays(1, &(mesh.vao_shadow_pass));
        glDeleteBuffers(1, &(mesh.vertex_buffer));
        glDeleteBuffers(1, &(mesh.index_buffer));
    }
    glDeleteBuffers(1, &instances_vbo);
}

STKInstancedSceneNode::~STKInstancedSceneNode()
{
    irr_driver->dropAllTextures(getMesh());
    irr_driver->removeMeshFromCache(getMesh());

    if (irr_driver->isGLSL())
        cleanGL();
}

void STKInstancedSceneNode::createGLMeshes()
{
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        GLmeshes.push_back(allocateMeshBuffer(mb));
        fillLocalBuffer(GLmeshes.back(), mb);
    }
    isMaterialInitialized = false;
}

void STKInstancedSceneNode::initinstancedvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat)
{
    mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
    glGenBuffers(1, &instances_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glBufferData(GL_ARRAY_BUFFER, instance_pos.size() * sizeof(float), instance_pos.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
    glVertexAttribDivisor(7, 1);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glVertexAttribDivisor(8, 1);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glVertexAttribDivisor(9, 1);

    mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
    glVertexAttribDivisor(7, 4);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glVertexAttribDivisor(8, 4);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glVertexAttribDivisor(9, 4);

    glBindVertexArray(0);
}

void STKInstancedSceneNode::setFirstTimeMaterial()
{
    if (isMaterialInitialized)
        return;
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;

        GLMesh &mesh = GLmeshes[i];
        GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type, mb->getVertexType());
        ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures, mb->getVertexType());
        initinstancedvaostate(mesh, GeometricType, ShadedType);
        GeometricMesh[GeometricType].push_back(&mesh);
        ShadedMesh[ShadedType].push_back(&mesh);
    }
    isMaterialInitialized = true;
}

void STKInstancedSceneNode::addInstance(const core::vector3df &origin, const core::vector3df &orientation, const core::vector3df &scale)
{
    instance_pos.push_back(origin.X);
    instance_pos.push_back(origin.Y);
    instance_pos.push_back(origin.Z);
    instance_pos.push_back(orientation.X);
    instance_pos.push_back(orientation.Y);
    instance_pos.push_back(orientation.Z);
    instance_pos.push_back(scale.X);
    instance_pos.push_back(scale.Y);
    instance_pos.push_back(scale.Z);
}

static void drawFSPMDefault(GLMesh &mesh, size_t instance_count)
{
  irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  if (mesh.textures[0])
  {
      compressTexture(mesh.textures[0], true);
      setTexture(MeshShader::InstancedObjectPass1ShaderInstance->TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  }
  else
  {
      setTexture(MeshShader::InstancedObjectPass1ShaderInstance->TU_tex, 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, false);
      GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  MeshShader::InstancedObjectPass1ShaderInstance->setUniforms();

  glBindVertexArray(mesh.vao);
  glDrawElementsInstanced(ptype, count, itype, 0, instance_count);

  if (!mesh.textures[0])
  {
      GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
}

static void drawShadowDefault(GLMesh &mesh, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    MeshShader::InstancedShadowShader::setUniforms();

    glBindVertexArray(mesh.vao_shadow_pass);
    glDrawElementsInstanced(ptype, count, itype, 0, 4 * instance_count);
}

static void drawFSPMAlphaRefTexture(GLMesh &mesh, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    MeshShader::InstancedObjectRefPass1Shader::setUniforms(0);

    glBindVertexArray(mesh.vao);
    glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
}

static void drawShadowAlphaRefTexture(GLMesh &mesh, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    MeshShader::InstancedRefShadowShader::setUniforms(0);

    glBindVertexArray(mesh.vao_shadow_pass);
    glDrawElementsInstanced(ptype, count, itype, 0, 4 * instance_count);
}

static void drawFSPMGrass(GLMesh &mesh, const core::vector3df &windDir, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    MeshShader::InstancedGrassPass1Shader::setUniforms(windDir, 0);

    glBindVertexArray(mesh.vao);
    glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
}

static void drawSMDefault(GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, size_t instance_count)
{
  irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  setTexture(MeshShader::InstancedObjectPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
      GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
      GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  MeshShader::InstancedObjectPass2Shader::setUniforms(ModelViewProjectionMatrix, core::matrix4::EM4CONST_IDENTITY);

  glBindVertexArray(mesh.vao);
  glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
}

static void drawSMAlphaRefTexture(GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::InstancedObjectRefPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    if (irr_driver->getLightViz())
    {
        GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    else
    {
        GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }

    MeshShader::InstancedObjectRefPass2Shader::setUniforms(ModelViewProjectionMatrix, core::matrix4::EM4CONST_IDENTITY);

    glBindVertexArray(mesh.vao);
    glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
}

static void drawSMGrass(GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::vector3df &windDir, size_t instance_count)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::InstancedGrassPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    if (irr_driver->getLightViz())
    {
        GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    else
    {
        GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    setTexture(MeshShader::InstancedGrassPass2Shader::TU_dtex, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);

    MeshShader::InstancedGrassPass2Shader::setUniforms(ModelViewProjectionMatrix, irr_driver->getInvViewMatrix(), irr_driver->getInvProjMatrix(), windDir, cb->getPosition());

    glBindVertexArray(mesh.vao);
    glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
}

void STKInstancedSceneNode::render()
{
    if (!irr_driver->isGLSL())
    {
        CMeshSceneNode::render();
        return;
    }

    setFirstTimeMaterial();

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
    {
        ModelViewProjectionMatrix = irr_driver->getProjMatrix();
        ModelViewProjectionMatrix *= irr_driver->getViewMatrix();

        if (!GeometricMesh[FPSM_DEFAULT_STANDARD].empty())
            glUseProgram(MeshShader::InstancedObjectPass1ShaderInstance->Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT_STANDARD].size(); i++)
            drawFSPMDefault(*GeometricMesh[FPSM_DEFAULT_STANDARD][i], instance_pos.size() / 9);

        if (!GeometricMesh[FPSM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::InstancedObjectRefPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawFSPMAlphaRefTexture(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i], instance_pos.size() / 9);

        windDir = getWind();
        if (!GeometricMesh[FPSM_GRASS].empty())
            glUseProgram(MeshShader::InstancedGrassPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_GRASS].size(); i++)
            drawFSPMGrass(*GeometricMesh[FPSM_GRASS][i], windDir, instance_pos.size() / 9);
        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        if (!ShadedMesh[SM_DEFAULT_STANDARD].empty())
            glUseProgram(MeshShader::InstancedObjectPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_DEFAULT_STANDARD].size(); i++)
            drawSMDefault(*ShadedMesh[SM_DEFAULT_STANDARD][i], ModelViewProjectionMatrix, instance_pos.size() / 9);

        if (!ShadedMesh[SM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::InstancedObjectRefPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_ALPHA_REF_TEXTURE].size(); i++)
            drawSMAlphaRefTexture(*ShadedMesh[SM_ALPHA_REF_TEXTURE][i], ModelViewProjectionMatrix, instance_pos.size() / 9);

        if (!ShadedMesh[SM_GRASS].empty())
            glUseProgram(MeshShader::InstancedGrassPass2Shader::Program);
        for (unsigned i = 0; i < ShadedMesh[SM_GRASS].size(); i++)
            drawSMGrass(*ShadedMesh[SM_GRASS][i], ModelViewProjectionMatrix, windDir, instance_pos.size() / 9);
        return;
    }

    if (irr_driver->getPhase() == SHADOW_PASS)
    {
        if (!GeometricMesh[FPSM_DEFAULT_STANDARD].empty())
            glUseProgram(MeshShader::InstancedShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT_STANDARD].size(); i++)
            drawShadowDefault(*GeometricMesh[FPSM_DEFAULT_STANDARD][i], instance_pos.size() / 9);

        if (!GeometricMesh[FPSM_ALPHA_REF_TEXTURE].empty())
            glUseProgram(MeshShader::InstancedRefShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawShadowAlphaRefTexture(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i], instance_pos.size() / 9);
        return;
    }
}
