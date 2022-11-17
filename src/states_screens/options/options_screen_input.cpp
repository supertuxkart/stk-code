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

#include "states_screens/options/options_screen_input.hpp"

#include "config/user_config.hpp"
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
#include "states_screens/options/options_screen_device.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/dialogs/add_device_dialog.hpp"
#include "states_screens/dialogs/multitouch_settings_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>
#include <set>
#include <algorithm>

#include <IrrlichtDevice.h>

using namespace GUIEngine;

// -----------------------------------------------------------------------------

OptionsScreenInput::OptionsScreenInput() : Screen("options_input.stkgui")
{
}   // OptionsScreenInput

// -----------------------------------------------------------------------------

void OptionsScreenInput::loadedFromFile()
{
    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,"keyboard.png"   ));
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,"keyboard_off.png"   ));
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,"gamepad.png"    ));
    video::ITexture* icon4 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,"gamepad_off.png"));
    video::ITexture* icon5 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,"tablet.png"    ));

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv() );
    m_icon_bank->addTextureAsSprite(icon1);
    m_icon_bank->addTextureAsSprite(icon2);
    m_icon_bank->addTextureAsSprite(icon3);
    m_icon_bank->addTextureAsSprite(icon4);
    m_icon_bank->addTextureAsSprite(icon5);

    // scale icons depending on font height
    const float scale = GUIEngine::getFontHeight() / 72.0f;
    m_icon_bank->setScale(scale);
    m_icon_bank->setTargetIconSize(128, 128);
    m_gamepad_count = 0;
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
        KeyboardConfig *config = device_manager->getKeyboardConfig(i);
        
        std::ostringstream kbname;
        kbname << "keyboard" << i;
        const std::string internal_name = kbname.str();
        
        const int icon = (config->isEnabled() ? 0 : 1);

        //Display the configName instead of default name if it exists
        if (!config->getConfigName().empty())
        {
            // since irrLicht's list widget has the nasty tendency to put the
            // icons very close to the text, I'm adding spaces to compensate.
            devices->addItem(internal_name, (core::stringw("   ") +
                config->getConfigName()), icon);
        }
        else
        {
            devices->addItem(internal_name, (core::stringw("   ") +
                            _("Keyboard %i", i)).c_str(), icon);
        }
    }

    const int gpad_config_count = device_manager->getGamePadConfigAmount();
    m_gamepad_count = input_manager->getGamepadCount();

    for (int i = 0; i < gpad_config_count; i++)
    {
        GamepadConfig *config = device_manager->getGamepadConfig(i);

        // Don't display the configuration if a matching device is not available
        if (config->isPlugged())
        {
            irr::core::stringw name;

            //Display the configName instead of default name if it exists
            if (!config->getConfigName().empty())
            {
                // since irrLicht's list widget has the nasty tendency to put the
                // icons very close to the text, I'm adding spaces to compensate.
                name = (core::stringw("   ") + 
                             config->getConfigName());
            }
            else
            {
                name = ("   " + config->getName()).c_str();   

                if (config->getNumberOfDevices() > 1)
                {
                    name += core::stringw(L" (x");
                    name += config->getNumberOfDevices();
                    name += core::stringw(L")");
                }
            }

            std::ostringstream gpname;
            gpname << "gamepad" << i;
            const std::string internal_name = gpname.str();

            const int icon = (config->isEnabled() ? 2 : 3);

            devices->addItem(internal_name, name, icon);
        }   // if config->isPlugged
    }   // for i<gpad_config_count
    
    MultitouchDevice* touch_device = device_manager->getMultitouchDevice();
                                                        
    if (touch_device != NULL)
    {
        devices->addItem("touch_device", (core::stringw("   ") + 
                                                _("Touch Device")).c_str(), 4);
    }
}   // buildDeviceList

// -----------------------------------------------------------------------------
void OptionsScreenInput::init()
{
    Screen::init();
    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    assert(tabBar != NULL);
    tabBar->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );
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

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 &&
        irr_driver->getDevice()->supportsTouchDevice()) ||
        UserConfigParams::m_multitouch_active > 1;
    if (multitouch_enabled)
    {
        // I18N: In the input configuration screen, help for touch device
        getWidget("help1")->setText(_("Tap on a device to configure it"));
        getWidget("help2")->setVisible(false);
    }
    else
    {
        getWidget("help1")->setText(_("Press enter or double-click on a device to configure it"));
        getWidget("help2")->setVisible(true);
    }
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

    int id = devices->getSelectionID();
    devices->clear();
    buildDeviceList();
    if (id < devices->getItemCount())
        devices->setSelectionID(id);
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
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
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
            new MultitouchSettingsDialog(0.8f, 0.9f);
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
                m_highlights[internal_name] = 0.25f;
        }
    }
}

// -----------------------------------------------------------------------------

void OptionsScreenInput::onUpdate(float dt)
{
    // Only rebuild device list when there is difference in gamepad count
    // This allow the list to be scrolled (keyboard config can only be add or
    // remove in new screen
    size_t gamepad_count = input_manager->getGamepadCount();
    if (gamepad_count != m_gamepad_count)
        rebuildDeviceList();

    std::map<std::string, float>::iterator it;
    ListWidget* devices = this->getWidget<ListWidget>("devices");
    assert(devices != NULL);
    for (it = m_highlights.begin(); it != m_highlights.end();)
    {
        it->second -= dt;
        if (it->second < 0.0f)
        {
            devices->markItemRed(it->first.c_str(), false);
            m_highlights.erase(it++);
        }
        else
        {
            devices->markItemRed(it->first.c_str(), true);
            it++;
        }
    }
    //m_highlights[internal_name]
}   // onUpdate
