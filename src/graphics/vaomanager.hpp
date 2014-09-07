#ifndef VAOMANAGER_HPP
#define VAOMANAGER_HPP

#include "gl_headers.hpp"
#include "utils/singleton.hpp"
#include "irr_driver.hpp"
#include <vector>
#include <map>
#include <unordered_map>

enum InstanceType
{
    InstanceTypeDefault,
    InstanceTypeShadow,
    InstanceTypeRSM,
    InstanceTypeGlow,
    InstanceTypeCount,
};

#ifdef WIN32
#pragma pack(push, 1)
#endif
struct InstanceData
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
    size_t instance_count[InstanceTypeCount];
    void *Ptr[InstanceTypeCount];
    void *VBOPtr[VTXTYPE_COUNT];
    std::vector<scene::IMeshBuffer *> storedCPUBuffer[VTXTYPE_COUNT];
    void *vtx_mirror[VTXTYPE_COUNT], *idx_mirror[VTXTYPE_COUNT];
    size_t vtx_cnt[VTXTYPE_COUNT], idx_cnt[VTXTYPE_COUNT];
    std::unordered_map<scene::IMeshBuffer*, unsigned> mappedBaseVertex[VTXTYPE_COUNT], mappedBaseIndex[VTXTYPE_COUNT];
    std::map<std::pair<video::E_VERTEX_TYPE, InstanceType>, GLuint> InstanceVAO;

    void cleanInstanceVAOs();
    void regenerateBuffer(enum VTXTYPE);
    void regenerateVAO(enum VTXTYPE);
    void regenerateInstancedVAO();
    size_t getVertexPitch(enum VTXTYPE) const;
    VTXTYPE getVTXTYPE(video::E_VERTEX_TYPE type);
    void append(scene::IMeshBuffer *, VTXTYPE tp);
public:
    VAOManager();
    std::pair<unsigned, unsigned> getBase(scene::IMeshBuffer *);
    GLuint getInstanceBuffer(InstanceType it) { return instance_vbo[it]; }
    void *getInstanceBufferPtr(InstanceType it) { return Ptr[it]; }
    unsigned getVBO(video::E_VERTEX_TYPE type) { return vbo[getVTXTYPE(type)]; }
    void *getVBOPtr(video::E_VERTEX_TYPE type) { return VBOPtr[getVTXTYPE(type)]; }
    unsigned getVAO(video::E_VERTEX_TYPE type) { return vao[getVTXTYPE(type)]; }
    unsigned getInstanceVAO(video::E_VERTEX_TYPE vt, enum InstanceType it) { return InstanceVAO[std::pair<video::E_VERTEX_TYPE, InstanceType>(vt, it)]; }
    ~VAOManager();
};

#endif