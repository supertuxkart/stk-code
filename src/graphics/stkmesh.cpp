#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include "glwrap.hpp"

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
	}
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
