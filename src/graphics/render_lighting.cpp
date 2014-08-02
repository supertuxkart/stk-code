#include "graphics/irr_driver.hpp"

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/lens_flare.hpp"
#include "graphics/light.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "graphics/stkinstancedscenenode.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/helpers.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <algorithm>
#include <limits>

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

static LightShader::PointLightInfo PointLightsInfo[MAXLIGHT];

static void renderPointLights(unsigned count)
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(LightShader::PointLightShader::Program);
    glBindVertexArray(LightShader::PointLightShader::vao);
    glBindBuffer(GL_ARRAY_BUFFER, LightShader::PointLightShader::vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(LightShader::PointLightInfo), PointLightsInfo);

    setTexture(0, irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), GL_NEAREST, GL_NEAREST);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    LightShader::PointLightShader
        ::setUniforms(core::vector2df(float(UserConfigParams::m_width),
        float(UserConfigParams::m_height)),
        200, 0, 1);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

unsigned IrrDriver::UpdateLightsInfo(scene::ICameraSceneNode * const camnode, float dt)
{
    const u32 lightcount = m_lights.size();
    const core::vector3df &campos = camnode->getAbsolutePosition();

    std::vector<LightNode *> BucketedLN[15];
    for (unsigned int i = 0; i < lightcount; i++)
    {
        if (!m_lights[i]->isPointLight())
        {
            m_lights[i]->render();
            continue;
        }
        const core::vector3df &lightpos = (m_lights[i]->getAbsolutePosition() - campos);
        unsigned idx = (unsigned)(lightpos.getLength() / 10);
        if (idx > 14)
            idx = 14;
        BucketedLN[idx].push_back(m_lights[i]);
    }

    unsigned lightnum = 0;

    for (unsigned i = 0; i < 15; i++)
    {
        for (unsigned j = 0; j < BucketedLN[i].size(); j++)
        {
            if (++lightnum >= MAXLIGHT)
            {
                LightNode* light_node = BucketedLN[i].at(j);
                light_node->setEnergyMultiplier(0.0f);
            }
            else
            {
                LightNode* light_node = BucketedLN[i].at(j);

                float em = light_node->getEnergyMultiplier();
                if (em < 1.0f)
                {
                    light_node->setEnergyMultiplier(std::min(1.0f, em + dt));
                }

                const core::vector3df &pos = light_node->getAbsolutePosition();
                PointLightsInfo[lightnum].posX = pos.X;
                PointLightsInfo[lightnum].posY = pos.Y;
                PointLightsInfo[lightnum].posZ = pos.Z;

                PointLightsInfo[lightnum].energy = light_node->getEffectiveEnergy();

                const core::vector3df &col = light_node->getColor();
                PointLightsInfo[lightnum].red = col.X;
                PointLightsInfo[lightnum].green = col.Y;
                PointLightsInfo[lightnum].blue = col.Z;

                // Light radius
                PointLightsInfo[lightnum].radius = light_node->getRadius();
            }
        }
        if (lightnum > MAXLIGHT)
        {
            irr_driver->setLastLightBucketDistance(i * 10);
            break;
        }
    }

    lightnum++;
    return lightnum;
}

void IrrDriver::renderLights(unsigned pointlightcount)
{
    //RH
    if (UserConfigParams::m_gi)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_RH));
        glDisable(GL_BLEND);
        m_rtts->getRH().Bind();
        glUseProgram(FullScreenShader::RadianceHintsConstructionShader::Program);
        glBindVertexArray(FullScreenShader::RadianceHintsConstructionShader::vao);
        setTexture(0, m_rtts->getRSM().getRTT()[0], GL_LINEAR, GL_LINEAR);
        setTexture(1, m_rtts->getRSM().getRTT()[1], GL_LINEAR, GL_LINEAR);
        setTexture(2, m_rtts->getRSM().getDepthTexture(), GL_LINEAR, GL_LINEAR);
        FullScreenShader::RadianceHintsConstructionShader::setUniforms(rsm_matrix, rh_matrix, rh_extend, 0, 1, 2);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 32);
    }

    for (unsigned i = 0; i < sun_ortho_matrix.size(); i++)
        sun_ortho_matrix[i] *= getInvViewMatrix();
    m_rtts->getFBO(FBO_COMBINED_TMP1_TMP2).Bind();
    if (!UserConfigParams::m_dynamic_lights)
        glClearColor(.5, .5, .5, .5);
    glClear(GL_COLOR_BUFFER_BIT);
    if (!UserConfigParams::m_dynamic_lights)
        return;

    m_rtts->getFBO(FBO_TMP1_WITH_DS).Bind();
    if (UserConfigParams::m_gi)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_GI));
        m_post_processing->renderGI(rh_matrix, rh_extend, m_rtts->getRH().getRTT()[0], m_rtts->getRH().getRTT()[1], m_rtts->getRH().getRTT()[2]);
    }

    if (SkyboxCubeMap)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_ENVMAP));
        m_post_processing->renderDiffuseEnvMap(blueSHCoeff, greenSHCoeff, redSHCoeff);
    }
    m_rtts->getFBO(FBO_COMBINED_TMP1_TMP2).Bind();

    // Render sunlight if and only if track supports shadow
    if (!World::getWorld() || World::getWorld()->getTrack()->hasShadows())
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_SUN));
        if (World::getWorld() && UserConfigParams::m_shadows && !UserConfigParams::m_ubo_disabled)
            m_post_processing->renderShadowedSunlight(sun_ortho_matrix, m_rtts->getShadowDepthTex());
        else
            m_post_processing->renderSunlight();
    }
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_POINTLIGHTS));
        renderPointLights(MIN2(pointlightcount, MAXLIGHT));
    }
}

void IrrDriver::renderSSAO()
{
    m_rtts->getFBO(FBO_SSAO).Bind();
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    m_post_processing->renderSSAO();
    // Blur it to reduce noise.
    FrameBuffer::Blit(m_rtts->getFBO(FBO_SSAO), m_rtts->getFBO(FBO_HALF1_R), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    m_post_processing->renderGaussian17TapBlur(irr_driver->getFBO(FBO_HALF1_R), irr_driver->getFBO(FBO_HALF2_R));

}