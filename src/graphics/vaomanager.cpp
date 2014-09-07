#include "vaomanager.hpp"
#include "stkmesh.hpp"

VAOManager::VAOManager()
{
    vao[0] = vao[1] = vao[2] = 0;
    vbo[0] = vbo[1] = vbo[2] = 0;
    ibo[0] = ibo[1] = ibo[2] = 0;
    vtx_cnt[0] = vtx_cnt[1] = vtx_cnt[2] = 0;
    idx_cnt[0] = idx_cnt[1] = idx_cnt[2] = 0;
    vtx_mirror[0] = vtx_mirror[1] = vtx_mirror[2] = NULL;
    idx_mirror[0] = idx_mirror[1] = idx_mirror[2] = NULL;
    instance_count[0] = 0;

    for (unsigned i = 0; i < InstanceTypeCount; i++)
    {
        glGenBuffers(1, &instance_vbo[i]);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[i]);
#ifdef Buffer_Storage
        if (irr_driver->hasBufferStorageExtension())
        {
            glBufferStorage(GL_ARRAY_BUFFER, 10000 * sizeof(InstanceData), 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
            Ptr[i] = glMapBufferRange(GL_ARRAY_BUFFER, 0, 10000 * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        }
        else
#endif
        {
            glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(InstanceData), 0, GL_STREAM_DRAW);
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
    for (unsigned i = 0; i < 3; i++)
    {
        if (vtx_mirror[i])
            free(vtx_mirror[i]);
        if (idx_mirror[i])
            free(idx_mirror[i]);
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

void VAOManager::regenerateBuffer(enum VTXTYPE tp)
{
    glBindVertexArray(0);
    if (vbo[tp])
        glDeleteBuffers(1, &vbo[tp]);
    glGenBuffers(1, &vbo[tp]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[tp]);
#ifdef Buffer_Storage
    if (irr_driver->hasBufferStorageExtension())
    {
        glBufferStorage(GL_ARRAY_BUFFER, vtx_cnt[tp] * getVertexPitch(tp), vtx_mirror[tp], GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        VBOPtr[tp] = glMapBufferRange(GL_ARRAY_BUFFER, 0, vtx_cnt[tp] * getVertexPitch(tp), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
    }
    else
#endif
        glBufferData(GL_ARRAY_BUFFER, vtx_cnt[tp] * getVertexPitch(tp), vtx_mirror[tp], GL_DYNAMIC_DRAW);


    if (ibo[tp])
        glDeleteBuffers(1, &ibo[tp]);
    glGenBuffers(1, &ibo[tp]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[tp]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u16)* idx_cnt[tp], idx_mirror[tp], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VAOManager::regenerateVAO(enum VTXTYPE tp)
{
    if (vao[tp])
        glDeleteVertexArrays(1, &vao[tp]);
    glGenVertexArrays(1, &vao[tp]);
    glBindVertexArray(vao[tp]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[tp]);
    switch (tp)
    {
    case VTXTYPE_STANDARD:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitch(tp), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)28);
        break;
    case VTXTYPE_TCOORD:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitch(tp), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)28);
        // SecondTexcoord
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)36);
        break;
    case VTXTYPE_TANGENT:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitch(tp), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)28);
        // Tangent
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)36);
        // Bitangent
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, getVertexPitch(tp), (GLvoid*)48);
        break;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[tp]);
    glBindVertexArray(0);
}

void VAOManager::regenerateInstancedVAO()
{
    cleanInstanceVAOs();

    enum video::E_VERTEX_TYPE IrrVT[] = { video::EVT_STANDARD, video::EVT_2TCOORDS, video::EVT_TANGENTS };
    for (unsigned i = 0; i < VTXTYPE_COUNT; i++)
    {
        video::E_VERTEX_TYPE tp = IrrVT[i];
        if (!vbo[tp] || !ibo[tp])
            continue;
        GLuint vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeDefault]);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 1);
        glEnableVertexAttribArray(10);
        glVertexAttribIPointer(10, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float)));
        glVertexAttribDivisor(10, 1);
        glEnableVertexAttribArray(11);
        glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float) + 2 * sizeof(unsigned)));
        glVertexAttribDivisor(11, 1);
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeDefault)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeShadow]);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 1);
        glEnableVertexAttribArray(10);
        glVertexAttribIPointer(10, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float)));
        glVertexAttribDivisor(10, 1);
        glEnableVertexAttribArray(11);
        glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float) + 2 * sizeof(unsigned)));
        glVertexAttribDivisor(11, 1);
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeShadow)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeRSM]);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 1);
        glEnableVertexAttribArray(10);
        glVertexAttribIPointer(10, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float)));
        glVertexAttribDivisor(10, 1);
        glEnableVertexAttribArray(11);
        glVertexAttribIPointer(11, 2, GL_UNSIGNED_INT, sizeof(InstanceData), (GLvoid*)(9 * sizeof(float) + 2 * sizeof(unsigned)));
        glVertexAttribDivisor(11, 1);
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeRSM)] = vao;

        vao = createVAO(vbo[tp], ibo[tp], tp);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo[InstanceTypeGlow]);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(GlowInstanceData), 0);
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(GlowInstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(GlowInstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 1);
        glEnableVertexAttribArray(12);
        glVertexAttribPointer(12, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GlowInstanceData), (GLvoid*)(9 * sizeof(float)));
        glVertexAttribDivisor(12, 1);
        InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(tp, InstanceTypeGlow)] = vao;
        glBindVertexArray(0);
    }



}

size_t VAOManager::getVertexPitch(enum VTXTYPE tp) const
{
    switch (tp)
    {
    case VTXTYPE_STANDARD:
        return getVertexPitchFromType(video::EVT_STANDARD);
    case VTXTYPE_TCOORD:
        return getVertexPitchFromType(video::EVT_2TCOORDS);
    case VTXTYPE_TANGENT:
        return getVertexPitchFromType(video::EVT_TANGENTS);
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
    }
};

void VAOManager::append(scene::IMeshBuffer *mb, VTXTYPE tp)
{
    size_t old_vtx_cnt = vtx_cnt[tp];
    vtx_cnt[tp] += mb->getVertexCount();
    vtx_mirror[tp] = realloc(vtx_mirror[tp], vtx_cnt[tp] * getVertexPitch(tp));
    intptr_t dstptr = (intptr_t)vtx_mirror[tp] + (old_vtx_cnt * getVertexPitch(tp));
    memcpy((void *)dstptr, mb->getVertices(), mb->getVertexCount() * getVertexPitch(tp));
    mappedBaseVertex[tp][mb] = old_vtx_cnt;

    size_t old_idx_cnt = idx_cnt[tp];
    idx_cnt[tp] += mb->getIndexCount();
    idx_mirror[tp] = realloc(idx_mirror[tp], idx_cnt[tp] * sizeof(u16));

    dstptr = (intptr_t)idx_mirror[tp] + (old_idx_cnt * sizeof(u16));
    memcpy((void *)dstptr, mb->getIndices(), mb->getIndexCount() * sizeof(u16));
    mappedBaseIndex[tp][mb] = old_idx_cnt * sizeof(u16);
}

std::pair<unsigned, unsigned> VAOManager::getBase(scene::IMeshBuffer *mb)
{
    VTXTYPE tp = getVTXTYPE(mb->getVertexType());
    if (mappedBaseVertex[tp].find(mb) == mappedBaseVertex[tp].end())
    {
        assert(mappedBaseIndex[tp].find(mb) == mappedBaseIndex[tp].end());
        storedCPUBuffer[tp].push_back(mb);
        append(mb, tp);
        regenerateBuffer(tp);
        regenerateVAO(tp);
        regenerateInstancedVAO();
    }

    std::map<scene::IMeshBuffer*, unsigned>::iterator It;
    It = mappedBaseVertex[tp].find(mb);
    assert(It != mappedBaseVertex[tp].end());
    unsigned vtx = It->second;
    It = mappedBaseIndex[tp].find(mb);
    assert(It != mappedBaseIndex[tp].end());
    return std::pair<unsigned, unsigned>(vtx, It->second);
}
