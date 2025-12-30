//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/dialogs/custom_video_settings.hpp"

#include "config/user_config.hpp"
#include "graphics/graphical_presets.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#endif

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

CustomVideoSettingsDialog::CustomVideoSettingsDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("custom_video_settings.stkgui");
    m_widgets.bind(this);
    updateActivation("");
}

// -----------------------------------------------------------------------------

CustomVideoSettingsDialog::~CustomVideoSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void CustomVideoSettingsDialog::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    m_widgets.animated_characters->setState(UserConfigParams::m_animated_characters);
    m_widgets.dof->setState(UserConfigParams::m_dof);

    m_widgets.particles_effects->addLabel(_C("Particle effects", "Disabled"));
    m_widgets.particles_effects->addLabel(_C("Particle effects", "Important only"));
    m_widgets.particles_effects->addLabel(_C("Particle effects", "Enabled"));
    m_widgets.particles_effects->setValue(UserConfigParams::m_particles_effects);

    m_widgets.geometry_detail->addLabel(_C("Geometry level", "Very low"));
    m_widgets.geometry_detail->addLabel(_C("Geometry level", "Low"));
    m_widgets.geometry_detail->addLabel(_C("Geometry level", "Medium"));
    m_widgets.geometry_detail->addLabel(_C("Geometry level", "High"));
    m_widgets.geometry_detail->addLabel(_C("Geometry level", "Very high"));
    m_widgets.geometry_detail->addLabel(_C("Geometry level", "Ultra high"));
    m_widgets.geometry_detail->setValue(UserConfigParams::m_geometry_level);

    m_widgets.image_quality->addLabel(_C("Image quality", "Very low"));
    m_widgets.image_quality->addLabel(_C("Image quality", "Low"));
    m_widgets.image_quality->addLabel(_C("Image quality", "High"));
    m_widgets.image_quality->setValue(GraphicalPresets::getImageQuality());

    m_widgets.shadows->addLabel(_C("Shadows", "Disabled"));   // 0
    m_widgets.shadows->addLabel(_C("Shadows", "Low"));        // 1
    m_widgets.shadows->addLabel(_C("Shadows", "Medium"));     // 2
    m_widgets.shadows->addLabel(_C("Shadows", "High"));       // 3
    m_widgets.shadows->addLabel(_C("Shadows", "Very high"));  // 4
    m_widgets.shadows->setValue(UserConfigParams::m_shadows_resolution == 2048 ?
                      (UserConfigParams::m_pcss ? 4 : 3) :
                      UserConfigParams::m_shadows_resolution == 1024 ? 2 :
                      UserConfigParams::m_shadows_resolution ==  512 ? 1 : 0);

    m_widgets.dynamiclight->setState(UserConfigParams::m_dynamic_lights);
    m_widgets.lightshaft->setState(UserConfigParams::m_light_shaft);
    m_widgets.ibl->setState(!UserConfigParams::m_degraded_IBL);
    m_widgets.motionblur->setState(UserConfigParams::m_motionblur);
    m_widgets.mlaa->setState(UserConfigParams::m_mlaa);
    m_widgets.glow->setState(UserConfigParams::m_glow);
    m_widgets.ssao->setState(UserConfigParams::m_ssao);
    m_widgets.ssr->setState(UserConfigParams::m_ssr);
    m_widgets.bloom->setState(UserConfigParams::m_bloom);
    m_widgets.lightscattering->setState(UserConfigParams::m_light_scatter);
    if (CVS->isEXTTextureCompressionS3TCUsable())
    {
        m_widgets.texture_compression->setState(UserConfigParams::m_texture_compression);
    }
    else
    {
        m_widgets.texture_compression->setState(false);
        m_widgets.texture_compression->setActive(false);
    }

    m_widgets.render_driver->m_properties[PROP_WRAP_AROUND] = "true";
    m_widgets.render_driver->clearLabels();
    m_widgets.render_driver->addLabel("OpenGL");
    m_widgets.render_driver->addLabel("Vulkan");
#ifndef WIN32
    const int rd_count = 2;
#else
    const int rd_count = 3;
    m_widgets.render_driver->addLabel("DirectX9");
#endif
    bool found = false;
    for (int i = 0; i < rd_count; i++)
    {
        std::string rd = StringUtils::wideToUtf8(m_widgets.render_driver->getStringValueFromID(i).make_lower());
        if (std::string(UserConfigParams::m_render_driver) == rd)
        {
            m_widgets.render_driver->setValue(i);
            found = true;
            break;
        }
    }
    if (!found)
    {
        m_widgets.render_driver->addLabel(StringUtils::utf8ToWide(UserConfigParams::m_render_driver));
        m_widgets.render_driver->setValue(rd_count);
    }
    m_widgets.render_driver->setActive(StateManager::get()->getGameState() != GUIEngine::INGAME_MENU);
#endif
} // beforeAddingWidgets

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomVideoSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "render_driver")
    {
        // We will only update settings if the changed renderer is
        // kept when the players chooses to apply the new config
        // However, we immediately update the GUI to show which
        // advanced settings are available or not with the chosen renderer
        std::string rd = StringUtils::wideToUtf8(
            m_widgets.render_driver->getStringValue().make_lower());

        updateActivation(rd);
    }
    if (eventSource == "buttons")
    {
        const std::string& selection = m_widgets.buttons->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            bool advanced_pipeline = m_widgets.dynamiclight->getState();
            bool pbr_changed = false;
            bool ibl_changed = false;
            if (UserConfigParams::m_dynamic_lights != advanced_pipeline)
            {
                pbr_changed = true;
                GE::getGEConfig()->m_pbr = advanced_pipeline;
            }
            UserConfigParams::m_dynamic_lights = advanced_pipeline;

            UserConfigParams::m_dof =
                advanced_pipeline && m_widgets.dof->getState();

            UserConfigParams::m_motionblur      =
                advanced_pipeline && m_widgets.motionblur->getState();

            if (advanced_pipeline)
            {
                UserConfigParams::m_shadows_resolution =
                    m_widgets.shadows->getValue() == 1 ?  512 :
                    m_widgets.shadows->getValue() == 2 ? 1024 :
                    m_widgets.shadows->getValue() >= 3 ? 2048 : 0;
                UserConfigParams::m_pcss =
                    m_widgets.shadows->getValue() == 4 ? true : false;
            }
            else
            {
                UserConfigParams::m_shadows_resolution = 0;
            }

            UserConfigParams::m_mlaa =
                advanced_pipeline && m_widgets.mlaa->getState();

            UserConfigParams::m_ssao =
                advanced_pipeline && m_widgets.ssao->getState();
            UserConfigParams::m_ssr =
                advanced_pipeline && m_widgets.ssr->getState();
            UserConfigParams::m_light_shaft =
                advanced_pipeline && m_widgets.lightshaft->getState();

            bool degraded_ibl = !advanced_pipeline || !m_widgets.ibl->getState();
            if (UserConfigParams::m_degraded_IBL != degraded_ibl)
            {
                ibl_changed = true;
                GE::getGEConfig()->m_ibl = !degraded_ibl;
                UserConfigParams::m_degraded_IBL = degraded_ibl;
            }

            UserConfigParams::m_glow =
                advanced_pipeline && m_widgets.glow->getState();

            UserConfigParams::m_bloom =
                advanced_pipeline && m_widgets.bloom->getState();

            UserConfigParams::m_light_scatter =
                advanced_pipeline && m_widgets.lightscattering->getState();

            bool force_reload_texture = m_widgets.texture_compression->getState() !=
                UserConfigParams::m_texture_compression;
            UserConfigParams::m_texture_compression =
                m_widgets.texture_compression->getState();
            GE::getGEConfig()->m_texture_compression = UserConfigParams::m_texture_compression;

            UserConfigParams::m_particles_effects =
                m_widgets.particles_effects->getValue();

            UserConfigParams::m_animated_characters =
                m_widgets.animated_characters->getState();

            UserConfigParams::m_geometry_level =
                m_widgets.geometry_detail->getValue();;
            int quality = m_widgets.image_quality->getValue();

            std::string rd = StringUtils::wideToUtf8(
                m_widgets.render_driver->getStringValue().make_lower());

            bool need_restart = false;
            if (std::string(UserConfigParams::m_render_driver) != rd)
            {
                UserConfigParams::m_render_driver = rd;
                need_restart = true;
            }

            user_config->saveConfig();

            ModalDialog::dismiss();
            OptionsScreenVideo::getInstance()->updateGfxSlider();
            OptionsScreenVideo::getInstance()->updateBlurSlider();
            GE::GEScreenSpaceReflectionType prev_gssrt = GE::getGEConfig()->m_screen_space_reflection_type;
            OptionsScreenVideo::setSSR();
            if (GE::getDriver()->getDriverType() == video::EDT_VULKAN)
            {
                bool need_recreate_swapchain = GE::getGEConfig()->m_screen_space_reflection_type != prev_gssrt;
                if (need_recreate_swapchain || pbr_changed || ibl_changed)
                    GE::getVKDriver()->updateDriver(need_recreate_swapchain, pbr_changed, ibl_changed);
            }
            // sameRestart will have the same effect
            if (!(CVS->isGLSL() && pbr_changed))
            {
                GraphicalPresets::setImageQuality(quality);
                OptionsScreenVideo::updateImageQuality(force_reload_texture);
            }

            if (need_restart)
                irr_driver->fullRestart();

            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    else if (eventSource == "dynamiclight")
    {
        updateActivation("");
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void CustomVideoSettingsDialog::updateActivation(const std::string& renderer)
{
#ifndef SERVER_ONLY
    bool light = m_widgets.dynamiclight->getState();
    bool real_light = light;
    bool vk = GE::getDriver()->getDriverType() == video::EDT_VULKAN;
    bool modern_gl = CVS->isGLSL();

    // If showing enabled options for a specific renderer has
    // been requested, prioritize that
    if (renderer == "vulkan")
    {
        vk = true;
        modern_gl = false;
    }
    else if (renderer != "") // OpenGL or DirectX
    {
        vk = false;

        if (renderer == "opengl" && !UserConfigParams::m_force_legacy_device)
            modern_gl = true;
        else
            modern_gl = false;
    }

    // Disable the options for advanced lighting if unavailable for this renderer
    if (!vk && !modern_gl)
    {
        m_widgets.dynamiclight->setActive(false);
        light = false;
    }

    if (vk)
    {
        m_widgets.dynamiclight->setActive(true);
        light = false;
    }
    m_widgets.motionblur->setActive(light);
    m_widgets.dof->setActive(light);
    m_widgets.shadows->setActive(light);
    m_widgets.mlaa->setActive(light);
    m_widgets.ssao->setActive(light);
    m_widgets.ssr->setActive(light || (vk && real_light));
    m_widgets.lightshaft->setActive(light);
    m_widgets.ibl->setActive(light || (vk && real_light));
    m_widgets.glow->setActive(light);
    m_widgets.bloom->setActive(light);
    m_widgets.lightscattering->setActive(light);
#endif
}   // updateActivation
