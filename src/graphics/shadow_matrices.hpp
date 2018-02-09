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
    float                      m_mat_ubo[16 * 9 + 2];

    core::matrix4 getTighestFitOrthoProj(const core::matrix4 &transform,
                              const std::vector<core::vector3df> &pointsInside,
                              std::pair<float, float> &size);
    void renderWireFrameFrustrum(float *tmp, unsigned i);
public:

    ShadowMatrices();
    ~ShadowMatrices();

    void computeMatrixesAndCameras(scene::ICameraSceneNode *const camnode,
                                   unsigned int width, unsigned int height);
    void addLight(const core::vector3df &pos);
    void updateSunOrthoMatrices();
    void renderShadowsDebug(const FrameBuffer* shadow_framebuffer,
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
    std::vector<core::matrix4>& getSunOrthoMatrices()
    {
        return m_sun_ortho_matrices;
    }
    // ------------------------------------------------------------------------
    const std::pair<float, float>* getShadowScales() const
    {
        return m_shadow_scales;
    }
    // ------------------------------------------------------------------------
    const float* getMatricesData() const { return m_mat_ubo; }

};   // class ShadowMatrices

#endif
