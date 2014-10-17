#ifndef GLWRAP_HEADER_H
#define GLWRAP_HEADER_H

#include "graphics/gl_headers.hpp"

#include "graphics/irr_driver.hpp"
#include "graphics/vaomanager.hpp"
#include "utils/log.hpp"

#include <vector>

namespace HardwareStats
{
    class Json;
}

void initGL();
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);
video::ITexture* getUnicolorTexture(const video::SColor &c);
void setTexture(unsigned TextureUnit, GLuint TextureId, GLenum MagFilter, GLenum MinFilter, bool allowAF = false);
GLuint LoadShader(const char * file, unsigned type);

template<typename ... Types>
void loadAndAttach(GLint ProgramID)
{
    return;
}

template<typename ... Types>
void loadAndAttach(GLint ProgramID, GLint ShaderType, const char *filepath, Types ... args)
{
    GLint ShaderID = LoadShader(filepath, ShaderType);
    glAttachShader(ProgramID, ShaderID);
    glDeleteShader(ShaderID);
    loadAndAttach(ProgramID, args...);
}

template<typename ...Types>
void printFileList()
{
    return;
}

template<typename ...Types>
void printFileList(GLint ShaderType, const char *filepath, Types ... args)
{
    Log::error("GLWrapp", filepath);
    printFileList(args...);
}

enum AttributeType
{
    OBJECT,
    PARTICLES_SIM,
    PARTICLES_RENDERING,
};

void setAttribute(AttributeType Tp, GLuint ProgramID);

template<typename ... Types>
GLint LoadProgram(AttributeType Tp, Types ... args)
{
    GLint ProgramID = glCreateProgram();
    loadAndAttach(ProgramID, args...);
    if (irr_driver->getGLSLVersion() < 330)
        setAttribute(Tp, ProgramID);
    glLinkProgram(ProgramID);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) {
        Log::error("GLWrapp", "Error when linking these shaders :");
        printFileList(args...);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrapp", ErrorMessage);
        delete[] ErrorMessage;
    }

    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        Log::warn("IrrDriver", "GLWrap : OpenGL error %i\n", glErr);
    }

    return ProgramID;
}

class GPUTimer;

class ScopedGPUTimer
{
protected:
    GPUTimer &timer;
public:
    ScopedGPUTimer(GPUTimer &);
    ~ScopedGPUTimer();
};

class GPUTimer
{
    friend class ScopedGPUTimer;
    GLuint query;
    bool initialised;
    unsigned lastResult;
    bool canSubmitQuery;
public:
    GPUTimer();
    unsigned elapsedTimeus();
};

class FrameBuffer
{
private:
    GLuint fbo;
    std::vector<GLuint> RenderTargets;
    GLuint DepthTexture;
    size_t width, height;
public:
    FrameBuffer();
    FrameBuffer(const std::vector <GLuint> &RTTs, size_t w, size_t h, bool layered = false);
    FrameBuffer(const std::vector <GLuint> &RTTs, GLuint DS, size_t w, size_t h, bool layered = false);
    ~FrameBuffer();
    void Bind();
    const std::vector<GLuint> &getRTT() const { return RenderTargets; }
    GLuint &getDepthTexture() { assert(DepthTexture); return DepthTexture; }
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    static void Blit(const FrameBuffer &Src, FrameBuffer &Dst, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);
    void BlitToDefault(size_t, size_t, size_t, size_t);
};

class VertexUtils
{
public:
    static void bindVertexArrayAttrib(enum video::E_VERTEX_TYPE tp)
    {
        switch (tp)
        {
        case video::EVT_STANDARD:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            break;
        case video::EVT_2TCOORDS:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            // SecondTexcoord
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)36);
            break;
        case video::EVT_TANGENTS:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            // Tangent
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)36);
            // Bitangent
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)48);
            break;
        }
    }
};

// core::rect<s32> needs these includes
#include <rect.h>
#include "utils/vec3.hpp"
#include "texturemanager.hpp"

void draw3DLine(const core::vector3df& start,
    const core::vector3df& end, irr::video::SColor color);

void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
    const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor &colors, bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<s32>& destRect,
    const irr::core::rect<s32>& sourceRect, const irr::core::rect<s32>* clipRect,
    const irr::video::SColor &color, bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<s32>& destRect,
    const irr::core::rect<s32>& sourceRect, const irr::core::rect<s32>* clipRect,
    const irr::video::SColor* const colors, bool useAlphaChannelOfTexture);

void draw2DVertexPrimitiveList(const void* vertices,
    u32 vertexCount, const void* indexList, u32 primitiveCount,
    video::E_VERTEX_TYPE vType = video::EVT_STANDARD, scene::E_PRIMITIVE_TYPE pType = scene::EPT_TRIANGLES, video::E_INDEX_TYPE iType = video::EIT_16BIT);

void GL32_draw2DRectangle(irr::video::SColor color, const irr::core::rect<s32>& position,
    const irr::core::rect<s32>* clip = 0);

bool hasGLExtension(const char* extension);
const std::string getGLExtensions();
void getGLLimits(HardwareStats::Json *json);

#endif
