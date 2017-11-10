//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#ifndef HEADER_DRAW_CALLS_HPP
#define HEADER_DRAW_CALLS_HPP

#ifndef SERVER_ONLY
#include "graphics/material.hpp"
#include "graphics/vao_manager.hpp"
#include <irrlicht.h>
#include <set>
#include <unordered_map>

class GlowCommandBuffer;
class ReflectiveShadowMapCommandBuffer;
class ShadowMatrices;
class ShadowCommandBuffer;
class SolidCommandBuffer;
class STKAnimatedMesh;
class STKMeshCommon;

class DrawCalls
{
private:
    std::vector<STKMeshCommon *>          m_deferred_update;
    irr::core::vector3df                  m_wind_dir;
    GLsync                                m_sync;

    std::vector<irr::scene::ISceneNode *> m_immediate_draw_list;
    std::set<STKAnimatedMesh*>            m_mesh_for_skinning;

    std::vector<float>                    m_bounding_boxes;

    /** meshes to draw */
    MeshMap m_solid_pass_mesh            [    Material::SHADERTYPE_COUNT];
    MeshMap m_shadow_pass_mesh           [4 * Material::SHADERTYPE_COUNT];
    MeshMap m_reflective_shadow_map_mesh [    Material::SHADERTYPE_COUNT];
    MeshMap m_glow_pass_mesh;

#if !defined(USE_GLES2)
    /** meshes data in VRAM */
    SolidCommandBuffer                   *m_solid_cmd_buffer;
    ShadowCommandBuffer                  *m_shadow_cmd_buffer;
    ReflectiveShadowMapCommandBuffer     *m_reflective_shadow_map_cmd_buffer;
    GlowCommandBuffer                    *m_glow_cmd_buffer;
#endif // !defined(USE_GLES2)

    void clearLists();

    void handleSTKCommon(scene::ISceneNode *Node,
                         std::vector<scene::ISceneNode *> *ImmediateDraw,
                         const scene::ICameraSceneNode *cam,
                         ShadowMatrices& shadow_matrices);

    void parseSceneManager(core::list<scene::ISceneNode*> &List,
                           std::vector<scene::ISceneNode *> *ImmediateDraw,
                           const scene::ICameraSceneNode *cam,
                           ShadowMatrices& shadow_matrices);

    bool isCulledPrecise(const scene::ICameraSceneNode *cam,
                         const scene::ISceneNode* node,
                         bool visualization = false);

    bool isBoxInFrontOfPlane(const core::plane3df &plane,
                             const core::vector3df* edges);

    void addEdgeForViz(const core::vector3df &p0, const core::vector3df &p1);

public:
    DrawCalls();
    ~DrawCalls();

    void prepareDrawCalls(ShadowMatrices& shadow_matrices,
                          irr::scene::ICameraSceneNode *camnode,
                          unsigned &solid_poly_count,
                          unsigned &shadow_poly_count);
                          
    void setFenceSync() { m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); }

    void renderImmediateDrawList() const;
    void renderParticlesList() const;

    void drawIndirectSolidFirstPass() const;
    void multidrawSolidFirstPass() const;
    void drawIndirectSolidSecondPass(const std::vector<GLuint> &prefilled_tex) const;
    void multidrawSolidSecondPass(const std::vector<uint64_t> &handles) const;
    void drawIndirectNormals() const;
    void multidrawNormals() const;
    
    void drawIndirectShadows(unsigned cascade) const;
    void multidrawShadows(unsigned cascade) const;

    void drawIndirectReflectiveShadowMaps(const core::matrix4 &rsm_matrix) const;
    void multidrawReflectiveShadowMaps(const core::matrix4 &rsm_matrix) const;
    
    void drawIndirectGlow() const;
    void multidrawGlow() const;
    void renderBoundingBoxes();
    int32_t getSkinningOffset() const;
};

#endif   // !SERVER_ONLY
#endif //HEADER_DRAW_CALLS_HPP
