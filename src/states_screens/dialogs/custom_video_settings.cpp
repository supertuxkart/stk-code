//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

CustomVideoSettingsialog::CustomVideoSettingsialog(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("custom_video_settings.stkgui");
}

// -----------------------------------------------------------------------------

CustomVideoSettingsialog::~CustomVideoSettingsialog()
{
}

// -----------------------------------------------------------------------------

void CustomVideoSettingsialog::beforeAddingWidgets()
{
    getWidget<CheckBoxWidget>("anim_gfx")->setState( UserConfigParams::m_graphical_effects );
    getWidget<CheckBoxWidget>("weather_gfx")->setState( UserConfigParams::m_weather_effects );
    
    SpinnerWidget* kart_anim = getWidget<SpinnerWidget>("steering_animations");
    kart_anim->addLabel( _("Disabled") ); // 0
    //I18N: animations setting (only karts with human players are animated)
    kart_anim->addLabel( _("Human players only") ); // 1
    //I18N: animations setting (all karts are animated)
    kart_anim->addLabel( _("Enabled for all") ); // 2
    kart_anim->setValue( UserConfigParams::m_show_steering_animations );

    SpinnerWidget* filtering = getWidget<SpinnerWidget>("filtering");
    int value = 0;
    if (UserConfigParams::m_anisotropic)   value = 2;
    else if (UserConfigParams::m_trilinear) value = 1;
    filtering->addLabel( L"Bilinear" );    // 0
    filtering->addLabel( L"Trilinear" );   // 1
    filtering->addLabel( L"Anisotropic" ); // 2
    printf("filtering : %i\n", value);
    filtering->setValue( value );

    getWidget<CheckBoxWidget>("antialiasing")->setState( UserConfigParams::m_fullscreen_antialiasing );
    getWidget<CheckBoxWidget>("postprocessing")->setState( UserConfigParams::m_postprocess_enabled );
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomVideoSettingsialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "close")
    {
        UserConfigParams::m_graphical_effects        =
            getWidget<CheckBoxWidget>("anim_gfx")->getState();
        UserConfigParams::m_weather_effects          =
            getWidget<CheckBoxWidget>("weather_gfx")->getState();
        UserConfigParams::m_fullscreen_antialiasing  =
            getWidget<CheckBoxWidget>("antialiasing")->getState();
        UserConfigParams::m_postprocess_enabled      =
            getWidget<CheckBoxWidget>("postprocessing")->getState();
        UserConfigParams::m_show_steering_animations = 
            getWidget<SpinnerWidget>("steering_animations")->getValue();
        
        switch (getWidget<SpinnerWidget>("filtering")->getValue())
        {
            case 0:
                UserConfigParams::m_anisotropic = false;
                UserConfigParams::m_trilinear   = false;
                break;
            case 1:
                UserConfigParams::m_anisotropic = false;
                UserConfigParams::m_trilinear   = true;
                break;
            case 2:
                UserConfigParams::m_anisotropic = true;
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


