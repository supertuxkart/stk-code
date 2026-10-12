// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES2_PARALLAX_MAP_RENDERER_H_INCLUDED__
#define __C_OGLES2_PARALLAX_MAP_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES_

#include "COGLESMaterialRenderer.h"

namespace irr
{
namespace video
{

//! Class for parallax mapping in OpenGL ES 2.0
class COGLESParallaxMapRenderer : public COGLESMaterialRenderer
{
public:
	//! Constructor
	COGLESParallaxMapRenderer(const c8* vertexShaderProgram,
		const c8* pixelShaderProgram, E_MATERIAL_TYPE baseMaterial,
		COGLESDriver* driver);

	//! Destructor
	~COGLESParallaxMapRenderer();

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services);

	virtual bool OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype);

protected:

	COGLESMaterialRenderer* SharedRenderer;
};


} // end namespace video
} // end namespace irr

#endif
#endif


