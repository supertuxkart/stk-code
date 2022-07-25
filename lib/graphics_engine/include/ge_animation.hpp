/* ==========================================================================
 * Copyright (c) 2022 SuperTuxKart-Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */

#ifndef HEADER_GE_ANIMATION_HPP
#define HEADER_GE_ANIMATION_HPP

#include <IReadFile.h>
#include <matrix4.h>
#include <quaternion.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>

using namespace irr;

namespace GE
{

struct LocRotScale
{
    core::vector3df m_loc;

    core::quaternion m_rot;

    core::vector3df m_scale;
    // ------------------------------------------------------------------------
    inline core::matrix4 toMatrix() const
    {
        core::matrix4 lm, sm, rm;
        lm.setTranslation(m_loc);
        sm.setScale(m_scale);
        m_rot.getMatrix(rm);
        return lm * rm * sm;
    }
    // ------------------------------------------------------------------------
    void read(irr::io::IReadFile* spm)
    {
        float tmp[10];
        spm->read(&tmp, 40);
        m_loc = core::vector3df(tmp[0], tmp[1], tmp[2]);
        m_rot = core::quaternion(tmp[3], tmp[4], tmp[5], tmp[6]);
        m_rot.normalize();
        m_scale = core::vector3df(tmp[7], tmp[8], tmp[9]);
    }
};

struct Armature
{
    unsigned m_joint_used;

    std::vector<std::string> m_joint_names;

    std::vector<core::matrix4> m_joint_matrices;

    std::vector<LocRotScale> m_interpolated_matrices;

    std::vector<std::pair<core::matrix4, bool> > m_world_matrices;

    std::vector<int> m_parent_infos;

    std::vector<std::pair<int, std::vector<LocRotScale> > >
        m_frame_pose_matrices;

    // ------------------------------------------------------------------------
    void read(irr::io::IReadFile* spm)
    {
        LocRotScale lrs;
        spm->read(&m_joint_used, 2);
        assert(m_joint_used > 0);
        unsigned all_joints_size = 0;
        spm->read(&all_joints_size, 2);
        assert(all_joints_size > 0);
        m_joint_names.resize(all_joints_size);
        for (unsigned i = 0; i < all_joints_size; i++)
        {
            unsigned str_len = 0;
            spm->read(&str_len, 1);
            m_joint_names[i].resize(str_len);
            spm->read(&m_joint_names[i].front(), str_len);
        }
        m_joint_matrices.resize(all_joints_size);
        m_interpolated_matrices.resize(all_joints_size);
        for (unsigned i = 0; i < all_joints_size; i++)
        {
            lrs.read(spm);
            m_joint_matrices[i] = lrs.toMatrix();
        }
        m_world_matrices.resize(m_interpolated_matrices.size(),
            std::make_pair(core::matrix4(), false));
        m_parent_infos.resize(all_joints_size);
        bool non_parent_bone = false;
        for (unsigned i = 0; i < all_joints_size; i++)
        {
            int16_t info = 0;
            spm->read(&info, 2);
            if (info == -1)
            {
                non_parent_bone = true;
            }
            m_parent_infos[i] = info;
        }
        if (!non_parent_bone)
        {
            printf("SPMeshLoader::Armature: Non-parent bone missing in armature");
            exit(-1);
        }
        unsigned frame_size = 0;
        spm->read(&frame_size, 2);
        m_frame_pose_matrices.resize(frame_size);
        for (unsigned i = 0; i < frame_size; i++)
        {
            m_frame_pose_matrices[i].second.resize(all_joints_size);
            unsigned frame_index = 0;
            spm->read(&frame_index, 2);
            m_frame_pose_matrices[i].first = frame_index;
            for (unsigned j = 0; j < m_frame_pose_matrices[i].second.size(); j++)
            {
                m_frame_pose_matrices[i].second[j].read(spm);
            }
        }
    }
    // ------------------------------------------------------------------------
    void getPose(float frame, core::matrix4* dest,
                 float frame_interpolating = -1.0f, float rate = -1.0f)
    {
        getInterpolatedMatrices(frame);
        if (frame_interpolating != -1.0f && rate != -1.0f)
        {
            auto copied = m_interpolated_matrices;
            getInterpolatedMatrices(frame_interpolating);
            for (unsigned i = 0; i < m_interpolated_matrices.size(); i++)
            {
                m_interpolated_matrices[i].m_loc =
                    copied[i].m_loc.getInterpolated(
                    m_interpolated_matrices[i].m_loc, rate);
                m_interpolated_matrices[i].m_rot =
                    m_interpolated_matrices[i].m_rot.slerp(
                    m_interpolated_matrices[i].m_rot, copied[i].m_rot, rate);
                m_interpolated_matrices[i].m_scale =
                    copied[i].m_scale.getInterpolated(
                    m_interpolated_matrices[i].m_scale, rate);
            }
        }
        for (auto& p : m_world_matrices)
        {
            p.second = false;
        }
        for (unsigned i = 0; i < m_joint_used; i++)
        {
            dest[i] = getWorldMatrix(m_interpolated_matrices, i) *
                m_joint_matrices[i];
        }
    }
    // ------------------------------------------------------------------------
    void getPose(core::matrix4* dest, float frame)
    {
        getInterpolatedMatrices(frame);
        for (auto& p : m_world_matrices)
        {
            p.second = false;
        }
        for (unsigned i = 0; i < m_joint_used; i++)
        {
            dest[i] = getWorldMatrix(m_interpolated_matrices, i) *
                m_joint_matrices[i];
        }
    }
    // ------------------------------------------------------------------------
    void getInterpolatedMatrices(float frame)
    {
        if (frame < float(m_frame_pose_matrices.front().first) ||
            frame >= float(m_frame_pose_matrices.back().first))
        {
            for (unsigned i = 0; i < m_interpolated_matrices.size(); i++)
            {
                m_interpolated_matrices[i] =
                    frame >= float(m_frame_pose_matrices.back().first) ?
                    m_frame_pose_matrices.back().second[i] :
                    m_frame_pose_matrices.front().second[i];
            }
            return;
        }
        int frame_1 = -1;
        int frame_2 = -1;
        float interpolation = 0.0f;
        for (unsigned i = 0; i < m_frame_pose_matrices.size(); i++)
        {
            assert(i + 1 < m_frame_pose_matrices.size());
            if (frame >= float(m_frame_pose_matrices[i].first) &&
                frame < float(m_frame_pose_matrices[i + 1].first))
            {
                frame_1 = i;
                frame_2 = i + 1;
                interpolation =
                    (frame - float(m_frame_pose_matrices[i].first)) /
                    float(m_frame_pose_matrices[i + 1].first -
                    m_frame_pose_matrices[i].first);
                break;
            }
        }
        assert(frame_1 != -1);
        assert(frame_2 != -1);
        for (unsigned i = 0; i < m_interpolated_matrices.size(); i++)
        {
            LocRotScale interpolated;
            interpolated.m_loc =
                m_frame_pose_matrices[frame_2].second[i].m_loc.getInterpolated
                (m_frame_pose_matrices[frame_1].second[i].m_loc, interpolation);
            interpolated.m_rot.slerp
                (m_frame_pose_matrices[frame_1].second[i].m_rot,
                m_frame_pose_matrices[frame_2].second[i].m_rot, interpolation);
            interpolated.m_scale =
                m_frame_pose_matrices[frame_2].second[i].m_scale.getInterpolated
                (m_frame_pose_matrices[frame_1].second[i].m_scale, interpolation);
            m_interpolated_matrices[i] = interpolated;
        }
    }
    // ------------------------------------------------------------------------
    core::matrix4 getWorldMatrix(const std::vector<LocRotScale>& lrs,
                                 unsigned id)
    {
        core::matrix4 mat = lrs[id].toMatrix();
        int parent_id = m_parent_infos[id];
        if (parent_id == -1)
        {
            m_world_matrices[id] = std::make_pair(mat, true);
            return mat;
        }
        if (!m_world_matrices[parent_id].second)
        {
            m_world_matrices[parent_id] = std::make_pair
                (getWorldMatrix(lrs, parent_id), true);
        }
        m_world_matrices[id] =
            std::make_pair(m_world_matrices[parent_id].first * mat, true);
        return m_world_matrices[id].first;
    }
};

}

#endif
