//  SuperTuxKart - a fun racing game with }go-kart
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

#ifndef HEADER_ABSTRACT_RENDERER_HPP
#define HEADER_ABSTRACT_RENDERER_HPP

#include "graphics/gl_headers.hpp"
#include "graphics/rtts.hpp"
#include "graphics/sphericalHarmonics.hpp"
#include <irrlicht.h>
#include <string>
#include <vector>

class RenderTarget;


struct GlowData {
    irr::scene::ISceneNode * node;
    float r, g, b;
};

class AbstractRenderer
{
protected:
    irr::core::vector2df m_current_screen_size;
    RTT                  *m_rtts;


#ifdef DEBUG
    void drawDebugMeshes() const;

    void drawJoint(bool drawline, bool drawname,
                   irr::scene::ISkinnedMesh::SJoint* joint,
                   irr::scene::ISkinnedMesh* mesh, int id) const;
#endif

    void renderSkybox(const irr::scene::ICameraSceneNode *camera) const;


public:
    virtual ~AbstractRenderer(){}
    
    RTT* getRTT() { return m_rtts;} //FIXME: remove this
    void setRTT(RTT *rtts) { m_rtts = rtts; } //FIXME: remove this
    
    virtual void onLoadWorld()   = 0;
    virtual void onUnloadWorld() = 0;


    virtual void addSkyBox(const std::vector<irr::video::ITexture*> &texture,
                           const std::vector<irr::video::ITexture*> &spherical_harmonics_textures) {}
    virtual void removeSkyBox() {}
    virtual const SHCoefficients* getSHCoefficients() const { return NULL; }
    virtual void setAmbientLight(const irr::video::SColorf &light,
                                  bool force_SH_computation = true) {}


    virtual void addSunLight(const irr::core::vector3df &pos){}

    virtual void addGlowingNode(irr::scene::ISceneNode *n,
                                float r = 1.0f, float g = 1.0f, float b = 1.0f) {}
    
    virtual void clearGlowingNodes() {}

    
    virtual void render(float dt) = 0;

    virtual void renderScene(irr::scene::ICameraSceneNode * const camnode,
                             float dt, bool hasShadows, bool forceRTT) = 0;
                             
    virtual void updateLightsInfo(irr::scene::ICameraSceneNode * const camnode,
                                      float dt) = 0;
    virtual void uploadLightingData() const {}

    virtual void computeMatrixesAndCameras(irr::scene::ICameraSceneNode * const camnode,
                                   size_t width, size_t height) {}

    virtual void resetShadowCamNodes() {}

 
     // ------------------------------------------------------------------------
    const irr::core::vector2df &getCurrentScreenSize() const
    {
        return m_current_screen_size;
    }
 
    /*virtual GLuint renderToTexture(size_t width,
                                   size_t height,
                                   irr::scene::ICameraSceneNode* camera,
                                   float dt,
                                   const std::string &rtt_name) { return 0;}*/
    virtual void renderToTexture(RenderTarget *render_target,
                                 irr::scene::ICameraSceneNode* camera,
                                 float dt) {}
};

#endif //HEADER_ABSTRACT_RENDERER_HPP
