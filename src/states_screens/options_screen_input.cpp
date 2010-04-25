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
#include "states_screens/options_screen_av.hpp"
#include "states_screens/options_screen_players.hpp"

#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/add_device_dialog.hpp"
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>
#include <set>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenInput );

// -----------------------------------------------------------------------------

OptionsScreenInput::OptionsScreenInput() : Screen("options_input.stkgui")
{
    m_inited = false;
}

// -----------------------------------------------------------------------------

void OptionsScreenInput::updateInputButtons(DeviceConfig* config)
{
    
    // Should never happen
    if (config == NULL)
    {
        printf("ERROR: No configuration associated with device?\n");
        abort();
    }
    
    // to detect duplicate entries
    std::set<core::stringw> existing_bindings;
    
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_up");
        core::stringw binding_name = config->getBindingAsString(PA_ACCEL);
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
        core::stringw binding_name = config->getBindingAsString(PA_BRAKE);
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
        core::stringw binding_name = config->getBindingAsString(PA_LEFT);
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
        core::stringw binding_name = config->getBindingAsString(PA_RIGHT);
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
    
    /*
    std::cout << "existing_bindings contains:";
    for ( it=existing_bindings.begin() ; it != existing_bindings.end(); it++ )
    {
        std::wcout << (*it).c_str() << ", ";
    }
    std::cout << "\n";
    */
    
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_fire");
        core::stringw binding_name = config->getBindingAsString(PA_FIRE);
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
        core::stringw binding_name = config->getBindingAsString(PA_NITRO);
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
        core::stringw binding_name = config->getBindingAsString(PA_DRIFT);
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
        core::stringw binding_name = config->getBindingAsString(PA_RESCUE);
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
        core::stringw binding_name = config->getBindingAsString(PA_LOOK_BACK);
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

void OptionsScreenInput::buildDeviceList()
{
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    const int keyboard_config_count = input_manager->getDeviceList()->getKeyboardConfigAmount();
    
    for (int i=0; i<keyboard_config_count; i++)
    {
        //KeyboardConfig *config = input_manager->getDeviceList()->getKeyboardConfig(i);
        
        std::ostringstream kbname;
        kbname << "keyboard" << i;
        const std::string internal_name = kbname.str();
        
        
        devices->addItem(StringUtils::insertValues(_("Keyboard %i"), i), internal_name, "/gui/keyboard.png");
    }
    
    const int gpad_config_count = input_manager->getDeviceList()->getGamePadConfigAmount();
    
    for (int i = 0; i < gpad_config_count; i++)
    {
        GamepadConfig *config = input_manager->getDeviceList()->getGamepadConfig(i);
        // Don't display the configuration if a matching device is not available
        if (config->isInUse())
        {
            const irr::core::stringw name = config->getName().c_str();
            
            std::ostringstream gpname;
            gpname << "gamepad" << i;
            const std::string internal_name = gpname.str();
            
            devices->addItem(name, internal_name, "/gui/gamepad.png");
        }
    }    
}

// -----------------------------------------------------------------------------
void OptionsScreenInput::init()
{
    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    if (tabBar != NULL)  tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );
    
    
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    if (!m_inited)
    {        
        buildDeviceList();        
        m_inited = true;
    }
    devices->updateItemDisplay();
    
    // trigger displaying bindings for default selected device
    const std::string name2("devices");
    eventCallback(devices, name2, PLAYER_ID_GAME_MASTER);
}

// -----------------------------------------------------------------------------

void OptionsScreenInput::rebuildDeviceList()
{
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    devices->clearItems();
    buildDeviceList();        
    devices->updateItemDisplay();
}

// -----------------------------------------------------------------------------

static PlayerAction binding_to_set;
static std::string binding_to_set_button;

void OptionsScreenInput::gotSensedInput(Input* sensedInput)
{
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    std::string deviceID = devices->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
    const bool keyboard = sensedInput->type == Input::IT_KEYBOARD && deviceID.find("keyboard") != std::string::npos;
    const bool gamepad =  (sensedInput->type == Input::IT_STICKMOTION ||
                           sensedInput->type == Input::IT_STICKBUTTON) &&
                           deviceID.find("gamepad") != std::string::npos;
    
    if (!keyboard && !gamepad) return;
    if (gamepad)
    {
        if (sensedInput->type != Input::IT_STICKMOTION &&
            sensedInput->type != Input::IT_STICKBUTTON)
        {
            return; // that kind of input does not interest us
        }
    }
    
    
    if (keyboard)
    {
        std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to keyboard key " << sensedInput->btnID << " \n\n";
        
        // extract keyboard ID from name
        int configID = -1;
        sscanf( devices->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str(), "keyboard%i", &configID );
        
        KeyboardConfig* keyboard = input_manager->getDeviceList()->getKeyboardConfig(configID);
        keyboard->setBinding(binding_to_set, Input::IT_KEYBOARD, sensedInput->btnID, Input::AD_NEUTRAL);
        
        // refresh display
        init();
    }
    else if (gamepad)
    {
        std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to gamepad #" << sensedInput->deviceID << " : ";
        if (sensedInput->type == Input::IT_STICKMOTION)
        {
            std::cout << "axis " << sensedInput->btnID << " direction " <<
            (sensedInput->axisDirection == Input::AD_NEGATIVE ? "-" : "+") << "\n\n";
        }
        else if (sensedInput->type == Input::IT_STICKBUTTON)
        {
            std::cout << "button " << sensedInput->btnID << "\n\n";
        }
        else
        {
            std::cout << "Sensed unknown gamepad event type??\n";
        }
        
        // extract gamepad ID from name
        int configID = -1;
        sscanf( devices->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str(), "gamepad%i", &configID );
        
        GamepadConfig* config =  input_manager->getDeviceList()->getGamepadConfig(configID);
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

void OptionsScreenInput::tearDown()
{
}

// -----------------------------------------------------------------------------

void OptionsScreenInput::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    //const std::string& screen_name = this->getName();
    
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        
        if (selection == "tab_audio_video") StateManager::get()->replaceTopMostScreen(OptionsScreenAV::getInstance());
        else if (selection == "tab_players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "tab_controls") StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if (name == "add_device")
    {
        new AddDeviceDialog();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "devices")
    {
        DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
        assert(devices != NULL);
        
        const std::string& selection = devices->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection.find("gamepad") != std::string::npos)
        {
            int i = -1, read = 0;
            read = sscanf( selection.c_str(), "gamepad%i", &i );
            if (read == 1 && i != -1)
            {
                updateInputButtons( input_manager->getDeviceList()->getGamepadConfig(i) );
            }
            else
            {
                std::cerr << "Cannot read internal gamepad input device ID : " << selection.c_str() << std::endl;
            }
        }
        else if (selection.find("keyboard") != std::string::npos)
        {
            int i = -1, read = 0;
            read = sscanf( selection.c_str(), "keyboard%i", &i );
            if (read == 1 && i != -1)
            {
                updateInputButtons( input_manager->getDeviceList()->getKeyboardConfig(i) );
            }
            else
            {
                std::cerr << "Cannot read internal keyboard input device ID : " << selection.c_str() << std::endl;
            }
        }
        else
        {
            std::cerr << "Cannot read internal input device ID : " << selection.c_str() << std::endl;
        }
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
        
        DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
        assert( devices != NULL );
        std::cout << "\n% Entering sensing mode for " << devices->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str() << std::endl;
        
        new PressAKeyDialog(0.4f, 0.4f);
        
        std::string selection = devices->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection.find("keyboard") != std::string::npos)
        {
            input_manager->setMode(InputManager::INPUT_SENSE_KEYBOARD);
        }
        else if (selection.find("gamepad") != std::string::npos)
        {
            input_manager->setMode(InputManager::INPUT_SENSE_GAMEPAD);
        }
        else
        {
            std::cerr << "unknown selection device in options : " << selection.c_str() << std::endl;
        }
        
    }

}

// -----------------------------------------------------------------------------

void OptionsScreenInput::forgetWhatWasLoaded()
{
    Screen::forgetWhatWasLoaded();
    m_inited = false;
}

