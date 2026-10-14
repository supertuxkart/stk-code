// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES_

#include "COGLESNormalMapRenderer.h"
#include "IGPUProgrammingServices.h"
#include "os.h"
#include "COGLESDriver.h"

namespace irr
{
namespace video
{

//! Constructor
COGLESNormalMapRenderer::COGLESNormalMapRenderer(const c8* vertexShaderProgram,
							const c8* pixelShaderProgram, E_MATERIAL_TYPE baseMaterial,
							COGLESDriver* driver)
	: COGLESMaterialRenderer(driver, 0, baseMaterial)
{
	#ifdef _DEBUG
	setDebugName("COGLESNormalMapRenderer");
	#endif

	int Temp = 0;

	SharedRenderer = reinterpret_cast<COGLESMaterialRenderer*>(driver->getMaterialRenderer(EMT_NORMAL_MAP_SOLID));

	if (SharedRenderer)
		SharedRenderer->grab();
	else
		init(Temp, vertexShaderProgram, pixelShaderProgram, false);
}


//! Destructor
COGLESNormalMapRenderer::~COGLESNormalMapRenderer()
{
	if(SharedRenderer)
		SharedRenderer->drop();
}


void COGLESNormalMapRenderer::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
	if (SharedRenderer)
		SharedRenderer->OnSetMaterial(material, lastMaterial, resetAllRenderstates, services);
	else
		COGLESMaterialRenderer::OnSetMaterial(material, lastMaterial, resetAllRenderstates, services);
}


bool COGLESNormalMapRenderer::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
{
	if (SharedRenderer)
		return SharedRenderer->OnRender(service, vtxtype);
	else
	{
		/* Vertex Shader part */

		return true;
	}
}


} // end namespace video
} // end namespace irr


#endif
