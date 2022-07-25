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

#include "ge_animation.hpp"

#include <IMeshLoader.h>
#include <ISceneManager.h>
#include <ISkinnedMesh.h>
#include <IReadFile.h>
#include <array>
#include <vector>

using namespace irr;

class Material;

class SPMeshLoader : public scene::IMeshLoader
{
private:

    // ------------------------------------------------------------------------
    unsigned m_bind_frame, m_joint_count, m_frame_count;
    // ------------------------------------------------------------------------
    std::vector<GE::Armature> m_all_armatures;
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
    void decompressGESPM(irr::io::IReadFile* spm, unsigned vertices_count,
                         unsigned indices_count, bool read_normal,
                         bool read_vcolor, bool read_tangent, bool uv_one,
                         bool uv_two, SPVertexType vt,
                         const video::SMaterial& m);
    // ------------------------------------------------------------------------
    void decompressSPM(irr::io::IReadFile* spm, unsigned vertices_count,
                       unsigned indices_count, bool read_normal,
                       bool read_vcolor, bool read_tangent, bool uv_one,
                       bool uv_two, SPVertexType vt,
                       Material* m);
    // ------------------------------------------------------------------------
    void createAnimationData(irr::io::IReadFile* spm);
    // ------------------------------------------------------------------------
    void convertIrrlicht();

    scene::IAnimatedMesh* m_mesh;

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

