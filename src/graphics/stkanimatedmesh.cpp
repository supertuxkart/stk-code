#include "graphics/stkanimatedmesh.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include <ISkinnedMesh.h>

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
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


	// render original meshes
	for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
	{
		video::IMaterialRenderer* rnd = driver->getMaterialRenderer(Materials[i].MaterialType);
		bool transparent = (rnd && rnd->isTransparent());

		// only render transparent buffer if this is the transparent render pass
		// and solid only in solid pass
		if (transparent == isTransparentPass)
		{
			scene::IMeshBuffer* mb = m->getMeshBuffer(i);
			const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
			if (RenderFromIdentity)
				driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
			else if (Mesh->getMeshType() == scene::EAMT_SKINNED)
				driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((scene::SSkinMeshBuffer*)mb)->Transformation);
			printf("Material type is %d\n", material.MaterialType);
			driver->setMaterial(material);
			driver->drawMeshBuffer(mb);
		}
	}
}
