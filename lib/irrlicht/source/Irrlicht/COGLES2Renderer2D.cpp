// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2Renderer2D.h"
#include "IGPUProgrammingServices.h"
#include "os.h"
#include "COGLES2Driver.h"

namespace irr
{
namespace video
{

//! Constructor
COGLES2Renderer2D::COGLES2Renderer2D(const c8* vertexShaderProgram, const c8* pixelShaderProgram, COGLES2Driver* driver)
	:	COGLES2MaterialRenderer(driver, 0, EMT_SOLID), RenderTargetSize(core::dimension2d<u32>(0,0)),
		Matrix(core::matrix4::EM4CONST_NOTHING), Texture(0)
{
	#ifdef _DEBUG
	setDebugName("COGLES2Renderer2D");
	#endif

	int Temp = 0;

	init(Temp, vertexShaderProgram, pixelShaderProgram, false);

	Driver->getBridgeCalls()->setProgram(Program);

	// These states doesn't change later.

	MatrixID = getPixelShaderConstantID("uOrthoMatrix");
	UseTextureID = getPixelShaderConstantID("uUseTexture");
	s32 TextureUnitID = getPixelShaderConstantID("uTextureUnit");	

	int TextureUnit = 0;
	setPixelShaderConstant(TextureUnitID, &TextureUnit, 1);

	Driver->getBridgeCalls()->setProgram(0);
}


//! Destructor
COGLES2Renderer2D::~COGLES2Renderer2D()
{
}


void COGLES2Renderer2D::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
	Driver->getBridgeCalls()->setProgram(Program);

	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}


bool COGLES2Renderer2D::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
{
	Driver->setTextureRenderStates(Driver->getCurrentMaterial(), false);

	const core::dimension2d<u32>& renderTargetSize = Driver->getCurrentRenderTargetSize();

	if (RenderTargetSize != renderTargetSize)
	{
		Matrix.buildProjectionMatrixOrthoLH(f32(renderTargetSize.Width), f32(-(s32)(renderTargetSize.Height)), -1.0f, 1.0f);
		Matrix.setTranslation(core::vector3df(-1,1,0));

		setPixelShaderConstant(MatrixID, Matrix.pointer(), 16);

		RenderTargetSize = renderTargetSize;
	}

	int UseTexture = Texture ? 1 : 0;
	setPixelShaderConstant(UseTextureID, &UseTexture, 1);

	return true;
}


void COGLES2Renderer2D::setTexture(const ITexture* texture)
{
	Texture = texture;
}


} // end namespace video
} // end namespace irr


#endif
