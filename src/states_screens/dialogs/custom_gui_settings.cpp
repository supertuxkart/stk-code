//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 Gustavo Barreira
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

#include "states_screens/dialogs/custom_gui_settings.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
// -----------------------------------------------------------------------------

CustomGuiSettingsDialog::CustomGuiSettingsDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    m_self_destroy = false;
    loadFromFile("custom_gui_settings.stkgui");
}

// -----------------------------------------------------------------------------

CustomGuiSettingsDialog::~CustomGuiSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void CustomGuiSettingsDialog::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    // Steering editing
    getWidget<SpinnerWidget>("steer_position_x")->setRange(0.0f, 20.0f, 0.1f);
    getWidget<SpinnerWidget>("steer_position_y")->setRange(0.0f, 20.0f, 0.1f);

    getWidget<SpinnerWidget>("steer_position_x")->setFloatValue(UserConfigParams::m_steering_btn_pos_x);
    getWidget<SpinnerWidget>("steer_position_y")->setFloatValue(UserConfigParams::m_steering_btn_pos_y);

    // Buttons editing
    getWidget<SpinnerWidget>("btns_position_x")->setRange(0.0f, 20.0f, 0.1f);
    getWidget<SpinnerWidget>("btns_position_y")->setRange(0.0f, 20.0f, 0.1f);
    getWidget<SpinnerWidget>("btns_spacing")->setRange(0.5f, 1.5f, 0.1f);

    getWidget<SpinnerWidget>("btns_position_x")->setFloatValue(UserConfigParams::m_buttons_pos_x);
    getWidget<SpinnerWidget>("btns_position_y")->setFloatValue(UserConfigParams::m_buttons_pos_y);
    getWidget<SpinnerWidget>("btns_spacing")->setFloatValue(UserConfigParams::m_buttons_spacing);

#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomGuiSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
        getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            // Steering editing
            UserConfigParams::m_steering_btn_pos_x = getWidget<SpinnerWidget>("steer_position_x")->getFloatValue();
            UserConfigParams::m_steering_btn_pos_y = getWidget<SpinnerWidget>("steer_position_y")->getFloatValue();

            // Buttons editing
            UserConfigParams::m_buttons_pos_x = getWidget<SpinnerWidget>("btns_position_x")->getFloatValue();
            UserConfigParams::m_buttons_pos_y = getWidget<SpinnerWidget>("btns_position_y")->getFloatValue();
            UserConfigParams::m_buttons_spacing = getWidget<SpinnerWidget>("btns_spacing")->getFloatValue();

            if (World::getWorld() && World::getWorld()->getRaceGUI())
            {
                World::getWorld()->getRaceGUI()->recreateGUI();
            }

            user_config->saveConfig();

            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "reset") // Discard all the changes
        {
        // Steering editing
        getWidget<SpinnerWidget>("steer_position_x")->setFloatValue(12.0f);
        getWidget<SpinnerWidget>("steer_position_y")->setFloatValue(15.0f);

        // Buttons editing
        getWidget<SpinnerWidget>("btns_position_x")->setFloatValue(12.0f);
        getWidget<SpinnerWidget>("btns_position_y")->setFloatValue(15.0f);
        getWidget<SpinnerWidget>("btns_spacing")->setFloatValue(1.0f);
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent
