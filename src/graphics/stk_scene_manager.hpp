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


// Not really a scene manager yet but hold algorithm that
// rework scene manager output

#ifndef HEADER_STKSCENEMANAGER_HPP
#define HEADER_STKSCENEMANAGER_HPP

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/gpu_particles.hpp"
#include "graphics/stk_billboard.hpp"
#include "graphics/stk_mesh.hpp"
#include "utils/singleton.hpp"

template<typename T>
class CommandBuffer : public Singleton<T>
{
public:
    GLuint drawindirectcmd;
    DrawElementsIndirectCommand *Ptr;
    CommandBuffer()
    {
#if !defined(USE_GLES2)
        glGenBuffers(1, &drawindirectcmd);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawindirectcmd);
        if (CVS->supportsAsyncInstanceUpload())
        {
            glBufferStorage(GL_DRAW_INDIRECT_BUFFER, 10000 * sizeof(DrawElementsIndirectCommand), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
            Ptr = (DrawElementsIndirectCommand *)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
        }
        else
        {
            glBufferData(GL_DRAW_INDIRECT_BUFFER, 10000 * sizeof(DrawElementsIndirectCommand), 0, GL_STREAM_DRAW);
        }
#endif
    }
};

class ImmediateDrawList : public Singleton<ImmediateDrawList>, public std::vector<scene::ISceneNode *>
{};

class BillBoardList : public Singleton<BillBoardList>, public std::vector<STKBillboard *>
{};

class ParticlesList : public Singleton<ParticlesList>, public std::vector<ParticleSystemProxy *>
{};


class SolidPassCmd : public CommandBuffer<SolidPassCmd>
{
public:
    size_t Offset[Material::SHADERTYPE_COUNT], Size[Material::SHADERTYPE_COUNT];
};

class ShadowPassCmd : public CommandBuffer<ShadowPassCmd>
{
public:
    size_t Offset[4][Material::SHADERTYPE_COUNT], Size[4][Material::SHADERTYPE_COUNT];
};

class RSMPassCmd : public CommandBuffer<RSMPassCmd>
{
public:
    size_t Offset[Material::SHADERTYPE_COUNT], Size[Material::SHADERTYPE_COUNT];
};

class GlowPassCmd : public CommandBuffer<GlowPassCmd>
{
public:
    size_t Offset, Size;
};


#endif
