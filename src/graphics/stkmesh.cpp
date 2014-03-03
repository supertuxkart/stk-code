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


GLuint createVAO(GLuint vbo, GLuint idx, GLuint attrib_position, GLuint attrib_texcoord, GLuint attrib_second_texcoord, GLuint attrib_normal, GLuint attrib_tangent, GLuint attrib_bitangent, GLuint attrib_color, size_t stride)
{
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
	glBindVertexArray(0);
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
	{
		tex = mb->getMaterial().getTexture(i);
		if (tex)
			result.textures[i] = getTextureGLuint(tex);
		else
			result.textures[i] = 0;
	}
	return result;
}


void computeMVP(core::matrix4 &ModelViewProjectionMatrix)
{
	ModelViewProjectionMatrix = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION);
	ModelViewProjectionMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
	ModelViewProjectionMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_WORLD);
}

void computeTIMV(core::matrix4 &TransposeInverseModelView)
{
	TransposeInverseModelView = irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
	TransposeInverseModelView *= irr_driver->getVideoDriver()->getTransform(video::ETS_WORLD);
	TransposeInverseModelView.makeInverse();
	TransposeInverseModelView = TransposeInverseModelView.getTransposed();
}

void drawObjectPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  glUseProgram(MeshShader::ObjectPass1Shader::Program);
  MeshShader::ObjectPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView);

  glBindVertexArray(mesh.vao_first_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectRefPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;


  setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

  glUseProgram(MeshShader::ObjectRefPass1Shader::Program);
  MeshShader::ObjectRefPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, TextureMatrix, 0);

  glBindVertexArray(mesh.vao_first_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	glUseProgram(MeshShader::GrassPass1Shader::Program);
	MeshShader::GrassPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, windDir, 0);

	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawNormalPass(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	assert(mesh.textures[1]);
	setTexture(0, mesh.textures[1], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	glUseProgram(MeshShader::NormalMapShader::Program);
	MeshShader::NormalMapShader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, 0);

	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawSphereMap(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

  glUseProgram(MeshShader::SphereMapShader::Program);
  MeshShader::SphereMapShader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, 0);

  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawSplatting(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  // Texlayout
  setTexture(0, mesh.textures[1], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  setTexture(1, mesh.textures[2], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  setTexture(2, mesh.textures[3], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  //Tex detail2
  setTexture(3, mesh.textures[4], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  setTexture(4, mesh.textures[5], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  // Diffuse
  setTexture(5, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);

  // Specular
  setTexture(6, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);

  // SSAO
  setTexture(7, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
  if (!UserConfigParams::m_ssao)
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  glUseProgram(MeshShader::SplattingShader::Program);
  MeshShader::SplattingShader::setUniforms(ModelViewProjectionMatrix, 0, 1, 2, 3, 4, 5, 6, 7);

  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectRefPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
  setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
  setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
  setTexture(3, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
  if (!UserConfigParams::m_ssao)
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  glUseProgram(MeshShader::ObjectRefPass2Shader::Program);
  MeshShader::ObjectRefPass2Shader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, 0, 1, 2, 3);

  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
    setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
    setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
    setTexture(3, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
	if (!UserConfigParams::m_ssao)
	{
	  GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
	  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	glUseProgram(MeshShader::GrassPass2Shader::Program);
	MeshShader::GrassPass2Shader::setUniforms(ModelViewProjectionMatrix, windDir, 0, 1, 2, 3);

	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawUntexturedObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  setTexture(0, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
  setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
  setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
  if (!UserConfigParams::m_ssao)
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }
  

  glUseProgram(MeshShader::UntexturedObjectShader::Program);
  MeshShader::UntexturedObjectShader::setUniforms(ModelViewProjectionMatrix, 0, 1, 2);

  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectRimLimit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

    setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
    setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
    setTexture(3, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
	if (!UserConfigParams::m_ssao)
	{
		GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	glUseProgram(MeshShader::ObjectRimLimitShader::Program);
	MeshShader::ObjectRimLimitShader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, TextureMatrix, 0, 1, 2, 3);

	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawObjectUnlit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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


	glUseProgram(MeshShader::ObjectUnlitShader::Program);
	MeshShader::ObjectUnlitShader::setUniforms(ModelViewProjectionMatrix, 0);

	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawDetailledObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
  GLenum ptype = mesh.PrimitiveType;
  GLenum itype = mesh.IndexType;
  size_t count = mesh.IndexCount;

  setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

  setTexture(1, mesh.textures[1], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

  setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
  setTexture(3, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
  setTexture(4, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
  if (!UserConfigParams::m_ssao)
  {
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
  }

  glUseProgram(MeshShader::DetailledObjectPass2Shader::Program);
  MeshShader::DetailledObjectPass2Shader::setUniforms(ModelViewProjectionMatrix, 0, 1, 2, 3, 4);

  glBindVertexArray(mesh.vao_second_pass);
  glDrawElements(ptype, count, itype, 0);
}

void drawObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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

    setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
    setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
    setTexture(3, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
	if (!UserConfigParams::m_ssao)
	{
		GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	glUseProgram(MeshShader::ObjectPass2Shader::Program);
	MeshShader::ObjectPass2Shader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, 0, 1, 2, 3);

	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	glUseProgram(MeshShader::TransparentShader::Program);
    MeshShader::TransparentShader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, 0);

	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
}

void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix)
{
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

    setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    glUseProgram(MeshShader::TransparentFogShader::Program);
    MeshShader::TransparentFogShader::setUniforms(ModelViewProjectionMatrix, TextureMatrix, irr_driver->getInvProjMatrix(), fogmax, startH, endH, start, end, col, Camera::getCamera(0)->getCameraSceneNode()->getAbsolutePosition(), 0);

    glBindVertexArray(mesh.vao_first_pass);
    glDrawElements(ptype, count, itype, 0);
}

void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
	const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
	float transparency = 1.;

	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

	glUseProgram(MeshShader::BubbleShader::Program);
	MeshShader::BubbleShader::setUniforms(ModelViewProjectionMatrix, 0, time, transparency);

	glBindVertexArray(mesh.vao_first_pass);
	glDrawElements(ptype, count, itype, 0);
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
	if (type == video::EMT_TRANSPARENT_ALPHA_CHANNEL)
		return true;
	if (type == video::EMT_ONETEXTURE_BLEND)
		return true;
	if (type == video::EMT_TRANSPARENT_ADD_COLOR)
		return true;
	return false;
}

void initvaostate(GLMesh &mesh, video::E_MATERIAL_TYPE type)
{
	switch (irr_driver->getPhase())
	{
	case SOLID_NORMAL_AND_DEPTH_PASS:
		if (mesh.vao_first_pass)
			return;
		if (type == irr_driver->getShader(ES_NORMAL_MAP))
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::NormalMapShader::attrib_position, MeshShader::NormalMapShader::attrib_texcoord, -1, -1, MeshShader::NormalMapShader::attrib_tangent, MeshShader::NormalMapShader::attrib_bitangent, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectPass1Shader::attrib_position, MeshShader::ObjectRefPass1Shader::attrib_texcoord, -1, MeshShader::ObjectPass1Shader::attrib_normal, -1, -1, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_GRASS) || type == irr_driver->getShader(ES_GRASS_REF))
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::GrassPass1Shader::attrib_position, MeshShader::GrassPass1Shader::attrib_texcoord, -1, MeshShader::GrassPass1Shader::attrib_normal, -1, -1, MeshShader::GrassPass1Shader::attrib_color, mesh.Stride);
		}
		else
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectPass1Shader::attrib_position, -1, -1, MeshShader::ObjectPass1Shader::attrib_normal, -1, -1, -1, mesh.Stride);
		}
		return;
	case SOLID_LIT_PASS:
		if (mesh.vao_second_pass)
			return;
		if (type == irr_driver->getShader(ES_SPHERE_MAP))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::SphereMapShader::attrib_position, -1, -1, MeshShader::SphereMapShader::attrib_normal, -1, -1, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_SPLATTING))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::SplattingShader::attrib_position, MeshShader::SplattingShader::attrib_texcoord, MeshShader::SplattingShader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectRefPass2Shader::attrib_position, MeshShader::ObjectRefPass2Shader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectRimLimitShader::attrib_position, MeshShader::ObjectRimLimitShader::attrib_texcoord, -1, MeshShader::ObjectRimLimitShader::attrib_normal, -1, -1, -1, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_GRASS) || type == irr_driver->getShader(ES_GRASS_REF))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::GrassPass2Shader::attrib_position, MeshShader::GrassPass2Shader::attrib_texcoord, -1, -1, -1, -1, MeshShader::GrassPass2Shader::attrib_color, mesh.Stride);
		}
		else if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectUnlitShader::attrib_position, MeshShader::ObjectUnlitShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
		}
		else if (mesh.textures[1])
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::DetailledObjectPass2Shader::attrib_position, MeshShader::DetailledObjectPass2Shader::attrib_texcoord, MeshShader::DetailledObjectPass2Shader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
		}
		else if (!mesh.textures[0])
		{
		  mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
			  MeshShader::UntexturedObjectShader::attrib_position, -1, -1, -1, -1, -1, MeshShader::UntexturedObjectShader::attrib_color, mesh.Stride);
		}
		else
		{
			mesh.vao_second_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::ObjectPass2Shader::attrib_position, MeshShader::ObjectPass2Shader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
		}
		return;
	case GLOW_PASS:
		if (mesh.vao_glow_pass)
			return;
		mesh.vao_glow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ColorizeShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
		return;
	case TRANSPARENT_PASS:
		if (mesh.vao_first_pass)
			return;
		if (type == irr_driver->getShader(ES_BUBBLES))
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::BubbleShader::attrib_position, MeshShader::BubbleShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
		}
        else if (World::getWorld()->getTrack()->isFogEnabled())
        {
            mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
                MeshShader::TransparentFogShader::attrib_position, MeshShader::TransparentFogShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        }
		else
		{
			mesh.vao_first_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer,
				MeshShader::TransparentShader::attrib_position, MeshShader::TransparentShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
		}
		return;
	case DISPLACEMENT_PASS:
		if (mesh.vao_displace_pass)
			return;
        mesh.vao_displace_mask_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position,-1, -1, -1, -1, -1, -1, mesh.Stride);
		mesh.vao_displace_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::DisplaceShader::attrib_position, MeshShader::DisplaceShader::attrib_texcoord, MeshShader::DisplaceShader::attrib_second_texcoord, -1, -1, -1, -1, mesh.Stride);
		return;
    case SHADOW_PASS:
        if (mesh.vao_shadow_pass)
            return;
        if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
        {
            mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::RefShadowShader::attrib_position, MeshShader::RefShadowShader::attrib_texcoord, -1, -1, -1, -1, -1, mesh.Stride);
        }
        /*else if (type == irr_driver->getShader(ES_GRASS) || type == irr_driver->getShader(ES_GRASS_REF))
        {
            mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::GrassShadowShader::attrib_position, MeshShader::GrassShadowShader::attrib_texcoord, -1, -1, -1, -1, MeshShader::GrassShadowShader::attrib_color, mesh.Stride);
        }*/
        else
        {
            mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, MeshShader::ShadowShader::attrib_position, -1, -1, -1, -1, -1, -1, mesh.Stride);
        }
        return;
	}
}

