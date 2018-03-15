// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2MaterialRenderer.h"
#include "IGPUProgrammingServices.h"
#include "IShaderConstantSetCallBack.h"
#include "IMaterialRendererServices.h"
#include "IVideoDriver.h"
#include "os.h"
#include "COGLES2Driver.h"
#include "COGLES2MaterialRenderer.h"

namespace irr
{
namespace video
{


//! Constructor
COGLES2MaterialRenderer::COGLES2MaterialRenderer(COGLES2Driver* driver,
		s32& outMaterialTypeNr,
		const c8* vertexShaderProgram,
		const c8* pixelShaderProgram,
		IShaderConstantSetCallBack* callback,
		E_MATERIAL_TYPE baseMaterial,
		s32 userData)
	: Driver(driver), CallBack(callback), Alpha(false), Blending(false), FixedBlending(false), Program(0), UserData(userData)
{
	#ifdef _DEBUG
	setDebugName("COGLES2MaterialRenderer");
	#endif

	if (baseMaterial == EMT_TRANSPARENT_VERTEX_ALPHA || baseMaterial == EMT_TRANSPARENT_ALPHA_CHANNEL ||
		/*baseMaterial == EMT_TRANSPARENT_ALPHA_CHANNEL_REF || */baseMaterial == EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA ||
		baseMaterial == EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA)
	{
		Alpha = true;
	}
	else if (baseMaterial == EMT_TRANSPARENT_ADD_COLOR || baseMaterial == EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR ||
		baseMaterial == EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR)
	{
		FixedBlending = true;
	}
	else if (baseMaterial == EMT_ONETEXTURE_BLEND)
		Blending = true;

	if (CallBack)
		CallBack->grab();

	init(outMaterialTypeNr, vertexShaderProgram, pixelShaderProgram);
}


//! constructor only for use by derived classes who want to
//! create a fall back material for example.
COGLES2MaterialRenderer::COGLES2MaterialRenderer(COGLES2Driver* driver,
					IShaderConstantSetCallBack* callback,
					E_MATERIAL_TYPE baseMaterial, s32 userData)
: Driver(driver), CallBack(callback), Alpha(false), Blending(false), FixedBlending(false), Program(0), UserData(userData)
{
	if (baseMaterial == EMT_TRANSPARENT_VERTEX_ALPHA || baseMaterial == EMT_TRANSPARENT_ALPHA_CHANNEL ||
		/*baseMaterial == EMT_TRANSPARENT_ALPHA_CHANNEL_REF || */baseMaterial == EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA ||
		baseMaterial == EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA)
	{
		Alpha = true;
	}
	else if (baseMaterial == EMT_TRANSPARENT_ADD_COLOR || baseMaterial == EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR ||
		baseMaterial == EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR)
	{
		FixedBlending = true;
	}
	else if (baseMaterial == EMT_ONETEXTURE_BLEND)
		Blending = true;

	if (CallBack)
		CallBack->grab();
}


//! Destructor
COGLES2MaterialRenderer::~COGLES2MaterialRenderer()
{
	if (CallBack)
		CallBack->drop();

	if (Program)
	{
		GLuint shaders[8];
		GLint count = 0;
		glGetAttachedShaders(Program, 8, &count, shaders);

		count=core::min_(count,8);
		for (GLint i=0; i<count; ++i)
			glDeleteShader(shaders[i]);
		glDeleteProgram(Program);
		Program = 0;
	}

	UniformInfo.clear();
}

GLuint COGLES2MaterialRenderer::getProgram() const
{
	return Program;
}

void COGLES2MaterialRenderer::init(s32& outMaterialTypeNr,
		const c8* vertexShaderProgram,
		const c8* pixelShaderProgram,
		bool addMaterial)
{
	outMaterialTypeNr = -1;

	if (!vertexShaderProgram || !pixelShaderProgram)
		return;

	Program = glCreateProgram();

	if (!Program)
		return;

	if (vertexShaderProgram)
		if (!createShader(GL_VERTEX_SHADER, vertexShaderProgram))
			return;

	if (pixelShaderProgram)
		if (!createShader(GL_FRAGMENT_SHADER, pixelShaderProgram))
			return;

	for ( size_t i = 0; i < EVA_COUNT; ++i )
			glBindAttribLocation( Program, i, sBuiltInVertexAttributeNames[i]);

	if (!linkProgram())
		return;

	// register myself as new material
	if (addMaterial)
		outMaterialTypeNr = Driver->addMaterialRenderer(this);
}


bool COGLES2MaterialRenderer::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
{
    Driver->setTextureRenderStates(Driver->getCurrentMaterial(), false);

	// call callback to set shader constants
	if (CallBack && Program)
		CallBack->OnSetConstants(this, UserData);

	return true;
}


void COGLES2MaterialRenderer::OnSetMaterial(const video::SMaterial& material,
				const video::SMaterial& lastMaterial,
				bool resetAllRenderstates,
				video::IMaterialRendererServices* services)
{
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

	if (CallBack)
		CallBack->OnSetMaterial(material);
}


void COGLES2MaterialRenderer::OnUnsetMaterial()
{
}


//! Returns if the material is transparent.
bool COGLES2MaterialRenderer::isTransparent() const
{
	return (Alpha || Blending || FixedBlending);
}


bool COGLES2MaterialRenderer::createShader(GLenum shaderType, const char* shader)
{
	if (Program)
	{
		GLuint shaderHandle = glCreateShader(shaderType);
		glShaderSource(shaderHandle, 1, &shader, NULL);
		glCompileShader(shaderHandle);

		GLint status = 0;

		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status);

		if (status != GL_TRUE)
		{
			os::Printer::log("GLSL shader failed to compile", ELL_ERROR);
			// check error message and log it
			GLint maxLength=0;
			GLint length;

			glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH,
					&maxLength);

			if (maxLength)
			{
				GLchar *infoLog = new GLchar[maxLength];
				glGetShaderInfoLog(shaderHandle, maxLength, &length, infoLog);
				os::Printer::log(reinterpret_cast<const c8*>(infoLog), ELL_ERROR);
				delete [] infoLog;
			}

			return false;
		}

		glAttachShader(Program, shaderHandle);
	}

	return true;
}


bool COGLES2MaterialRenderer::linkProgram()
{
	if (Program)
	{
		glLinkProgram(Program);

		GLint status = 0;

		glGetProgramiv(Program, GL_LINK_STATUS, &status);

		if (!status)
		{
			os::Printer::log("GLSL shader program failed to link", ELL_ERROR);
			// check error message and log it
			GLint maxLength=0;
			GLsizei length;

			glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &maxLength);

			if (maxLength)
			{
				GLchar *infoLog = new GLchar[maxLength];
				glGetProgramInfoLog(Program, maxLength, &length, infoLog);
				os::Printer::log(reinterpret_cast<const c8*>(infoLog), ELL_ERROR);
				delete [] infoLog;
			}

			return false;
		}

		// get uniforms information

		GLint num = 0;

		glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &num);

		if (num == 0)
		{
			// no uniforms
			return true;
		}

		GLint maxlen = 0;

		glGetProgramiv(Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxlen);

		if (maxlen == 0)
		{
			os::Printer::log("GLSL: failed to retrieve uniform information", ELL_ERROR);
			return false;
		}

		// seems that some implementations use an extra null terminator
		++maxlen;
		c8 *buf = new c8[maxlen];

		UniformInfo.clear();
		UniformInfo.reallocate(num);

		for (GLint i=0; i < num; ++i)
		{
			SUniformInfo ui;
			memset(buf, 0, maxlen);

			GLint size;
			glGetActiveUniform(Program, i, maxlen, 0, &size, &ui.type, reinterpret_cast<GLchar*>(buf));
			ui.name = buf;
			ui.location = glGetUniformLocation(Program, buf);

			UniformInfo.push_back(ui);
		}

		delete [] buf;
	}

	return true;
}


void COGLES2MaterialRenderer::setBasicRenderStates(const SMaterial& material,
						const SMaterial& lastMaterial,
						bool resetAllRenderstates)
{
	// forward
	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}

s32 COGLES2MaterialRenderer::getVertexShaderConstantID(const c8* name)
{
	return getPixelShaderConstantID(name);
}

s32 COGLES2MaterialRenderer::getPixelShaderConstantID(const c8* name)
{
	for (u32 i = 0; i < UniformInfo.size(); ++i)
	{
		if (UniformInfo[i].name == name)
			return i;
	}

	return -1;
}

void COGLES2MaterialRenderer::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	os::Printer::log("Cannot set constant, please use high level shader call instead.", ELL_WARNING);
}

void COGLES2MaterialRenderer::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	os::Printer::log("Cannot set constant, use high level shader call.", ELL_WARNING);
}

bool COGLES2MaterialRenderer::setVertexShaderConstant(s32 index, const f32* floats, int count)
{
	return setPixelShaderConstant(index, floats, count);
}

bool COGLES2MaterialRenderer::setVertexShaderConstant(s32 index, const s32* ints, int count)
{
	return setPixelShaderConstant(index, ints, count);
}

bool COGLES2MaterialRenderer::setPixelShaderConstant(s32 index, const f32* floats, int count)
{
	if(index < 0 || UniformInfo[index].location < 0)
		return false;

	bool status = true;

	switch (UniformInfo[index].type)
	{
		case GL_FLOAT:
			glUniform1fv(UniformInfo[index].location, count, floats);
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv(UniformInfo[index].location, count/2, floats);
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv(UniformInfo[index].location, count/3, floats);
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv(UniformInfo[index].location, count/4, floats);
			break;
		case GL_FLOAT_MAT2:
			glUniformMatrix2fv(UniformInfo[index].location, count/4, false, floats);
			break;
		case GL_FLOAT_MAT3:
			glUniformMatrix3fv(UniformInfo[index].location, count/9, false, floats);
			break;
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv(UniformInfo[index].location, count/16, false, floats);
			break;
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			{
				if(floats)
				{
					const GLint id = (GLint)(*floats);
					glUniform1iv(UniformInfo[index].location, 1, &id);
				}
				else
					status = false;
			}
			break;
		default:
			status = false;
			break;
	}

	return status;
}

bool COGLES2MaterialRenderer::setPixelShaderConstant(s32 index, const s32* ints, int count)
{
	if(index < 0 || UniformInfo[index].location < 0)
		return false;

	bool status = true;

	switch (UniformInfo[index].type)
	{
		case GL_INT:
		case GL_BOOL:
			glUniform1iv(UniformInfo[index].location, count, ints);
			break;
		case GL_INT_VEC2:
		case GL_BOOL_VEC2:
			glUniform2iv(UniformInfo[index].location, count/2, ints);
			break;
		case GL_INT_VEC3:
		case GL_BOOL_VEC3:
			glUniform3iv(UniformInfo[index].location, count/3, ints);
			break;
		case GL_INT_VEC4:
		case GL_BOOL_VEC4:
			glUniform4iv(UniformInfo[index].location, count/4, ints);
			break;
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			glUniform1iv(UniformInfo[index].location, 1, ints);
			break;
		default:
			status = false;
			break;
	}

	return status;
}

IVideoDriver* COGLES2MaterialRenderer::getVideoDriver()
{
	return Driver;
}

} // end namespace video
} // end namespace irr


#endif
