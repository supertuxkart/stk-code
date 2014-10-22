#ifndef VAOMANAGER_HPP
#define VAOMANAGER_HPP

#include "gl_headers.hpp"
#include "utils/singleton.hpp"
#include <S3DVertex.h>
#include <IMeshBuffer.h>
#include <vector>
#include <map>
#include <unordered_map>

enum InstanceType
{
    InstanceTypeDualTex,
    InstanceTypeThreeTex,
    InstanceTypeShadow,
    InstanceTypeRSM,
    InstanceTypeGlow,
    InstanceTypeCount,
};

#ifdef WIN32
#pragma pack(push, 1)
#endif
struct InstanceDataSingleTex
{
    struct
    {
        float X;
        float Y;
        float Z;
    } Origin;
    struct
    {
        float X;
        float Y;
        float Z;
    } Orientation;
    struct
    {
        float X;
        float Y;
        float Z;
    } Scale;
    uint64_t Texture;
#ifdef WIN32
};
#else
} __attribute__((packed));
#endif

struct InstanceDataDualTex
{
    struct
    {
        float X;
        float Y;
        float Z;
    } Origin;
    struct
    {
        float X;
        float Y;
        float Z;
    } Orientation;
    struct
    {
        float X;
        float Y;
        float Z;
    } Scale;
    uint64_t Texture;
    uint64_t SecondTexture;
#ifdef WIN32
};
#else
} __attribute__((packed));
#endif

struct InstanceDataThreeTex
{
    struct
    {
        float X;
        float Y;
        float Z;
    } Origin;
    struct
    {
        float X;
        float Y;
        float Z;
    } Orientation;
    struct
    {
        float X;
        float Y;
        float Z;
    } Scale;
    uint64_t Texture;
    uint64_t SecondTexture;
    uint64_t ThirdTexture;
#ifdef WIN32
};
#else
} __attribute__((packed));
#endif

struct GlowInstanceData
{
    struct
    {
        float X;
        float Y;
        float Z;
    } Origin;
    struct
    {
        float X;
        float Y;
        float Z;
    } Orientation;
    struct
    {
        float X;
        float Y;
        float Z;
    } Scale;
    unsigned Color;
#ifdef WIN32
};
#else
} __attribute__((packed));
#endif
#ifdef WIN32
#pragma pack(pop)
#endif

class VAOManager : public Singleton<VAOManager>
{
    enum VTXTYPE { VTXTYPE_STANDARD, VTXTYPE_TCOORD, VTXTYPE_TANGENT, VTXTYPE_COUNT };
    GLuint vbo[VTXTYPE_COUNT], ibo[VTXTYPE_COUNT], vao[VTXTYPE_COUNT];
    GLuint instance_vbo[InstanceTypeCount];
    void *Ptr[InstanceTypeCount];
    void *VBOPtr[VTXTYPE_COUNT], *IBOPtr[VTXTYPE_COUNT];
    size_t RealVBOSize[VTXTYPE_COUNT], RealIBOSize[VTXTYPE_COUNT];
    size_t last_vertex[VTXTYPE_COUNT], last_index[VTXTYPE_COUNT];
    std::unordered_map<irr::scene::IMeshBuffer*, unsigned> mappedBaseVertex[VTXTYPE_COUNT], mappedBaseIndex[VTXTYPE_COUNT];
    std::map<std::pair<irr::video::E_VERTEX_TYPE, InstanceType>, GLuint> InstanceVAO;

    void cleanInstanceVAOs();
    void regenerateBuffer(enum VTXTYPE, size_t, size_t);
    void regenerateVAO(enum VTXTYPE);
    void regenerateInstancedVAO();
    size_t getVertexPitch(enum VTXTYPE) const;
    VTXTYPE getVTXTYPE(irr::video::E_VERTEX_TYPE type);
    irr::video::E_VERTEX_TYPE getVertexType(enum VTXTYPE tp);
    void append(irr::scene::IMeshBuffer *, VTXTYPE tp);
public:
    VAOManager();
    std::pair<unsigned, unsigned> getBase(irr::scene::IMeshBuffer *);
    GLuint getInstanceBuffer(InstanceType it) { return instance_vbo[it]; }
    void *getInstanceBufferPtr(InstanceType it) { return Ptr[it]; }
    unsigned getVBO(irr::video::E_VERTEX_TYPE type) { return vbo[getVTXTYPE(type)]; }
    void *getVBOPtr(irr::video::E_VERTEX_TYPE type) { return VBOPtr[getVTXTYPE(type)]; }
    unsigned getVAO(irr::video::E_VERTEX_TYPE type) { return vao[getVTXTYPE(type)]; }
    unsigned getInstanceVAO(irr::video::E_VERTEX_TYPE vt, enum InstanceType it) { return InstanceVAO[std::pair<irr::video::E_VERTEX_TYPE, InstanceType>(vt, it)]; }
    ~VAOManager();
};

#endif