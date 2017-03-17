// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2FixedPipelineRenderer.h"
#include "IGPUProgrammingServices.h"
#include "os.h"
#include "COGLES2Driver.h"

namespace irr
{
namespace video
{

//! Constructor
COGLES2FixedPipelineRenderer::COGLES2FixedPipelineRenderer(const c8* vertexShaderProgram,
							const c8* pixelShaderProgram, E_MATERIAL_TYPE baseMaterial,
							COGLES2Driver* driver)
	: COGLES2MaterialRenderer(driver, 0, baseMaterial)
{
	#ifdef _DEBUG
	setDebugName("COGLES2FixedPipelineRenderer");
	#endif
	yy = (int)baseMaterial;
	int Temp = 0;

	SharedRenderer = reinterpret_cast<COGLES2MaterialRenderer*>(driver->getMaterialRenderer(EMT_SOLID));

	if (SharedRenderer)
		SharedRenderer->grab();
	else
		init(Temp, vertexShaderProgram, pixelShaderProgram, false);
}


//! Destructor
COGLES2FixedPipelineRenderer::~COGLES2FixedPipelineRenderer()
{
	if(SharedRenderer)
		SharedRenderer->drop();
}


void COGLES2FixedPipelineRenderer::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
	if (SharedRenderer)
		Driver->getBridgeCalls()->setProgram(SharedRenderer->getProgram());
	else
		Driver->getBridgeCalls()->setProgram(Program);

	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

	if (FixedBlending)
	{
		Driver->getBridgeCalls()->setBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		Driver->getBridgeCalls()->setBlend(true);
	}
	else if (Blending)
	{
		E_BLEND_FACTOR srcFact,dstFact;
		E_MODULATE_FUNC modulate;
		u32 alphaSource;
		unpack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam);

		Driver->getBridgeCalls()->setBlendFunc(Driver->getGLBlend(srcFact), Driver->getGLBlend(dstFact));
		Driver->getBridgeCalls()->setBlend(true);
	}
	else
		Driver->getBridgeCalls()->setBlend(false);
}


bool COGLES2FixedPipelineRenderer::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
{
	if (SharedRenderer)
		return SharedRenderer->OnRender(service, vtxtype);
	else
	{
		Driver->setTextureRenderStates(Driver->getCurrentMaterial(), false);

		s32 materialType = 0;
		bool second_texture = false;

		switch(Driver->getCurrentMaterial().MaterialType)
		{
		case EMT_SOLID_2_LAYER:
			materialType = 1;
			break;
		case EMT_LIGHTMAP:
		case EMT_LIGHTMAP_ADD:
		case EMT_LIGHTMAP_M2:
		case EMT_LIGHTMAP_M4:
			materialType = 2;
			break;
		case EMT_LIGHTMAP_LIGHTING:
		case EMT_LIGHTMAP_LIGHTING_M2:
		case EMT_LIGHTMAP_LIGHTING_M4:
			materialType = 2;
			second_texture = true;
			break;
		case EMT_DETAIL_MAP:
			materialType = 3;
			break;
		case EMT_SPHERE_MAP:
			materialType = 4;
			break;
		case EMT_REFLECTION_2_LAYER:
			materialType = 5;
			break;
		case EMT_TRANSPARENT_ALPHA_CHANNEL:
			materialType = 6;
			break;
		case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
			materialType = 7;
			break;
		case EMT_TRANSPARENT_VERTEX_ALPHA:
			materialType = 8;
			break;
		case EMT_TRANSPARENT_REFLECTION_2_LAYER:
			materialType = 9;
			break;
		default:
			break;
		}

		IMaterialRendererServices::setPixelShaderConstant("uMaterialType", &materialType, 1);

		/* Transform Matrices Upload */

		core::matrix4 world = Driver->getTransform(ETS_WORLD);
		IMaterialRendererServices::setPixelShaderConstant("uWorldMatrix", world.pointer(), 16);

		core::matrix4 worldViewProj = Driver->getTransform(video::ETS_PROJECTION);
		worldViewProj *= Driver->getTransform(video::ETS_VIEW);
		worldViewProj *= Driver->getTransform(ETS_WORLD);
		IMaterialRendererServices::setPixelShaderConstant("uMvpMatrix", worldViewProj.pointer(), 16);

		/* Textures Upload */

		s32 TextureUsage0 = Driver->isActiveTexture(0);
		//s32 TextureUsage1 = Driver->isActiveTexture(1);

		IMaterialRendererServices::setPixelShaderConstant("uTextureUsage0", &TextureUsage0, 1);
		//IMaterialRendererServices::setPixelShaderConstant("uTextureUsage1", &TextureUsage1, 1);

		core::matrix4 textureMatrix0 = Driver->getTransform(video::ETS_TEXTURE_0);
		//core::matrix4 textureMatrix1 = Driver->getTransform(video::ETS_TEXTURE_0);

		IMaterialRendererServices::setPixelShaderConstant("uTextureMatrix0", textureMatrix0.pointer(), 16);
		//IMaterialRendererServices::setPixelShaderConstant("uTextureMatrix1", textureMatrix1.pointer(), 16);

		s32 TextureUnit0 = 0;
		//s32 TextureUnit1 = 1;
		
		if (second_texture)
			TextureUnit0 = 1;

		IMaterialRendererServices::setPixelShaderConstant("uTextureUnit0", &TextureUnit0, 1);
		//IMaterialRendererServices::setPixelShaderConstant("uTextureUnit1", &TextureUnit1, 1);

		return true;
	}
}


} // end namespace video
} // end namespace irr


#endif
