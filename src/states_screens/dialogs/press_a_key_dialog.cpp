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


#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "states_screens/options/options_screen_device.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

PressAKeyDialog::PressAKeyDialog(const float w, const float h, const bool isKeyboardFlag) :
        ModalDialog(w, h)
{
    loadFromFile("press_a_key_dialog.stkgui");
    Widget* title = getWidget("title");
    if(isKeyboardFlag)
    {
        // I18N: In press a key dialog, tell user to press a key to bind configuration
        title->setText(_("Press any key...\n(Press ESC to cancel)"));
    }
    else
    {
        // Gamepad configuration, rename title to omit keyboard buttons
        // and hide assign esc button
        title->setText(_("Press any key..."));
        getWidget<IconButtonWidget>("assignEsc")->setVisible(false);
    }
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation PressAKeyDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "assignEsc")
        {
            Input simulatedInput(Input::IT_KEYBOARD, 0 /* deviceID */,
                             IRR_KEY_ESCAPE);
            OptionsScreenDevice::getInstance()->gotSensedInput(simulatedInput);
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "assignNone")
        {
            Input simulatedInput;
            OptionsScreenDevice::getInstance()->gotSensedInput(simulatedInput);
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "cancel")
        {
            input_manager->setMode(InputManager::MENU);
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------
