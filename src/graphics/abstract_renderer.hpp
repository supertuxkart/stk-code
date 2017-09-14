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

#ifndef HEADER_ABSTRACT_RENDERER_HPP
#define HEADER_ABSTRACT_RENDERER_HPP

#include "graphics/gl_headers.hpp"
#include <irrlicht.h>
#include <memory>
#include <string>
#include <vector>

class RenderTarget;

enum STKRenderingPass
{
    SOLID_NORMAL_AND_DEPTH_PASS,
    SOLID_LIT_PASS,
    TRANSPARENT_PASS,
    GLOW_PASS,
    SHADOW_PASS,
    PASS_COUNT,
};

enum TypeRTT : unsigned int;

struct GlowData {
    irr::scene::ISceneNode * node;
    float r, g, b;
};

struct SHCoefficients;

/**
 *  \class AbstractRenderer
 *  \brief Virtual base class for the renderer
 *
 *  \ingroup graphics
 */
class AbstractRenderer
{
protected:
    irr::core::vector2df m_current_screen_size;

    /** Performance stats */
    unsigned             m_object_count[PASS_COUNT];
    unsigned             m_poly_count  [PASS_COUNT];

#ifdef DEBUG
    void drawDebugMeshes() const;

    void drawJoint(bool drawline, bool drawname,
                   irr::scene::ISkinnedMesh::SJoint* joint,
                   irr::scene::ISkinnedMesh* mesh, int id) const;
#endif

    void renderSkybox(const irr::scene::ICameraSceneNode *camera) const;

public:
    AbstractRenderer();
    virtual ~AbstractRenderer(){}
        
    virtual void onLoadWorld()   = 0;
    virtual void onUnloadWorld() = 0;

    virtual void resetPostProcessing() {}
    virtual void giveBoost(unsigned int cam_index) {}

    virtual void addSkyBox(const std::vector<irr::video::ITexture*> &texture,
                           const std::vector<irr::video::ITexture*> &spherical_harmonics_textures) {}
    virtual void removeSkyBox() {}
    
    //FIXME: these three methods should not appear in the public Renderer interface
    virtual const SHCoefficients* getSHCoefficients()    const { return NULL; }
    virtual GLuint getRenderTargetTexture(TypeRTT which) const { return 0;}
    virtual GLuint getDepthStencilTexture(             ) const { return 0;}
    
    virtual void setAmbientLight(const irr::video::SColorf &light,
                                  bool force_SH_computation = true) {}

    virtual void addSunLight(const irr::core::vector3df &pos){}

    virtual void addGlowingNode(irr::scene::ISceneNode *n,
                                float r = 1.0f, float g = 1.0f, float b = 1.0f) {}
    
    virtual void clearGlowingNodes() {}

    virtual void render(float dt) = 0;
 
     // ------------------------------------------------------------------------
    const irr::core::vector2df &getCurrentScreenSize() const
    {
        return m_current_screen_size;
    }
    
    // ----------------------------------------------------------------------------
    /** Create a RenderTarget (for rendering to a texture)
     *  \param dimension The dimension of the texture
     *  \param name A unique name for the render target
     */
    virtual std::unique_ptr<RenderTarget> createRenderTarget(const irr::core::dimension2du &dimension,
                                                             const std::string &name) = 0;
    
    void resetObjectCount()                     { memset(m_object_count, 0, sizeof(m_object_count));}
    void resetPolyCount()                       { memset(m_poly_count, 0, sizeof(m_poly_count));    }
    void incObjectCount(STKRenderingPass phase) {  m_object_count[phase]++;                         }
    
    unsigned getObjectCount(STKRenderingPass pass) const { return m_object_count[pass];             }
    unsigned getPolyCount(STKRenderingPass pass) const   { return m_poly_count  [pass];             }
    
};

#endif //HEADER_ABSTRACT_RENDERER_HPP
