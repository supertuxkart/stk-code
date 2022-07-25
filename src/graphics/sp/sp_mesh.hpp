//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef HEADER_SP_MESH_HPP
#define HEADER_SP_MESH_HPP

#include <array>
#include <cassert>
#include <IAnimatedMeshSceneNode.h>
#include <ISkinnedMesh.h>
#include <vector>


using namespace irr;
using namespace scene;

class B3DMeshLoader;
class SPMeshLoader;

namespace GE
{
    struct Armature;
}

namespace SP
{
class SPMeshBuffer;

class SPMesh : public ISkinnedMesh
{
friend class ::B3DMeshLoader;
friend class ::SPMeshLoader;
private:
    std::vector<SPMeshBuffer*> m_buffer;

    core::aabbox3d<f32> m_bounding_box;

    float m_fps;

    unsigned m_bind_frame, m_total_joints, m_joint_using, m_frame_count;

    std::vector<GE::Armature> m_all_armatures;

public:
    // ------------------------------------------------------------------------
    SPMesh();
    // ------------------------------------------------------------------------
    virtual ~SPMesh();
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
    virtual void animateMesh(f32 frame, f32 blend) {}
    // ------------------------------------------------------------------------
    virtual void skinMesh(f32 strength = 1.0f) {}
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
    virtual E_ANIMATED_MESH_TYPE getMeshType() const { return EAMT_SKINNED; }
    // ------------------------------------------------------------------------
    virtual u32 getJointCount() const { return m_joint_using; }
    // ------------------------------------------------------------------------
    virtual const c8* getJointName(u32 number) const;
    // ------------------------------------------------------------------------
    virtual s32 getJointNumber(const c8* name) const
    {
        // Real SPM doesn't use this
        return -1;
    }
    // ------------------------------------------------------------------------
    virtual bool useAnimationFrom(const ISkinnedMesh *mesh) { return true; }
    // ------------------------------------------------------------------------
    virtual void updateNormalsWhenAnimating(bool on) {}
    // ------------------------------------------------------------------------
    virtual void setInterpolationMode(E_INTERPOLATION_MODE mode) {}
    // ------------------------------------------------------------------------
    virtual bool isStatic() { return m_all_armatures.empty(); }
    // ------------------------------------------------------------------------
    virtual bool setHardwareSkinning(bool on) { return true; }
    // ------------------------------------------------------------------------
    virtual core::array<SSkinMeshBuffer*>& getMeshBuffers()
    {
        assert(false);
        static auto unused = core::array<SSkinMeshBuffer*>();
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual core::array<SJoint*>& getAllJoints()
    {
        assert(false);
        static auto unused = core::array<SJoint*>();
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual const core::array<SJoint*>& getAllJoints() const
    {
        assert(false);
        static auto unused = core::array<SJoint*>();
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual void finalize();
    // ------------------------------------------------------------------------
    virtual SSkinMeshBuffer *addMeshBuffer() { return NULL; }
    // ------------------------------------------------------------------------
    virtual SJoint *addJoint(SJoint *parent) { return NULL; }
    // ------------------------------------------------------------------------
    virtual SPositionKey *addPositionKey(SJoint *joint) { return NULL; }
    // ------------------------------------------------------------------------
    virtual SRotationKey *addRotationKey(SJoint *joint) { return NULL; }
    // ------------------------------------------------------------------------
    virtual SScaleKey *addScaleKey(SJoint *joint) { return NULL; }
    // ------------------------------------------------------------------------
    virtual SWeight *addWeight(SJoint *joint) { return NULL; }
    // ------------------------------------------------------------------------
    virtual void updateBoundingBox(void);
    // ------------------------------------------------------------------------
    std::vector<GE::Armature>& getArmatures() { return m_all_armatures; }
    // ------------------------------------------------------------------------
    void getSkinningMatrices(f32 frame, std::vector<core::matrix4>& dest,
                        float frame_interpolating = -1.0f, float rate = -1.0f);
    // ------------------------------------------------------------------------
    s32 getJointIDWithArm(const c8* name, unsigned* arm_id) const;
    // ------------------------------------------------------------------------
    void addSPMeshBuffer(SPMeshBuffer* spmb)      { m_buffer.push_back(spmb); }
    // ------------------------------------------------------------------------
    SPMeshBuffer* getSPMeshBuffer(u32 nr) const;

};

}
#endif
