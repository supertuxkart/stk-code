#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>

namespace ObjectShader
{
	GLuint Program;
	GLuint attrib_position, attrib_texcoord;
	GLuint uniform_MVP, uniform_texture;

	void init()
	{
		initGL();
		Program = LoadProgram(file_manager->getAsset("shaders/object.vert").c_str(), file_manager->getAsset("shaders/object.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_texture = glGetUniformLocation(Program, "texture");
	}
}

static
void allocateMeshBuffer(scene::IMeshBuffer* mb, GLuint &vbo, GLuint &idx)
{
	initGL();
	GLuint bufferid, indexbufferid;
	glGenBuffers(1, &bufferid);
	glGenBuffers(1, &indexbufferid);

	glBindBuffer(GL_ARRAY_BUFFER, bufferid);
	const void* vertices=mb->getVertices();
	const u32 vertexCount=mb->getVertexCount();
	const irr::video::E_VERTEX_TYPE vType=mb->getVertexType();
	const u32 vertexSize = getVertexPitchFromType(vType);
	const c8* vbuf = static_cast<const c8*>(vertices);
	core::array<c8> buffer;
	buffer.set_used(vertexSize * vertexCount);
	memcpy(buffer.pointer(), vertices, vertexSize * vertexCount);
	vbuf = buffer.const_pointer();
	glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vbuf, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferid);
	const void* indices=mb->getIndices();
	u32 indexCount= mb->getIndexCount();
	GLenum indexSize;
	switch (mb->getIndexType())
	{
		case irr::video::EIT_16BIT:
		{
			indexSize=sizeof(u16);
			break;
		}
		case irr::video::EIT_32BIT:
		{
			indexSize=sizeof(u32);
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
	vbo = bufferid;
	idx = indexbufferid;
}

STKMesh::STKMesh(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,	irr::s32 id,
	const irr::core::vector3df& position,
	const irr::core::vector3df& rotation,
	const irr::core::vector3df& scale) :
		CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
	for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		if (!mb)
			continue;
		GLuint vbo, idx;
		allocateMeshBuffer(mb, vbo, idx);
		vertex_buffer.push_back(vbo);
		index_buffer.push_back(idx);
		Indexcount.push_back(mb->getIndexCount());
		switch (mb->getPrimitiveType())
		{
		case scene::EPT_POINTS:
			Primitivetype.push_back(GL_POINTS);
			break;
		case scene::EPT_TRIANGLE_STRIP:
			Primitivetype.push_back(GL_TRIANGLE_STRIP);
			break;
		case scene::EPT_TRIANGLE_FAN:
			Primitivetype.push_back(GL_TRIANGLE_FAN);
			break;
		case scene::EPT_LINES:
			Primitivetype.push_back(GL_LINES);
		case scene::EPT_TRIANGLES:
			Primitivetype.push_back(GL_TRIANGLES);
			break;
		case scene::EPT_POINT_SPRITES:
		case scene::EPT_LINE_LOOP:
		case scene::EPT_POLYGON:
		case scene::EPT_LINE_STRIP:
		case scene::EPT_QUAD_STRIP:
		case scene::EPT_QUADS:
			assert(0 && "Unsupported primitive type");
		}
		switch (mb->getVertexType())
		{
		case video::EVT_STANDARD:
			Stride.push_back(sizeof(video::S3DVertex));
			break;
		case video::EVT_2TCOORDS:
			Stride.push_back(sizeof(video::S3DVertex2TCoords));
			break;
		case video::EVT_TANGENTS:
			Stride.push_back(sizeof(video::S3DVertexTangents));
			break;
		}
		switch (mb->getIndexType())
		{
		case video::EIT_16BIT:
			Indextype.push_back(GL_UNSIGNED_SHORT);
			break;
		case video::EIT_32BIT:
			Indextype.push_back(GL_UNSIGNED_INT);
			break;
		}
		ITexture *tex = mb->getMaterial().getTexture(0);
		if (tex)
			textures.push_back(static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName());
		else
			textures.push_back(0);
	}
	if (!ObjectShader::Program)
		ObjectShader::init();
}

STKMesh::~STKMesh()
{
	glDeleteBuffers(vertex_buffer.size(), vertex_buffer.data());
	glDeleteBuffers(index_buffer.size(), index_buffer.data());
}

void STKMesh::draw(unsigned i)
{
	if (!textures[i])
	  return;
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	GLuint vbo = vertex_buffer[i], idx = index_buffer[i];
	GLenum ptype = Primitivetype[i];
	size_t count = Indexcount[i];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);

	glUseProgram(ObjectShader::Program);
	core::matrix4 ModelViewProjectionMatrix = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION);
	ModelViewProjectionMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
	ModelViewProjectionMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_WORLD);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[i]);
	glUniform1f(ObjectShader::uniform_texture, 0);
	glUniformMatrix4fv(ObjectShader::uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
	glEnableVertexAttribArray(ObjectShader::attrib_position);
	glEnableVertexAttribArray(ObjectShader::attrib_texcoord);
	glVertexAttribPointer(ObjectShader::attrib_position, 3, GL_FLOAT, GL_FALSE, Stride[i], 0);
	glVertexAttribPointer(ObjectShader::attrib_texcoord, 2, GL_FLOAT, GL_FALSE, Stride[i], (GLvoid*) 28);
	glDrawElements(ptype, count, Indextype[i], 0);
	glDisableVertexAttribArray(ObjectShader::attrib_position);
	glDisableVertexAttribArray(ObjectShader::attrib_texcoord);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	video::SMaterial material;
	material.MaterialType = irr_driver->getShader(ES_RAIN);
//	fakemat.setTexture(0, tex);
//	fakemat.BlendOperation = video::EBO_NONE;
	irr_driver->getVideoDriver()->setMaterial(material);
	static_cast<irr::video::COpenGLDriver*>(irr_driver->getVideoDriver())->setRenderStates3DMode();
}

void STKMesh::render()
{
	irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();

	if (!Mesh || !driver)
		return;

	bool isTransparentPass =
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	Box = Mesh->getBoundingBox();

	for (unsigned i = 0; i < index_buffer.size(); i++)
	{
//		draw(i);
	}

	for (u32 i=0; i<Mesh->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		if (mb)
		{
			const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];

			video::IMaterialRenderer* rnd = driver->getMaterialRenderer(material.MaterialType);
			bool transparent = (rnd && rnd->isTransparent());

			// only render transparent buffer if this is the transparent render pass
			// and solid only in solid pass
			if (transparent == isTransparentPass)
			{
				driver->setMaterial(material);
				driver->drawMeshBuffer(mb);
			}
		}
	}
}
