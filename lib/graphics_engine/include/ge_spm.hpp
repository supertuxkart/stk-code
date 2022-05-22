#ifndef HEADER_GE_SPM_HPP
#define HEADER_GE_SPM_HPP

#include <array>
#include <cassert>
#include <IAnimatedMesh.h>
#include <vector>

using namespace irr;
using namespace scene;

class B3DMeshLoader;
class SPMeshLoader;

namespace GE
{
class GESPMBuffer;

class GESPM : public IAnimatedMesh
{
friend class ::B3DMeshLoader;
friend class ::SPMeshLoader;
private:
    std::vector<GESPMBuffer*> m_buffer;

    core::aabbox3d<f32> m_bounding_box;

    float m_fps;

    unsigned m_bind_frame, m_total_joints, m_joint_using, m_frame_count;

public:
    // ------------------------------------------------------------------------
    GESPM();
    // ------------------------------------------------------------------------
    virtual ~GESPM();
    // ------------------------------------------------------------------------
    virtual u32 getFrameCount() const { return m_frame_count; }
    // ------------------------------------------------------------------------
    virtual f32 getAnimationSpeed() const { return m_fps; }
    // ------------------------------------------------------------------------
    virtual void setAnimationSpeed(f32 fps) { m_fps = fps; }
    // ------------------------------------------------------------------------
    virtual IMesh* getMesh(s32 frame, s32 detailLevel=255,
                           s32 startFrameLoop=-1, s32 endFrameLoop=-1)
                                                               { return this; }
    // ------------------------------------------------------------------------
    virtual u32 getMeshBufferCount() const
                                          { return (unsigned)m_buffer.size(); }
    // ------------------------------------------------------------------------
    virtual IMeshBuffer* getMeshBuffer(u32 nr) const;
    // ------------------------------------------------------------------------
    virtual IMeshBuffer* getMeshBuffer(const video::SMaterial &material) const;
    // ------------------------------------------------------------------------
    virtual const core::aabbox3d<f32>& getBoundingBox() const
                                                     { return m_bounding_box; }
    // ------------------------------------------------------------------------
    virtual void setBoundingBox(const core::aabbox3df& box)
                                                      { m_bounding_box = box; }
    // ------------------------------------------------------------------------
    virtual void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue) {}
    // ------------------------------------------------------------------------
    virtual void setHardwareMappingHint(E_HARDWARE_MAPPING newMappingHint,
                                        E_BUFFER_TYPE buffer) {}
    // ------------------------------------------------------------------------
    virtual void setDirty(E_BUFFER_TYPE buffer=EBT_VERTEX_AND_INDEX) {}
    // ------------------------------------------------------------------------
    virtual E_ANIMATED_MESH_TYPE getMeshType() const       { return EAMT_SPM; }
    // ------------------------------------------------------------------------
    virtual void finalize();

};

}
#endif
