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

#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_input2.hpp"
#include "states_screens/options_screen_av.hpp"
#include "states_screens/options_screen_players.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>
#include <set>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenInput2 );

// -----------------------------------------------------------------------------

OptionsScreenInput2::OptionsScreenInput2() : Screen("options_device.stkgui")
{
    m_config = NULL;
}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::loadedFromFile()
{    
}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::updateInputButtons()
{
    assert(m_config != NULL);
    
    // to detect duplicate entries
    std::set<core::stringw> existing_bindings;
    
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_up");
        core::stringw binding_name = m_config->getBindingAsString(PA_ACCEL);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_down");
        core::stringw binding_name = m_config->getBindingAsString(PA_BRAKE);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_left");
        core::stringw binding_name = m_config->getBindingAsString(PA_LEFT);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_right");
        core::stringw binding_name = m_config->getBindingAsString(PA_RIGHT);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    
    std::set<core::stringw>::iterator it;
    
    
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_fire");
        core::stringw binding_name = m_config->getBindingAsString(PA_FIRE);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
            //std::cout << "Setting bad badge!!!!\n";
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_nitro");
        core::stringw binding_name = m_config->getBindingAsString(PA_NITRO);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_drift");
        core::stringw binding_name = m_config->getBindingAsString(PA_DRIFT);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_rescue");
        core::stringw binding_name = m_config->getBindingAsString(PA_RESCUE);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_look_back");
        core::stringw binding_name = m_config->getBindingAsString(PA_LOOK_BACK);
        btn->setLabel( binding_name );
        
        // check if another binding already uses this key
        if (existing_bindings.find(binding_name) != existing_bindings.end())
        {
            btn->setBadge(BAD_BADGE);
        }
        else
        {
            existing_bindings.insert(binding_name);
            btn->resetAllBadges();
        }
    }
}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::init()
{
    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    if (tabBar != NULL)  tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );

    updateInputButtons();
    
    LabelWidget* label = this->getWidget<LabelWidget>("title");
    label->setText( m_config->getName().c_str() );
}

// -----------------------------------------------------------------------------

static PlayerAction binding_to_set;
static std::string binding_to_set_button;

void OptionsScreenInput2::gotSensedInput(Input* sensedInput)
{
    const bool keyboard = (m_config->getType() == DEVICE_CONFIG_TYPE_KEYBOARD &&
                           sensedInput->type == Input::IT_KEYBOARD);
    const bool gamepad =  (sensedInput->type == Input::IT_STICKMOTION ||
                           sensedInput->type == Input::IT_STICKBUTTON) &&
                           m_config->getType() == DEVICE_CONFIG_TYPE_GAMEPAD;
        
    if (keyboard)
    {
		if (UserConfigParams::m_verbosity>=5)
        {
			std::cout << "% Binding " << KartActionStrings[binding_to_set] 
				      << " : setting to keyboard key " << sensedInput->btnID << " \n\n";
        }
        
        KeyboardConfig* keyboard = (KeyboardConfig*)m_config;
        keyboard->setBinding(binding_to_set, Input::IT_KEYBOARD, sensedInput->btnID, Input::AD_NEUTRAL);
        
        // refresh display
        init();
    }
    else if (gamepad)
    {
		if (UserConfigParams::m_verbosity>=5)
        {
			std::cout << "% Binding " << KartActionStrings[binding_to_set] 
		              << " : setting to gamepad #" << sensedInput->deviceID << " : ";
        
            if (sensedInput->type == Input::IT_STICKMOTION)
            {
                std::cout << "axis " << sensedInput->btnID << " direction "
                          << (sensedInput->axisDirection == Input::AD_NEGATIVE ? "-" : "+") << "\n\n";
            }
            else if (sensedInput->type == Input::IT_STICKBUTTON)
            {
                std::cout << "button " << sensedInput->btnID << "\n\n";
            }
            else
            {
                std::cout << "Sensed unknown gamepad event type??\n";
            }
        }
        
        GamepadConfig* config =  (GamepadConfig*)m_config;
        config->setBinding(binding_to_set, sensedInput->type, sensedInput->btnID,
                           (Input::AxisDirection)sensedInput->axisDirection);
        
        // refresh display
        init();
    }
    else
    {
        return;
    }
    
    ModalDialog::dismiss();
    input_manager->setMode(InputManager::MENU);
    
    // re-select the previous button
    ButtonWidget* btn = this->getWidget<ButtonWidget>(binding_to_set_button.c_str());
    if(btn != NULL) btn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    // save new binding to file
    input_manager->getDeviceList()->serialize();
}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::tearDown()
{
}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    //const std::string& screen_name = this->getName();
    
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        
        if (selection == "tab_audio_video")
        {
            StateManager::get()->replaceTopMostScreen(OptionsScreenAV::getInstance());
        }
        else if (selection == "tab_players")
        {
            StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        }
        else if (selection == "tab_controls")
        {
        }
    }
    else if (name == "back_to_device_list")
    {
        StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name.find("binding_") != std::string::npos)
    {
        binding_to_set_button = name;
        
        if(name == "binding_up")
        {
            binding_to_set = PA_ACCEL;
        }
        else if(name == "binding_down")
        {
            binding_to_set = PA_BRAKE;
        }
        else if(name == "binding_left")
        {
            binding_to_set = PA_LEFT;
        }
        else if(name == "binding_right")
        {
            binding_to_set = PA_RIGHT;
        }
        else if(name == "binding_fire")
        {
            binding_to_set = PA_FIRE;
        }
        else if(name == "binding_nitro")
        {
            binding_to_set = PA_NITRO;
        }
        else if(name == "binding_drift")
        {
            binding_to_set = PA_DRIFT;
        }
        else if(name == "binding_rescue")
        {
            binding_to_set = PA_RESCUE;
        }
        else if(name == "binding_look_back")
        {
            binding_to_set = PA_LOOK_BACK;
        }
        else
        {
            std::cerr << "Unknown binding name : " << name.c_str() << std::endl;
            return;
        }
        

		if (UserConfigParams::m_verbosity>=5)
        {
			std::cout << "\n% Entering sensing mode for " 
			          << m_config->getName().c_str() 
					  << std::endl;
        }
        
        new PressAKeyDialog(0.4f, 0.4f);
        
        if (m_config->getType() == DEVICE_CONFIG_TYPE_KEYBOARD)
        {
            input_manager->setMode(InputManager::INPUT_SENSE_KEYBOARD);
        }
        else if (m_config->getType() == DEVICE_CONFIG_TYPE_GAMEPAD)
        {
            input_manager->setMode(InputManager::INPUT_SENSE_GAMEPAD);
        }
        else
        {
            std::cerr << "unknown selection device in options : " << m_config->getName().c_str() << std::endl;
        }
        
    }

}

// -----------------------------------------------------------------------------

void OptionsScreenInput2::unloaded()
{
}

