// Not really a scene manager yet but hold algorithm that
// rework scene manager output

#ifndef HEADER_STKSCENEMANAGER_HPP
#define HEADER_STKSCENEMANAGER_HPP

#include "utils/singleton.hpp"
#include "gl_headers.hpp"
#include "stkmesh.hpp"

template<typename T>
class CommandBuffer : public Singleton<T>
{
public:
    GLuint drawindirectcmd;
    DrawElementsIndirectCommand *Ptr;
    CommandBuffer()
    {
#ifdef Buffer_Storage
        glGenBuffers(1, &drawindirectcmd);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawindirectcmd);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, 10000 * sizeof(DrawElementsIndirectCommand), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
        Ptr = (DrawElementsIndirectCommand *)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, 10000 * sizeof(DrawElementsIndirectCommand), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
#endif
    }
};


class SolidPassCmd : public CommandBuffer<SolidPassCmd>
{
public:
    size_t Offset[MAT_COUNT], Size[MAT_COUNT];
};

class ShadowPassCmd : public CommandBuffer<ShadowPassCmd>
{
public:
    size_t Offset[4][MAT_COUNT], Size[4][MAT_COUNT];
};


#endif