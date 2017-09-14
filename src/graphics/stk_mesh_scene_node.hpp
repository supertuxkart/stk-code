//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_STK_MESH_SCENE_NODE
#define HEADER_STK_MESH_SCENE_NODE

#include "graphics/stk_mesh.hpp"

#include "utils/ptr_vector.hpp"

class RenderInfo;

class STKMeshSceneNode : public irr::scene::CMeshSceneNode, public STKMeshCommon
{
protected:
    PtrVector<RenderInfo> m_static_render_info;
    bool m_got_animated_matrix;
    std::vector<GLMesh> GLmeshes;
    core::matrix4 ModelViewProjectionMatrix;
    core::vector3df windDir;
    core::vector2df caustic_dir, caustic_dir2;

    // Misc passes shaders (glow, displace...)
    void drawGlow(const GLMesh &mesh);
    void createGLMeshes(RenderInfo* render_info = NULL, bool all_parts_colorized = false);
    void cleanGLMeshes();
    void setFirstTimeMaterial();
    void updatevbo();
    bool isMaterialInitialized;
    bool isGLInitialized;
    bool immediate_draw;
    bool additive;
    bool update_each_frame;
    bool isDisplacement;
    bool isGlow;
    video::SColor glowcolor;
public:
    virtual void updateNoGL();
    virtual void updateGL();
    void setReloadEachFrame(bool);
    STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,
        irr::s32 id, const std::string& debug_name,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool createGLMeshes = true,
        RenderInfo* render_info = NULL, bool all_parts_colorized = false);
    virtual void render();
    virtual void setMesh(irr::scene::IMesh* mesh);
    virtual void OnRegisterSceneNode();
    virtual ~STKMeshSceneNode();
    virtual bool isImmediateDraw() const { return immediate_draw; }
    void setIsDisplacement(bool v);
    virtual bool glow() const { return isGlow; }
    void setGlowColors(const video::SColor &c) { isGlow = true; glowcolor = c; }
    video::SColor getGlowColor() const { return glowcolor; }
};

#endif
