#include "graphics/glwrap.hpp"
#include <fstream>
#include <string>

#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(X) wglGetProcAddress(reinterpret_cast<const char*>(X))
PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
#endif

static bool is_gl_init = false;
void initGL()
{
	if (is_gl_init)
		return;
	is_gl_init = true;
#ifdef _IRR_WINDOWS_API_
	glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)IRR_OGL_LOAD_EXTENSION("glGenTransformFeedbacks");
	glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBindTransformFeedback");
	glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glDrawTransformFeedback");
	glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBeginTransformFeedback");
	glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glEndTransformFeedback");
	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)IRR_OGL_LOAD_EXTENSION("glBindBufferBase");
	glGenBuffers = (PFNGLGENBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)IRR_OGL_LOAD_EXTENSION("glBufferData");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribPointer");
	glCreateShader = (PFNGLCREATESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCreateShader");
	glCompileShader = (PFNGLCOMPILESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCompileShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)IRR_OGL_LOAD_EXTENSION("glShaderSource");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)IRR_OGL_LOAD_EXTENSION("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glUseProgram");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glEnableVertexAttribArray");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetUniformLocation");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)IRR_OGL_LOAD_EXTENSION("glUniformMatrix4fv");
	glUniform1f = (PFNGLUNIFORM1FPROC)IRR_OGL_LOAD_EXTENSION("glUniform1f");
	glUniform3f = (PFNGLUNIFORM3FPROC)IRR_OGL_LOAD_EXTENSION("glUniform3f");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glDisableVertexAttribArray");
	glDeleteShader = (PFNGLDELETESHADERPROC)IRR_OGL_LOAD_EXTENSION("glDeleteShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderInfoLog");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
	glUniform2f = (PFNGLUNIFORM2FPROC)IRR_OGL_LOAD_EXTENSION("glUniform2f");
	glUniform1i = (PFNGLUNIFORM1IPROC)IRR_OGL_LOAD_EXTENSION("glUniform1i");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramInfoLog");
	glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)IRR_OGL_LOAD_EXTENSION("glTransformFeedbackVaryings");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetAttribLocation");
	glBlendEquation = (PFNGLBLENDEQUATIONPROC)IRR_OGL_LOAD_EXTENSION("glBlendEquation");
	glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribDivisor");
	glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)IRR_OGL_LOAD_EXTENSION("glDrawArraysInstanced");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteBuffers");
#endif
}

// Mostly from shader tutorial
static
GLuint LoadShader(const char * file, unsigned type) {
	GLuint Id = glCreateShader(type);
	std::string Code;
	std::ifstream Stream(file, std::ios::in);
	if (Stream.is_open())
	{
		std::string Line = "";
		while (getline(Stream, Line))
			Code += "\n" + Line;
		Stream.close();
	}
	GLint Result = GL_FALSE;
	int InfoLogLength;
	printf("Compiling shader : %s\n", file);
	char const * SourcePointer = Code.c_str();
	int length = strlen(SourcePointer);
	glShaderSource(Id, 1, &SourcePointer, &length);
	glCompileShader(Id);

	glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
	if (Result == GL_FALSE) {
		glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
		char *ErrorMessage = new char[InfoLogLength];
		glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
		printf(ErrorMessage);
		delete[] ErrorMessage;
	}

	return Id;
}

GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path) {
	GLuint VertexShaderID = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
	GLuint FragmentShaderID = LoadShader(fragment_file_path, GL_FRAGMENT_SHADER);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		char *ErrorMessage = new char[InfoLogLength];
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ErrorMessage);
		printf(ErrorMessage);
		delete[] ErrorMessage;
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount) {
	GLuint Shader = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
	GLuint Program = glCreateProgram();
	glAttachShader(Program, Shader);
	glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(Program);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetProgramiv(Program, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		char *ErrorMessage = new char[InfoLogLength];
		glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
		printf(ErrorMessage);
		delete[] ErrorMessage;
	}
	glDeleteShader(Shader);
	return Program;
}



void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit) {
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texid);
	glUniform1i(location, textureUnit);
}
