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

template<typename T, int nb_cascades, int nb_materials>
class CommandBufferOld : public Singleton<T>
{
public:
    GLuint drawindirectcmd;
    DrawElementsIndirectCommand *Ptr;
    
    //size_t Offset[nb_cascades][nb_materials];
    //size_t Size[nb_cascades][nb_materials];
    
    CommandBufferOld()
    {
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
    }
    
    void bind()
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawindirectcmd);
    }
};


class SolidPassCmd : public CommandBufferOld<SolidPassCmd, 1, static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    size_t Offset[Material::SHADERTYPE_COUNT], Size[Material::SHADERTYPE_COUNT];
};

class ShadowPassCmd : public CommandBufferOld<ShadowPassCmd, 4, static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    size_t Offset[4][Material::SHADERTYPE_COUNT], Size[4][Material::SHADERTYPE_COUNT];
};

class RSMPassCmd : public CommandBufferOld<RSMPassCmd, 1, static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    size_t Offset[Material::SHADERTYPE_COUNT], Size[Material::SHADERTYPE_COUNT];
};

class GlowPassCmd : public CommandBufferOld<GlowPassCmd, 1, 1>
{
public:
    size_t Offset, Size;
};



//TODO
void addEdge(const core::vector3df &P0, const core::vector3df &P1);

bool isCulledPrecise(const scene::ICameraSceneNode *cam, const scene::ISceneNode *node);


#endif
