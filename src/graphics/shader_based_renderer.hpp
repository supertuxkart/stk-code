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
#include "graphics/draw_policies.hpp"
#include "graphics/geometry_passes.hpp"
#include "graphics/lighting_passes.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/skybox.hpp"


class ShaderBasedRenderer: public AbstractRenderer
{
private:
    Skybox *m_skybox;
    SphericalHarmonics *m_spherical_harmonics;

    
    //GLsync          m_sync; //TODO
    DrawCalls       m_draw_calls;
    AbstractGeometryPasses *m_geometry_passes;
    LightingPasses  m_lighting_passes;
    ShadowMatrices  m_shadow_matrices;

    /** Static glowing things are loaded once per track.
     * Glowing items can appear ordisappear each frame */
    std::vector<GlowData> m_glowing;
    size_t m_nb_static_glowing;

    irr::core::vector3df m_wind_dir;
    
    void compressPowerUpTextures();
    void setOverrideMaterial();
    
    void addItemsInGlowingList();
    void removeItemsInGlowingList();
    
    void prepareForwardRenderer();
    
    void updateLightsInfo(irr::scene::ICameraSceneNode * const camnode,
                          float dt);
                          
    void uploadLightingData() const override;

    void computeMatrixesAndCameras(scene::ICameraSceneNode * const camnode,
                                   size_t width, size_t height);
    
    void resetShadowCamNodes(){m_shadow_matrices.resetShadowCamNodes();}
    
    void renderSkybox(const scene::ICameraSceneNode *camera) const;
    
    
    void prepareDrawCalls(scene::ICameraSceneNode *camnode);

    void renderSSAO() const;

    void renderScene(irr::scene::ICameraSceneNode * const camnode,
                     float dt, bool hasShadows, bool forceRTT);
                     
    void renderParticles();

    void renderBoundingBoxes();
    void debugPhysics();
    void renderPostProcessing(Camera * const camera);


public:
    ShaderBasedRenderer();
    ~ShaderBasedRenderer();

    void addSkyBox(const std::vector<irr::video::ITexture*> &texture,
                   const std::vector<irr::video::ITexture*> &spherical_harmonics_textures) override;
    void removeSkyBox() override;
    const SHCoefficients* getSHCoefficients() const override;
    void                  setAmbientLight(const irr::video::SColorf &light,
                                          bool force_SH_computation = true) override;

    void addSunLight(const irr::core::vector3df &pos) override;
    
    void addGlowingNode(scene::ISceneNode *n,
                        float r = 1.0f, float g = 1.0f, float b = 1.0f) override;
                        
    void clearGlowingNodes() override;
    
    void render(float dt);
};

#endif //HEADER_SHADER_BASED_RENDERER_HPP
