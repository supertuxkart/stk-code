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
#include "utils/translation.hpp"
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
    updateActivation();
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
    //I18N: Geometry level disabled : lowest level, no details
    geometry_level->addLabel(_("Very Low"));
    //I18N: Geometry level low : few details are displayed
    geometry_level->addLabel(_("Low"));
    //I18N: Geometry level high : everything is displayed
    geometry_level->addLabel(_("High"));
    geometry_level->setValue(
        UserConfigParams::m_geometry_level == 2 ? 0 :
        UserConfigParams::m_geometry_level == 0 ? 2 : 1);

    SpinnerWidget* filtering = getWidget<SpinnerWidget>("image_quality");
    filtering->addLabel(_("Very Low"));
    filtering->addLabel(_("Low"));
    filtering->addLabel(_("High"));
    filtering->setValue(OptionsScreenVideo::getImageQuality());

    SpinnerWidget* shadows = getWidget<SpinnerWidget>("shadows");
    shadows->addLabel(_("Disabled"));   // 0
    shadows->addLabel(_("Low"));        // 1
    shadows->addLabel(_("High"));       // 2
    shadows->setValue(UserConfigParams::m_shadows_resolution / 512);

    getWidget<CheckBoxWidget>("dynamiclight")->setState(UserConfigParams::m_dynamic_lights);
    getWidget<CheckBoxWidget>("lightshaft")->setState(UserConfigParams::m_light_shaft);
    getWidget<CheckBoxWidget>("ibl")->setState(!UserConfigParams::m_degraded_IBL);
    getWidget<CheckBoxWidget>("motionblur")->setState(UserConfigParams::m_motionblur);
    getWidget<CheckBoxWidget>("mlaa")->setState(UserConfigParams::m_mlaa);
    getWidget<CheckBoxWidget>("glow")->setState(UserConfigParams::m_glow);
    getWidget<CheckBoxWidget>("ssao")->setState(UserConfigParams::m_ssao);
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
#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomVideoSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            bool advanced_pipeline = getWidget<CheckBoxWidget>("dynamiclight")->getState();
            bool update_needed = false;
            if (UserConfigParams::m_dynamic_lights != advanced_pipeline)
            {
                update_needed = true;
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
                    getWidget<SpinnerWidget>("shadows")->getValue() * 512;
            }
            else
            {
                UserConfigParams::m_shadows_resolution = 0;
            }

            UserConfigParams::m_mlaa =
                advanced_pipeline && getWidget<CheckBoxWidget>("mlaa")->getState();

            UserConfigParams::m_ssao =
                advanced_pipeline && getWidget<CheckBoxWidget>("ssao")->getState();

            UserConfigParams::m_light_shaft =
                advanced_pipeline && getWidget<CheckBoxWidget>("lightshaft")->getState();

            UserConfigParams::m_degraded_IBL =
                !advanced_pipeline || !getWidget<CheckBoxWidget>("ibl")->getState();

            UserConfigParams::m_glow =
                advanced_pipeline && getWidget<CheckBoxWidget>("glow")->getState();

            UserConfigParams::m_bloom =
                advanced_pipeline && getWidget<CheckBoxWidget>("bloom")->getState();

            UserConfigParams::m_light_scatter =
                advanced_pipeline && getWidget<CheckBoxWidget>("lightscattering")->getState();

            UserConfigParams::m_texture_compression =
                getWidget<CheckBoxWidget>("texture_compression")->getState();
#ifndef SERVER_ONLY
            GE::getGEConfig()->m_texture_compression = UserConfigParams::m_texture_compression;
#endif

            UserConfigParams::m_particles_effects =
                getWidget<SpinnerWidget>("particles_effects")->getValue();

            UserConfigParams::m_animated_characters =
                getWidget<CheckBoxWidget>("animated_characters")->getState();

            const int val =
                getWidget<SpinnerWidget>("geometry_detail")->getValue();
            UserConfigParams::m_geometry_level = val == 2 ? 0 : val == 0 ? 2 : 1;
            int quality = getWidget<SpinnerWidget>("image_quality")->getValue();

            user_config->saveConfig();

            ModalDialog::dismiss();
            OptionsScreenVideo::getInstance()->updateGfxSlider();
            OptionsScreenVideo::getInstance()->updateBlurSlider();
#ifndef SERVER_ONLY
            if (update_needed && GE::getDriver()->getDriverType() == video::EDT_VULKAN)
                GE::getVKDriver()->updateDriver(true);
#endif
            OptionsScreenVideo::setImageQuality(quality);
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
        updateActivation();
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void CustomVideoSettingsDialog::updateActivation()
{
#ifndef SERVER_ONLY
    bool light = getWidget<CheckBoxWidget>("dynamiclight")->getState();
    if (!CVS->isGLSL())
    {
        getWidget<CheckBoxWidget>("dynamiclight")->setActive(false);
        light = false;
    }
    if (GE::getDriver()->getDriverType() == video::EDT_VULKAN)
        getWidget<CheckBoxWidget>("dynamiclight")->setActive(true);
    getWidget<CheckBoxWidget>("motionblur")->setActive(light);
    getWidget<CheckBoxWidget>("dof")->setActive(light);
    getWidget<SpinnerWidget>("shadows")->setActive(light);
    getWidget<CheckBoxWidget>("mlaa")->setActive(light);
    getWidget<CheckBoxWidget>("ssao")->setActive(light);
    getWidget<CheckBoxWidget>("lightshaft")->setActive(light);
    getWidget<CheckBoxWidget>("ibl")->setActive(light);
    getWidget<CheckBoxWidget>("glow")->setActive(light);
    getWidget<CheckBoxWidget>("bloom")->setActive(light);
    getWidget<CheckBoxWidget>("lightscattering")->setActive(light);
#endif
}   // updateActivation

