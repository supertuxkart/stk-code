#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>

static
void allocateMeshBuffer(scene::IMeshBuffer* mb, GLuint &vbo, GLuint idx)
{
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
	}
}

STKMesh::~STKMesh()
{
	glDeleteBuffers(vertex_buffer.size(), vertex_buffer.data());
	glDeleteBuffers(index_buffer.size(), index_buffer.data());
}

void STKMesh::draw(unsigned i)
{
	GLuint vbo = vertex_buffer[i], idx = index_buffer[i];
	GLenum ptype = Primitivetype[i];
	size_t count = Indexcount[i];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);

/*	glNormalPointer(GL_FLOAT, sizeof(S3DVertex2TCoords), buffer_offset(12));
	glColorPointer(colorSize, GL_UNSIGNED_BYTE, sizeof(S3DVertex2TCoords), buffer_offset(24));
	glTexCoordPointer(2, GL_FLOAT, sizeof(S3DVertex2TCoords), buffer_offset(28));
	glVertexPointer(3, GL_FLOAT, sizeof(S3DVertex2TCoords), buffer_offset(0));*/
	glDrawElements(ptype, count, GL_UNSIGNED_BYTE, 0);
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
