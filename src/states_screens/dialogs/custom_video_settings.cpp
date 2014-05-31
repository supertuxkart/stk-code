//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

CustomVideoSettingsDialog::CustomVideoSettingsDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("custom_video_settings.stkgui");
}

// -----------------------------------------------------------------------------

CustomVideoSettingsDialog::~CustomVideoSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void CustomVideoSettingsDialog::beforeAddingWidgets()
{
    getWidget<CheckBoxWidget>("anim_gfx")->setState( UserConfigParams::m_graphical_effects );
    getWidget<CheckBoxWidget>("weather_gfx")->setState( UserConfigParams::m_weather_effects );
    getWidget<CheckBoxWidget>("ubo")->setState(!UserConfigParams::m_ubo_disabled);
    getWidget<CheckBoxWidget>("dof")->setState(UserConfigParams::m_dof);
    getWidget<CheckBoxWidget>("hd-textures")->setState(UserConfigParams::m_high_definition_textures);

    SpinnerWidget* kart_anim = getWidget<SpinnerWidget>("steering_animations");
    kart_anim->addLabel( _("Disabled") ); // 0
    //I18N: animations setting (only karts with human players are animated)
    kart_anim->addLabel( _("Human players only") ); // 1
    //I18N: animations setting (all karts are animated)
    kart_anim->addLabel( _("Enabled for all") ); // 2
    kart_anim->setValue( UserConfigParams::m_show_steering_animations );

    SpinnerWidget* filtering = getWidget<SpinnerWidget>("filtering");
    int value = 0;
    if      (UserConfigParams::m_anisotropic == 2)  value = 2;
    else if (UserConfigParams::m_anisotropic == 4)  value = 3;
    else if (UserConfigParams::m_anisotropic == 8)  value = 4;
    else if (UserConfigParams::m_anisotropic == 16) value = 5;
    else if (UserConfigParams::m_trilinear)         value = 1;
    filtering->addLabel( _("Bilinear") );        // 0
    filtering->addLabel( _("Trilinear") );       // 1
    filtering->addLabel( _("Anisotropic x2") );  // 2
    filtering->addLabel( _("Anisotropic x4") );  // 3
    filtering->addLabel( _("Anisotropic x8") );  // 4
    filtering->addLabel( _("Anisotropic x16") ); // 5

    filtering->setValue( value );

    SpinnerWidget* shadows = getWidget<SpinnerWidget>("shadows");
    shadows->addLabel( _("Disabled") );   // 0
    shadows->addLabel( _("low") );        // 1
    shadows->addLabel( _("high") );       // 2
    shadows->setValue( UserConfigParams::m_shadows );
    
    getWidget<CheckBoxWidget>("dynamiclight")->setState(UserConfigParams::m_dynamic_lights);
    getWidget<CheckBoxWidget>("lightshaft")->setState(UserConfigParams::m_light_shaft);
    getWidget<CheckBoxWidget>("global_illumination")->setState(UserConfigParams::m_gi);
    getWidget<CheckBoxWidget>("motionblur")->setState(UserConfigParams::m_motionblur);
    getWidget<CheckBoxWidget>("mlaa")->setState(UserConfigParams::m_mlaa);
    getWidget<CheckBoxWidget>("glow")->setState(UserConfigParams::m_glow);
    getWidget<CheckBoxWidget>("ssao")->setState(UserConfigParams::m_ssao);
    getWidget<CheckBoxWidget>("bloom")->setState(UserConfigParams::m_bloom);
    getWidget<CheckBoxWidget>("texture_compression")->setState(UserConfigParams::m_texture_compression);
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomVideoSettingsDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "close")
    {
        bool dynamic_light = getWidget<CheckBoxWidget>("dynamiclight")->getState();
        UserConfigParams::m_dynamic_lights = dynamic_light;

        UserConfigParams::m_graphical_effects =
            getWidget<CheckBoxWidget>("anim_gfx")->getState();
        UserConfigParams::m_weather_effects =
            getWidget<CheckBoxWidget>("weather_gfx")->getState();
        UserConfigParams::m_ubo_disabled             =
            !getWidget<CheckBoxWidget>("ubo")->getState();
        UserConfigParams::m_dof =
            getWidget<CheckBoxWidget>("dof")->getState();
        UserConfigParams::m_high_definition_textures =
            getWidget<CheckBoxWidget>("hd-textures")->getState();

        UserConfigParams::m_motionblur      =
            getWidget<CheckBoxWidget>("motionblur")->getState();
        UserConfigParams::m_show_steering_animations =
            getWidget<SpinnerWidget>("steering_animations")->getValue();

        if (dynamic_light)
        {
            UserConfigParams::m_shadows =
                getWidget<SpinnerWidget>("shadows")->getValue();
        }
        else
        {
            UserConfigParams::m_shadows = 0;
        }

        UserConfigParams::m_mlaa =
            getWidget<CheckBoxWidget>("mlaa")->getState();

        UserConfigParams::m_ssao =
            getWidget<CheckBoxWidget>("ssao")->getState();

        UserConfigParams::m_light_shaft =
            getWidget<CheckBoxWidget>("lightshaft")->getState();

        UserConfigParams::m_gi =
            getWidget<CheckBoxWidget>("global_illumination")->getState();

        UserConfigParams::m_glow =
            getWidget<CheckBoxWidget>("glow")->getState();

        UserConfigParams::m_bloom =
            getWidget<CheckBoxWidget>("bloom")->getState();

        UserConfigParams::m_texture_compression =
            getWidget<CheckBoxWidget>("texture_compression")->getState();

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

    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------


