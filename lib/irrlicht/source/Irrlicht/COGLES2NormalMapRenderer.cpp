// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2NormalMapRenderer.h"
#include "IGPUProgrammingServices.h"
#include "os.h"
#include "COGLES2Driver.h"

namespace irr
{
namespace video
{

//! Constructor
COGLES2NormalMapRenderer::COGLES2NormalMapRenderer(const c8* vertexShaderProgram,
							const c8* pixelShaderProgram, E_MATERIAL_TYPE baseMaterial,
							COGLES2Driver* driver)
	: COGLES2MaterialRenderer(driver, 0, baseMaterial)
{
	#ifdef _DEBUG
	setDebugName("COGLES2NormalMapRenderer");
	#endif

	int Temp = 0;

	SharedRenderer = reinterpret_cast<COGLES2MaterialRenderer*>(driver->getMaterialRenderer(EMT_NORMAL_MAP_SOLID));

	if (SharedRenderer)
		SharedRenderer->grab();
	else
		init(Temp, vertexShaderProgram, pixelShaderProgram, false);
}


//! Destructor
COGLES2NormalMapRenderer::~COGLES2NormalMapRenderer()
{
	if(SharedRenderer)
		SharedRenderer->drop();
}


void COGLES2NormalMapRenderer::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
	if (SharedRenderer)
		SharedRenderer->OnSetMaterial(material, lastMaterial, resetAllRenderstates, services);
	else
		COGLES2MaterialRenderer::OnSetMaterial(material, lastMaterial, resetAllRenderstates, services);
}


bool COGLES2NormalMapRenderer::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
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
