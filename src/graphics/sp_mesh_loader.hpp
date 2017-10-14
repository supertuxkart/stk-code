//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#ifndef HEADER_SP_MESH_LOADER_HPP
#define HEADER_SP_MESH_LOADER_HPP

#include <IMeshLoader.h>
#include <ISceneManager.h>
#include <ISkinnedMesh.h>
#include <IReadFile.h>
#include <array>
#include <vector>

using namespace irr;

class SPMeshLoader : public scene::IMeshLoader
{
private:
    // ------------------------------------------------------------------------
    struct LocRotScale
    {
        core::vector3df m_loc;

        core::quaternion m_rot;

        core::vector3df m_scale;
        // --------------------------------------------------------------------
        inline core::matrix4 toMatrix() const
        {
            core::matrix4 lm, sm, rm;
            lm.setTranslation(m_loc);
            sm.setScale(m_scale);
            m_rot.getMatrix_transposed(rm);
            return lm * rm * sm;
        }
        // --------------------------------------------------------------------
        void read(irr::io::IReadFile* spm);

    };
    struct Armature
    {
        unsigned m_joint_used;

        std::vector<std::string> m_joint_names;

        std::vector<core::matrix4> m_joint_matrices;

        std::vector<core::matrix4> m_interpolated_matrices;

        std::vector<std::pair<core::matrix4, bool> > m_world_matrices;

        std::vector<int> m_parent_infos;

        std::vector<std::pair<int, std::vector<LocRotScale> > >
            m_frame_pose_matrices;

        // --------------------------------------------------------------------
        void read(irr::io::IReadFile* spm);
        // --------------------------------------------------------------------
        void getPose(float frame, core::matrix4* dest);
        // --------------------------------------------------------------------
        void getInterpolatedMatrices(float frame);
        // --------------------------------------------------------------------
        core::matrix4 getWorldMatrix(const std::vector<core::matrix4>& matrix,
                                     unsigned id);
    };
    // ------------------------------------------------------------------------
    unsigned m_bind_frame, m_joint_count;//, m_frame_count;
    // ------------------------------------------------------------------------
    std::vector<Armature> m_all_armatures;
    // ------------------------------------------------------------------------
    std::vector<core::matrix4> m_to_bind_pose_matrices;
    // ------------------------------------------------------------------------
    enum SPVertexType: unsigned int
    {
        SPVT_NORMAL,
        SPVT_SKINNED
    };
    // ------------------------------------------------------------------------
    void decompress(irr::io::IReadFile* spm, unsigned vertices_count,
                    unsigned indices_count, bool read_normal, bool read_vcolor,
                    bool read_tangent, bool uv_one, bool uv_two,
                    SPVertexType vt, const video::SMaterial& m);
    // ------------------------------------------------------------------------
    void createAnimationData(irr::io::IReadFile* spm);
    // ------------------------------------------------------------------------
    void convertIrrlicht();

    scene::ISkinnedMesh* m_mesh;

    scene::ISceneManager* m_scene_manager;

    std::vector<std::vector<
        std::pair<std::array<short, 4>, std::array<float, 4> > > > m_joints;

public:
    // ------------------------------------------------------------------------
    SPMeshLoader(scene::ISceneManager* smgr) : m_scene_manager(smgr) {}
    // ------------------------------------------------------------------------
    virtual bool isALoadableFileExtension(const io::path& filename) const;
    // ------------------------------------------------------------------------
    virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);

};

#endif

