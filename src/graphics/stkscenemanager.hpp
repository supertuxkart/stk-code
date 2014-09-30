// Not really a scene manager yet but hold algorithm that
// rework scene manager output

#ifndef HEADER_STKSCENEMANAGER_HPP
#define HEADER_STKSCENEMANAGER_HPP

#include "utils/singleton.hpp"
#include "gl_headers.hpp"
#include "stkmesh.hpp"
#include "gpuparticles.hpp"
#include "stkbillboard.hpp"

template<typename T>
class CommandBuffer : public Singleton<T>
{
public:
    GLuint drawindirectcmd;
    DrawElementsIndirectCommand *Ptr;
    CommandBuffer()
    {
        glGenBuffers(1, &drawindirectcmd);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawindirectcmd);
        if (irr_driver->hasBufferStorageExtension())
        {
            glBufferStorage(GL_DRAW_INDIRECT_BUFFER, 10000 * sizeof(DrawElementsIndirectCommand), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
            Ptr = (DrawElementsIndirectCommand *)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
        }
        else
        {
            glBufferData(GL_DRAW_INDIRECT_BUFFER, 10000 * sizeof(DrawElementsIndirectCommand), 0, GL_STREAM_DRAW);
        }
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