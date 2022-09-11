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

#include "graphics/sp_mesh_loader.hpp"

#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "utils/constants.hpp"
#include "mini_glm.hpp"
#include "utils/string_utils.hpp"

#include "../../lib/irrlicht/source/Irrlicht/CSkinnedMesh.h"
const uint8_t VERSION_NOW = 1;

#include <algorithm>
#include <cmath>
#include <IVideoDriver.h>
#include <IFileSystem.h>
#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_spm_buffer.hpp>
#include <ge_spm.hpp>
#endif

// ----------------------------------------------------------------------------
bool SPMeshLoader::isALoadableFileExtension(const io::path& filename) const
{
    return core::hasFileExtension(filename, "spm");
}   // isALoadableFileExtension

// ----------------------------------------------------------------------------
scene::IAnimatedMesh* SPMeshLoader::createMesh(io::IReadFile* f)
{
#ifndef SERVER_ONLY
    const bool real_spm = CVS->isGLSL();
#else
    const bool real_spm = false;
#endif
    if (!IS_LITTLE_ENDIAN)
    {
        Log::error("SPMeshLoader", "Not little endian machine.");
        return NULL;
    }
    if (f == NULL)
    {
        return NULL;
    }
    m_bind_frame = 0;
    m_joint_count = 0;
    m_frame_count = 0;
    m_mesh = NULL;
    bool ge_spm = false;
#ifndef SERVER_ONLY
    if (GE::getVKDriver())
    {
        ge_spm = true;
        m_mesh = new GE::GESPM();
    }
#endif
    if (!m_mesh)
    {
        m_mesh = real_spm ?
            new SP::SPMesh() : m_scene_manager->createSkinnedMesh();
    }
    io::IFileSystem* fs = m_scene_manager->getFileSystem();
    std::string base_path = fs->getFileDir(f->getFileName()).c_str();
    std::string header;
    header.resize(2);
    f->read(&header.front(), 2);
    if (header != "SP")
    {
        Log::error("SPMeshLoader", "Not a spm file.");
        m_mesh->drop();
        return NULL;
    }
    uint8_t byte = 0;
    f->read(&byte, 1);
    uint8_t version = byte >> 3;
    if (version != VERSION_NOW)
    {
        Log::error("SPMeshLoader", "Version mismatch, file %d SP %d", version,
            VERSION_NOW);
        m_mesh->drop();
        return NULL;
    }
    byte &= ~0x08;
    header = byte == 0 ? "SPMS" : byte == 1 ? "SPMA" : "SPMN";
    if (header == "SPMS")
    {
        Log::error("SPMeshLoader", "Space partitioned mesh not supported.");
        m_mesh->drop();
        return NULL;
    }
    f->read(&byte, 1);
    bool read_normal = byte & 0x01;
    bool read_vcolor = byte >> 1 & 0x01;
    bool read_tangent = byte >> 2 & 0x01;
    const bool is_skinned = header == "SPMA";
    const SPVertexType vt = is_skinned ? SPVT_SKINNED : SPVT_NORMAL;
    float bbox[6];
    f->read(bbox, 24);
    uint16_t size_num = 0;
    f->read(&size_num, 2);
    unsigned id = 0;
    std::unordered_map<unsigned, std::tuple<video::SMaterial, bool,
        bool> > mat_map;
    std::unordered_map<unsigned, std::tuple<Material*, bool,
        bool> > sp_mat_map;
    while (size_num != 0)
    {
        uint8_t tex_size;
        std::string tex_name_1, tex_name_2;
        f->read(&tex_size, 1);
        if (tex_size > 0)
        {
            tex_name_1.resize(tex_size);
            f->read(&tex_name_1.front(), tex_size);
        }
        f->read(&tex_size, 1);
        if (tex_size > 0)
        {
            tex_name_2.resize(tex_size);
            f->read(&tex_name_2.front(), tex_size);
        }
        if (real_spm)
        {
            if (!tex_name_1.empty())
            {
                std::string full_path = base_path + "/" + tex_name_1;
                if (fs->existFile(full_path.c_str()))
                {
                    tex_name_1 = full_path;
                }
            }
            sp_mat_map[id] =
                std::make_tuple(
                material_manager->getMaterialSPM(tex_name_1, tex_name_2),
                !tex_name_1.empty(), !tex_name_2.empty());
        }
        else
        {
            video::ITexture* textures[2] = { NULL, NULL };
            if (!tex_name_1.empty())
            {
                std::string full_path = base_path + "/" + tex_name_1;
                if (fs->existFile(full_path.c_str()))
                {
                    tex_name_1 = full_path;
                }
                std::function<void(irr::video::IImage*)> image_mani;
#ifndef SERVER_ONLY
                Material* m = material_manager->getMaterial(tex_name_1,
                    /*is_full_path*/false,
                    /*make_permanent*/false,
                    /*complain_if_not_found*/true,
                    /*strip_path*/true, /*install*/true,
                    /*create_if_not_found*/false);
                if (m)
                    image_mani = m->getMaskImageMani();
#endif
                video::ITexture* tex = STKTexManager::getInstance()
                    ->getTexture(tex_name_1, image_mani);
                if (tex != NULL)
                {
                    textures[0] = tex;
                }
            }
            if (!tex_name_2.empty())
            {
                std::string full_path = base_path + "/" + tex_name_2;
                if (fs->existFile(full_path.c_str()))
                {
                    tex_name_2 = full_path;
                }
                textures[1] = STKTexManager::getInstance()->getTexture
                    (tex_name_2);
            }

            video::SMaterial m;
            m.MaterialType = video::EMT_SOLID;
            if (textures[0] != NULL)
            {
                m.setTexture(0, textures[0]);
            }
            if (textures[1] != NULL)
            {
                m.setTexture(1, textures[1]);
            }
            mat_map[id] =
                std::make_tuple(m, !tex_name_1.empty(), !tex_name_2.empty());
        }
        size_num--;
        id++;
    }
    f->read(&size_num, 2);
    while (size_num != 0)
    {
        uint16_t mat_size;
        f->read(&mat_size, 2);
        while (mat_size != 0)
        {
            uint32_t vertices_count, indices_count;
            uint16_t mat_id;
            f->read(&vertices_count, 4);
            if (vertices_count > 65535)
            {
                Log::error("SPMeshLoader", "32bit index not supported.");
                m_mesh->drop();
                return NULL;
            }
            f->read(&indices_count, 4);
            f->read(&mat_id, 2);
            if (real_spm)
            {
                assert(mat_id < sp_mat_map.size());
                decompressSPM(f, vertices_count, indices_count, read_normal,
                    read_vcolor, read_tangent, std::get<1>(sp_mat_map[mat_id]),
                    std::get<2>(sp_mat_map[mat_id]), vt,
                    std::get<0>(sp_mat_map[mat_id]));
            }
            else if (ge_spm)
            {
                assert(mat_id < mat_map.size());
                decompressGESPM(f, vertices_count, indices_count, read_normal,
                    read_vcolor, read_tangent, std::get<1>(mat_map[mat_id]),
                    std::get<2>(mat_map[mat_id]), vt,
                    std::get<0>(mat_map[mat_id]));
            }
            else
            {
                assert(mat_id < mat_map.size());
                decompress(f, vertices_count, indices_count, read_normal,
                    read_vcolor, read_tangent, std::get<1>(mat_map[mat_id]),
                    std::get<2>(mat_map[mat_id]), vt,
                    std::get<0>(mat_map[mat_id]));
            }
            mat_size--;
        }
        if (header == "SPMS")
        {
            // Reserved, never used
            assert(false);
            f->read(bbox, 24);
        }
        size_num--;
    }

    // Calculate before finalize as spm has pre-computed straight frame
    Vec3 min, max;
    MeshTools::minMax3D(m_mesh, &min, &max);
    m_mesh->setMinMax(min.toIrrVector(), max.toIrrVector());

    if (header == "SPMA")
    {
        createAnimationData(f);
        convertIrrlicht();
    }
    else if (header == "SPMS")
    {
        // Reserved, never used
        assert(false);
        uint16_t pre_computed_size = 0;
        f->read(&pre_computed_size, 2);
    }
    const bool has_armature = !m_all_armatures.empty();
    if (real_spm)
    {
        SP::SPMesh* spm = static_cast<SP::SPMesh*>(m_mesh);
        spm->m_bind_frame = m_bind_frame;
        spm->m_joint_using = m_joint_count;
        // Because the last frame in spm is usable
        if (has_armature)
        {
            spm->m_frame_count = m_frame_count + 1;
        }
        for (unsigned i = 0; i < m_all_armatures.size(); i++)
        {
            // This is diffferent from m_joint_using
            spm->m_total_joints +=
                (unsigned)m_all_armatures[i].m_joint_names.size();
        }
        spm->m_all_armatures = std::move(m_all_armatures);
    }
#ifndef SERVER_ONLY
    else if (ge_spm)
    {
        GE::GESPM* spm = static_cast<GE::GESPM*>(m_mesh);
        spm->m_bind_frame = m_bind_frame;
        spm->m_joint_using = m_joint_count;
        // Because the last frame in spm is usable
        if (has_armature)
        {
            spm->m_frame_count = m_frame_count + 1;
        }
        for (unsigned i = 0; i < m_all_armatures.size(); i++)
        {
            // This is diffferent from m_joint_using
            spm->m_total_joints +=
                (unsigned)m_all_armatures[i].m_joint_names.size();
        }
        spm->m_all_armatures = std::move(m_all_armatures);
    }
#endif
    m_mesh->finalize();
    scene::CSkinnedMesh* cmesh = dynamic_cast<scene::CSkinnedMesh*>(m_mesh);
    if (cmesh && !real_spm && has_armature)
    {
        // Because the last frame in spm is usable
        cmesh->AnimationFrames = (float)m_frame_count + 1.0f;
    }
    m_all_armatures.clear();
    m_to_bind_pose_matrices.clear();
    m_joints.clear();
    return m_mesh;
}   // createMesh

// ----------------------------------------------------------------------------
void SPMeshLoader::decompressSPM(irr::io::IReadFile* spm,
                                 unsigned vertices_count,
                                 unsigned indices_count, bool read_normal,
                                 bool read_vcolor, bool read_tangent,
                                 bool uv_one, bool uv_two, SPVertexType vt,
                                 Material* m)
{
    assert(vertices_count != 0);
    assert(indices_count != 0);

    using namespace SP;
    SPMeshBuffer* mb = new SPMeshBuffer();
    static_cast<SPMesh*>(m_mesh)->m_buffer.push_back(mb);
    const unsigned idx_size = vertices_count > 255 ? 2 : 1;
    for (unsigned i = 0; i < vertices_count; i++)
    {
        video::S3DVertexSkinnedMesh vertex = {};
        // 3 * float position
        spm->read(&vertex.m_position, 12);
        if (read_normal)
        {
            spm->read(&vertex.m_normal, 4);
        }
        else
        {
            // 0, 1, 0
            vertex.m_normal = 0x1FF << 10;
        }
        if (read_vcolor)
        {
            // Color identifier
            uint8_t ci;
            spm->read(&ci, 1);
            if (ci == 128)
            {
                // All white
                vertex.m_color = video::SColor(255, 255, 255, 255);
            }
            else
            {
                uint8_t r, g, b;
                spm->read(&r, 1);
                spm->read(&g, 1);
                spm->read(&b, 1);
                vertex.m_color = video::SColor(255, r, g, b);
            }
        }
        else
        {
            vertex.m_color = video::SColor(255, 255, 255, 255);
        }
        if (uv_one)
        {
            spm->read(&vertex.m_all_uvs[0], 4);
            if (uv_two)
            {
                spm->read(&vertex.m_all_uvs[2], 4);
            }
            if (read_tangent)
            {
                spm->read(&vertex.m_tangent, 4);
            }
            else
            {
                vertex.m_tangent = MiniGLM::quickTangent(vertex.m_normal);
            }
        }
        if (vt == SPVT_SKINNED)
        {
            spm->read(&vertex.m_joint_idx[0], 16);
            if (vertex.m_joint_idx[0] == -1 ||
                vertex.m_weight[0] == 0 ||
                // -0.0 in half float (16bit)
                vertex.m_weight[0] == -32768)
            {
                // For the skinned mesh shader
                vertex.m_joint_idx[0] = -32767;
                // 1.0 in half float (16bit)
                vertex.m_weight[0] = 15360;
            }
        }
        mb->addSPMVertex(vertex);
    }

    std::vector<uint16_t> indices;
    indices.resize(indices_count);
    if (idx_size == 2)
    {
        spm->read(indices.data(), indices_count * 2);
    }
    else
    {
        std::vector<uint8_t> tmp_idx;
        tmp_idx.resize(indices_count);
        spm->read(tmp_idx.data(), indices_count);
        for (unsigned i = 0; i < indices_count; i++)
        {
            indices[i] = tmp_idx[i];
        }
    }
    mb->setIndices(indices);
    mb->setSTKMaterial(m);

}   // decompressSPM

// ----------------------------------------------------------------------------
void SPMeshLoader::decompressGESPM(irr::io::IReadFile* spm,
                                   unsigned vertices_count,
                                   unsigned indices_count, bool read_normal,
                                   bool read_vcolor, bool read_tangent,
                                   bool uv_one, bool uv_two, SPVertexType vt,
                                   const video::SMaterial& m)
{
#ifndef SERVER_ONLY
    assert(vertices_count != 0);
    assert(indices_count != 0);

    GE::GESPMBuffer* mb = new GE::GESPMBuffer();
    static_cast<GE::GESPM*>(m_mesh)->addMeshBuffer(mb);
    const unsigned idx_size = vertices_count > 255 ? 2 : 1;
    for (unsigned i = 0; i < vertices_count; i++)
    {
        video::S3DVertexSkinnedMesh vertex = {};
        // 3 * float position
        spm->read(&vertex.m_position, 12);
        if (read_normal)
        {
            spm->read(&vertex.m_normal, 4);
        }
        else
        {
            // 0, 1, 0
            vertex.m_normal = 0x1FF << 10;
        }
        if (read_vcolor)
        {
            // Color identifier
            uint8_t ci;
            spm->read(&ci, 1);
            if (ci == 128)
            {
                // All white
                vertex.m_color = video::SColor(255, 255, 255, 255);
            }
            else
            {
                uint8_t r, g, b;
                spm->read(&r, 1);
                spm->read(&g, 1);
                spm->read(&b, 1);
                vertex.m_color = video::SColor(255, r, g, b);
            }
        }
        else
        {
            vertex.m_color = video::SColor(255, 255, 255, 255);
        }
        if (uv_one)
        {
            spm->read(&vertex.m_all_uvs[0], 4);
            if (uv_two)
            {
                spm->read(&vertex.m_all_uvs[2], 4);
            }
            if (read_tangent)
            {
                spm->read(&vertex.m_tangent, 4);
            }
            else
            {
                vertex.m_tangent = MiniGLM::quickTangent(vertex.m_normal);
            }
        }
        if (vt == SPVT_SKINNED)
        {
            spm->read(&vertex.m_joint_idx[0], 16);
            if (vertex.m_joint_idx[0] == -1 ||
                vertex.m_weight[0] == 0 ||
                // -0.0 in half float (16bit)
                vertex.m_weight[0] == -32768)
            {
                // For the skinned mesh shader (reserve 1000 bones for offsets)
                vertex.m_joint_idx[0] = -31768;
                // 1.0 in half float (16bit)
                vertex.m_weight[0] = 15360;
            }
            mb->setHasSkinning(true);
        }
        mb->getVerticesVector().push_back(vertex);
    }

    std::vector<uint16_t>& indices = mb->getIndicesVector();
    indices.resize(indices_count);
    if (idx_size == 2)
    {
        spm->read(indices.data(), indices_count * 2);
    }
    else
    {
        std::vector<uint8_t> tmp_idx;
        tmp_idx.resize(indices_count);
        spm->read(tmp_idx.data(), indices_count);
        for (unsigned i = 0; i < indices_count; i++)
        {
            indices[i] = tmp_idx[i];
        }
    }
    if (m.TextureLayer[0].Texture != NULL)
    {
        mb->getMaterial() = m;
    }
    mb->recalculateBoundingBox();
#endif
}   // decompressGESPM

// ----------------------------------------------------------------------------
void SPMeshLoader::decompress(irr::io::IReadFile* spm, unsigned vertices_count,
                              unsigned indices_count, bool read_normal,
                              bool read_vcolor, bool read_tangent, bool uv_one,
                              bool uv_two, SPVertexType vt,
                              const video::SMaterial& m)
{
    assert(vertices_count != 0);
    assert(indices_count != 0);
    scene::SSkinMeshBuffer* mb =
        static_cast<scene::CSkinnedMesh*>(m_mesh)->addMeshBuffer();
    if (uv_two)
    {
        mb->convertTo2TCoords();
    }
    using namespace MiniGLM;
    const unsigned idx_size = vertices_count > 255 ? 2 : 1;
    char tmp[8] = {};
    std::vector<std::pair<std::array<short, 4>, std::array<float, 4> > >
        cur_joints;
    for (unsigned i = 0; i < vertices_count; i++)
    {
        video::S3DVertex2TCoords vertex;
        // 3 * float position
        spm->read(&vertex.Pos, 12);
        if (read_normal)
        {
            // 3 10 + 2 bits normal
            uint32_t packed;
            spm->read(&packed, 4);
            vertex.Normal = decompressVector3(packed);
        }
        if (read_vcolor)
        {
            // Color identifier
            uint8_t ci;
            spm->read(&ci, 1);
            if (ci == 128)
            {
                // All white
                vertex.Color = video::SColor(255, 255, 255, 255);
            }
            else
            {
                uint8_t r, g, b;
                spm->read(&r, 1);
                spm->read(&g, 1);
                spm->read(&b, 1);
                vertex.Color = video::SColor(255, r, g, b);
            }
        }
        else
        {
            vertex.Color = video::SColor(255, 255, 255, 255);
        }
        if (uv_one)
        {
            short hf[2];
            spm->read(hf, 4);
            vertex.TCoords.X = toFloat32(hf[0]);
            vertex.TCoords.Y = toFloat32(hf[1]);
            assert(!std::isnan(vertex.TCoords.X));
            assert(!std::isnan(vertex.TCoords.Y));
            if (uv_two)
            {
                spm->read(hf, 4);
                vertex.TCoords2.X = toFloat32(hf[0]);
                vertex.TCoords2.Y = toFloat32(hf[1]);
                assert(!std::isnan(vertex.TCoords2.X));
                assert(!std::isnan(vertex.TCoords2.Y));
            }
            if (read_tangent)
            {
                uint32_t packed;
                spm->read(&packed, 4);
            }
        }
        if (vt == SPVT_SKINNED)
        {
            std::array<short, 4> joint_idx;
            spm->read(joint_idx.data(), 8);
            spm->read(tmp, 8);
            std::array<float, 4> joint_weight = {};
            for (int j = 0; j < 8; j += 2)
            {
                short hf;
                memcpy(&hf, tmp + j, 2);
                const unsigned idx = j >> 1;
                joint_weight[idx] = toFloat32(hf);
                assert(!std::isnan(joint_weight[idx]));
            }
            cur_joints.emplace_back(joint_idx, joint_weight);
        }
        if (uv_two)
        {
            mb->Vertices_2TCoords.push_back(vertex);
        }
        else
        {
            mb->Vertices_Standard.push_back(vertex);
        }
    }
    if (vt == SPVT_SKINNED)
    {
        m_joints.emplace_back(std::move(cur_joints));
    }
    if (m.TextureLayer[0].Texture != NULL)
    {
        mb->Material = m;
    }
    mb->Indices.set_used(indices_count);
    if (idx_size == 2)
    {
        spm->read(mb->Indices.pointer(), indices_count * 2);
    }
    else
    {
        std::vector<uint8_t> tmp_idx;
        tmp_idx.resize(indices_count);
        spm->read(tmp_idx.data(), indices_count);
        for (unsigned i = 0; i < indices_count; i++)
        {
            mb->Indices[i] = tmp_idx[i];
        }
    }

    if (!read_normal)
    {
        for (unsigned i = 0; i < mb->Indices.size(); i += 3)
        {
            core::plane3df p(mb->getVertex(mb->Indices[i])->Pos,
                mb->getVertex(mb->Indices[i + 1])->Pos,
                mb->getVertex(mb->Indices[i + 2])->Pos);
            mb->getVertex(mb->Indices[i])->Normal += p.Normal;
            mb->getVertex(mb->Indices[i + 1])->Normal += p.Normal;
            mb->getVertex(mb->Indices[i + 2])->Normal += p.Normal;
        }
        for (unsigned i = 0; i < mb->getVertexCount(); i++)
        {
            mb->getVertex(i)->Normal.normalize();
        }
    }
}   // decompress

// ----------------------------------------------------------------------------
void SPMeshLoader::createAnimationData(irr::io::IReadFile* spm)
{
    uint8_t armature_size = 0;
    spm->read(&armature_size, 1);
    assert(armature_size > 0);
    m_bind_frame = 0;
    spm->read(&m_bind_frame, 2);
    m_all_armatures.resize(armature_size);
    for (unsigned i = 0; i < armature_size; i++)
    {
        m_all_armatures[i].read(spm);
    }
    for (unsigned i = 0; i < armature_size; i++)
    {
        m_frame_count = std::max(m_frame_count,
            (unsigned)m_all_armatures[i].m_frame_pose_matrices.back().first);
        m_joint_count += m_all_armatures[i].m_joint_used;
    }

    m_to_bind_pose_matrices.resize(m_joint_count);
    unsigned accumulated_joints = 0;
    for (unsigned i = 0; i < armature_size; i++)
    {
        m_all_armatures[i].getPose(&m_to_bind_pose_matrices[accumulated_joints],
            (float)m_bind_frame);
        accumulated_joints += m_all_armatures[i].m_joint_used;
    }

    scene::ISkinnedMesh* smesh = dynamic_cast<scene::ISkinnedMesh*>(m_mesh);
    // Only for legacy device
    if (!smesh || m_joints.empty())
    {
        return;
    }
    assert(m_joints.size() == smesh->getMeshBufferCount());
    for (unsigned i = 0; i < m_to_bind_pose_matrices.size(); i++)
    {
        m_to_bind_pose_matrices[i].makeInverse();
    }
    for (unsigned i = 0; i < smesh->getMeshBufferCount(); i++)
    {
        for (unsigned j = 0; j < m_joints[i].size(); j++)
        {
            if (!(m_joints[i][j].first[0] == -1 ||
                m_joints[i][j].second[0] == 0.0f))
            {
                core::vector3df bind_pos, bind_nor;
                for (unsigned k = 0; k < 4; k++)
                {
                    if (m_joints[i][j].second[k] == 0.0f)
                    {
                        break;
                    }
                    core::vector3df cur_pos, cur_nor;
                    m_to_bind_pose_matrices[m_joints[i][j].first[k]]
                        .transformVect(cur_pos,
                        smesh->getMeshBuffers()[i]->getVertex(j)->Pos);
                    bind_pos += cur_pos * m_joints[i][j].second[k];
                    m_to_bind_pose_matrices[m_joints[i][j].first[k]]
                        .rotateVect(cur_nor,
                        smesh->getMeshBuffers()[i]->getVertex(j)->Normal);
                    bind_nor += cur_nor * m_joints[i][j].second[k];
                }
                smesh->getMeshBuffers()[i]->getVertex(j)->Pos = bind_pos;
                smesh->getMeshBuffers()[i]->getVertex(j)->Normal = bind_nor;
            }
        }
    }
}   // createAnimationData

// ----------------------------------------------------------------------------
void SPMeshLoader::convertIrrlicht()
{
    scene::ISkinnedMesh* smesh = dynamic_cast<scene::ISkinnedMesh*>(m_mesh);
    // Only for legacy device
    if (!smesh || m_joints.empty())
    {
        return;
    }
    unsigned total_joints = 0;
    for (unsigned i = 0; i < m_all_armatures.size(); i++)
    {
        total_joints += (unsigned)m_all_armatures[i].m_joint_names.size();
    }
    for (unsigned i = 0; i < total_joints; i++)
    {
        smesh->addJoint(NULL);
    }
    core::array<scene::ISkinnedMesh::SJoint*>& joints = smesh->getAllJoints();
    std::vector<int> used_joints_map;
    used_joints_map.resize(m_joint_count);
    total_joints = 0;
    unsigned used_joints = 0;
    for (unsigned i = 0; i < m_all_armatures.size(); i++)
    {
        for (unsigned j = 0; j < m_all_armatures[i].m_joint_names.size(); j++)
        {
            if (m_all_armatures[i].m_joint_used > j)
            {
                used_joints_map[used_joints + j] = total_joints + j;
            }
            joints[total_joints + j]->Name =
                m_all_armatures[i].m_joint_names[j].c_str();
            const int p_id = m_all_armatures[i].m_parent_infos[j];
            core::matrix4 tmp;
            if (p_id != -1)
            {
                joints[total_joints + p_id]->Children.push_back
                    (joints[total_joints + j]);
                m_all_armatures[i].m_joint_matrices[j].getInverse(tmp);
                tmp = m_all_armatures[i].m_joint_matrices[p_id] * tmp;
            }
            else
            {
                m_all_armatures[i].m_joint_matrices[j].getInverse(tmp);
            }
            joints[total_joints + j]->LocalMatrix = tmp;
            for (unsigned k = 0; k <
                m_all_armatures[i].m_frame_pose_matrices.size(); k++)
            {
                float frame = (float)
                    m_all_armatures[i].m_frame_pose_matrices[k].first;
                core::vector3df pos = m_all_armatures[i]
                    .m_frame_pose_matrices[k].second[j].m_loc;
                core::quaternion q = m_all_armatures[i]
                    .m_frame_pose_matrices[k].second[j].m_rot;
                core::vector3df scl = m_all_armatures[i]
                    .m_frame_pose_matrices[k].second[j].m_scale;
                joints[total_joints + j]->PositionKeys.push_back({frame, pos});
                joints[total_joints + j]->RotationKeys.push_back({frame,
                    // Reverse for broken irrlicht quaternion
                    core::quaternion(q.X, q.Y, q.Z, -q.W)});
                joints[total_joints + j]->ScaleKeys.push_back({frame, scl});
            }
        }
        total_joints += (unsigned)m_all_armatures[i].m_joint_names.size();
        used_joints += m_all_armatures[i].m_joint_used;
    }

    for (unsigned i = 0; i < m_joints.size(); i++)
    {
        for (unsigned j = 0; j < m_joints[i].size(); j++)
        {
            for (unsigned k = 0; k < 4; k++)
            {
                if (m_joints[i][j].first[k] == -1 ||
                    m_joints[i][j].second[k] == 0.0f)
                {
                    break;
                }
                scene::ISkinnedMesh::SWeight* w = smesh->addWeight
                    (joints[used_joints_map[m_joints[i][j].first[k]]]);
                w->buffer_id = (uint16_t)i;
                w->vertex_id = j;
                w->strength = m_joints[i][j].second[k];
            }
        }
    }

}   // convertIrrlicht
