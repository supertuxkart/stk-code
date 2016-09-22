// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES2_RENDERER_2D_H_INCLUDED__
#define __C_OGLES2_RENDERER_2D_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2MaterialRenderer.h"

namespace irr
{
namespace video
{

//! Class for renderer 2D in OpenGL ES 2.0
class COGLES2Renderer2D : public COGLES2MaterialRenderer
{
public:
	//! Constructor
	COGLES2Renderer2D(const c8* vertexShaderProgram, const c8* pixelShaderProgram, COGLES2Driver* driver);

	//! Destructor
	~COGLES2Renderer2D();

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services);

	virtual bool OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype);

	void setTexture(const ITexture* texture);

protected:

	core::dimension2d<u32> RenderTargetSize;
	core::matrix4 Matrix;

	const ITexture* Texture;

	s32 MatrixID;
	s32 UseTextureID;
};


} // end namespace video
} // end namespace irr

#endif
#endif

