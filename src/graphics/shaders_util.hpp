#ifndef SHADERS_UTIL_HPP
#define SHADERS_UTIL_HPP

#include "utils/singleton.hpp"
#include <vector>
#include <matrix4.h>
#include <SColor.h>
#include <vector3d.h>
#include "gl_headers.hpp"

bool needsUBO();

unsigned getGLSLVersion();

GLuint LoadShader(const char * file, unsigned type);
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);

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
    if (getGLSLVersion() < 330)
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

struct UniformHelper
{
    template<unsigned N = 0>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms)
    {
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::core::matrix4 &mat, Args... arg)
    {
        glUniformMatrix4fv(uniforms[N], 1, GL_FALSE, mat.pointer());
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::video::SColorf &col, Args... arg)
    {
        glUniform3f(uniforms[N], col.r, col.g, col.b);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::video::SColor &col, Args... arg)
    {
        glUniform4i(uniforms[N], col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::core::vector3df &v, Args... arg)
    {
        glUniform3f(uniforms[N], v.X, v.Y, v.Z);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }


    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::core::vector2df &v, Args... arg)
    {
        glUniform2f(uniforms[N], v.X, v.Y);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const irr::core::dimension2df &v, Args... arg)
    {
        glUniform2f(uniforms[N], v.Width, v.Height);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, float f, Args... arg)
    {
        glUniform1f(uniforms[N], f);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, int f, Args... arg)
    {
        glUniform1i(uniforms[N], f);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const std::vector<float> &v, Args... arg)
    {
        glUniform1fv(uniforms[N], (int)v.size(), v.data());
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

};

void bypassUBO(GLuint Program);

extern std::vector<void(*)()> CleanTable;

template<typename T, typename... Args>
class ShaderHelperSingleton : public Singleton<T>
{
protected:
    std::vector<GLuint> uniforms;

    void AssignUniforms_impl()
    {
        GLuint uniform_ViewProjectionMatrixesUBO = glGetUniformBlockIndex(Program, "MatrixesData");
        if (uniform_ViewProjectionMatrixesUBO != GL_INVALID_INDEX)
            glUniformBlockBinding(Program, uniform_ViewProjectionMatrixesUBO, 0);
    }

    template<typename... U>
    void AssignUniforms_impl(const char* name, U... rest)
    {
        uniforms.push_back(glGetUniformLocation(Program, name));
        AssignUniforms_impl(rest...);
    }

    template<typename... U>
    void AssignUniforms(U... rest)
    {
        static_assert(sizeof...(rest) == sizeof...(Args), "Count of Uniform's name mismatch");
        AssignUniforms_impl(rest...);
    }

public:
    GLuint Program;

    ShaderHelperSingleton()
    {
        CleanTable.push_back(this->kill);
    }

    ~ShaderHelperSingleton()
    {
        glDeleteProgram(Program);
    }

    void setUniforms(const Args & ... args) const
    {
        if (needsUBO())
            bypassUBO(Program);
        UniformHelper::setUniformsHelper(uniforms, args...);
    }
};

enum SamplerType {
    Trilinear_Anisotropic_Filtered,
    Semi_trilinear,
    Bilinear_Filtered,
    Bilinear_Clamped_Filtered,
    Neared_Clamped_Filtered,
    Nearest_Filtered,
    Shadow_Sampler,
    Volume_Linear_Filtered,
    Trilinear_cubemap,
};

void setTextureSampler(GLenum, GLuint, GLuint, GLuint);

template<SamplerType...tp>
struct CreateSamplers;

template<SamplerType...tp>
struct BindTexture;

template<>
struct CreateSamplers<>
{
    static void exec(std::vector<unsigned> &, std::vector<GLenum> &e)
    {}
};

template<>
struct BindTexture<>
{
    template<int N>
    static void exec(const std::vector<unsigned> &TU)
    {}
};

GLuint createNearestSampler();

template<SamplerType...tp>
struct CreateSamplers<Nearest_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createNearestSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureNearest(unsigned TU, unsigned tid);

template<SamplerType...tp>
struct BindTexture<Nearest_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureNearest(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

template<SamplerType...tp>
struct CreateSamplers<Neared_Clamped_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        unsigned id;
        glGenSamplers(1, &id);
        glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);

        v.push_back(createNearestSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

template<SamplerType...tp>
struct BindTexture<Neared_Clamped_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        glActiveTexture(GL_TEXTURE0 + TU[N]);
        glBindTexture(GL_TEXTURE_2D, TexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

GLuint createBilinearSampler();

template<SamplerType...tp>
struct CreateSamplers<Bilinear_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createBilinearSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureBilinear(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Bilinear_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureBilinear(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

GLuint createBilinearClampedSampler();

template<SamplerType...tp>
struct CreateSamplers<Bilinear_Clamped_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createBilinearClampedSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureBilinearClamped(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Bilinear_Clamped_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureBilinearClamped(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

GLuint createSemiTrilinearSampler();

template<SamplerType...tp>
struct CreateSamplers<Semi_trilinear, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createSemiTrilinearSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureSemiTrilinear(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Semi_trilinear, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureSemiTrilinear(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

GLuint createTrilinearSampler();

template<SamplerType...tp>
struct CreateSamplers<Trilinear_Anisotropic_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createTrilinearSampler());
        e.push_back(GL_TEXTURE_2D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureTrilinearAnisotropic(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct CreateSamplers<Trilinear_cubemap, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createTrilinearSampler());
        e.push_back(GL_TEXTURE_CUBE_MAP);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindCubemapTrilinear(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Trilinear_cubemap, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindCubemapTrilinear(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

template<SamplerType...tp>
struct BindTexture<Trilinear_Anisotropic_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureTrilinearAnisotropic(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

template<SamplerType...tp>
struct CreateSamplers<Volume_Linear_Filtered, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createBilinearSampler());
        e.push_back(GL_TEXTURE_3D);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureVolume(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Volume_Linear_Filtered, tp...>
{
    template<int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureVolume(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

GLuint createShadowSampler();

template<SamplerType...tp>
struct CreateSamplers<Shadow_Sampler, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createShadowSampler());
        e.push_back(GL_TEXTURE_2D_ARRAY);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTextureShadow(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Shadow_Sampler, tp...>
{
    template <int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTextureShadow(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

template<SamplerType...tp>
class TextureRead
{
private:
    template<unsigned N, typename...Args>
    void AssignTextureNames_impl(GLuint)
    {
        static_assert(N == sizeof...(tp), "Wrong number of texture name");
    }

    template<unsigned N, typename...Args>
    void AssignTextureNames_impl(GLuint Program, GLuint TexUnit, const char *name, Args...args)
    {
        GLuint location = glGetUniformLocation(Program, name);
        TextureLocation.push_back(location);
        glUniform1i(location, TexUnit);
        TextureUnits.push_back(TexUnit);
        AssignTextureNames_impl<N + 1>(Program, args...);
    }

protected:
    std::vector<GLuint> TextureUnits;
    std::vector<GLenum> TextureType;
    std::vector<GLenum> TextureLocation;
    template<typename...Args>
    void AssignSamplerNames(GLuint Program, Args...args)
    {
        CreateSamplers<tp...>::exec(SamplersId, TextureType);

        glUseProgram(Program);
        AssignTextureNames_impl<0>(Program, args...);
        glUseProgram(0);
    }

    template<int N>
    void SetTextureUnits_impl()
    {
        static_assert(N == sizeof...(tp), "Not enough texture set");
    }

    template<int N, typename... TexIds>
    void SetTextureUnits_impl(GLuint texid, TexIds... args)
    {
        setTextureSampler(TextureType[N], TextureUnits[N], texid, SamplersId[N]);
        SetTextureUnits_impl<N + 1>(args...);
    }


    template<int N>
    void SetTextureHandles_impl()
    {
        static_assert(N == sizeof...(tp), "Not enough handle set");
    }

    template<int N, typename... HandlesId>
    void SetTextureHandles_impl(uint64_t handle, HandlesId... args)
    {
        if (handle)
            glUniformHandleui64ARB(TextureLocation[N], handle);
        SetTextureHandles_impl<N + 1>(args...);
    }

public:
    std::vector<GLuint> SamplersId;

    template<typename... TexIds>
    void SetTextureUnits(TexIds... args)
    {
        if (getGLSLVersion() >= 330)
            SetTextureUnits_impl<0>(args...);
        else
            BindTexture<tp...>::template exec<0>(TextureUnits, args...);
    }

    ~TextureRead()
    {
        for (unsigned i = 0; i < SamplersId.size(); i++)
            glDeleteSamplers(1, &SamplersId[i]);
    }

    template<typename... HandlesId>
    void SetTextureHandles(HandlesId... ids)
    {
        SetTextureHandles_impl<0>(ids...);
    }
};
#endif