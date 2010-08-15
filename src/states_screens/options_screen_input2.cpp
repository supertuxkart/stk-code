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

#include "states_screens/options_screen_input2.hpp"

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
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
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
}   // OptionsScreenInput2

// -----------------------------------------------------------------------------

void OptionsScreenInput2::loadedFromFile()
{    
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenInput2::init()
{
    Screen::init();
    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    if (tabBar != NULL)  tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );
    
    
    LabelWidget* label = this->getWidget<LabelWidget>("title");
    label->setText( m_config->getName().c_str() );
    
    GUIEngine::ListWidget* actions = this->getWidget<GUIEngine::ListWidget>("actions");
    assert( actions != NULL );
   
    // ---- create list skeleton (right number of items, right internal names)
    //      their actualy contents will be adapted as needed after
    
    //I18N: Key binding section
    actions->addItem("game_keys_section", _("Game Keys") );
    actions->addItem(KartActionStrings[PA_STEER_LEFT],  L"" );
    actions->addItem(KartActionStrings[PA_STEER_RIGHT], L"" );
    actions->addItem(KartActionStrings[PA_ACCEL],       L"" );
    actions->addItem(KartActionStrings[PA_BRAKE],       L"" );
    actions->addItem(KartActionStrings[PA_FIRE],        L"" );
    actions->addItem(KartActionStrings[PA_NITRO],       L"" );
    actions->addItem(KartActionStrings[PA_DRIFT],       L"" );
    actions->addItem(KartActionStrings[PA_LOOK_BACK],   L"" );
    actions->addItem(KartActionStrings[PA_RESCUE],      L"" );
    

    //I18N: Key binding section
    actions->addItem("menu_keys_section", _("Menu Keys") );
    actions->addItem(KartActionStrings[PA_MENU_UP],     L"" );
    actions->addItem(KartActionStrings[PA_MENU_DOWN],   L"" );
    actions->addItem(KartActionStrings[PA_MENU_LEFT],   L"" );
    actions->addItem(KartActionStrings[PA_MENU_RIGHT],  L"" );
    actions->addItem(KartActionStrings[PA_MENU_SELECT], L"");
    actions->addItem(KartActionStrings[PA_MENU_CANCEL], L"" );
    
    updateInputButtons();
}   // init

// -----------------------------------------------------------------------------

irr::core::stringw OptionsScreenInput2::makeLabel(const irr::core::stringw translatedName,
                                                  PlayerAction action) const
{
    //hack: one tab character is supported by out font object, it moves the cursor to the middle of the area
    core::stringw out = irr::core::stringw("    ") + translatedName + L"\t";
    
    out += m_config->getBindingAsString(action);
    return out;
}   // makeLabel

// -----------------------------------------------------------------------------

void OptionsScreenInput2::updateInputButtons()
{
    assert(m_config != NULL);
    
    //TODO: detect duplicates
    
    GUIEngine::ListWidget* actions = this->getWidget<GUIEngine::ListWidget>("actions");
    assert( actions != NULL );
    
    // item 0 is a section header
    
    //I18N: Key binding name
    actions->renameItem(1, makeLabel( _("Steer Left"), PA_STEER_LEFT) );
    
    //I18N: Key binding name
    actions->renameItem(2, makeLabel( _("Steer Right"), PA_STEER_RIGHT) );
    
    //I18N: Key binding name
    actions->renameItem(3, makeLabel( _("Accelerate"), PA_ACCEL) );
    
    //I18N: Key binding name
    actions->renameItem(4, makeLabel( _("Brake"), PA_BRAKE) );
    
    //I18N: Key binding name
    actions->renameItem(5, makeLabel( _("Fire"), PA_FIRE) );
    
    //I18N: Key binding name
    actions->renameItem(6, makeLabel( _("Nitro"), PA_NITRO) );
    
    //I18N: Key binding name
    actions->renameItem(7, makeLabel( _("Sharp Turn"), PA_DRIFT) );
    
    //I18N: Key binding name
    actions->renameItem(8, makeLabel( _("Look Back"), PA_LOOK_BACK) );
    
    //I18N: Key binding name
    actions->renameItem(9, makeLabel( _("Rescue"), PA_RESCUE) );
    
    
    // item 10 is a section header
    
    //I18N: Key binding name
    actions->renameItem(11, makeLabel( _("Up"), PA_MENU_UP) );
    
    //I18N: Key binding name
    actions->renameItem(12, makeLabel( _("Down"), PA_MENU_DOWN) );
    
    //I18N: Key binding name
    actions->renameItem(13, makeLabel( _("Left"), PA_MENU_LEFT) );
    
    //I18N: Key binding name
    actions->renameItem(14, makeLabel( _("Right"), PA_MENU_RIGHT) );
    
    //I18N: Key binding name
    actions->renameItem(15, makeLabel( _("Select"), PA_MENU_SELECT) );
    
    //I18N: Key binding name
    actions->renameItem(16, makeLabel( _("Cancel/Back"), PA_MENU_CANCEL) );
    
    
    // ---- make sure there are no binding conflicts (same key used for two actions)
    std::set<irr::core::stringw> currentlyUsedKeys;
    for (PlayerAction action = PA_FIRST_GAME_ACTION;
         action <= PA_LAST_GAME_ACTION;
         action=PlayerAction(action+1))
    {
        const irr::core::stringw item = m_config->getBindingAsString(action);
        if (currentlyUsedKeys.find(item) == currentlyUsedKeys.end())
        {
            currentlyUsedKeys.insert( item );
        }
        else
        {            
            // binding conflict!
            actions->markItemRed( KartActionStrings[action] );
            
            // also mark others
            for (PlayerAction others = PA_FIRST_GAME_ACTION;
                 others < action; others=PlayerAction(others+1))
            {
                const irr::core::stringw others_item = m_config->getBindingAsString(others);
                if (others_item == item)
                {
                    actions->markItemRed( KartActionStrings[others] );
                }
            }
            
            //actions->renameItem( KartActionStrings[action], _("Binding Conflict!") ); 
        }
    }
    
    // menu keys and game keys can overlap, no problem, so forget game keys before checking menu keys
    currentlyUsedKeys.clear();
    for (PlayerAction action = PA_FIRST_MENU_ACTION;
         action <= PA_LAST_MENU_ACTION;
         action=PlayerAction(action+1))
    {
        const irr::core::stringw item = m_config->getBindingAsString(action);
        if (currentlyUsedKeys.find(item) == currentlyUsedKeys.end())
        {
            currentlyUsedKeys.insert( item );
        }
        else
        {            
            // binding conflict!
            actions->markItemRed( KartActionStrings[action] );

            // also mark others
            for (PlayerAction others = PA_FIRST_MENU_ACTION;
                 others < action; others=PlayerAction(others+1))
            {
                const irr::core::stringw others_item = m_config->getBindingAsString(others);
                if (others_item == item)
                {
                    actions->markItemRed( KartActionStrings[others] );
                }
            }
            
            //actions->renameItem( KartActionStrings[action], _("Binding Conflict!") ); 
        }
    }
}   // updateInputButtons

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
        updateInputButtons();
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
        updateInputButtons();
    }
    else
    {
        return;
    }
    
    ModalDialog::dismiss();
    input_manager->setMode(InputManager::MENU);
    
    // re-select the previous button (TODO!)
    //ButtonWidget* btn = this->getWidget<ButtonWidget>(binding_to_set_button.c_str());
    //if(btn != NULL) btn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    
    // save new binding to file
    input_manager->getDeviceList()->serialize();
}   // gotSensedInput


// -----------------------------------------------------------------------------

void OptionsScreenInput2::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    //const std::string& screen_name = this->getName();
    
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        
        if (selection == "tab_audio")        StateManager::get()->replaceTopMostScreen(OptionsScreenAudio::getInstance());
        else if (selection == "tab_video")   StateManager::get()->replaceTopMostScreen(OptionsScreenVideo::getInstance());
        else if (selection == "tab_players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "tab_controls") {}
    }
    else if (name == "back_to_device_list")
    {
        StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->popMenu();
    }
    else if (name == "actions")
    {
        GUIEngine::ListWidget* actions = this->getWidget<GUIEngine::ListWidget>("actions");
        assert( actions != NULL );
        
        // a player action in the list was clicked. find which one
        const std::string& clicked = actions->getSelectionInternalName();
        for (int n=PA_BEFORE_FIRST+1; n<PA_COUNT; n++)
        {
            if (KartActionStrings[n] == clicked)
            {                
                // we found which one. show the "press a key" dialog.
                if (UserConfigParams::m_verbosity>=5)
                {
                    std::cout << "\n% Entering sensing mode for " 
                              << m_config->getName().c_str() 
                              << std::endl;
                }
                
                binding_to_set = (PlayerAction)n;
                
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
                    std::cerr << "unknown selection device in options : "
                              << m_config->getName().c_str() << std::endl;
                }                
                break;
            }
        }
    }
    //TODO!
    /*
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
            binding_to_set = PA_STEER_LEFT;
        }
        else if(name == "binding_right")
        {
            binding_to_set = PA_STEER_RIGHT;
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
    }*/

}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenInput2::unloaded()
{
}   // unloaded

// -----------------------------------------------------------------------------

bool OptionsScreenInput2::onEscapePressed()
{
    StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    return false; // don't use standard escape key handler, we handled it differently
}   // onEscapePressed

// -----------------------------------------------------------------------------
