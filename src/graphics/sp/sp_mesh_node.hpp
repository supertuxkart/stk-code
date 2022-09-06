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

#ifndef HEADER_SP_MESH_NODE_HPP
#define HEADER_SP_MESH_NODE_HPP

#include "../../../lib/irrlicht/source/Irrlicht/CAnimatedMeshSceneNode.h"
#include <array>
#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

using namespace irr;
using namespace scene;
namespace GE { class GERenderInfo; }

namespace SP
{
class SPMesh;
class SPShader;

class SPMeshNode : public irr::scene::CAnimatedMeshSceneNode
{
private:
    std::vector<std::shared_ptr<GE::GERenderInfo> > m_render_info;

    std::shared_ptr<GE::GERenderInfo> m_first_render_info;

    std::unordered_map<std::string, IBoneSceneNode*> m_joint_nodes;

    SPMesh* m_mesh;

    int m_skinning_offset;

    bool m_animated;

    bool m_is_in_shadowpass;

    float m_saved_transition_frame;

    std::vector<core::matrix4> m_skinning_matrices;

    video::SColorf m_glow_color;

    std::vector<std::array<float, 2> > m_texture_matrices;

    // ------------------------------------------------------------------------
    void cleanRenderInfo();
    // ------------------------------------------------------------------------
    void cleanJoints()
    {
        for (auto& p : m_joint_nodes)
        {
            removeChild(p.second);
        }
        m_joint_nodes.clear();
        m_skinning_matrices.clear();
    }

public:
    // ------------------------------------------------------------------------
    SPMeshNode(IAnimatedMesh* mesh, ISceneNode* parent, ISceneManager* mgr,
               s32 id, const std::string& debug_name,
               const core::vector3df& position = core::vector3df(),
               const core::vector3df& rotation = core::vector3df(),
               const core::vector3df& scale = core::vector3df(1, 1, 1),
               std::shared_ptr<GE::GERenderInfo> render_info = nullptr);
    // ------------------------------------------------------------------------
    ~SPMeshNode();
    // ------------------------------------------------------------------------
    virtual void render() {}
    // ------------------------------------------------------------------------
    virtual void setMesh(irr::scene::IAnimatedMesh* mesh);
    // ------------------------------------------------------------------------
    virtual void OnAnimate(u32 time_ms);
    // ------------------------------------------------------------------------
    virtual void animateJoints(bool calculate_absolute_positions = true) {}
    // ------------------------------------------------------------------------
    virtual irr::scene::IMesh* getMeshForCurrentFrame();
    // ------------------------------------------------------------------------
    virtual IBoneSceneNode* getJointNode(const c8* joint_name);
    // ------------------------------------------------------------------------
    virtual IBoneSceneNode* getJointNode(u32 joint_id)
    {
        // SPM uses joint_name only
        assert(false);
        return NULL;
    }
    // ------------------------------------------------------------------------
    virtual void setJointMode(E_JOINT_UPDATE_ON_RENDER mode) {}
    // ------------------------------------------------------------------------
    virtual void checkJoints() {}
    // ------------------------------------------------------------------------
    SPMesh* getSPM() const { return m_mesh; }
    // ------------------------------------------------------------------------
    int getTotalJoints() const;
    // ------------------------------------------------------------------------
    void setSkinningOffset(int offset)          { m_skinning_offset = offset; }
    // ------------------------------------------------------------------------
    int getSkinningOffset() const                 { return m_skinning_offset; }
    // ------------------------------------------------------------------------
    void setAnimationState(bool val);
    // ------------------------------------------------------------------------
    bool getAnimationState() const                       { return m_animated; }
    // ------------------------------------------------------------------------
    bool isInShadowPass() const                  { return m_is_in_shadowpass; }
    // ------------------------------------------------------------------------
    void setInShadowPass(const bool is_in_shadowpass)
    {
        m_is_in_shadowpass = is_in_shadowpass;
    }
    // ------------------------------------------------------------------------
    SPShader* getShader(unsigned mesh_buffer_id) const;
    // ------------------------------------------------------------------------
    const core::matrix4* getSkinningMatrices() const 
                                         { return m_skinning_matrices.data(); }
    // ------------------------------------------------------------------------
    GE::GERenderInfo* getRenderInfo(unsigned mb_id) const
    {
        if (m_render_info.size() > mb_id && m_render_info[mb_id].get())
        {
            return m_render_info[mb_id].get();
        }
        return NULL;
    }
    // ------------------------------------------------------------------------
    virtual void resetFirstRenderInfo(std::shared_ptr<GE::GERenderInfo> ri)
    {
        m_render_info.clear();
        m_first_render_info = ri;
        m_render_info.resize(getMesh()->getMeshBufferCount(),
            m_first_render_info);
    }
    // ------------------------------------------------------------------------
    void setGlowColor(const video::SColorf& color)    { m_glow_color = color; }
    // ------------------------------------------------------------------------
    const video::SColorf& getGlowColor() const         { return m_glow_color; }
    // ------------------------------------------------------------------------
    bool hasGlowColor() const
    {
        return !(m_glow_color.r == 0.0f && m_glow_color.g == 0.0f &&
            m_glow_color.b == 0.0f);
    }
    // ------------------------------------------------------------------------
    std::array<float, 2>& getTextureMatrix(unsigned mb_id)
    {
        assert(mb_id < m_texture_matrices.size());
        return m_texture_matrices[mb_id];
    }
    // ------------------------------------------------------------------------
    void setTextureMatrix(unsigned mb_id, const std::array<float, 2>& tm)
    {
        assert(mb_id < m_texture_matrices.size());
        m_texture_matrices[mb_id] = tm;
    }
    // ------------------------------------------------------------------------
    virtual void setTransitionTime(f32 Time);
};

}

#endif
