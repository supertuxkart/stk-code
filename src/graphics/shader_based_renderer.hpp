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

#ifndef SERVER_ONLY

#include "graphics/abstract_renderer.hpp"
#include "graphics/draw_calls.hpp"
#include "graphics/lighting_passes.hpp"
#include "graphics/shadow_matrices.hpp"
#include "utils/cpp2011.hpp"
#include <map>
#include <string>

class AbstractGeometryPasses;
class Camera;
class GL3RenderTarget;
class RenderTarget;
class RTT;
class Skybox;
class SphericalHarmonics;
class PostProcessing;

namespace irr
{
    namespace video
    {
        class IImage;
    }
}

class ShaderBasedRenderer: public AbstractRenderer
{
private:
    bool                        m_dump_rtt;
    RTT                        *m_rtts;
    Skybox                     *m_skybox;
    SphericalHarmonics         *m_spherical_harmonics;
    DrawCalls                   m_draw_calls;
    LightingPasses              m_lighting_passes;
    ShadowMatrices              m_shadow_matrices;
    std::unique_ptr<PostProcessing> m_post_processing;

    void prepareForwardRenderer();

    void uploadLightingData() const;

    void computeMatrixesAndCameras(scene::ICameraSceneNode * const camnode,
                                   unsigned int width, unsigned int height);
    
    void resetShadowCamNodes(){m_shadow_matrices.resetShadowCamNodes();}
    
    void renderSkybox(const scene::ICameraSceneNode *camera) const;
    
    
    void prepareDrawCalls(scene::ICameraSceneNode *camnode);

    void renderSSAO() const;

    void renderGlow() const;

    void renderScene(irr::scene::ICameraSceneNode * const camnode,
                     float dt, bool hasShadows, bool forceRTT);
    void renderSceneDeferred(irr::scene::ICameraSceneNode * const camnode,
                     float dt, bool hasShadows, bool forceRTT);

    void debugPhysics();
    void renderPostProcessing(Camera * const camera, bool first_cam);
    void preloadShaderFiles();
    void renderShadows();

public:
    ShaderBasedRenderer();
    ~ShaderBasedRenderer();
    
    void onLoadWorld() OVERRIDE;
    void onUnloadWorld() OVERRIDE;
    
    void resetPostProcessing() OVERRIDE;
    void giveBoost(unsigned int cam_index) OVERRIDE;

    void addSkyBox(const std::vector<irr::video::IImage*> &texture,
                   const std::vector<irr::video::IImage*> &spherical_harmonics_textures);
    void removeSkyBox() OVERRIDE;
    const SHCoefficients* getSHCoefficients() const OVERRIDE;
    GLuint getRenderTargetTexture(TypeRTT which) const OVERRIDE;
    GLuint getDepthStencilTexture() const OVERRIDE;
    
    void                  setAmbientLight(const irr::video::SColorf &light,
                                          bool force_SH_computation = true) OVERRIDE;

    void addSunLight(const irr::core::vector3df &pos) OVERRIDE;

    void render(float dt, bool is_loading=false) OVERRIDE;

    std::unique_ptr<RenderTarget> createRenderTarget(const irr::core::dimension2du &dimension,
                                                     const std::string &name) OVERRIDE;
    
    void renderToTexture(GL3RenderTarget *render_target,
                         irr::scene::ICameraSceneNode* camera,
                         float dt);

    void setRTT(RTT* rtts);

    RTT* getRTTs() { return m_rtts; }
    ShadowMatrices* getShadowMatrices() { return &m_shadow_matrices; }
    PostProcessing* getPostProcessing() { return m_post_processing.get(); }
    void dumpRTT() { m_dump_rtt = true; }
    void createPostProcessing() OVERRIDE;

};

#endif   // !SERVER_ONLY
#endif   // HEADER_SHADER_BASED_RENDERER_HPP
