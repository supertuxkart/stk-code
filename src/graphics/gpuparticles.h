#if 0
#ifndef GPUPARTICLES_H
#define GPUPARTICLES_H

#define GL_GLEXT_PROTOTYPES 1
#include "graphics/glwrap.hpp"
#include <ISceneManager.h>

/*#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(X) wglGetProcAddress(reinterpret_cast<const char*>(X))
#else
#include <GL/glx.h>
#define IRR_OGL_LOAD_EXTENSION(X) glXGetProcAddress(reinterpret_cast<const GLubyte*>(X))
#endif


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
#ifdef _IRR_WINDOWS_API_
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;*/

void initGL();
GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path);
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);
GLuint getTextureGLuint(irr::video::ITexture *tex);
void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit);


class GPUParticle {
private:
    unsigned count;
    GLuint SimulationProgram, RenderProgram, tfb_vertex_buffer[2];
    GLuint texture, normal_and_depth;
    GLuint loc_campos, loc_viewm, loc_time;
    GLuint loc_screenw, loc_screen, loc_invproj, texloc_tex, texloc_normal_and_depths;
public:
    GPUParticle(unsigned c, float *initialSamples, GLuint tex, GLuint rtt);
    void simulate();
    void render();
};

#endif // GPUPARTICLES_H
#endif
