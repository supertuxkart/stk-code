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

#ifndef SERVER_ONLY

#include "graphics/vao_manager.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_mesh.hpp"

VAOManager::VAOManager()
{
    for (unsigned i = 0; i < VTXTYPE_COUNT; i++)
    {
        vao[i] = 0;
        vbo[i] = 0;
        ibo[i] = 0;
        last_vertex[i] = 0;
        last_index[i] = 0;
        RealVBOSize[i] = 0;
        RealIBOSize[i] = 0;
    }

    for (unsigned i = 0; i < InstanceTypeCount; i++)
    {
        glGenBuffers(1, &instance_vbo[i]);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[i]);
#if !defined(USE_GLES2)
        if (CVS->supportsAsyncInstanceUpload())
        {
            glBufferStorage(GL_ARRAY_BUFFER, 10000 * sizeof(InstanceDataThreeTex), 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            Ptr[i] = glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceDataThreeTex), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        }
        else
#endif
        {
            glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(InstanceDataThreeTex), 0, GL_STREAM_DRAW);
        }
    }
}

void VAOManager::cleanInstanceVAOs()
{
    std::map<std::pair<video::E_VERTEX_TYPE, InstanceType>, GLuint>::iterator It = InstanceVAO.begin(), E = InstanceVAO.end();
    for (; It != E; It++)
        glDeleteVertexArrays(1, &(It->second));
    InstanceVAO.clear();
}

VAOManager::~VAOManager()
{
    cleanInstanceVAOs();
    for (unsigned i = 0; i < VTXTYPE_COUNT; i++)
    {
        if (vbo[i])
            glDeleteBuffers(1, &vbo[i]);
        if (ibo[i])
            glDeleteBuffers(1, &ibo[i]);
        if (vao[i])
            glDeleteVertexArrays(1, &vao[i]);
    }
    for (unsigned i = 0; i < InstanceTypeCount; i++)
    {
        glDeleteBuffers(1, &instance_vbo[i]);
    }

}

static void
resizeBufferIfNecessary(unsigned int &lastIndex, unsigned int  newLastIndex,
                        unsigned int &bufferSize, unsigned int stride, GLenum type,
                        GLuint &id, void *&Pointer)
{
    if (newLastIndex >= bufferSize)
    {
        while (newLastIndex >= bufferSize)
            bufferSize = bufferSize == 0 ? 1 : bufferSize * 2;
        GLuint newVBO;
        glGenBuffers(1, &newVBO);
        glBindBuffer(type, newVBO);
#if !defined(USE_GLES2)
        if (CVS->supportsAsyncInstanceUpload())
        {
            glBufferStorage(type, bufferSize *stride, 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            Pointer = glMapBufferRange(type, 0, bufferSize * stride, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        }
        else
#endif
            glBufferData(type, bufferSize * stride, 0, GL_DYNAMIC_DRAW);

        if (id)
        {
            // Copy old data
            GLuint oldVBO = id;
            glBindBuffer(GL_COPY_WRITE_BUFFER, newVBO);
            glBindBuffer(GL_COPY_READ_BUFFER, oldVBO);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, lastIndex * stride);
            glDeleteBuffers(1, &oldVBO);
        }
        id = newVBO;
    }
    lastIndex = newLastIndex;
}

void VAOManager::regenerateBuffer(enum VTXTYPE tp, unsigned int newlastvertex,
                                  unsigned int newlastindex)
{
    glBindVertexArray(0);
    resizeBufferIfNecessary(last_vertex[tp], newlastvertex, RealVBOSize[tp],
                            getVertexPitch(tp), GL_ARRAY_BUFFER, vbo[tp], VBOPtr[tp]);
    resizeBufferIfNecessary(last_index[tp], newlastindex, RealIBOSize[tp],
                            sizeof(u16), GL_ELEMENT_ARRAY_BUFFER, ibo[tp], IBOPtr[tp]);
}

void VAOManager::regenerateVAO(enum VTXTYPE tp)
{
    if (vao[tp])
        glDeleteVertexArrays(1, &vao[tp]);
    glGenVertexArrays(1, &vao[tp]);
    glBindVertexArray(vao[tp]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[tp]);

    VertexUtils::bindVertexArrayAttrib(getVertexType(tp));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[tp]);
    glBindVertexArray(0);
}

template<typename T>
struct VAOInstanceUtil
{
    static void SetVertexAttrib_impl()
    {
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(T), 0);
        glVertexAttribDivisorARB(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(T), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisorARB(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(T), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisorARB(9, 1);
    }

    static void SetVertexAttrib();
};

template<>
void VAOInstanceUtil<InstanceDataSingleTex>::SetVertexAttrib()
{
    SetVertexAttrib_impl();
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceDataSingleTex), (GLvoid*)(9 * sizeof(float)));
    glVertexAttribDivisorARB(11, 1);
    glEnableVertexAttribArray(15);
    glVertexAttribIPointer(15, 1, GL_INT, sizeof(InstanceDataSingleTex), (GLvoid*)(11 * sizeof(float)));
    glVertexAttribDivisorARB(15, 1);
}

template<>
void VAOInstanceUtil<InstanceDataThreeTex>::SetVertexAttrib()
{
    SetVertexAttrib_impl();
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataThreeTex), (GLvoid*)(9 * sizeof(float)));
    glVertexAttribDivisorARB(10, 1);
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceDataThreeTex), (GLvoid*)(13 * sizeof(float)));
    glVertexAttribDivisorARB(11, 1);
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_UNSIGNED_INT, sizeof(InstanceDataThreeTex), (GLvoid*)(13 * sizeof(float) + 2 * sizeof(unsigned)));
    glVertexAttribDivisorARB(12, 1);
    glEnableVertexAttribArray(13);
    glVertexAttribIPointer(13, 2, GL_UNSIGNED_INT, sizeof(InstanceDataThreeTex), (GLvoid*)(13 * sizeof(float) + 4 * sizeof(unsigned)));
    glVertexAttribDivisorARB(13, 1);
    glEnableVertexAttribArray(15);
    glVertexAttribIPointer(15, 1, GL_INT, sizeof(InstanceDataThreeTex), (GLvoid*)(13 * sizeof(float) + 6 * sizeof(unsigned)));;
    glVertexAttribDivisorARB(15, 1);
}

template<>
void VAOInstanceUtil<InstanceDataFourTex>::SetVertexAttrib()
{
    SetVertexAttrib_impl();
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceDataFourTex), (GLvoid*)(9 * sizeof(float)));
    glVertexAttribDivisorARB(10, 1);
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceDataFourTex), (GLvoid*)(13 * sizeof(float)));
    glVertexAttribDivisorARB(11, 1);
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_UNSIGNED_INT, sizeof(InstanceDataFourTex), (GLvoid*)(13 * sizeof(float) + 2 * sizeof(unsigned)));
    glVertexAttribDivisorARB(12, 1);
    glEnableVertexAttribArray(13);
    glVertexAttribIPointer(13, 2, GL_UNSIGNED_INT, sizeof(InstanceDataFourTex), (GLvoid*)(13 * sizeof(float) + 4 * sizeof(unsigned)));
    glVertexAttribDivisorARB(13, 1);
    glEnableVertexAttribArray(14);
    glVertexAttribIPointer(14, 2, GL_UNSIGNED_INT, sizeof(InstanceDataFourTex), (GLvoid*)(13 * sizeof(float) + 6 * sizeof(unsigned)));
    glVertexAttribDivisorARB(14, 1);
    glEnableVertexAttribArray(15);
    glVertexAttribIPointer(15, 1, GL_INT, sizeof(InstanceDataFourTex), (GLvoid*)(13 * sizeof(float) + 8 * sizeof(unsigned)));
    glVertexAttribDivisorARB(15, 1);
}

template<>
void VAOInstanceUtil<GlowInstanceData>::SetVertexAttrib()
{
    SetVertexAttrib_impl();
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GlowInstanceData), (GLvoid*)(9 * sizeof(float)));
    glVertexAttribDivisorARB(10, 1);
}


void VAOManager::regenerateInstancedVAO()
{
    cleanInstanceVAOs();

    enum video::E_VERTEX_TYPE IrrVT[] = { video::EVT_STANDARD, video::EVT_2TCOORDS, video::EVT_TANGENTS, video::EVT_SKINNED_MESH };
    for (unsigned i = 0; i < VTXTYPE_COUNT; i++)
    {
        video::E_VERTEX_TYPE tp = IrrVT[i];
        if (!vbo[tp] || !ibo[tp])
            continue;
        GLuint vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeThreeTex]);
        VAOInstanceUtil<InstanceDataThreeTex>::SetVertexAttrib();
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeThreeTex)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeFourTex]);
        VAOInstanceUtil<InstanceDataFourTex>::SetVertexAttrib();
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeFourTex)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeShadow]);
        VAOInstanceUtil<InstanceDataSingleTex>::SetVertexAttrib();
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeShadow)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeRSM]);
        VAOInstanceUtil<InstanceDataSingleTex>::SetVertexAttrib();
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeRSM)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeGlow]);
        VAOInstanceUtil<GlowInstanceData>::SetVertexAttrib();
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeGlow)] = vao;

        glBindVertexArray(0);
    }



}

unsigned int VAOManager::getVertexPitch(enum VTXTYPE tp) const
{
    switch (tp)
    {
    case VTXTYPE_STANDARD:
        return getVertexPitchFromType(video::EVT_STANDARD);
    case VTXTYPE_TCOORD:
        return getVertexPitchFromType(video::EVT_2TCOORDS);
    case VTXTYPE_TANGENT:
        return getVertexPitchFromType(video::EVT_TANGENTS);
    case VTXTYPE_SKINNED_MESH:
        return getVertexPitchFromType(video::EVT_SKINNED_MESH);
    default:
        assert(0 && "Wrong vtxtype");
        return -1;
    }
}

VAOManager::VTXTYPE VAOManager::getVTXTYPE(video::E_VERTEX_TYPE type)
{
    switch (type)
    {
    default:
        assert(0 && "Wrong vtxtype");
    case video::EVT_STANDARD:
        return VTXTYPE_STANDARD;
    case video::EVT_2TCOORDS:
        return VTXTYPE_TCOORD;
    case video::EVT_TANGENTS:
        return VTXTYPE_TANGENT;
    case video::EVT_SKINNED_MESH:
        return VTXTYPE_SKINNED_MESH;
    }
};

irr::video::E_VERTEX_TYPE VAOManager::getVertexType(enum VTXTYPE tp)
{
    switch (tp)
    {
    default:
    case VTXTYPE_STANDARD:
        return video::EVT_STANDARD;
    case VTXTYPE_TCOORD:
        return video::EVT_2TCOORDS;
    case VTXTYPE_TANGENT:
        return video::EVT_TANGENTS;
    case VTXTYPE_SKINNED_MESH:
        return video::EVT_SKINNED_MESH;
    }
}

void VAOManager::append(scene::IMeshBuffer *mb, VTXTYPE tp)
{
    unsigned int old_vtx_cnt = last_vertex[tp];
    unsigned int old_idx_cnt = last_index[tp];

    regenerateBuffer(tp, old_vtx_cnt + mb->getVertexCount(), old_idx_cnt + mb->getIndexCount());
#if !defined(USE_GLES2)
    if (CVS->supportsAsyncInstanceUpload())
    {
        void *tmp = (char*)VBOPtr[tp] + old_vtx_cnt * getVertexPitch(tp);
        memcpy(tmp, mb->getVertices(), mb->getVertexCount() * getVertexPitch(tp));
    }
    else
#endif
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo[tp]);
        glBufferSubData(GL_ARRAY_BUFFER, old_vtx_cnt * getVertexPitch(tp), mb->getVertexCount() * getVertexPitch(tp), mb->getVertices());
    }
#if !defined(USE_GLES2)
    if (CVS->supportsAsyncInstanceUpload())
    {
        void *tmp = (char*)IBOPtr[tp] + old_idx_cnt * sizeof(u16);
        memcpy(tmp, mb->getIndices(), mb->getIndexCount() * sizeof(u16));
    }
    else
#endif
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[tp]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, old_idx_cnt * sizeof(u16), mb->getIndexCount() * sizeof(u16), mb->getIndices());
    }

    mappedBaseVertex[tp][mb] = old_vtx_cnt;
    mappedBaseIndex[tp][mb] = old_idx_cnt * sizeof(u16);
}

std::pair<unsigned, unsigned> VAOManager::getBase(scene::IMeshBuffer *mb)
{
    VTXTYPE tp = getVTXTYPE(mb->getVertexType());
    if (mappedBaseVertex[tp].find(mb) == mappedBaseVertex[tp].end())
    {
        assert(mappedBaseIndex[tp].find(mb) == mappedBaseIndex[tp].end());
        append(mb, tp);
        regenerateVAO(tp);
        regenerateInstancedVAO();
    }

    std::unordered_map<scene::IMeshBuffer*, unsigned>::iterator It;
    It = mappedBaseVertex[tp].find(mb);
    assert(It != mappedBaseVertex[tp].end());
    unsigned vtx = It->second;
    It = mappedBaseIndex[tp].find(mb);
    assert(It != mappedBaseIndex[tp].end());
    return std::pair<unsigned, unsigned>(vtx, It->second);
}

#endif   // !SERVER_ONLY
