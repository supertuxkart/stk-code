#include <limits>
#include <ICameraSceneNode.h>
#include <SViewFrustum.h>
#include "../../lib/irrlicht/source/Irrlicht/CSceneManager.h"
#include "../../lib/irrlicht/source/Irrlicht/os.h"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

static std::vector<vector3df>
getFrustrumVertex(const scene::SViewFrustum &frustrum)
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

/** Given a matrix transform and a set of points returns an orthogonal projection matrix that maps coordinates of
transformed points between -1 and 1.
*  \param transform a transform matrix.
*  \param pointsInside a vector of point in 3d space.
*  \param size returns the size (width, height) of shadowmap coverage
*/
static core::matrix4
getTighestFitOrthoProj(const core::matrix4 &transform, const std::vector<vector3df> &pointsInside, std::pair<float, float> &size)
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
}

float shadowSplit[5] = { 1., 5., 20., 50., 150 };

struct CascadeBoundingBox
{
    int xmin;
    int xmax;
    int ymin;
    int ymax;
    int zmin;
    int zmax;
};

static size_t currentCBB = 0;
static CascadeBoundingBox *CBB[2];

struct Histogram
{
    int bin[1024];
    int mindepth;
    int maxdepth;
    int count;
};

/** Update shadowSplit values and make Cascade Bounding Box pointer valid.
* The function aunches two compute kernel that generates an histogram of the depth buffer value (between 0 and 250 with increment of 0.25)
* and get an axis aligned bounding box (from SunCamMatrix view) containing all depth buffer value.
* It also retrieves the result from the previous computations (in a Round Robin fashion) and update CBB pointer.
* \param width of the depth buffer
* \param height of the depth buffer
* TODO : The depth histogram part is commented out, needs to tweak it when I have some motivation
*/
void IrrDriver::UpdateSplitAndLightcoordRangeFromComputeShaders(size_t width, size_t height)
{
    // Value that should be kept between multiple calls
    static GLuint ssbo[2];
    static Histogram *Hist[2];
    static GLsync LightcoordBBFence = 0;
    static size_t currentHist = 0;
    static GLuint ssboSplit[2];
    static float tmpshadowSplit[5] = { 1., 5., 20., 50., 150. };

    if (!LightcoordBBFence)
    {
        glGenBuffers(2, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(CascadeBoundingBox), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        CBB[0] = (CascadeBoundingBox *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(CascadeBoundingBox), GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, 4 * sizeof(CascadeBoundingBox), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        CBB[1] = (CascadeBoundingBox *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(CascadeBoundingBox), GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

        /*        glGenBuffers(2, ssboSplit);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSplit[0]);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Histogram), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        Hist[0] = (Histogram *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Histogram), GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSplit[1]);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Histogram), 0, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
        Hist[1] = (Histogram *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Histogram), GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);*/
    }

    // Use bounding boxes from last frame
    if (LightcoordBBFence)
    {
        while (glClientWaitSync(LightcoordBBFence, GL_SYNC_FLUSH_COMMANDS_BIT, 0) != GL_ALREADY_SIGNALED);
        glDeleteSync(LightcoordBBFence);
    }

    /*    {
    memcpy(shadowSplit, tmpshadowSplit, 5 * sizeof(float));
    unsigned numpix = Hist[currentHist]->count;
    unsigned split = 0;
    unsigned i;
    for (i = 0; i < 1022; i++)
    {
    split += Hist[currentHist]->bin[i];
    if (split > numpix / 2)
    break;
    }
    tmpshadowSplit[1] = (float)++i / 4.;

    for (; i < 1023; i++)
    {
    split += Hist[currentHist]->bin[i];
    if (split > 3 * numpix / 4)
    break;
    }
    tmpshadowSplit[2] = (float)++i / 4.;

    for (; i < 1024; i++)
    {
    split += Hist[currentHist]->bin[i];
    if (split > 7 * numpix / 8)
    break;
    }
    tmpshadowSplit[3] = (float)++i / 4.;

    for (; i < 1024; i++)
    {
    split += Hist[currentHist]->bin[i];
    }

    tmpshadowSplit[0] = (float)(Hist[currentHist]->bin[1024] - 1) / 4.;
    tmpshadowSplit[4] = (float)(Hist[currentHist]->bin[1025] + 1) / 4.;
    printf("numpix is %d\n", numpix);
    printf("total : %d\n", split);
    printf("split 0 : %f\n", tmpshadowSplit[1]);
    printf("split 1 : %f\n", tmpshadowSplit[2]);
    printf("split 2 : %f\n", tmpshadowSplit[3]);
    printf("min %f max %f\n", tmpshadowSplit[0], tmpshadowSplit[4]);
    currentHist = (currentHist + 1) % 2;
    }*/

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[currentCBB]);
    //    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboSplit[currentHist]);
    for (unsigned i = 0; i < 4; i++)
    {
        CBB[currentCBB][i].xmin = CBB[currentCBB][i].ymin = CBB[currentCBB][i].zmin = 1000;
        CBB[currentCBB][i].xmax = CBB[currentCBB][i].ymax = CBB[currentCBB][i].zmax = -1000;
    }
    //    memset(Hist[currentHist], 0, sizeof(Histogram));
    //    Hist[currentHist]->mindepth = 3000;
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glUseProgram(FullScreenShader::LightspaceBoundingBoxShader::getInstance()->Program);
    FullScreenShader::LightspaceBoundingBoxShader::getInstance()->SetTextureUnits(getDepthStencilTexture());
    FullScreenShader::LightspaceBoundingBoxShader::getInstance()->setUniforms(m_suncam->getViewMatrix(), tmpshadowSplit[1], tmpshadowSplit[2], tmpshadowSplit[3], tmpshadowSplit[4]);
    glDispatchCompute((int)width / 64, (int)height / 64, 1);

    /*    glUseProgram(FullScreenShader::DepthHistogramShader::getInstance()->Program);
    FullScreenShader::DepthHistogramShader::getInstance()->SetTextureUnits(getDepthStencilTexture());
    FullScreenShader::DepthHistogramShader::getInstance()->setUniforms();
    glDispatchCompute((int)width / 32, (int)height / 32, 1);*/

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    LightcoordBBFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    currentCBB = (currentCBB + 1) % 2;

}

/** Generate View, Projection, Inverse View, Inverse Projection, ViewProjection and InverseProjection matrixes
and matrixes and cameras for the four shadow cascade and RSM.
*   \param camnode point of view used
*   \param width of the rendering viewport
*   \param height of the rendering viewport
*/
void IrrDriver::computeMatrixesAndCameras(scene::ICameraSceneNode * const camnode, size_t width, size_t height)
{
    if (CVS->isSDSMEnabled())
        UpdateSplitAndLightcoordRangeFromComputeShaders(width, height);
    static_cast<scene::CSceneManager *>(m_scene_manager)->OnAnimate(os::Timer::getTime());
    camnode->render();
    irr_driver->setProjMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION));
    irr_driver->setViewMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW));
    irr_driver->genProjViewMatrix();

    m_current_screen_size = core::vector2df(float(width), float(height));

    const float oldfar = camnode->getFarValue();
    const float oldnear = camnode->getNearValue();
    float FarValues[] =
    {
        shadowSplit[1],
        shadowSplit[2],
        shadowSplit[3],
        shadowSplit[4],
    };
    float NearValues[] =
    {
        shadowSplit[0],
        shadowSplit[1],
        shadowSplit[2],
        shadowSplit[3]
    };

    float tmp[16 * 9 + 2];
    memcpy(tmp, irr_driver->getViewMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[16], irr_driver->getProjMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[32], irr_driver->getInvViewMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[48], irr_driver->getInvProjMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[64], irr_driver->getProjViewMatrix().pointer(), 16 * sizeof(float));

    m_suncam->render();
    for (unsigned i = 0; i < 4; i++)
    {
        if (m_shadow_camnodes[i])
            delete m_shadow_camnodes[i];
        m_shadow_camnodes[i] = (scene::ICameraSceneNode *) m_suncam->clone();
    }
    sun_ortho_matrix.clear();
    const core::matrix4 &SunCamViewMatrix = m_suncam->getViewMatrix();

    if (World::getWorld() && World::getWorld()->getTrack())
    {
        // Compute track extent
        btVector3 btmin, btmax;
        if (World::getWorld()->getTrack()->getPtrTriangleMesh())
        {
            World::getWorld()->getTrack()->getTriangleMesh().getCollisionShape().getAabb(btTransform::getIdentity(), btmin, btmax);
        }
        const Vec3 vmin = btmin, vmax = btmax;
        core::aabbox3df trackbox(vmin.toIrrVector(), vmax.toIrrVector() -
            core::vector3df(0, 30, 0));

        // Shadow Matrixes and cameras
        for (unsigned i = 0; i < 4; i++)
        {
            core::matrix4 tmp_matrix;
            if (!CVS->isSDSMEnabled())
            {
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
                tmp_matrix = getTighestFitOrthoProj(SunCamViewMatrix, vectors, m_shadow_scales[i]);
            }
            else
            {
                float left = float(CBB[currentCBB][i].xmin / 4 - 2);
                float right = float(CBB[currentCBB][i].xmax / 4 + 2);
                float up = float(CBB[currentCBB][i].ymin / 4 - 2);
                float down = float(CBB[currentCBB][i].ymax / 4 + 2);

                // Prevent Matrix without extend
                if (left != right && up != down)
                {
                    tmp_matrix.buildProjectionMatrixOrthoLH(left, right,
                        down, up,
                        float(CBB[currentCBB][i].zmin / 4 - 100),
                        float(CBB[currentCBB][i].zmax / 4 + 2));
                    m_shadow_scales[i] = std::make_pair(right - left, down - up);
                }
            }

            m_shadow_camnodes[i]->setProjectionMatrix(tmp_matrix, true);
            m_shadow_camnodes[i]->render();

            sun_ortho_matrix.push_back(getVideoDriver()->getTransform(video::ETS_PROJECTION) * getVideoDriver()->getTransform(video::ETS_VIEW));
        }

        // Rsm Matrix and camera
        if (!m_rsm_matrix_initialized)
        {
            if (trackbox.MinEdge.X != trackbox.MaxEdge.X &&
                trackbox.MinEdge.Y != trackbox.MaxEdge.Y &&
                // Cover the case where SunCamViewMatrix is null
                SunCamViewMatrix.getScale() != core::vector3df(0., 0., 0.))
            {
                SunCamViewMatrix.transformBoxEx(trackbox);
                core::matrix4 tmp_matrix;
                tmp_matrix.buildProjectionMatrixOrthoLH(trackbox.MinEdge.X, trackbox.MaxEdge.X,
                    trackbox.MaxEdge.Y, trackbox.MinEdge.Y,
                    30, trackbox.MaxEdge.Z);
                m_suncam->setProjectionMatrix(tmp_matrix, true);
                m_suncam->render();
            }
            rsm_matrix = getVideoDriver()->getTransform(video::ETS_PROJECTION) * getVideoDriver()->getTransform(video::ETS_VIEW);
            m_rsm_matrix_initialized = true;
            m_rsm_map_available = false;
        }
        rh_extend = core::vector3df(128, 64, 128);
        core::vector3df campos = camnode->getAbsolutePosition();
        core::vector3df translation(8 * floor(campos.X / 8), 8 * floor(campos.Y / 8), 8 * floor(campos.Z / 8));
        rh_matrix.setTranslation(translation);


        assert(sun_ortho_matrix.size() == 4);
        // reset normal camera
        camnode->setNearValue(oldnear);
        camnode->setFarValue(oldfar);
        camnode->render();

        size_t size = irr_driver->getShadowViewProj().size();
        for (unsigned i = 0; i < size; i++)
            memcpy(&tmp[16 * i + 80], irr_driver->getShadowViewProj()[i].pointer(), 16 * sizeof(float));
    }

    tmp[144] = float(width);
    tmp[145] = float(height);
    glBindBuffer(GL_UNIFORM_BUFFER, SharedObject::ViewProjectionMatrixesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, (16 * 9 + 2) * sizeof(float), tmp);
}


