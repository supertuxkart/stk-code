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

#ifndef SERVER_ONLY

#include "graphics/shadow_matrices.hpp"

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"
#include "modes/world.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"

#include <limits>
#include <ICameraSceneNode.h>
#include <SViewFrustum.h>
#include "../../lib/irrlicht/source/Irrlicht/CSceneManager.h"
#include "../../lib/irrlicht/source/Irrlicht/os.h"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

float ShadowMatrices:: m_shadow_split[5] = { 1., 5., 20., 50., 150 };

// ============================================================================
class LightspaceBoundingBoxShader
    : public TextureShader<LightspaceBoundingBoxShader, 1,
                           core::matrix4, float, float, float, float>
{
public:
    LightspaceBoundingBoxShader()
    {
#if !defined(USE_GLES2)
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "Lightspaceboundingbox.comp");
        assignSamplerNames(0, "depth", ST_NEAREST_FILTERED);
        assignUniforms("SunCamMatrix", "split0", "split1", "split2", "splitmax");
        GLuint block_idx =
            glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK,
                                      "BoundingBoxes");
        glShaderStorageBlockBinding(m_program, block_idx, 2);
#endif
    }   // LightspaceBoundingBoxShader
};   // LightspaceBoundingBoxShader

// ============================================================================
class ShadowMatricesGenerationShader
    : public Shader <ShadowMatricesGenerationShader, core::matrix4>
{
public:
    ShadowMatricesGenerationShader()
    {
#if !defined(USE_GLES2)
        loadProgram(OBJECT,  GL_COMPUTE_SHADER, "shadowmatrixgeneration.comp");
        assignUniforms("SunCamMatrix");
        GLuint block_idx =
            glGetProgramResourceIndex(m_program,
                                      GL_SHADER_STORAGE_BLOCK, "BoundingBoxes");
        glShaderStorageBlockBinding(m_program, block_idx, 2);
        block_idx =
            glGetProgramResourceIndex(m_program, GL_SHADER_STORAGE_BLOCK,
                                      "NewMatrixData");
        glShaderStorageBlockBinding(m_program, block_idx, 1);
#endif
    }


};   // ShadowMatricesGenerationShader

// ============================================================================
class ViewFrustrumShader : public Shader<ViewFrustrumShader, video::SColor, int>
{
private:
    GLuint m_frustrum_vao;

public:    ViewFrustrumShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "frustrum.vert",
                            GL_FRAGMENT_SHADER, "coloredquad.frag");

        assignUniforms("color", "idx");

        glGenVertexArrays(1, &m_frustrum_vao);
        glBindVertexArray(m_frustrum_vao);
        glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getFrustrumVBO());
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     SharedGPUObjects::getFrustrumIndices());
        glBindVertexArray(0);
    }   // ViewFrustrumShader
    // ------------------------------------------------------------------------
    void bindVertexArray()
    {
        glBindVertexArray(m_frustrum_vao);
    }   // bindVertexArray

};   // ViewFrustrumShader

// ============================================================================
ShadowMatrices::ShadowMatrices()
{
    m_sun_cam = irr_driver->getSceneManager()
              ->addCameraSceneNode(0, vector3df(0), vector3df(0), -1, false);
    m_sun_cam->grab();
    m_sun_cam->setParent(NULL);

    m_shadow_cam_nodes[0] = NULL;
    m_shadow_cam_nodes[1] = NULL;
    m_shadow_cam_nodes[2] = NULL;
    m_shadow_cam_nodes[3] = NULL;
    m_rsm_map_available = false;
    m_rsm_matrix_initialized = false;
}   // ShadowMatrices
// ----------------------------------------------------------------------------
ShadowMatrices::~ShadowMatrices()
{
    resetShadowCamNodes();
    m_sun_cam->drop();
}   // ~ShadowMatrices
// ----------------------------------------------------------------------------
void ShadowMatrices::resetShadowCamNodes()
{
    for (unsigned i = 0; i < 4; i++)
    {
        if (m_shadow_cam_nodes[i])
        {
            m_shadow_cam_nodes[i]->drop();
            m_shadow_cam_nodes[i] = NULL;
        }
    }
}   // resetShadowCamNodes

// ----------------------------------------------------------------------------
void ShadowMatrices::addLight(const core::vector3df &pos)
{
    m_sun_cam->setPosition(pos);
    m_sun_cam->updateAbsolutePosition();
    m_rsm_matrix_initialized = false;
}   // addLight

// ----------------------------------------------------------------------------
void ShadowMatrices::updateSunOrthoMatrices()
{
    for (unsigned i = 0; i < m_sun_ortho_matrices.size(); i++)
        m_sun_ortho_matrices[i] *= irr_driver->getInvViewMatrix();
}   // updateSunOrthoMatrices

// ============================================================================
static std::vector<vector3df> getFrustrumVertex(const scene::SViewFrustum &frustrum)
{
    std::vector<vector3df> vectors;
    vectors.push_back(frustrum.getFarLeftDown());
    vectors.push_back(frustrum.getFarLeftUp());
    vectors.push_back(frustrum.getFarRightDown());
    vectors.push_back(frustrum.getFarRightUp());
    vectors.push_back(frustrum.getNearLeftDown());
    vectors.push_back(frustrum.getNearLeftUp());
    vectors.push_back(frustrum.getNearRightDown());
    vectors.push_back(frustrum.getNearRightUp());
    return vectors;
}

// ----------------------------------------------------------------------------
/** Given a matrix transform and a set of points returns an orthogonal
 *  projection matrix that maps coordinates of transformed points between -1
 *  and 1.
 *  \param transform a transform matrix.
 *  \param pointsInside a vector of point in 3d space.
 *  \param size returns the size (width, height) of shadowmap coverage
 */
core::matrix4 ShadowMatrices::getTighestFitOrthoProj(const core::matrix4 &transform,
                                    const std::vector<vector3df> &pointsInside,
                                    std::pair<float, float> &size)
{
    float xmin = std::numeric_limits<float>::infinity();
    float xmax = -std::numeric_limits<float>::infinity();
    float ymin = std::numeric_limits<float>::infinity();
    float ymax = -std::numeric_limits<float>::infinity();
    float zmin = std::numeric_limits<float>::infinity();
    float zmax = -std::numeric_limits<float>::infinity();

    for (unsigned i = 0; i < pointsInside.size(); i++)
    {
        vector3df TransformedVector;
        transform.transformVect(TransformedVector, pointsInside[i]);
        xmin = MIN2(xmin, TransformedVector.X);
        xmax = MAX2(xmax, TransformedVector.X);
        ymin = MIN2(ymin, TransformedVector.Y);
        ymax = MAX2(ymax, TransformedVector.Y);
        zmin = MIN2(zmin, TransformedVector.Z);
        zmax = MAX2(zmax, TransformedVector.Z);
    }

    float left = xmin;
    float right = xmax;
    float up = ymin;
    float down = ymax;

    size.first = right - left;
    size.second = down - up;

    core::matrix4 tmp_matrix;
    // Prevent Matrix without extend
    if (left == right || up == down)
        return tmp_matrix;
    tmp_matrix.buildProjectionMatrixOrthoLH(left, right,
        down, up,
        zmin - 100, zmax);
    return tmp_matrix;
}   // getTighestFitOrthoProj

// ----------------------------------------------------------------------------
/** Update shadowSplit values and make Cascade Bounding Box pointer valid.
 *  The function aunches two compute kernel that generates an histogram of the
 *  depth buffer value (between 0 and 250 with increment of 0.25) and get an
 *  axis aligned bounding box (from SunCamMatrix view) containing all depth
 *  buffer value. It also retrieves the result from the previous computations
 *  (in a Round Robin fashion) and update CBB pointer.
 *  \param width of the depth buffer
 *  \param height of the depth buffer
 *  TODO : The depth histogram part is commented out, needs to tweak it when
 *         I have some motivation
 */
void ShadowMatrices::updateSplitAndLightcoordRangeFromComputeShaders(unsigned int width,
                                                                     unsigned int height,
                                                                     GLuint depth_stencil_texture)
{
#if !defined(USE_GLES2)
    struct CascadeBoundingBox
    {
        int xmin;
        int xmax;
        int ymin;
        int ymax;
        int zmin;
        int zmax;
    };   // struct CascadeBoundingBox

    // Value that should be kept between multiple calls
    static bool ssboInit = false;
    static GLuint CBBssbo, tempShadowMatssbo;
    CascadeBoundingBox InitialCBB[4];

    for (unsigned i = 0; i < 4; i++)
    {
        InitialCBB[i].xmin = InitialCBB[i].ymin = InitialCBB[i].zmin = 1000;
        InitialCBB[i].xmax = InitialCBB[i].ymax = InitialCBB[i].zmax = -1000;
    }

    if (!ssboInit)
    {
        glGenBuffers(1, &CBBssbo);
        glGenBuffers(1, &tempShadowMatssbo);
        ssboInit = true;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, CBBssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(CascadeBoundingBox),
                 InitialCBB, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CBBssbo);

    LightspaceBoundingBoxShader::getInstance()->use();
    LightspaceBoundingBoxShader::getInstance()
        ->setTextureUnits(depth_stencil_texture);
    LightspaceBoundingBoxShader::getInstance()
        ->setUniforms(m_sun_cam->getViewMatrix(),
                      ShadowMatrices::m_shadow_split[1],
                      ShadowMatrices::m_shadow_split[2],
                      ShadowMatrices::m_shadow_split[3],
                      ShadowMatrices::m_shadow_split[4]);
    glDispatchCompute((int)width / 64, (int)height / 64, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tempShadowMatssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * 16 * sizeof(float), 0,
                 GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tempShadowMatssbo);

    ShadowMatricesGenerationShader::getInstance()->use();
    ShadowMatricesGenerationShader::getInstance()
        ->setUniforms(m_sun_cam->getViewMatrix());
    glDispatchCompute(4, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_COPY_READ_BUFFER, tempShadowMatssbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER,
                 SharedGPUObjects::getViewProjectionMatricesUBO());
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0,
                        80 * sizeof(float), 4 * 16 * sizeof(float));
#endif
}   // updateSplitAndLightcoordRangeFromComputeShaders

// ----------------------------------------------------------------------------
/** Generate View, Projection, Inverse View, Inverse Projection, ViewProjection
 *  and InverseProjection matrixes and matrixes and cameras for the four shadow
 *   cascade and RSM.
 *   \param camnode point of view used
 *   \param width of the rendering viewport
 *   \param height of the rendering viewport
 */
void ShadowMatrices::computeMatrixesAndCameras(scene::ICameraSceneNode *const camnode,
                                               unsigned int width, unsigned int height,
                                               GLuint depth_stencil_texture)
{
    if (CVS->isSDSMEnabled())
        updateSplitAndLightcoordRangeFromComputeShaders(width, height, depth_stencil_texture);
    static_cast<scene::CSceneManager *>(irr_driver->getSceneManager())
        ->OnAnimate(os::Timer::getTime());
    camnode->render();
    irr_driver->setProjMatrix(irr_driver->getVideoDriver()
                              ->getTransform(video::ETS_PROJECTION));
    irr_driver->setViewMatrix(irr_driver->getVideoDriver()
                              ->getTransform(video::ETS_VIEW));
    irr_driver->genProjViewMatrix();


    const float oldfar = camnode->getFarValue();
    const float oldnear = camnode->getNearValue();

    float tmp[16 * 9 + 2];
    memcpy(tmp, irr_driver->getViewMatrix().pointer(),          16 * sizeof(float));
    memcpy(&tmp[16], irr_driver->getProjMatrix().pointer(),     16 * sizeof(float));
    memcpy(&tmp[32], irr_driver->getInvViewMatrix().pointer(),  16 * sizeof(float));
    memcpy(&tmp[48], irr_driver->getInvProjMatrix().pointer(),  16 * sizeof(float));
    memcpy(&tmp[64], irr_driver->getProjViewMatrix().pointer(), 16 * sizeof(float));

    m_sun_cam->render();
    for (unsigned i = 0; i < 4; i++)
    {
        if (m_shadow_cam_nodes[i])
            delete m_shadow_cam_nodes[i];
        m_shadow_cam_nodes[i] = (scene::ICameraSceneNode *) m_sun_cam->clone();
    }
    m_sun_ortho_matrices.clear();
    const core::matrix4 &sun_cam_view_matrix = m_sun_cam->getViewMatrix();

    const Track* const track = Track::getCurrentTrack();
    if (track)
    {
        float FarValues[] =
        {
            ShadowMatrices::m_shadow_split[1],
            ShadowMatrices::m_shadow_split[2],
            ShadowMatrices::m_shadow_split[3],
            ShadowMatrices::m_shadow_split[4],
        };
        float NearValues[] =
        {
            ShadowMatrices::m_shadow_split[0],
            ShadowMatrices::m_shadow_split[1],
            ShadowMatrices::m_shadow_split[2],
            ShadowMatrices::m_shadow_split[3]
        };

        // Shadow Matrixes and cameras
        for (unsigned i = 0; i < 4; i++)
        {
            core::matrix4 tmp_matrix;

            camnode->setFarValue(FarValues[i]);
            camnode->setNearValue(NearValues[i]);
            camnode->render();
            const scene::SViewFrustum *frustrum = camnode->getViewFrustum();
            float tmp[24] = {
                frustrum->getFarLeftDown().X,
                frustrum->getFarLeftDown().Y,
                frustrum->getFarLeftDown().Z,
                frustrum->getFarLeftUp().X,
                frustrum->getFarLeftUp().Y,
                frustrum->getFarLeftUp().Z,
                frustrum->getFarRightDown().X,
                frustrum->getFarRightDown().Y,
                frustrum->getFarRightDown().Z,
                frustrum->getFarRightUp().X,
                frustrum->getFarRightUp().Y,
                frustrum->getFarRightUp().Z,
                frustrum->getNearLeftDown().X,
                frustrum->getNearLeftDown().Y,
                frustrum->getNearLeftDown().Z,
                frustrum->getNearLeftUp().X,
                frustrum->getNearLeftUp().Y,
                frustrum->getNearLeftUp().Z,
                frustrum->getNearRightDown().X,
                frustrum->getNearRightDown().Y,
                frustrum->getNearRightDown().Z,
                frustrum->getNearRightUp().X,
                frustrum->getNearRightUp().Y,
                frustrum->getNearRightUp().Z,
            };
            memcpy(m_shadows_cam[i], tmp, 24 * sizeof(float));

            std::vector<vector3df> vectors = getFrustrumVertex(*frustrum);
            tmp_matrix = getTighestFitOrthoProj(sun_cam_view_matrix, vectors,
                                                m_shadow_scales[i]);


            m_shadow_cam_nodes[i]->setProjectionMatrix(tmp_matrix, true);
            m_shadow_cam_nodes[i]->render();

            m_sun_ortho_matrices.push_back(
                  irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION)
                * irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW)       );
        }

        // Rsm Matrix and camera
        if (!m_rsm_matrix_initialized && track->getPtrTriangleMesh())
        {
            // Compute track extent
            Vec3 vmin, vmax;
            track->getTriangleMesh().getCollisionShape()
                  .getAabb(btTransform::getIdentity(), vmin, vmax);
            core::aabbox3df trackbox(vmin.toIrrVector(), vmax.toIrrVector() -
                core::vector3df(0, 30, 0));

            if (trackbox.MinEdge.X != trackbox.MaxEdge.X &&
                trackbox.MinEdge.Y != trackbox.MaxEdge.Y &&
                // Cover the case where sun_cam_view_matrix is null
                sun_cam_view_matrix.getScale() != core::vector3df(0., 0., 0.))
            {
                sun_cam_view_matrix.transformBoxEx(trackbox);
                core::matrix4 tmp_matrix;
                tmp_matrix.buildProjectionMatrixOrthoLH(trackbox.MinEdge.X,
                                                        trackbox.MaxEdge.X,
                                                        trackbox.MaxEdge.Y,
                                                        trackbox.MinEdge.Y,
                                                        30, trackbox.MaxEdge.Z);
                m_sun_cam->setProjectionMatrix(tmp_matrix, true);
                m_sun_cam->render();
            }
            m_rsm_matrix = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION)
                         * irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);
            m_rsm_matrix_initialized = true;
            m_rsm_map_available = false;
        }
        m_rh_extend = core::vector3df(128, 64, 128);
        core::vector3df campos = camnode->getAbsolutePosition();
        core::vector3df translation(8 * floor(campos.X / 8),
                                    8 * floor(campos.Y / 8),
                                    8 * floor(campos.Z / 8));
        m_rh_matrix.setTranslation(translation);


        assert(m_sun_ortho_matrices.size() == 4);
        // reset normal camera
        camnode->setNearValue(oldnear);
        camnode->setFarValue(oldfar);
        camnode->render();

        size_t size = m_sun_ortho_matrices.size();
        for (unsigned i = 0; i < size; i++)
            memcpy(&tmp[16 * i + 80],
                   m_sun_ortho_matrices[i].pointer(),
                   16 * sizeof(float));
    }

    if(!CVS->isARBUniformBufferObjectUsable())
        return;

    tmp[144] = float(width);
    tmp[145] = float(height);
    glBindBuffer(GL_UNIFORM_BUFFER,
                 SharedGPUObjects::getViewProjectionMatricesUBO());
    if (CVS->isSDSMEnabled())
    {
        glBufferSubData(GL_UNIFORM_BUFFER, 0, (16 * 5) * sizeof(float), tmp);
        glBufferSubData(GL_UNIFORM_BUFFER, (16 * 9) * sizeof(float),
                        2 * sizeof(float), &tmp[144]);
    }
    else
        glBufferSubData(GL_UNIFORM_BUFFER, 0, (16 * 9 + 2) * sizeof(float),
                        tmp);
}   // computeMatrixesAndCameras

// ----------------------------------------------------------------------------
void ShadowMatrices::renderWireFrameFrustrum(float *tmp, unsigned i)
{
    ViewFrustrumShader::getInstance()->use();
    ViewFrustrumShader::getInstance()->bindVertexArray();
    glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getFrustrumVBO());

    glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * 3 * sizeof(float), (void *)tmp);
    ViewFrustrumShader::getInstance()->setUniforms(video::SColor(255, 0, 255, 0), i);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
}
// ----------------------------------------------------------------------------
void ShadowMatrices::renderShadowsDebug(const FrameBuffer &shadow_framebuffer,
                                        const PostProcessing *post_processing)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, UserConfigParams::m_height / 2,
               UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    post_processing->renderTextureLayer(shadow_framebuffer.getRTT()[0], 0);
    renderWireFrameFrustrum(m_shadows_cam[0], 0);
    glViewport(UserConfigParams::m_width / 2, UserConfigParams::m_height / 2,
               UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    post_processing->renderTextureLayer(shadow_framebuffer.getRTT()[0], 1);
    renderWireFrameFrustrum(m_shadows_cam[1], 1);
    glViewport(0, 0, UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    post_processing->renderTextureLayer(shadow_framebuffer.getRTT()[0], 2);
    renderWireFrameFrustrum(m_shadows_cam[2], 2);
    glViewport(UserConfigParams::m_width / 2, 0, UserConfigParams::m_width / 2,
               UserConfigParams::m_height / 2);
    post_processing->renderTextureLayer(shadow_framebuffer.getRTT()[0], 3);
    renderWireFrameFrustrum(m_shadows_cam[3], 3);
    glViewport(0, 0, UserConfigParams::m_width, UserConfigParams::m_height);
}

#endif   // !SERVER_ONLY
