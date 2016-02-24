// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OGLES1_MATERIAL_RENDERER_H_INCLUDED__
#define __C_OGLES1_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OGLES1_

#include "COGLESDriver.h"
#include "IMaterialRenderer.h"

namespace irr
{
namespace video
{

//! Base class for all internal OGLES1 material renderers
class COGLES1MaterialRenderer : public IMaterialRenderer
{
public:

	//! Constructor
	COGLES1MaterialRenderer(video::COGLES1Driver* driver) : Driver(driver)
	{
	}

protected:

	video::COGLES1Driver* Driver;
};


//! Solid material renderer
class COGLES1MaterialRenderer_SOLID : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_SOLID(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (resetAllRenderstates || (material.MaterialType != lastMaterial.MaterialType))
		{
			// thanks to Murphy, the following line removed some
			// bugs with several OGLES1 implementations.
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
	}
};


//! Generic Texture Blend
class COGLES1MaterialRenderer_ONETEXTURE_BLEND : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_ONETEXTURE_BLEND(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

//		if (material.MaterialType != lastMaterial.MaterialType ||
//			material.MaterialTypeParam != lastMaterial.MaterialTypeParam ||
//			resetAllRenderstates)
		{
			E_BLEND_FACTOR srcFact,dstFact;
			E_MODULATE_FUNC modulate;
			u32 alphaSource;
			unpack_textureBlendFunc ( srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam );

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);

			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, (f32) modulate );

			glBlendFunc( getGLBlend(srcFact), getGLBlend(dstFact) );
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.f);
			glEnable(GL_BLEND);

			if ( textureBlendFunc_hasAlpha(srcFact) || textureBlendFunc_hasAlpha(dstFact) )
			{
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);

				glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			}
		}
	}

	virtual void OnUnsetMaterial()
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.f );
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);

		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}

	//! Returns if the material is transparent.
	/** Is not always transparent, but mostly. */
	virtual bool isTransparent() const
	{
		return true;
	}

	private:

		u32 getGLBlend ( E_BLEND_FACTOR factor ) const
		{
			u32 r = 0;
			switch ( factor )
			{
				case EBF_ZERO:			r = GL_ZERO; break;
				case EBF_ONE:			r = GL_ONE; break;
				case EBF_DST_COLOR:		r = GL_DST_COLOR; break;
				case EBF_ONE_MINUS_DST_COLOR:	r = GL_ONE_MINUS_DST_COLOR; break;
				case EBF_SRC_COLOR:		r = GL_SRC_COLOR; break;
				case EBF_ONE_MINUS_SRC_COLOR:	r = GL_ONE_MINUS_SRC_COLOR; break;
				case EBF_SRC_ALPHA:		r = GL_SRC_ALPHA; break;
				case EBF_ONE_MINUS_SRC_ALPHA:	r = GL_ONE_MINUS_SRC_ALPHA; break;
				case EBF_DST_ALPHA:		r = GL_DST_ALPHA; break;
				case EBF_ONE_MINUS_DST_ALPHA:	r = GL_ONE_MINUS_DST_ALPHA; break;
				case EBF_SRC_ALPHA_SATURATE:	r = GL_SRC_ALPHA_SATURATE; break;
			}
			return r;
		}
};


//! Solid 2 layer material renderer
class COGLES1MaterialRenderer_SOLID_2_LAYER : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_SOLID_2_LAYER(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(2);
		Driver->setActiveTexture(1, material.getTexture(1));
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->queryFeature(EVDF_MULTITEXTURE))
			{
				Driver->extGlActiveTexture(GL_TEXTURE1);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
				glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
				glTexEnvf(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_PRIMARY_COLOR);
				glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
			}
		}
	}

	virtual void OnUnsetMaterial()
	{
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
			Driver->extGlActiveTexture(GL_TEXTURE0);
		}
	}
};


//! Transparent add color material renderer
class COGLES1MaterialRenderer_TRANSPARENT_ADD_COLOR : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_TRANSPARENT_ADD_COLOR(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if ((material.MaterialType != lastMaterial.MaterialType) || resetAllRenderstates)
		{
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
		}
	}

	virtual void OnUnsetMaterial()
	{
		glDisable(GL_BLEND);
	}

	//! Returns if the material is transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};


//! Transparent vertex alpha material renderer
class COGLES1MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR );

			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR );
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		}
	}

	virtual void OnUnsetMaterial()
	{
		// default values
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE );
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS );
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE );
		glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);

		glDisable(GL_BLEND);
	}

	//! Returns if the material is transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};


//! Transparent alpha channel material renderer
class COGLES1MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates
			|| material.MaterialTypeParam != lastMaterial.MaterialTypeParam )
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS);

			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);

			glAlphaFunc(GL_GREATER, material.MaterialTypeParam);
		}
	}

	virtual void OnUnsetMaterial()
	{
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
	}

	//! Returns if the material is transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};



//! Transparent alpha channel material renderer
class COGLES1MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5f);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
	}

	virtual void OnUnsetMaterial()
	{
		glDisable(GL_ALPHA_TEST);
	}

	//! Returns if the material is transparent.
	virtual bool isTransparent() const
	{
		return false;  // this material is not really transparent because it does no blending.
	}
};


//! material renderer for all kinds of lightmaps
class COGLES1MaterialRenderer_LIGHTMAP : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_LIGHTMAP(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(2);
		Driver->setActiveTexture(1, material.getTexture(1));
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			// diffuse map

			switch (material.MaterialType)
			{
				case EMT_LIGHTMAP_LIGHTING:
				case EMT_LIGHTMAP_LIGHTING_M2:
				case EMT_LIGHTMAP_LIGHTING_M4:
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					break;
				case EMT_LIGHTMAP_ADD:
				case EMT_LIGHTMAP:
				case EMT_LIGHTMAP_M2:
				case EMT_LIGHTMAP_M4:
				default:
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
					break;
			}

			if (Driver->queryFeature(EVDF_MULTITEXTURE))
			{
				// lightmap

				Driver->extGlActiveTexture(GL_TEXTURE1);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

				if (material.MaterialType == EMT_LIGHTMAP_ADD)
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD_SIGNED);
				else
					glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);

				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS);

				switch (material.MaterialType)
				{
					case EMT_LIGHTMAP_M4:
					case EMT_LIGHTMAP_LIGHTING_M4:
						glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 4.0f);
						break;
					case EMT_LIGHTMAP_M2:
					case EMT_LIGHTMAP_LIGHTING_M2:
						glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
						break;
					default:
						glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.0f);
				}
			}
		}
	}

	virtual void OnUnsetMaterial()
	{
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE1);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.f );
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			Driver->extGlActiveTexture(GL_TEXTURE0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
	}
};



//! detail map  material renderer
class COGLES1MaterialRenderer_DETAIL_MAP : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_DETAIL_MAP(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(2);
		Driver->setActiveTexture(1, material.getTexture(1));
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			// diffuse map

			if (Driver->queryFeature(EVDF_MULTITEXTURE))
			{
				// detail map
				Driver->extGlActiveTexture(GL_TEXTURE1);

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD_SIGNED);
				glTexEnvi(GL_TEXTURE_ENV,GL_SRC0_RGB,GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV,GL_SRC1_RGB, GL_TEXTURE);
				Driver->extGlActiveTexture(GL_TEXTURE0);
			}
		}
	}
};


//! sphere map material renderer
class COGLES1MaterialRenderer_SPHERE_MAP : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_SPHERE_MAP(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(1);
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
//			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
//			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

//			glEnable(GL_TEXTURE_GEN_S);
//			glEnable(GL_TEXTURE_GEN_T);
		}
	}

	virtual void OnUnsetMaterial()
	{
//		glDisable(GL_TEXTURE_GEN_S);
//		glDisable(GL_TEXTURE_GEN_T);
	}
};


//! reflection 2 layer material renderer
class COGLES1MaterialRenderer_REFLECTION_2_LAYER : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_REFLECTION_2_LAYER(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(2);
		Driver->setActiveTexture(1, material.getTexture(1));
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->queryFeature(EVDF_MULTITEXTURE))
			{
				Driver->extGlActiveTexture(GL_TEXTURE1);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);

			}
//			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
//			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
//			glEnable(GL_TEXTURE_GEN_S);
//			glEnable(GL_TEXTURE_GEN_T);
		}
	}

	virtual void OnUnsetMaterial()
	{
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
//		glDisable(GL_TEXTURE_GEN_S);
//		glDisable(GL_TEXTURE_GEN_T);
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE0);
		}
	}
};


//! reflection 2 layer material renderer
class COGLES1MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public COGLES1MaterialRenderer
{
public:

	COGLES1MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(video::COGLES1Driver* d)
		: COGLES1MaterialRenderer(d) {}

	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services)
	{
		Driver->disableTextures(2);
		Driver->setActiveTexture(1, material.getTexture(1));
		Driver->setActiveTexture(0, material.getTexture(0));
		Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

		if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
		{
			if (Driver->queryFeature(EVDF_MULTITEXTURE))
			{
				Driver->extGlActiveTexture(GL_TEXTURE1);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			}
//			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
//			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
//			glEnable(GL_TEXTURE_GEN_S);
//			glEnable(GL_TEXTURE_GEN_T);

			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			glEnable(GL_BLEND);
		}
	}

	virtual void OnUnsetMaterial()
	{
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
//		glDisable(GL_TEXTURE_GEN_S);
//		glDisable(GL_TEXTURE_GEN_T);
		if (Driver->queryFeature(EVDF_MULTITEXTURE))
		{
			Driver->extGlActiveTexture(GL_TEXTURE0);
		}
		glDisable(GL_BLEND);
	}

	//! Returns if the material is transparent.
	virtual bool isTransparent() const
	{
		return true;
	}
};

} // end namespace video
} // end namespace irr

#endif
#endif
