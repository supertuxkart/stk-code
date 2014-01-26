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

void STKAnimatedMesh::setMesh(scene::IAnimatedMesh* mesh)
{
	firstTime = true;
	GLmeshes.clear();
	CAnimatedMeshSceneNode::setMesh(mesh);
}

void STKAnimatedMesh::drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	computeMVP(ModelViewProjectionMatrix);

	drawTransparentObject(mesh, ModelViewProjectionMatrix);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return;
}

void STKAnimatedMesh::drawSolid(const GLMesh &mesh, video::E_MATERIAL_TYPE type)
{
	switch (irr_driver->getPhase())
	{
	case SOLID_NORMAL_AND_DEPTH_PASS:
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
	case SOLID_LIT_PASS:
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
			  else if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
				  drawObjectUnlit(mesh, ModelViewProjectionMatrix);
			  else if (mesh.textures[1])
				  drawDetailledObjectPass2(mesh, ModelViewProjectionMatrix);
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
	if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
		return true;
	if (type == video::EMT_ONETEXTURE_BLEND)
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
			if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
			{
				glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
				glBufferSubData(GL_ARRAY_BUFFER, 0, mb->getVertexCount() * GLmeshes[i].Stride, mb->getVertices());
			}
			if (isTransparentPass)
				drawTransparent(GLmeshes[i], material.MaterialType);
			else
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
