#ifndef STKMESH_H
#define STKMESH_H


#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"
#include "glwrap.hpp"
#include <vector>

struct GLMesh {
	GLuint vao_first_pass;
	GLuint vao_second_pass;
	GLuint vao_glow_pass;
	GLuint vao_displace_pass;
    GLuint vao_shadow_pass;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLuint textures[6];
	GLenum PrimitiveType;
	GLenum IndexType;
	size_t IndexCount;
	size_t Stride;
};

GLuint createVAO(GLuint vbo, GLuint idx, GLuint attrib_position, GLuint attrib_texcoord, GLuint attrib_second_texcoord, GLuint attrib_normal, GLuint attrib_tangent, GLuint attrib_bitangent, GLuint attrib_color, size_t stride);
GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb);
void initvaostate(GLMesh &mesh, video::E_MATERIAL_TYPE type);
void computeMVP(core::matrix4 &ModelViewProjectionMatrix);
void computeTIMV(core::matrix4 &TransposeInverseModelView);

// Pass 1 shader (ie shaders that outputs normals and depth)
void drawObjectPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawNormalPass(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawObjectRefPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir);

// Pass 2 shader (ie shaders that outputs final color)
void drawDetailledObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawUntexturedObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawObjectRefPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawSphereMap(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawSplatting(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir);
void drawObjectRimLimit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawObjectUnlit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

// Forward pass (for transparents meshes)
void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

class STKMesh : public irr::scene::CMeshSceneNode
{
protected:
	std::vector<GLMesh> GLmeshes;
	core::matrix4 ModelViewProjectionMatrix, TransposeInverseModelView;
	core::vector3df windDir;
	void drawSolid(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
	void drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type);

	// Misc passes shaders (glow, displace...)
	void drawGlow(const GLMesh &mesh);
	void drawDisplace(const GLMesh &mesh);
    void drawShadow(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
	void createGLMeshes();
	void cleanGLMeshes();
public:
	STKMesh(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,	irr::s32 id,
		const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
	virtual void render();
	virtual void setMesh(irr::scene::IMesh* mesh);
    void setReadOnlyMaterials(bool readonly);
	~STKMesh();
};

#endif // STKMESH_H
