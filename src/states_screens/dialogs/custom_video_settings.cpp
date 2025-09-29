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
    getWidget<CheckBoxWidget>("animated_characters")
        ->setState(UserConfigParams::m_animated_characters);
    getWidget<CheckBoxWidget>("dof")->setState(UserConfigParams::m_dof);
    
    SpinnerWidget* particles_effects = getWidget<SpinnerWidget>("particles_effects");
    assert(particles_effects != NULL);
    particles_effects->addLabel(_("Disabled"));
    particles_effects->addLabel(_("Important only"));
    particles_effects->addLabel(_("Enabled"));
    particles_effects->setValue(UserConfigParams::m_particles_effects);

    SpinnerWidget* geometry_level = getWidget<SpinnerWidget>("geometry_detail");
    //I18N: Geometry level disabled : lowest level, Level-of-Details distances are very low
    geometry_level->addLabel(_("Very Low"));
    //I18N: Geometry level low : everything is displayed, Level-of-Details distances are low
    geometry_level->addLabel(_("Low"));
    //I18N: Geometry level medium : everything is displayed, Level-of-Details distances are medium
    geometry_level->addLabel(_("Medium"));
    //I18N: Geometry level high : everything is displayed, Level-of-Details distances are high
    geometry_level->addLabel(_("High"));
    //I18N: Geometry level very high : everything is displayed, Level-of-Details distances are very high
    geometry_level->addLabel(_("Very High"));
    //I18N: Geometry level ultra : everything is displayed, Level-of-Details distances are extremely high
    geometry_level->addLabel(_("Ultra"));
    geometry_level->setValue(UserConfigParams::m_geometry_level);

    SpinnerWidget* filtering = getWidget<SpinnerWidget>("image_quality");
    filtering->addLabel(_("Very Low"));
    filtering->addLabel(_("Low"));
    filtering->addLabel(_("Medium"));
    filtering->addLabel(_("High"));
    filtering->setValue(OptionsScreenVideo::getImageQuality());

    SpinnerWidget* shadows = getWidget<SpinnerWidget>("shadows");
    shadows->addLabel(_("Disabled"));   // 0
    shadows->addLabel(_("Low"));        // 1
    shadows->addLabel(_("Medium"));     // 2
    shadows->addLabel(_("High"));       // 3
    shadows->addLabel(_("Very High"));  // 4
    shadows->setValue(UserConfigParams::m_shadows_resolution == 2048 ? 
                      (UserConfigParams::m_pcss ? 4 : 3) :
                      UserConfigParams::m_shadows_resolution == 1024 ? 2 :
                      UserConfigParams::m_shadows_resolution ==  512 ? 1 : 0);

    getWidget<CheckBoxWidget>("dynamiclight")->setState(UserConfigParams::m_dynamic_lights);
    getWidget<CheckBoxWidget>("lightshaft")->setState(UserConfigParams::m_light_shaft);
    getWidget<CheckBoxWidget>("ibl")->setState(!UserConfigParams::m_degraded_IBL);
    getWidget<CheckBoxWidget>("motionblur")->setState(UserConfigParams::m_motionblur);
    getWidget<CheckBoxWidget>("mlaa")->setState(UserConfigParams::m_mlaa);
    getWidget<CheckBoxWidget>("glow")->setState(UserConfigParams::m_glow);
    getWidget<CheckBoxWidget>("ssao")->setState(UserConfigParams::m_ssao);
    getWidget<CheckBoxWidget>("ssr")->setState(UserConfigParams::m_ssr);
    getWidget<CheckBoxWidget>("bloom")->setState(UserConfigParams::m_bloom);
    getWidget<CheckBoxWidget>("lightscattering")->setState(UserConfigParams::m_light_scatter);
    if (CVS->isEXTTextureCompressionS3TCUsable())
    {
        getWidget<CheckBoxWidget>("texture_compression")->setState(UserConfigParams::m_texture_compression);
    }
    else
    {
        CheckBoxWidget* cb_tex_cmp = getWidget<CheckBoxWidget>("texture_compression");
        cb_tex_cmp->setState(false);
        cb_tex_cmp->setActive(false);
    }

    GUIEngine::SpinnerWidget* rds = getWidget<GUIEngine::SpinnerWidget>("render_driver");
    assert( rds != NULL );

    rds->m_properties[PROP_WRAP_AROUND] = "true";
    rds->clearLabels();
    rds->addLabel("OpenGL");
    rds->addLabel("Vulkan");
#ifndef WIN32
    const int rd_count = 2;
#else
    const int rd_count = 3;
    rds->addLabel("DirectX9");
#endif
    bool found = false;
    for (int i = 0; i < rd_count; i++)
    {
        std::string rd = StringUtils::wideToUtf8(rds->getStringValueFromID(i).make_lower());
        if (std::string(UserConfigParams::m_render_driver) == rd)
        {
            rds->setValue(i);
            found = true;
            break;
        }
    }
    if (!found)
    {
        rds->addLabel(StringUtils::utf8ToWide(UserConfigParams::m_render_driver));
        rds->setValue(rd_count);
    }
    rds->setActive(StateManager::get()->getGameState() != GUIEngine::INGAME_MENU);
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
            getWidget<GUIEngine::SpinnerWidget>("render_driver")->getStringValue().make_lower());

        updateActivation(rd);
    }
    if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            bool advanced_pipeline = getWidget<CheckBoxWidget>("dynamiclight")->getState();
            bool pbr_changed = false;
            bool ibl_changed = false;
            if (UserConfigParams::m_dynamic_lights != advanced_pipeline)
            {
                pbr_changed = true;
                GE::getGEConfig()->m_pbr = advanced_pipeline;
            }
            UserConfigParams::m_dynamic_lights = advanced_pipeline;

            UserConfigParams::m_dof =
                advanced_pipeline && getWidget<CheckBoxWidget>("dof")->getState();

            UserConfigParams::m_motionblur      =
                advanced_pipeline && getWidget<CheckBoxWidget>("motionblur")->getState();

            if (advanced_pipeline)
            {
                UserConfigParams::m_shadows_resolution =
                    getWidget<SpinnerWidget>("shadows")->getValue() == 1 ?  512 :
                    getWidget<SpinnerWidget>("shadows")->getValue() == 2 ? 1024 :
                    getWidget<SpinnerWidget>("shadows")->getValue() >= 3 ? 2048 : 0;
                UserConfigParams::m_pcss = 
                    getWidget<SpinnerWidget>("shadows")->getValue() == 4 ? true : false;
            }
            else
            {
                UserConfigParams::m_shadows_resolution = 0;
            }

            UserConfigParams::m_mlaa =
                advanced_pipeline && getWidget<CheckBoxWidget>("mlaa")->getState();

            UserConfigParams::m_ssao =
                advanced_pipeline && getWidget<CheckBoxWidget>("ssao")->getState();
            UserConfigParams::m_ssr =
                advanced_pipeline && getWidget<CheckBoxWidget>("ssr")->getState();
            UserConfigParams::m_light_shaft =
                advanced_pipeline && getWidget<CheckBoxWidget>("lightshaft")->getState();

            bool degraded_ibl = !advanced_pipeline || !getWidget<CheckBoxWidget>("ibl")->getState();
            if (UserConfigParams::m_degraded_IBL != degraded_ibl)
            {
                ibl_changed = true;
                GE::getGEConfig()->m_ibl = !degraded_ibl;
                UserConfigParams::m_degraded_IBL = degraded_ibl;
            }

            UserConfigParams::m_glow =
                advanced_pipeline && getWidget<CheckBoxWidget>("glow")->getState();

            UserConfigParams::m_bloom =
                advanced_pipeline && getWidget<CheckBoxWidget>("bloom")->getState();

            UserConfigParams::m_light_scatter =
                advanced_pipeline && getWidget<CheckBoxWidget>("lightscattering")->getState();

            bool force_reload_texture = getWidget<CheckBoxWidget>("texture_compression")->getState() !=
                UserConfigParams::m_texture_compression;
            UserConfigParams::m_texture_compression =
                getWidget<CheckBoxWidget>("texture_compression")->getState();
            GE::getGEConfig()->m_texture_compression = UserConfigParams::m_texture_compression;

            UserConfigParams::m_particles_effects =
                getWidget<SpinnerWidget>("particles_effects")->getValue();

            UserConfigParams::m_animated_characters =
                getWidget<CheckBoxWidget>("animated_characters")->getState();

            UserConfigParams::m_geometry_level =
                getWidget<SpinnerWidget>("geometry_detail")->getValue();;
            int quality = getWidget<SpinnerWidget>("image_quality")->getValue();

            std::string rd = StringUtils::wideToUtf8(
                getWidget<GUIEngine::SpinnerWidget>("render_driver")->getStringValue().make_lower());

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
                OptionsScreenVideo::setImageQuality(quality, force_reload_texture);

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
    bool light = getWidget<CheckBoxWidget>("dynamiclight")->getState();
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
        getWidget<CheckBoxWidget>("dynamiclight")->setActive(false);
        light = false;
    }

    if (vk)
    {
        getWidget<CheckBoxWidget>("dynamiclight")->setActive(true);
        light = false;
    }
    getWidget<CheckBoxWidget>("motionblur")->setActive(light);
    getWidget<CheckBoxWidget>("dof")->setActive(light);
    getWidget<SpinnerWidget>("shadows")->setActive(light);
    getWidget<CheckBoxWidget>("mlaa")->setActive(light);
    getWidget<CheckBoxWidget>("ssao")->setActive(light);
    getWidget<CheckBoxWidget>("ssr")->setActive(light || (vk && real_light));
    getWidget<CheckBoxWidget>("lightshaft")->setActive(light);
    getWidget<CheckBoxWidget>("ibl")->setActive(light || (vk && real_light));
    getWidget<CheckBoxWidget>("glow")->setActive(light);
    getWidget<CheckBoxWidget>("bloom")->setActive(light);
    getWidget<CheckBoxWidget>("lightscattering")->setActive(light);
#endif
}   // updateActivation
