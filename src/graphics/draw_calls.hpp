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

#include "graphics/command_buffer.hpp"
#include "graphics/draw_tools.hpp"
#include "graphics/gpu_particles.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/stk_billboard.hpp"
#include "graphics/stk_mesh.hpp"
#include <irrlicht.h>

#include <unordered_map>


class DrawCalls
{
public:


    
    GLsync          m_sync = 0; //TODO: make it private. Should it be in shader_based_renderer?

    
private:
    
    std::vector<irr::scene::ISceneNode *> m_immediate_draw_list;
    std::vector<STKBillboard *>           m_billboard_list;
    std::vector<ParticleSystemProxy *>    m_particles_list;
    
    SolidCommandBuffer                    m_solid_cmd_buffer;
    ShadowCommandBuffer                   m_shadow_cmd_buffer;
    ReflectiveShadowMapCommandBuffer      m_reflective_shadow_map_cmd_buffer;
    GlowCommandBuffer                     m_glow_cmd_buffer;
    
    
    irr::core::vector3df windDir; //TODO: same member in geometry_passes
    
    MeshMap m_solid_pass_mesh           [Material::SHADERTYPE_COUNT]    ;
    MeshMap m_shadow_pass_mesh          [4 * Material::SHADERTYPE_COUNT] ;
    MeshMap m_reflective_shadow_map_mesh[Material::SHADERTYPE_COUNT]    ;
    MeshMap m_glow_pass_mesh;
    std::vector <STKMeshCommon *> m_deferred_update;
    
    
    void clearLists();
    
    template<Material::ShaderType Mat, typename T>
    void genDrawCalls(  unsigned cascade,
                        std::vector<GLMesh *> &InstancedList,
                        T *InstanceBuffer,
                        DrawElementsIndirectCommand *CommandBuffer,
                        size_t &InstanceBufferOffset,
                        size_t &CommandBufferOffset,
                        size_t &PolyCount);

    void handleSTKCommon(scene::ISceneNode *Node,
                                std::vector<scene::ISceneNode *> *ImmediateDraw,
                                const scene::ICameraSceneNode *cam,
                                scene::ICameraSceneNode *shadowcam[4],
                                const scene::ICameraSceneNode *rsmcam,
                                bool &culledforcam,
                                bool culledforshadowcam[4],
                                bool &culledforrsm,
                                bool drawRSM);
    
     void parseSceneManager(core::list<scene::ISceneNode*> &List,
                            std::vector<scene::ISceneNode *> *ImmediateDraw,
                            const scene::ICameraSceneNode* cam,
                            scene::ICameraSceneNode *shadow_cam[4],const scene::ICameraSceneNode *rsmcam,
                            bool culledforcam,
                            bool culledforshadowcam[4],
                            bool culledforrsm,
                            bool drawRSM);

    
public:
    void prepareDrawCalls(ShadowMatrices& shadow_matrices, irr::scene::ICameraSceneNode *camnode);

    void renderImmediateDrawList() const;
    void renderBillboardList()      const;
    void renderParticlesList()     const;

    inline void bindSolidCmd() const { m_solid_cmd_buffer.bind(); }
    inline void bindShadowCmd() const { m_shadow_cmd_buffer.bind(); }
    inline void bindReflectiveShadowMapsCmd() const { m_reflective_shadow_map_cmd_buffer.bind(); }

    
    
    inline bool isSolidCmdEmpty(Material::ShaderType shader_type) const
    { return m_solid_cmd_buffer.isEmpty(shader_type); }
    void drawIndirectSolidCmd(Material::ShaderType shader_type, int i) const;
    void multidrawIndirectSolidCmd(Material::ShaderType shader_type) const;
    
    void drawIndirectSolidFirstPass() const;
    //void multidrawIndirectSolidCmd() const; //TODO
    void drawIndirectSolidSecondPass(const std::vector<GLuint> &prefilled_tex) const;
    void drawIndirectShadows(unsigned cascade) const;
    void drawIndirectReflectiveShadowMaps(const core::matrix4 &rsm_matrix) const;
    
};

#endif //HEADER_DRAW_CALLS_HPP
