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


#ifndef HEADER_SHADOW_MATRICES_HPP
#define HEADER_SHADOW_MATRICES_HPP

#include "graphics/gl_headers.hpp"

#include <matrix4.h>
#include <tuple>
#include <vector>
#include <vector3d.h>

namespace irr
{
    namespace scene { class ICameraSceneNode; }
}

using namespace irr;

class FrameBuffer;
class PostProcessing;

class ShadowMatrices
{
public:
    static float m_shadow_split[5];

private:
    std::vector<core::matrix4> m_sun_ortho_matrices;
    scene::ICameraSceneNode   *m_sun_cam;
    scene::ICameraSceneNode   *m_shadow_cam_nodes[4];
    std::pair<float, float>    m_shadow_scales[4];
    core::matrix4              m_rsm_matrix;
    bool                       m_rsm_matrix_initialized;
    float                      m_shadows_cam[4][24];
    bool                       m_rsm_map_available;
    core::vector3df            m_rh_extend;
    core::matrix4              m_rh_matrix;


    void updateSplitAndLightcoordRangeFromComputeShaders(unsigned int width,
                                                         unsigned int height,
                                                         GLuint depth_stencil_texture);
    core::matrix4 getTighestFitOrthoProj(const core::matrix4 &transform,
                              const std::vector<core::vector3df> &pointsInside,
                              std::pair<float, float> &size);
    void renderWireFrameFrustrum(float *tmp, unsigned i);
public:

    ShadowMatrices();
    ~ShadowMatrices();

    void computeMatrixesAndCameras(scene::ICameraSceneNode *const camnode,
                                   unsigned int width, unsigned int height,
                                   GLuint depth_stencil_texture);
    void addLight(const core::vector3df &pos);
    void updateSunOrthoMatrices();
    void renderShadowsDebug(const FrameBuffer &shadow_framebuffer,
                            const PostProcessing *post_processing);

    // ------------------------------------------------------------------------
    void resetShadowCamNodes();
    // ------------------------------------------------------------------------
    scene::ICameraSceneNode** getShadowCamNodes()
    {
        return m_shadow_cam_nodes;
    }   // getShadowCamNodes
    // ------------------------------------------------------------------------
    scene::ICameraSceneNode* getSunCam() { return m_sun_cam; }
    // ------------------------------------------------------------------------
    const core::matrix4& getRHMatrix() const { return m_rh_matrix;  }
    // ------------------------------------------------------------------------
    const core::vector3df& getRHExtend() const { return m_rh_extend;  }
    // ------------------------------------------------------------------------
    const core::matrix4& getRSMMatrix() const { return m_rsm_matrix; }
    // ------------------------------------------------------------------------
    std::vector<core::matrix4>& getSunOrthoMatrices()
    {
        return m_sun_ortho_matrices;
    }
    // ------------------------------------------------------------------------
    void setRSMMapAvail(bool b) { m_rsm_map_available = b; }
    // ------------------------------------------------------------------------
    bool isRSMMapAvail() const { return m_rsm_map_available; }
    // ------------------------------------------------------------------------
    const std::pair<float, float>* getShadowScales() const
    {
        return m_shadow_scales;
    }
    // ------------------------------------------------------------------------

};   // class ShadowMatrices

#endif
