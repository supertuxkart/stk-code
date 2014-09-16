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
        glUniform1fv(uniforms[N], v.size(), v.data());
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureNearest(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        glActiveTexture(GL_TEXTURE0 + TU[N]);
        glBindTexture(GL_TEXTURE_2D, TexId[N]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureBilinear(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureBilinearClamped(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureSemiTrilinear(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindCubemapTrilinear(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
    }
};

template<SamplerType...tp>
struct BindTexture<Trilinear_Anisotropic_Filtered, tp...>
{
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureTrilinearAnisotropic(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureVolume(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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
    static void exec(const std::vector<unsigned> &TU, const std::vector<unsigned> &TexId, unsigned N)
    {
        BindTextureShadow(TU[N], TexId[N]);
        BindTexture<tp...>::exec(TU, TexId, N + 1);
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

public:
    std::vector<GLuint> SamplersId;
    void SetTextureUnits(const std::vector<GLuint> &args)
    {
        assert(args.size() == sizeof...(tp) && "Too much texture unit provided");
        if (getGLSLVersion() >= 330)
        {
            for (unsigned i = 0; i < args.size(); i++)
            {
                setTextureSampler(TextureType[i], TextureUnits[i], args[i], SamplersId[i]);
            }
        }
        else
            BindTexture<tp...>::exec(TextureUnits, args, 0);
    }

    ~TextureRead()
    {
        for (unsigned i = 0; i < SamplersId.size(); i++)
            glDeleteSamplers(1, &SamplersId[i]);
    }

    void SetTextureHandles(const std::vector<uint64_t> &args)
    {
        assert(args.size() == TextureLocation.size() && "Wrong Handle count");
        for (unsigned i = 0; i < args.size(); i++)
        {
            if (args[i])
                glUniformHandleui64ARB(TextureLocation[i], args[i]);
        }
    }
};
#endif