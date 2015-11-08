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

#ifndef HEADER_SHADER_BASED_RENDERER_HPP
#define HEADER_SHADER_BASED_RENDERER_HPP

#include "graphics/abstract_renderer.hpp"
#include "graphics/draw_calls.hpp"
#include "graphics/geometry_passes.hpp"
#include "graphics/lighting_passes.hpp"
#include "graphics/shadow_matrices.hpp"


class ShaderBasedRenderer: public AbstractRenderer
{
private:
    
    //GLsync          m_sync; //TODO
    DrawCalls       m_draw_calls;
    GeometryPasses  m_geometry_passes;
    LightingPasses  m_lighting_passes;
    ShadowMatrices  m_shadow_matrices;

    irr::core::vector3df m_wind_dir;
    
    void compressPowerUpTextures();
    void setOverrideMaterial();
    std::vector<GlowData> updateGlowingList();
    void prepareForwardRenderer();
    
    void updateLightsInfo(irr::scene::ICameraSceneNode * const camnode,
                          float dt);

    void computeMatrixesAndCameras(scene::ICameraSceneNode * const camnode,
                                   size_t width, size_t height);
    
    void resetShadowCamNodes(){m_shadow_matrices.resetShadowCamNodes();}
    
    void prepareDrawCalls(scene::ICameraSceneNode *camnode);

    void renderScene(irr::scene::ICameraSceneNode * const camnode,
                     std::vector<GlowData>& glows,
                     float dt, bool hasShadows, bool forceRTT);
                     
    void renderParticles();

    void renderBoundingBoxes();
    void debugPhysics();
    void renderPostProcessing(Camera * const camera);


public:
    ShaderBasedRenderer();
    ~ShaderBasedRenderer();

    void addSunLight(const irr::core::vector3df &pos) override;
    
    void render(float dt);
};

#endif //HEADER_SHADER_BASED_RENDERER_HPP
