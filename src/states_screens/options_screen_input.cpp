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

#include "states_screens/options_screen_input.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/gamepad_device.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/options_screen_device.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/dialogs/add_device_dialog.hpp"
#include "states_screens/dialogs/multitouch_settings_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenInput );

// -----------------------------------------------------------------------------

OptionsScreenInput::OptionsScreenInput() : Screen("options_input.stkgui")
{
}   // OptionsScreenInput

// -----------------------------------------------------------------------------

void OptionsScreenInput::loadedFromFile()
{
    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI,"keyboard.png"   ));
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI,"gamepad.png"    ));
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI,"gamepad_off.png"));

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv() );
    m_icon_bank->addTextureAsSprite(icon1);
    m_icon_bank->addTextureAsSprite(icon2);
    m_icon_bank->addTextureAsSprite(icon3);

    // scale icons depending on screen resolution. the numbers below are a bit arbitrary
    const int screen_width = irr_driver->getFrameSize().Width;
    const float scale = 0.3f + 0.2f*std::max(0, screen_width - 640)/564.0f;
    m_icon_bank->setScale(scale);
}   // loadFromFile

// -----------------------------------------------------------------------------

void OptionsScreenInput::buildDeviceList()
{
    GUIEngine::ListWidget* devices = this->getWidget<GUIEngine::ListWidget>("devices");
    assert( devices != NULL );

    assert( m_icon_bank != NULL );
    devices->setIcons(m_icon_bank);
    
    DeviceManager* device_manager = input_manager->getDeviceManager();

    const int keyboard_config_count = device_manager->getKeyboardConfigAmount();

    for (int i=0; i<keyboard_config_count; i++)
    {
        //KeyboardConfig *config = input_manager->getDeviceList()->getKeyboardConfig(i);

        std::ostringstream kbname;
        kbname << "keyboard" << i;
        const std::string internal_name = kbname.str();

        // since irrLicht's list widget has the nasty tendency to put the
        // icons very close to the text, I'm adding spaces to compensate.
        devices->addItem(internal_name, (core::stringw("   ") + _("Keyboard %i", i)).c_str(), 0 /* icon */);
    }

    const int gpad_config_count = device_manager->getGamePadConfigAmount();

    for (int i = 0; i < gpad_config_count; i++)
    {
        GamepadConfig *config = device_manager->getGamepadConfig(i);

        // Don't display the configuration if a matching device is not available
        if (config->isPlugged())
        {
            // since irrLicht's list widget has the nasty tendency to put the
            // icons very close to the text, I'm adding spaces to compensate.
            irr::core::stringw name = ("   " + config->getName()).c_str();

            if (config->getNumberOfDevices() > 1)
            {
                name += core::stringw(L" (x");
                name += config->getNumberOfDevices();
                name += core::stringw(L")");
            }

            std::ostringstream gpname;
            gpname << "gamepad" << i;
            const std::string internal_name = gpname.str();

            const int icon = (config->isEnabled() ? 1 : 2);

            devices->addItem(internal_name, name, icon);
        }   // if config->isPlugged
    }   // for i<gpad_config_count
    
    MultitouchDevice* touch_device = device_manager->getMultitouchDevice();
                                                        
    if (touch_device != NULL)
    {
        devices->addItem("touch_device", (core::stringw("   ") + 
                                                _("Touch Device")).c_str(), 1);
    }
}   // buildDeviceList

// -----------------------------------------------------------------------------
void OptionsScreenInput::init()
{
    Screen::init();
    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    assert(tabBar != NULL);
    tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );

    tabBar->getRibbonChildren()[0].setTooltip( _("Graphics") );
    tabBar->getRibbonChildren()[1].setTooltip( _("Audio") );
    tabBar->getRibbonChildren()[2].setTooltip( _("User Interface") );
    tabBar->getRibbonChildren()[3].setTooltip( _("Players") );

    /*
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );
    */


    buildDeviceList();

    //devices->updateItemDisplay();

    /*
    // trigger displaying bindings for default selected device
    const std::string name2("devices");
    eventCallback(devices, name2, PLAYER_ID_GAME_MASTER);
     */
    // Disable adding keyboard configurations
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    getWidget<ButtonWidget>("add_device")->setActive(!in_game);
}   // init

// -----------------------------------------------------------------------------

void OptionsScreenInput::rebuildDeviceList()
{
    /*
    DynamicRibbonWidget* devices = this->getWidget<DynamicRibbonWidget>("devices");
    assert( devices != NULL );

    devices->clearItems();
    buildDeviceList();
    devices->updateItemDisplay();
     */

    ListWidget* devices = this->getWidget<ListWidget>("devices");
    assert( devices != NULL );

    devices->clear();
    buildDeviceList();
}   // rebuildDeviceList

// -----------------------------------------------------------------------------

void OptionsScreenInput::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    //const std::string& screen_name = this->getName();

    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        //else if (selection == "tab_controls")
        //    screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
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
        ListWidget* devices = this->getWidget<ListWidget>("devices");
        assert(devices != NULL);

        const std::string& selection = devices->getSelectionInternalName();
        if (selection.find("gamepad") != std::string::npos)
        {
            int i = -1, read = 0;
            read = sscanf( selection.c_str(), "gamepad%i", &i );
            if (read == 1 && i != -1)
            {
                OptionsScreenDevice::getInstance()->setDevice( input_manager->getDeviceManager()->getGamepadConfig(i) );
                StateManager::get()->replaceTopMostScreen(OptionsScreenDevice::getInstance());
                //updateInputButtons( input_manager->getDeviceList()->getGamepadConfig(i) );
            }
            else
            {
                Log::error("OptionsScreenInput", "Cannot read internal gamepad input device ID: %s",
                    selection.c_str());
            }
        }
        else if (selection.find("keyboard") != std::string::npos)
        {
            int i = -1, read = 0;
            read = sscanf( selection.c_str(), "keyboard%i", &i );
            if (read == 1 && i != -1)
            {
                // updateInputButtons( input_manager->getDeviceList()->getKeyboardConfig(i) );
                OptionsScreenDevice::getInstance()
                    ->setDevice( input_manager->getDeviceManager()->getKeyboardConfig(i) );
                StateManager::get()->replaceTopMostScreen(OptionsScreenDevice::getInstance());
            }
            else
            {
                Log::error("OptionsScreenInput", "Cannot read internal keyboard input device ID: %s",
                    selection.c_str());
            }
        }
        else if (selection.find("touch_device") != std::string::npos)
        {
            // Don't edit multitouch settings during a race, because it needs
            // to re-create all buttons to take an effect
            if (StateManager::get()->getGameState() != GUIEngine::INGAME_MENU)
            {
                new MultitouchSettingsDialog(0.8f, 0.9f);
            }
        }
        else
        {
            Log::error("OptionsScreenInput", "Cannot read internal input device ID: %s", selection.c_str());
        }
    }

}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenInput::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded

// -----------------------------------------------------------------------------

void OptionsScreenInput::filterInput(Input::InputType type,
                                     int deviceID,
                                     int btnID,
                                     int axisDir,
                                     int value)
{
    if (type == Input::IT_STICKMOTION || type == Input::IT_STICKBUTTON)
    {
        GamePadDevice* gamepad = input_manager->getDeviceManager()->getGamePadFromIrrID(deviceID);
        if (gamepad != NULL && gamepad->getConfiguration() != NULL)
        {
            //printf("'%s'\n", gamepad->getConfiguration()->getName().c_str());

            ListWidget* devices = this->getWidget<ListWidget>("devices");
            assert(devices != NULL);

            std::string internal_name;
            const int gpad_config_count = input_manager->getDeviceManager()->getGamePadConfigAmount();
            for (int i = 0; i < gpad_config_count; i++)
            {
                GamepadConfig *config = input_manager->getDeviceManager()->getGamepadConfig(i);

                // Don't display the configuration if a matching device is not available
                if (config == gamepad->getConfiguration())
                {
                    std::ostringstream gpname;
                    gpname << "gamepad" << i;
                    internal_name = gpname.str();
                }
            }

            if (internal_name.size() > 0 && abs(value) > Input::MAX_VALUE/2)
            {
                devices->markItemRed(internal_name.c_str());
                m_highlights[internal_name] = 0.25f;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void OptionsScreenInput::onUpdate(float dt)
{
    std::map<std::string, float>::iterator it;
    for (it = m_highlights.begin(); it != m_highlights.end();)
    {
        it->second -= dt;
        if (it->second < 0.0f)
        {
            ListWidget* devices = this->getWidget<ListWidget>("devices");
            assert(devices != NULL);

            devices->markItemRed(it->first.c_str(), false);

            m_highlights.erase(it++);
        }
        else
        {
            it++;
        }
    }
    //m_highlights[internal_name]
}   // onUpdate
