//  SuperTuxKart - a fun racing game with go-karts
//  Copyright (C) 2006-2017 The SuperTuxKart Team and contributers
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
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/options_screen_video.hpp"
#include "utils/translation.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shared_gpu_objects.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

CustomVideoSettingsDialog::CustomVideoSettingsDialog(const float w, const float h) :
        ModalDialog(w, h), m_all_kart_animated(true)
{
#ifndef SERVER_ONLY
    m_all_kart_animated = SharedGPUObjects::getMaxMat4Size() > 512 ||
        !CVS->supportsHardwareSkinning();
#endif
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
    getWidget<CheckBoxWidget>("anim_gfx")->setState(UserConfigParams::m_graphical_effects);
    getWidget<CheckBoxWidget>("weather_gfx")->setState(UserConfigParams::m_weather_effects);
    getWidget<CheckBoxWidget>("dof")->setState(UserConfigParams::m_dof);
    getWidget<CheckBoxWidget>("hd-textures")
        ->setState((UserConfigParams::m_high_definition_textures & 0x01)==0x01);

    SpinnerWidget* kart_anim = getWidget<SpinnerWidget>("steering_animations");
    kart_anim->addLabel(_("Disabled")); // 0
    //I18N: animations setting (only karts with human players are animated)
    kart_anim->addLabel(_("Human players only")); // 1
    //I18N: animations setting (all karts are animated)
    if (m_all_kart_animated)
        kart_anim->addLabel(_("Enabled for all")); // 2
    kart_anim->setValue(!m_all_kart_animated &&
        UserConfigParams::m_show_steering_animations == 2 ?
        1 : UserConfigParams::m_show_steering_animations);

    SpinnerWidget* geometry_lod = getWidget<SpinnerWidget>("geometry_lod");
    //I18N: Geometry level of detail low : lowest level, no details
    geometry_lod->addLabel(_("Low"));
    //I18N: Geometry level of detail medium : few details are displayed
    geometry_lod->addLabel(_("Medium"));
    //I18N: Geometry level of detail high : everything is displayed
    geometry_lod->addLabel(_("High"));
    geometry_lod->setValue(
        UserConfigParams::m_geometry_lod == 2 ? 0 :
        UserConfigParams::m_geometry_lod == 0 ? 2 : 1);

    SpinnerWidget* filtering = getWidget<SpinnerWidget>("filtering");
    int value = 0;
    if (UserConfigParams::m_anisotropic == 2)  value = 2;
    else if (UserConfigParams::m_anisotropic == 4)  value = 3;
    else if (UserConfigParams::m_anisotropic == 8)  value = 4;
    else if (UserConfigParams::m_anisotropic == 16) value = 5;
    else if (UserConfigParams::m_trilinear)         value = 1;
    filtering->addLabel(_("Bilinear"));        // 0
    filtering->addLabel(_("Trilinear"));       // 1
    filtering->addLabel(_("Anisotropic x2"));  // 2
    filtering->addLabel(_("Anisotropic x4"));  // 3
    filtering->addLabel(_("Anisotropic x8"));  // 4
    filtering->addLabel(_("Anisotropic x16")); // 5

    filtering->setValue(value);

    SpinnerWidget* shadows = getWidget<SpinnerWidget>("shadows");
    shadows->addLabel(_("Disabled"));   // 0
    shadows->addLabel(_("low"));        // 1
    shadows->addLabel(_("high"));       // 2
    if (CVS->supportsShadows())
        shadows->setValue(UserConfigParams::m_shadows_resolution / 512);
    else
        shadows->setValue(0);
    getWidget<CheckBoxWidget>("dynamiclight")->setState(UserConfigParams::m_dynamic_lights);
    getWidget<CheckBoxWidget>("lightshaft")->setState(UserConfigParams::m_light_shaft);
    getWidget<CheckBoxWidget>("ibl")->setState(!UserConfigParams::m_degraded_IBL);
    getWidget<CheckBoxWidget>("global_illumination")->setState(UserConfigParams::m_gi);
    getWidget<CheckBoxWidget>("motionblur")->setState(UserConfigParams::m_motionblur);
    getWidget<CheckBoxWidget>("mlaa")->setState(UserConfigParams::m_mlaa);
    getWidget<CheckBoxWidget>("glow")->setState(UserConfigParams::m_glow);
    getWidget<CheckBoxWidget>("ssao")->setState(UserConfigParams::m_ssao);
    getWidget<CheckBoxWidget>("bloom")->setState(UserConfigParams::m_bloom);
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

    if (!CVS->supportsGlobalIllumination())
    {
        shadows->setActive(false);
        getWidget<CheckBoxWidget>("global_illumination")->setActive(false);
    }
#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomVideoSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "close")
    {
        bool advanced_pipeline = getWidget<CheckBoxWidget>("dynamiclight")->getState();
        UserConfigParams::m_dynamic_lights = advanced_pipeline;

        UserConfigParams::m_dof =
            advanced_pipeline && getWidget<CheckBoxWidget>("dof")->getState();

        UserConfigParams::m_motionblur      =
            advanced_pipeline && getWidget<CheckBoxWidget>("motionblur")->getState();

        if (advanced_pipeline && CVS->supportsShadows())
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
            !getWidget<CheckBoxWidget>("ibl")->getState();

        UserConfigParams::m_gi =
            advanced_pipeline && CVS->supportsGlobalIllumination() &&
            getWidget<CheckBoxWidget>("global_illumination")->getState();

        UserConfigParams::m_glow =
            advanced_pipeline && getWidget<CheckBoxWidget>("glow")->getState();

        UserConfigParams::m_bloom =
            advanced_pipeline && getWidget<CheckBoxWidget>("bloom")->getState();

        UserConfigParams::m_texture_compression =
            getWidget<CheckBoxWidget>("texture_compression")->getState();

        UserConfigParams::m_graphical_effects =
            getWidget<CheckBoxWidget>("anim_gfx")->getState();

        UserConfigParams::m_weather_effects =
            getWidget<CheckBoxWidget>("weather_gfx")->getState();

        // Set bit 0 for enabled/disabled, and set bit 1 to indicate that this
        // is now a user's choice and should not be overwritten by any default
        UserConfigParams::m_high_definition_textures =
            getWidget<CheckBoxWidget>("hd-textures")->getState() ? 0x03 : 0x02;

        UserConfigParams::m_show_steering_animations =
            getWidget<SpinnerWidget>("steering_animations")->getValue();

        const int val =
            getWidget<SpinnerWidget>("geometry_lod")->getValue();
        UserConfigParams::m_geometry_lod = val == 2 ? 0 : val == 0 ? 2 : 1;

        switch (getWidget<SpinnerWidget>("filtering")->getValue())
        {
            case 0:
                UserConfigParams::m_anisotropic = 0;
                UserConfigParams::m_trilinear   = false;
                break;
            case 1:
                UserConfigParams::m_anisotropic = 0;
                UserConfigParams::m_trilinear   = true;
                break;
            case 2:
                UserConfigParams::m_anisotropic = 2;
                UserConfigParams::m_trilinear   = true;
                break;
            case 3:
                UserConfigParams::m_anisotropic = 4;
                UserConfigParams::m_trilinear   = true;
                break;
            case 4:
                UserConfigParams::m_anisotropic = 8;
                UserConfigParams::m_trilinear   = true;
                break;
            case 5:
                UserConfigParams::m_anisotropic = 16;
                UserConfigParams::m_trilinear   = true;
                break;
        }

        user_config->saveConfig();

        ModalDialog::dismiss();
        OptionsScreenVideo::getInstance()->updateGfxSlider();
        return GUIEngine::EVENT_BLOCK;
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
    getWidget<CheckBoxWidget>("motionblur")->setActive(light);
    getWidget<CheckBoxWidget>("dof")->setActive(light);
    getWidget<SpinnerWidget>("shadows")->setActive(light);
    getWidget<CheckBoxWidget>("mlaa")->setActive(light);
    getWidget<CheckBoxWidget>("ssao")->setActive(light);
    getWidget<CheckBoxWidget>("lightshaft")->setActive(light);
    getWidget<CheckBoxWidget>("ibl")->setActive(light);
    getWidget<CheckBoxWidget>("global_illumination")->setActive(light);
    getWidget<CheckBoxWidget>("glow")->setActive(light);
    getWidget<CheckBoxWidget>("bloom")->setActive(light);
    getWidget<SpinnerWidget>("steering_animations")
        ->setMax(m_all_kart_animated ? 2 : 1);

    if (!CVS->supportsShadows() && !CVS->supportsGlobalIllumination())
    {
        getWidget<SpinnerWidget>("shadows")->setActive(false);
        getWidget<CheckBoxWidget>("global_illumination")->setActive(false);
    }
#endif
}   // updateActivation

