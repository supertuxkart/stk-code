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

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE MaterialType)
{
    if (MaterialType == irr_driver->getShader(ES_NORMAL_MAP))
        return FPSM_NORMAL_MAP;
    else if (MaterialType == irr_driver->getShader(ES_OBJECTPASS_REF) || MaterialType == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return FPSM_ALPHA_REF_TEXTURE;
    else if (MaterialType == irr_driver->getShader(ES_GRASS) || MaterialType == irr_driver->getShader(ES_GRASS_REF))
        return FPSM_GRASS;
    else
        return FPSM_DEFAULT;
}

ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE type, video::ITexture **textures)
{
    if (type == irr_driver->getShader(ES_SPHERE_MAP))
        return SM_SPHEREMAP;
    else if (type == irr_driver->getShader(ES_SPLATTING))
        return SM_SPLATTING;
    else if (type == irr_driver->getShader(ES_OBJECTPASS_REF) || type == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return SM_ALPHA_REF_TEXTURE;
    else if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT) && textures[0])
        return SM_RIMLIT;
    else if (type == irr_driver->getShader(ES_GRASS) || type == irr_driver->getShader(ES_GRASS_REF))
        return SM_GRASS;
    else if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
        return SM_UNLIT;
    else if (type == irr_driver->getShader(ES_CAUSTICS))
        return SM_CAUSTICS;
    else if (textures[1] && type != irr_driver->getShader(ES_NORMAL_MAP))
        return SM_DETAILS;
    else if (!textures[0])
        return SM_UNTEXTURED;
    else
        return SM_DEFAULT;
}

TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE type, f32 MaterialTypeParam)
{
    if (type == irr_driver->getShader(ES_BUBBLES))
        return TM_BUBBLE;
    video::E_BLEND_FACTOR srcFact, DstFact;
    video::E_MODULATE_FUNC mod;
    u32 alpha;
    unpack_textureBlendFunc(srcFact, DstFact, mod, alpha, MaterialTypeParam);
    if (DstFact == video::EBF_ONE)
        return TM_ADDITIVE;
    return TM_DEFAULT;
}

GLuint createVAO(GLuint vbo, GLuint idx, GLuint attrib_position, GLuint attrib_texcoord, GLuint attrib_second_texcoord, GLuint attrib_normal, GLuint attrib_tangent, GLuint attrib_bitangent, GLuint attrib_color, size_t stride)
{
    if (attrib_position == -1)
        return 0;
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(attrib_position);
	if ((GLint)attrib_texcoord != -1)
		glEnableVertexAttribArray(attrib_texcoord);
	if ((GLint)attrib_second_texcoord != -1)
		glEnableVertexAttribArray(attrib_second_texcoord);
	if ((GLint)attrib_normal != -1)
		glEnableVertexAttribArray(attrib_normal);
	if ((GLint)attrib_tangent != -1)
		glEnableVertexAttribArray(attrib_tangent);
	if ((GLint)attrib_bitangent != -1)
		glEnableVertexAttribArray(attrib_bitangent);
	if ((GLint)attrib_color != -1)
		glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, stride, 0);
	if ((GLint)attrib_texcoord != -1)
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*) 28);
	if ((GLint)attrib_second_texcoord != -1)
	{
		if (stride < 44)
			Log::error("material", "Second texcoords not present in VBO");
		glVertexAttribPointer(attrib_second_texcoord, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*) 36);
	}
	if ((GLint)attrib_normal != -1)
		glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*) 12);
	if ((GLint)attrib_tangent != -1)
	{
		if (stride < 48)
			Log::error("material", "Tangents not present in VBO");
		glVertexAttribPointer(attrib_tangent, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)36);
	}
		
	if ((GLint)attrib_bitangent != -1)
	{
		if (stride < 60)
			Log::error("material", "Bitangents not present in VBO");
		glVertexAttribPointer(attrib_bitangent, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)48);
	}
	if ((GLint)attrib_color != -1)
		glVertexAttribPointer(attrib_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)24);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
	return vao;
}

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb)
{
	GLMesh result = {};
	if (!mb)
		return result;
	glBindVertexArray(0);
	glGenBuffers(1, &(result.vertex_buffer));
	glGenBuffers(1, &(result.index_buffer));

	glBindBuffer(GL_ARRAY_BUFFER, result.vertex_buffer);
	const void* vertices = mb->getVertices();
	const u32 vertexCount = mb->getVertexCount();
	const irr::video::E_VERTEX_TYPE vType = mb->getVertexType();
	result.Stride = getVertexPitchFromType(vType);
	const c8* vbuf = static_cast<const c8*>(vertices);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * result.Stride, vbuf, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.index_buffer);
	const void* indices = mb->getIndices();
	u32 indexCount = mb->getIndexCount();
	GLenum indexSize;
	switch (mb->getIndexType())
	{
		case irr::video::EIT_16BIT:
		{
			indexSize = sizeof(u16);
			result.IndexType = GL_UNSIGNED_SHORT;
			break;
		}
		case irr::video::EIT_32BIT:
		{
			indexSize = sizeof(u32);
			result.IndexType = GL_UNSIGNED_INT;
			break;
		}
		default:
		{
			assert(0 && "Wrong index size");
		}
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	result.IndexCount = mb->getIndexCount();
	switch (mb->getPrimitiveType())
	{
	case scene::EPT_POINTS:
		result.PrimitiveType = GL_POINTS;
		break;
	case scene::EPT_TRIANGLE_STRIP:
		result.PrimitiveType = GL_TRIANGLE_STRIP;
		break;
	case scene::EPT_TRIANGLE_FAN:
		result.PrimitiveType = GL_TRIANGLE_FAN;
		break;
	case scene::EPT_LINES:
		result.PrimitiveType = GL_LINES;
        break;
	case scene::EPT_TRIANGLES:
		result.PrimitiveType = GL_TRIANGLES;
		break;
	case scene::EPT_POINT_SPRITES:
	case scene::EPT_LINE_LOOP:
	case scene::EPT_POLYGON:
	case scene::EPT_LINE_STRIP:
	case scene::EPT_QUAD_STRIP:
	case scene::EPT_QUADS:
		assert(0 && "Unsupported primitive type");
	}
	ITexture *tex;
	for (unsigned i = 0; i < 6; i++)
        result.textures[i] = mb->getMaterial().getTexture(i);
    result.TextureMatrix = 0;
	return result;
}


core::matrix4 computeMVP(const core::matrix4 &ModelMatrix)
{
    core::matrix4 ModelViewProjectionMatrix = irr_driver->getProjMatrix();
    ModelViewProjectionMatrix *= irr_driver->getViewMatrix();
    ModelViewProjectionMatrix *= ModelMatrix;
    return ModelViewProjectionMatrix;
}

core::matrix4 computeTIMV(const core::matrix4 &ModelMatrix)
{
    core::matrix4 TransposeInverseModelView = irr_driver->getViewMatrix();
    TransposeInverseModelView *= ModelMatrix;
    TransposeInverseModelView.makeInverse();
    TransposeInverseModelView = TransposeInverseModelView.getTransposed();
    return TransposeInverseModelView;
}

core::vector3df getWind()
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    GrassShaderProvider *gsp = (GrassShaderProvider *)irr_driver->getCallback(ES_GRASS);
    float m_speed = gsp->getSpeed(), m_amplitude = gsp->getAmplitude();

    return m_speed * vector3df(1., 0., 0.) * cos(time);
}



void drawObjectPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  if (mesh.textures[0])
  {
      compressTexture(mesh.textures[0], true);
      setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  }
  else
  {
      setTexture(0, 0, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, false);
      GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  MeshShader::ObjectPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, 0);

  assert(mesh.vao_first_pass);
  glBindVertexArray(mesh.vao_first_pass);
  glDrawElements(ptype, count, itype, 0);

  if (!mesh.textures[0])
  {
      GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
}

void drawObjectRefPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  compressTexture(mesh.textures[0], true);
  setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

  MeshShader::ObjectRefPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, TextureMatrix, 0);

  assert(mesh.vao_first_pass);
  glBindVertexArray(mesh.vao_first_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	MeshShader::GrassPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, windDir, 0);

    assert(mesh.vao_first_pass);
	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawNormalPass(const GLMesh &mesh, const core::matrix4 & ModelMatrix, const core::matrix4 &InverseModelMatrix)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	assert(mesh.textures[1]);
    compressTexture(mesh.textures[1], false);
    setTexture(0, getTextureGLuint(mesh.textures[1]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    compressTexture(mesh.textures[0], true);
    setTexture(1, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    MeshShader::NormalMapShader::setUniforms(ModelMatrix, InverseModelMatrix, 0, 1);

    assert(mesh.vao_first_pass);
	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawSphereMap(const GLMesh &mesh, const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  glActiveTexture(GL_TEXTURE0 + MeshShader::SphereMapShader::TU_tex);
  if (!irr_driver->SkyboxCubeMap)
  {
      GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
      glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
      glBindTexture(GL_TEXTURE_CUBE_MAP, irr_driver->SkyboxCubeMap);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  MeshShader::SphereMapShader::setUniforms(ModelMatrix, InverseModelMatrix, 
                                           core::vector2df(float(UserConfigParams::m_width), 
                                                           float(UserConfigParams::m_height)));
  assert(mesh.vao_second_pass);
  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
  if (!irr_driver->SkyboxCubeMap)
  {
      GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
      glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
}

void drawSplatting(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  // Texlayout
  compressTexture(mesh.textures[1], true);
  setTexture(MeshShader::SplattingShader::TU_tex_layout, getTextureGLuint(mesh.textures[1]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  //Tex detail0
  compressTexture(mesh.textures[2], true);
  setTexture(MeshShader::SplattingShader::TU_tex_detail0, getTextureGLuint(mesh.textures[2]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  //Tex detail1
  compressTexture(mesh.textures[3], true);
  setTexture(MeshShader::SplattingShader::TU_tex_detail1, getTextureGLuint(mesh.textures[3]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  compressTexture(mesh.textures[4], true);
  //Tex detail2
  setTexture(MeshShader::SplattingShader::TU_tex_detail2, getTextureGLuint(mesh.textures[4]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  //Tex detail3
  compressTexture(mesh.textures[5], true);
  setTexture(MeshShader::SplattingShader::TU_tex_detail3, getTextureGLuint(mesh.textures[5]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  MeshShader::SplattingShader::setUniforms(ModelViewProjectionMatrix);

  assert(mesh.vao_second_pass);
  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectRefPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  compressTexture(mesh.textures[0], true);
  setTexture(MeshShader::ObjectRefPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  MeshShader::ObjectRefPass2Shader::setUniforms(ModelViewProjectionMatrix, TextureMatrix);

  assert(mesh.vao_second_pass);
  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

static video::ITexture *CausticTex = 0;

void drawCaustics(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector2df dir, core::vector2df dir2)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::CausticsShader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
    if (!CausticTex)
        CausticTex = irr_driver->getTexture(file_manager->getAsset("textures/caustics.png").c_str());
    compressTexture(CausticTex, false);
    setTexture(MeshShader::CausticsShader::TU_caustictex, getTextureGLuint(CausticTex), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    MeshShader::CausticsShader::setUniforms(ModelViewProjectionMatrix, dir, dir2,
                                            core::vector2df(float(UserConfigParams::m_width),
                                                            float(UserConfigParams::m_height)));

    assert(mesh.vao_second_pass);
    glBindVertexArray(mesh.vao_second_pass);
    glDrawElements(ptype, count, itype, 0);
}

void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::GrassPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
	if (irr_driver->getLightViz())
	{
	  GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
	  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}
	else
	{
	  GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
	  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

    MeshShader::GrassPass2Shader::setUniforms(ModelViewProjectionMatrix, windDir);

    assert(mesh.vao_second_pass);
	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawUntexturedObject(const GLMesh &mesh, const core::matrix4 &ModelMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  MeshShader::UntexturedObjectShader::setUniforms(ModelMatrix);

  assert(mesh.vao_second_pass);
  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectRimLimit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::ObjectRimLimitShader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

    MeshShader::ObjectRimLimitShader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, TextureMatrix);

    assert(mesh.vao_second_pass);
	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawObjectUnlit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::ObjectUnlitShader::TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

    MeshShader::ObjectUnlitShader::setUniforms(ModelViewProjectionMatrix);

    assert(mesh.vao_second_pass);
	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawDetailledObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
    irr_driver->IncreaseObjectCount();
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  compressTexture(mesh.textures[0], true);
  setTexture(MeshShader::DetailledObjectPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
  if (irr_driver->getLightViz())
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  else
  {
    GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  compressTexture(mesh.textures[1], true);
  setTexture(MeshShader::DetailledObjectPass2Shader::TU_detail, getTextureGLuint(mesh.textures[1]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

  MeshShader::DetailledObjectPass2Shader::setUniforms(ModelViewProjectionMatrix);

  assert(mesh.vao_second_pass);
  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::ObjectPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

    MeshShader::ObjectPass2Shader::setUniforms(ModelViewProjectionMatrix, TextureMatrix);

    assert(mesh.vao_second_pass);
	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
    irr_driver->IncreaseObjectCount();
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    MeshShader::TransparentShader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, 0);

    assert(mesh.vao_first_pass);
	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
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

    glUseProgram(MeshShader::TransparentFogShader::Program);
    MeshShader::TransparentFogShader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, irr_driver->getInvProjMatrix(), fogmax, startH, endH, start, end, col, Camera::getCamera(0)->getCameraSceneNode()->getAbsolutePosition(), 0);

    assert(mesh.vao_first_pass);
    glBindVertexArray(mesh.vao_first_pass);
    glDrawElements(ptype, count, itype, 0);
}

void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
    irr_driver->IncreaseObjectCount();
	const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
	float transparency = 1.;

	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	MeshShader::BubbleShader::setUniforms(ModelViewProjectionMatrix, 0, time, transparency);

    assert(mesh.vao_first_pass);
	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawShadowRef(const GLMesh &mesh, const core::matrix4 &ModelMatrix)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    MeshShader::RefShadowShader::setUniforms(ModelMatrix, 0);

    assert(mesh.vao_shadow_pass);
    glBindVertexArray(mesh.vao_shadow_pass);
    glDrawElementsInstanced(ptype, count, itype, 0, 4);
}

void drawShadow(const GLMesh &mesh, const core::matrix4 &ModelMatrix)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    MeshShader::ShadowShader::setUniforms(ModelMatrix);

    assert(mesh.vao_shadow_pass);
    glBindVertexArray(mesh.vao_shadow_pass);
    glDrawElementsInstanced(ptype, count, itype, 0, 4);
}

bool isObject(video::E_MATERIAL_TYPE type)
{
	if (type == irr_driver->getShader(ES_OBJECTPASS))
		return true;
	if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
		return true;
	if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT))
		return true;
	if (type == irr_driver->getShader(ES_NORMAL_MAP))
		return true;
	if (type == irr_driver->getShader(ES_SPHERE_MAP))
		return true;
	if (type == irr_driver->getShader(ES_SPLATTING))
		return true;
	if (type == irr_driver->getShader(ES_GRASS))
		return true;
	if (type == irr_driver->getShader(ES_GRASS_REF))
		return true;
	if (type == irr_driver->getShader(ES_BUBBLES))
		return true;
	if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
		return true;
    if (type == irr_driver->getShader(ES_CAUSTICS))
        return true;
	if (type == video::EMT_TRANSPARENT_ALPHA_CHANNEL)
		return true;
	if (type == video::EMT_ONETEXTURE_BLEND)
		return true;
	if (type == video::EMT_TRANSPARENT_ADD_COLOR)
		return true;
    if (type == video::EMT_SOLID)
        return true;
    if (type == video::EMT_LIGHTMAP_LIGHTING)
        return true;
    if (type == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return true;
	return false;
}

void initvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat)
{
    switch (GeoMat)
    {
    case FPSM_DEFAULT:
        mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectPass1Shader::attrib_position, MeshShader::ObjectPass1Shader::attrib_texcoord, -1, MeshShader::ObjectPass1Shader::attrib_normal, -1, -1, -1, mesh.Stride);
        mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ShadowShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case FPSM_ALPHA_REF_TEXTURE:
        mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectRefPass1Shader::attrib_position, MeshShader::ObjectRefPass1Shader::attrib_texcoord, -1, MeshShader::ObjectRefPass1Shader::attrib_normal, -1, -1, -1, mesh.Stride);
        mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::RefShadowShader::attrib_position, MeshShader::RefShadowShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case FPSM_NORMAL_MAP:
        mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::NormalMapShader::attrib_position, MeshShader::NormalMapShader::attrib_texcoord, -1, -1, MeshShader::NormalMapShader::attrib_tangent, MeshShader::NormalMapShader::attrib_bitangent, -1, mesh.Stride);
        mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ShadowShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case FPSM_GRASS:
        mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::GrassPass1Shader::attrib_position, MeshShader::GrassPass1Shader::attrib_texcoord, -1, MeshShader::GrassPass1Shader::attrib_normal, -1, -1, MeshShader::GrassPass1Shader::attrib_color, mesh.Stride);
        mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::GrassShadowShader::attrib_position, MeshShader::GrassShadowShader::attrib_texcoord, -1, -1, -1, -1, MeshShader::GrassShadowShader::attrib_color, mesh.Stride);
        break;
    default:
        assert(0 && "Unknow material");
        break;
    }

    switch (ShadedMat)
    {
    case SM_SPHEREMAP:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::SphereMapShader::attrib_position, -1, -1, MeshShader::SphereMapShader::attrib_normal, -1, -1, -1, mesh.Stride);
        break;
    case SM_SPLATTING:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::SplattingShader::attrib_position, MeshShader::SplattingShader::attrib_texcoord, MeshShader::SplattingShader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
        break;
    case SM_ALPHA_REF_TEXTURE:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectRefPass2Shader::attrib_position, MeshShader::ObjectRefPass2Shader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case SM_RIMLIT:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectRimLimitShader::attrib_position, MeshShader::ObjectRimLimitShader::attrib_texcoord, -1, MeshShader::ObjectRimLimitShader::attrib_normal, -1, -1, -1, mesh.Stride);
        break;
    case SM_GRASS:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::GrassPass2Shader::attrib_position, MeshShader::GrassPass2Shader::attrib_texcoord, -1, -1, -1, -1, MeshShader::GrassPass2Shader::attrib_color, mesh.Stride);
        break;
    case SM_UNLIT:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectUnlitShader::attrib_position, MeshShader::ObjectUnlitShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case SM_CAUSTICS:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::CausticsShader::attrib_position, MeshShader::CausticsShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case SM_DETAILS:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::DetailledObjectPass2Shader::attrib_position, MeshShader::DetailledObjectPass2Shader::attrib_texcoord, MeshShader::DetailledObjectPass2Shader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
        break;
    case SM_UNTEXTURED:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::UntexturedObjectShader::attrib_position, -1, -1, -1, -1, -1, MeshShader::UntexturedObjectShader::attrib_color, mesh.Stride);
        break;
    case SM_DEFAULT:
        mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::ObjectPass2Shader::attrib_position, MeshShader::ObjectPass2Shader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    default:
        assert(0 && "unknow shaded material");
        break;
    }

    mesh.vao_glow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ColorizeShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
    mesh.vao_displace_mask_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
    if (mesh.Stride >= 44)
        mesh.vao_displace_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position, MeshShader::DisplaceShader::attrib_texcoord, MeshShader::DisplaceShader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
}

void initvaostate(GLMesh &mesh, TransparentMaterial TranspMat)
{
    switch (TranspMat)
    {
    case TM_BUBBLE:
        mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
            MeshShader::BubbleShader::attrib_position, MeshShader::BubbleShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        break;
    case TM_DEFAULT:
    case TM_ADDITIVE:
        if (World::getWorld()->isFogEnabled())
            mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
                MeshShader::TransparentFogShader::attrib_position, MeshShader::TransparentFogShader::attrib_texcoord, -1, -1, -1, -1, MeshShader::TransparentFogShader::attrib_color, mesh.Stride);
        else
            mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
                MeshShader::TransparentShader::attrib_position, MeshShader::TransparentShader::attrib_texcoord, -1, -1, -1, -1, MeshShader::TransparentShader::attrib_color, mesh.Stride);
        break;
    }
    mesh.vao_glow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ColorizeShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
    mesh.vao_displace_mask_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
    if (mesh.Stride >= 44)
        mesh.vao_displace_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position, MeshShader::DisplaceShader::attrib_texcoord, MeshShader::DisplaceShader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
}

