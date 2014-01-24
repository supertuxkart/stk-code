#include "graphics/stkanimatedmesh.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include <ISkinnedMesh.h>
#include "graphics/irr_driver.hpp"
#include "config/user_config.hpp"

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
	firstTime = true;
}

void drawObjectRimLimit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
{
	GLenum ptype = mesh.PrimitiveType;
	GLenum itype = mesh.IndexType;
	size_t count = mesh.IndexCount;

	setTexture(0, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
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

	setTexture(1, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_TMP1))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	setTexture(2, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_TMP2))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	setTexture(3, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_SSAO))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	if (!UserConfigParams::m_ssao)
	{
		GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}

	glUseProgram(MeshShader::ObjectRimLimitShader::Program);
	MeshShader::ObjectRimLimitShader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, 0, 1, 2, 3);

	glBindVertexArray(mesh.vao_second_pass);
	glDrawElements(ptype, count, itype, 0);
}

void STKAnimatedMesh::drawSolid(const GLMesh &mesh, video::E_MATERIAL_TYPE type)
{
	switch (irr_driver->getPhase())
	{
	case 0:
	{
			  irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH), false, false);

			  glEnable(GL_DEPTH_TEST);
			  glDisable(GL_ALPHA_TEST);
			  glDepthMask(GL_TRUE);
			  glDisable(GL_BLEND);

			  computeMVP(ModelViewProjectionMatrix);
			  computeTIMV(TransposeInverseModelView);

			  if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
				  drawObjectRefPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
			  else
				  drawObjectPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
			  irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getMainSetup(), false, false);
			  break;
	}
	case 1:
	{
			  irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_COLOR), false, false);

			  glEnable(GL_DEPTH_TEST);
			  glDisable(GL_ALPHA_TEST);
			  glDepthMask(GL_FALSE);
			  glDisable(GL_BLEND);

			  if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
				  drawObjectRefPass2(mesh, ModelViewProjectionMatrix);
			  else if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT))
				  drawObjectRimLimit(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
			  else
				  drawObjectPass2(mesh, ModelViewProjectionMatrix);
			  break;
	}
	default:
	{
			   assert(0 && "wrong pass");
	}
	}
}

static bool
isObjectPass(video::E_MATERIAL_TYPE type)
{
	if (type == irr_driver->getShader(ES_OBJECTPASS))
		return true;
	if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
		return true;
	if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT))
		return true;
	return false;
}

void STKAnimatedMesh::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	bool isTransparentPass =
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	scene::IMesh* m = getMeshForCurrentFrame();

	if (m)
	{
		Box = m->getBoundingBox();
	}
	else
	{
#ifdef _DEBUG
		os::Printer::log("Animated Mesh returned no mesh to render.", Mesh->getDebugName(), ELL_WARNING);
#endif
	}

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	if (firstTime)
	for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
	{
		scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
		GLmeshes.push_back(allocateMeshBuffer(mb));
	}
	firstTime = false;

	// render original meshes
	for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
	{
		video::IMaterialRenderer* rnd = driver->getMaterialRenderer(Materials[i].MaterialType);
		bool transparent = (rnd && rnd->isTransparent());

		// only render transparent buffer if this is the transparent render pass
		// and solid only in solid pass
		if (transparent != isTransparentPass)
			continue;
		scene::IMeshBuffer* mb = m->getMeshBuffer(i);
		const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
		if (RenderFromIdentity)
			driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
		else if (Mesh->getMeshType() == scene::EAMT_SKINNED)
			driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((scene::SSkinMeshBuffer*)mb)->Transformation);
		if (isObjectPass(material.MaterialType))
		{
			initvaostate(GLmeshes[i], material.MaterialType);
			if (irr_driver->getPhase() == 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
				glBufferSubData(GL_ARRAY_BUFFER, 0, mb->getVertexCount() * GLmeshes[i].Stride, mb->getVertices());
			}
			drawSolid(GLmeshes[i], material.MaterialType);
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			video::SMaterial material;
			material.MaterialType = irr_driver->getShader(ES_RAIN);
			material.BlendOperation = video::EBO_NONE;
			material.ZWriteEnable = true;
			material.Lighting = false;
			irr_driver->getVideoDriver()->setMaterial(material);
			static_cast<irr::video::COpenGLDriver*>(irr_driver->getVideoDriver())->setRenderStates3DMode();
		}
		else 
		{
			driver->setMaterial(material);
			driver->drawMeshBuffer(mb);
		}
	}
}
