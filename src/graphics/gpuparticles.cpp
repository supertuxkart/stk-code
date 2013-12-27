#if 0
#include "graphics/irr_driver.hpp"
#include "gpuparticles.h"
#include <fstream>
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include <ICameraSceneNode.h>

void initGL()
{
/*    glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)IRR_OGL_LOAD_EXTENSION("glGenTransformFeedbacks");
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
#ifdef _IRR_WINDOWS_API_
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
#endif
    glUniform2f = (PFNGLUNIFORM2FPROC)IRR_OGL_LOAD_EXTENSION("glUniform2f");
    glUniform1i = (PFNGLUNIFORM1IPROC)IRR_OGL_LOAD_EXTENSION("glUniform1i");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramInfoLog");
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)IRR_OGL_LOAD_EXTENSION("glTransformFeedbackVaryings");*/
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

GLuint getTextureGLuint(irr::video::ITexture *tex) {
    return static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName();
}

void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texid);
    glUniform1i(location, textureUnit);
}

GPUParticle::GPUParticle(unsigned c, float *initialSamples, GLuint tex, GLuint rtt) :
      count(c), texture(tex), normal_and_depth(rtt) {
    initGL();
    glGenBuffers(2, tfb_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), initialSamples, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), 0, GL_STREAM_DRAW);

    RenderProgram = LoadProgram(file_manager->getAsset("shaders/rain.vert").c_str(), file_manager->getAsset("shaders/rain.frag").c_str());
    loc_screenw = glGetUniformLocation(RenderProgram, "screenw");
    loc_screen = glGetUniformLocation(RenderProgram, "screen");
    loc_invproj = glGetUniformLocation(RenderProgram, "invproj");
    texloc_tex = glGetUniformLocation(RenderProgram, "tex");
    texloc_normal_and_depths = glGetUniformLocation(RenderProgram, "normals_and_depth");

    const char *varyings[] = {"currentPosition"};
    SimulationProgram = LoadTFBProgram(file_manager->getAsset("shaders/rainsim.vert").c_str(), varyings, 1);
    loc_campos = glGetUniformLocation(SimulationProgram, "campos");
    loc_viewm = glGetUniformLocation(SimulationProgram, "viewm");
    loc_time = glGetUniformLocation(SimulationProgram, "time");
  }

void GPUParticle::simulate() {
    glUseProgram(SimulationProgram);
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 90.0f;
    const irr::core::matrix4 viewm = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_VIEW);
    const irr::core::vector3df campos = irr_driver->getSceneManager()->getActiveCamera()->getPosition();

    glEnable(GL_RASTERIZER_DISCARD);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_vertex_buffer[1]);

    glUniformMatrix4fv(loc_viewm, 1, GL_FALSE, viewm.pointer());
    glUniform1f(loc_time, time);
    glUniform3f(loc_campos, campos.X, campos.Y, campos.Z);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, count);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
}

void GPUParticle::render() {
    const float screenw = (float)UserConfigParams::m_width;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glUseProgram(RenderProgram);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    float screen[2] = {
        (float)UserConfigParams::m_width,
        (float)UserConfigParams::m_height
    };
    irr::core::matrix4 invproj = irr_driver->getVideoDriver()->getTransform(irr::video::ETS_PROJECTION);
    invproj.makeInverse();

    bindUniformToTextureUnit(texloc_tex, texture, 0);
    bindUniformToTextureUnit(texloc_normal_and_depths, normal_and_depth, 1);

    glUniformMatrix4fv(loc_invproj, 1, GL_FALSE, invproj.pointer());
    glUniform2f(loc_screen, screen[0], screen[1]);
    glUniform1f(loc_screenw, screenw);
    glDrawArrays(GL_POINTS, 0, count);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}
#endif
