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
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "states_screens/state_manager.hpp"


#include <iostream>
#include <sstream>

using namespace GUIEngine;

OptionsScreenInput::OptionsScreenInput() : Screen("options_input.stkgui")
{
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
    
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_up");
        btn->setLabel( config->getBindingAsString(PA_ACCEL) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_down");
        btn->setLabel( config->getBindingAsString(PA_BRAKE) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_left");
        btn->setLabel( config->getBindingAsString(PA_LEFT) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_right");
        btn->setLabel( config->getBindingAsString(PA_RIGHT) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_fire");
        btn->setLabel( config->getBindingAsString(PA_FIRE) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_nitro");
        btn->setLabel( config->getBindingAsString(PA_NITRO) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_drift");
        btn->setLabel( config->getBindingAsString(PA_DRIFT) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_rescue");
        btn->setLabel( config->getBindingAsString(PA_RESCUE) );
    }
    {
        ButtonWidget* btn = this->getWidget<ButtonWidget>("binding_look_back");
        btn->setLabel( config->getBindingAsString(PA_LOOK_BACK) );
    }
    
}

// -----------------------------------------------------------------------------
void OptionsScreenInput::init()
{
    RibbonWidget* ribbon = this->getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "controls", GUI_PLAYER_ID );
    
    
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    if(!this->m_inited)
    {
        devices->addItem("Keyboard","keyboard", file_manager->getDataDir() + "/gui/keyboard.png");
        
        const int gpad_config_count = input_manager->getDeviceList()->getGamePadConfigAmount();
        
        for(int i = 0; i < gpad_config_count; i++)
        {
            GamepadConfig *config = input_manager->getDeviceList()->getGamepadConfig(i);
            // Don't display the configuration if a matching device is not available
            if (config->isInUse())
            {
                const irr::core::stringw name = config->getName().c_str();
                
                std::ostringstream gpname;
                gpname << "gamepad" << i;
                const std::string internal_name = gpname.str();
                
                const std::string iconpath = file_manager->getDataDir() + "/gui/gamepad.png";
                devices->addItem(name, internal_name, iconpath);
            }
        }
        
        this->m_inited = true;
        
    }
    devices->updateItemDisplay();
    
    // trigger displaying bindings for default selected device
    const std::string name2("devices");
    eventCallback(devices, name2, GUI_PLAYER_ID);
}

// -----------------------------------------------------------------------------
static PlayerAction binding_to_set;
static std::string binding_to_set_button;

#define MAX_VALUE 32768

void OptionsScreenInput::gotSensedInput(Input* sensedInput)
{
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    
    std::string deviceID = devices->getSelectionIDString(GUI_PLAYER_ID);
    
    const bool keyboard = sensedInput->type == Input::IT_KEYBOARD && deviceID== "keyboard";
    const bool gamepad =  (sensedInput->type == Input::IT_STICKMOTION ||
                           sensedInput->type == Input::IT_STICKBUTTON) &&
    deviceID.find("gamepad") != std::string::npos;
    
    if(!keyboard && !gamepad) return;
    if(gamepad)
    {
        if(sensedInput->type != Input::IT_STICKMOTION &&
           sensedInput->type != Input::IT_STICKBUTTON)
            return; // that kind of input does not interest us
    }
    
    
    if (keyboard)
    {
        std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to keyboard key " << sensedInput->btnID << " \n\n";
        
        KeyboardDevice* keyboard = input_manager->getDeviceList()->getKeyboard();
        keyboard->getConfiguration()->setBinding(binding_to_set, Input::IT_KEYBOARD, sensedInput->btnID, Input::AD_NEUTRAL);
        
        // refresh display
        init();
    }
    else if (gamepad)
    {
        std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to gamepad #" << sensedInput->deviceID << " : ";
        if(sensedInput->type == Input::IT_STICKMOTION)
        {
            std::cout << "axis " << sensedInput->btnID << " direction " <<
            (sensedInput->axisDirection == Input::AD_NEGATIVE ? "-" : "+") << "\n\n";
        }
        else if(sensedInput->type == Input::IT_STICKBUTTON)
        {
            std::cout << "button " << sensedInput->btnID << "\n\n";
        }
        else
            std::cout << "Sensed unknown gamepad event type??\n";
        
        int configID = -1;
        sscanf( devices->getSelectionIDString(GUI_PLAYER_ID).c_str(), "gamepad%i", &configID );
        
        /*
         if(sscanf( devices->getSelectionIDString().c_str(), "gamepad%i", &gamepadID ) != 1 ||
         gamepadID >= input_manager->getDeviceList()->getGamePadAmount())
         {
         if(gamepadID >= input_manager->getDeviceList()->getGamePadAmount() || gamepadID == -1 )
         {
         std::cerr << "gamepad ID does not exist (or failed to read it) : " << gamepadID << "\n";
         gamepadID = sensedInput->deviceID;
         }
         
         if(input_manager->getDeviceList()->getGamepadConfig(gamepadID)->m_index != sensedInput->deviceID)
         {
         // should not happen, but let's try to be bulletproof...
         std::cerr << "The key that was pressed is not on the gamepad we're trying to configure! ID in list=" << gamepadID <<
         " which has irrID " << input_manager->getDeviceList()->getGamePad(gamepadID)->m_index <<
         " and we got input from " << sensedInput->deviceID << "\n";
         }
         
         }
         */
        
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
    if(btn != NULL) btn->setFocusForPlayer(GUI_PLAYER_ID);
    
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
    
    if(name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(GUI_PLAYER_ID).c_str();
        
        if (selection == "audio_video") StateManager::get()->replaceTopMostScreen(OptionsScreenAV::getInstance());
        else if (selection == "players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "controls") StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "devices")
    {
        DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
        assert(devices != NULL);
        
        const std::string& selection = devices->getSelectionIDString(GUI_PLAYER_ID);
        if( selection.find("gamepad") != std::string::npos )
        {
            int i = -1, read = 0;
            read = sscanf( selection.c_str(), "gamepad%i", &i );
            if(read == 1 && i != -1)
            {
                updateInputButtons( input_manager->getDeviceList()->getGamepadConfig(i) );
            }
            else
            {
                std::cerr << "Cannot read internal input device ID : " << selection.c_str() << std::endl;
            }
        }
        else if(selection == "keyboard")
        {
            updateInputButtons( input_manager->getDeviceList()->getKeyboard()->getConfiguration() );
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
        std::cout << "\n% Entering sensing mode for " << devices->getSelectionIDString(GUI_PLAYER_ID).c_str() << std::endl;
        
        new PressAKeyDialog(0.4f, 0.4f);
        
        std::string selection = devices->getSelectionIDString(GUI_PLAYER_ID);
        if (selection == "keyboard")
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

