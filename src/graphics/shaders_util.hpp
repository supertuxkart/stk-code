//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SHADERS_UTIL_HPP
#define SHADERS_UTIL_HPP

#include "graphics/central_settings.hpp"
#include "utils/singleton.hpp"
#include <vector>
#include <matrix4.h>
#include <SColor.h>
#include <vector3d.h>
#include "gl_headers.hpp"

bool needsUBO();

unsigned getGLSLVersion();


enum AttributeType
{
    OBJECT,
    PARTICLES_SIM,
    PARTICLES_RENDERING,
};

void setAttribute(AttributeType Tp, GLuint ProgramID);

void bypassUBO(GLuint Program);

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
    Trilinear_Clamped_Array2D,
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

GLuint createTrilinearClampedArray();

template<SamplerType...tp>
struct CreateSamplers<Trilinear_Clamped_Array2D, tp...>
{
    static void exec(std::vector<unsigned> &v, std::vector<GLenum> &e)
    {
        v.push_back(createTrilinearClampedArray());
        e.push_back(GL_TEXTURE_2D_ARRAY);
        CreateSamplers<tp...>::exec(v, e);
    }
};

void BindTrilinearClampedArrayTexture(unsigned TU, unsigned tex);

template<SamplerType...tp>
struct BindTexture<Trilinear_Clamped_Array2D, tp...>
{
    template <int N, typename...Args>
    static void exec(const std::vector<unsigned> &TU, GLuint TexId, Args... args)
    {
        BindTrilinearClampedArrayTexture(TU[N], TexId);
        BindTexture<tp...>::template exec<N + 1>(TU, args...);
    }
};

#endif
