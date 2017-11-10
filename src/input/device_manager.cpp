//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#include "input/device_manager.hpp"

#include <iostream>
#include <fstream>

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "input/gamepad_device.hpp"
#include "input/keyboard_device.hpp"
#include "input/multitouch_device.hpp"
#include "input/wiimote_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"


#define INPUT_MODE_DEBUG 0

static const char  INPUT_FILE_NAME[]  = "input.xml";
static const int   INPUT_FILE_VERSION = 1;

DeviceManager::DeviceManager()
{
    m_latest_used_device = NULL;
    m_assign_mode = NO_ASSIGN;
    m_single_player = NULL;
    m_multitouch_device = NULL;
}   // DeviceManager

// -----------------------------------------------------------------------------
DeviceManager::~DeviceManager()
{
    delete m_multitouch_device;
}   // ~DeviceManager

// -----------------------------------------------------------------------------
bool DeviceManager::initialize()
{
    GamepadConfig *gamepadConfig = NULL;
    GamePadDevice *gamepadDevice = NULL;
    m_map_fire_to_select         = false;
    bool created                 = false;


    // Shutdown in case the device manager is being re-initialized
    shutdown();

    if(UserConfigParams::logMisc())
    {
        Log::info("Device manager","Initializing Device Manager");
        Log::info("-","---------------------------");
    }

    load();

    // Assign a configuration to the keyboard, or create one if we haven't yet
    if(UserConfigParams::logMisc()) Log::info("Device manager","Initializing keyboard support.");
    if (m_keyboard_configs.size() == 0)
    {
        if(UserConfigParams::logMisc())
            Log::info("Device manager","No keyboard configuration exists, creating one.");
        m_keyboard_configs.push_back(new KeyboardConfig());
        created = true;
    }

    const int keyboard_amount = m_keyboard_configs.size();
    for (int n = 0; n < keyboard_amount; n++)
    {
        m_keyboards.push_back(new KeyboardDevice(m_keyboard_configs.get(n)));
    }

    if(UserConfigParams::logMisc())
        Log::info("Device manager","Initializing gamepad support.");

    irr_driver->getDevice()->activateJoysticks(m_irrlicht_gamepads);

    int num_gamepads = m_irrlicht_gamepads.size();
    if(UserConfigParams::logMisc())
    {
        Log::info("Device manager","Irrlicht reports %d gamepads are attached to the system.",
               num_gamepads);
    }

    // Create GamePadDevice for each physical gamepad and find a GamepadConfig to match
    for (int id = 0; id < num_gamepads; id++)
    {
        core::stringc name = m_irrlicht_gamepads[id].Name;

        // Some systems report a disk accelerometer as a gamepad, skip that
        if (name.find("LIS3LV02DL") != -1) continue;
        if (name == "applesmc") continue;

        if(m_irrlicht_gamepads[id].HasGenericName)
        {
            // On Windows all gamepads are given the same name ('microsoft
            // pc-joystick driver'). Irrlicht now tries to get a better name
            // from the registry, but in case this should fail we still have
            // all gamepads with the same name shown in the GUI. This makes
            // configuration totally useless, so append an ID to the name.
            name = name + " " + StringUtils::toString(id).c_str();
        }

        if (UserConfigParams::logMisc())
        {
            Log::info("Device manager","#%d: %s detected...", id, name.c_str());
        }

        // Returns true if new configuration was created
        if (getConfigForGamepad(id, name.c_str(), &gamepadConfig) == true)
        {
            if(UserConfigParams::logMisc())
               Log::info("Device manager","creating new configuration.");
            created = true;
        }
        else
        {
            if(UserConfigParams::logMisc())
                Log::info("Device manager","using existing configuration.");
        }

        gamepadConfig->setPlugged();
        gamepadDevice = new GamePadDevice(id,
                                          name.c_str(),
                                          m_irrlicht_gamepads[id].Axes,
                                          m_irrlicht_gamepads[id].Buttons,
                                          gamepadConfig );
        addGamepad(gamepadDevice);
    } // end for

    if (UserConfigParams::m_multitouch_enabled)
    {
        m_multitouch_device = new MultitouchDevice();
    }

    if (created) save();

    return created;
}   // initialize

// -----------------------------------------------------------------------------
void DeviceManager::clearKeyboard()
{
    m_keyboards.clearAndDeleteAll();
}   // clearKeyboard

// -----------------------------------------------------------------------------
void DeviceManager::clearGamepads()
{
    m_gamepads.clearAndDeleteAll(); 
}   // clearGamepads
// -----------------------------------------------------------------------------
void DeviceManager::clearMultitouchDevices()
{
    delete m_multitouch_device;
    m_multitouch_device = NULL;
}   // clearMultitouchDevices

// -----------------------------------------------------------------------------
void DeviceManager::setAssignMode(const PlayerAssignMode assignMode)
{
    m_assign_mode = assignMode;

#if INPUT_MODE_DEBUG
    if (assignMode == NO_ASSIGN) Log::info("DeviceManager::setAssignMode(NO_ASSIGN)");
    if (assignMode == ASSIGN) Log::info("DeviceManager::setAssignMode(ASSIGN)");
    if (assignMode == DETECT_NEW) Log::info("DeviceManager::setAssignMode(DETECT_NEW)");
#endif

    // when going back to no-assign mode, do some cleanup
    if (assignMode == NO_ASSIGN)
    {
        for (unsigned int i=0; i < m_gamepads.size(); i++)
        {
            m_gamepads[i].setPlayer(NULL);
        }
        for (unsigned int i=0; i < m_keyboards.size(); i++)
        {
            m_keyboards[i].setPlayer(NULL);
        }

        if (m_multitouch_device != NULL)
            m_multitouch_device->setPlayer(NULL);
    }
}   // setAssignMode

// -----------------------------------------------------------------------------
GamePadDevice* DeviceManager::getGamePadFromIrrID(const int id)
{
    const int count = m_gamepads.size();
    for (int i = 0; i < count; i++)
    {
        if (m_gamepads[i].getIrrIndex()== id)
        {

            return m_gamepads.get(i);
        }
    }
    return NULL;
}   // getGamePadFromIrrID

// -----------------------------------------------------------------------------
/**
 * Check if we already have a config object for gamepad 'irr_id' as reported by
 * irrLicht, If no, create one. Returns true if new configuration was created,
 *  otherwise false.
 */
bool DeviceManager::getConfigForGamepad(const int irr_id,
                                        const std::string& name,
                                        GamepadConfig **config)
{
    bool found = false;
    bool configCreated = false;

    // Find appropriate configuration
    for(unsigned int n=0; n < m_gamepad_configs.size(); n++)
    {
        if(m_gamepad_configs[n].getName() == name)
        {
            *config = m_gamepad_configs.get(n);
            found = true;
        }
    }

    // If we can't find an appropriate configuration then create one.
    if (!found)
    {
        if(irr_id < (int)(m_irrlicht_gamepads.size()))
        {
            *config = new GamepadConfig( name,
                                         m_irrlicht_gamepads[irr_id].Axes,
                                         m_irrlicht_gamepads[irr_id].Buttons );
        }
#ifdef ENABLE_WIIUSE
        else    // Wiimotes have a higher ID and do not refer to m_irrlicht_gamepads
        {
            // The Wiimote manager will set number of buttons and axis
            *config = new GamepadConfig(name.c_str());
        }
#endif

        // Add new config to list
        m_gamepad_configs.push_back(*config);
        configCreated = true;
    }

    return configCreated;
}   // getConfigForGamepad

// -----------------------------------------------------------------------------
void DeviceManager::addKeyboard(KeyboardDevice* d)
{
    m_keyboards.push_back(d);
}   // addKeyboard

// -----------------------------------------------------------------------------
void DeviceManager::addEmptyKeyboard()
{
    KeyboardConfig* newConf = new KeyboardConfig();
    m_keyboard_configs.push_back(newConf);
    m_keyboards.push_back( new KeyboardDevice(newConf) );
}   // addEmptyKeyboard

// -----------------------------------------------------------------------------

void DeviceManager::addGamepad(GamePadDevice* d)
{
    m_gamepads.push_back(d);
}   // addGamepad

// -----------------------------------------------------------------------------

bool DeviceManager::deleteConfig(DeviceConfig* config)
{
    for (unsigned int n=0; n<m_keyboards.size(); n++)
    {
        if (m_keyboards[n].getConfiguration() == config)
        {
            m_keyboards.erase(n);
            n--;
        }
    }
    for (unsigned int n=0; n<m_gamepads.size(); n++)
    {
        if (m_gamepads[n].getConfiguration() == config)
        {
            m_gamepads.erase(n);
            n--;
        }
    }

    if (m_keyboard_configs.erase(config))
    {
        return true;
    }

    if (m_gamepad_configs.erase(config))
    {
        return true;
    }

    return false;
}   // deleteConfig

// -----------------------------------------------------------------------------
/** Helper method, only used internally. Takes care of analyzing keyboard input.
 *  \param[in]   button_id  Id of the key pressed.
 *  \param[in]   mode       Used to determine whether to determine menu actions
 *                          or game actions
 *  \param[out]  player     Which player this input belongs to (only set in 
 *                          ASSIGN mode).
 *  \param[out]  action     Which action is related to this input trigger.
 *  \return                 The device to which this input belongs
 */
InputDevice* DeviceManager::mapKeyboardInput(int button_id,
                                             InputManager::InputDriverMode mode,
                                             StateManager::ActivePlayer **player,
                                             PlayerAction *action /* out */)
{
    const int keyboard_amount = m_keyboards.size();

    for (int n=0; n<keyboard_amount; n++)
    {
        KeyboardDevice *keyboard = m_keyboards.get(n);

        if (keyboard->processAndMapInput(Input::IT_KEYBOARD, button_id, mode, action))
        {
            if (m_single_player != NULL)
            {
                *player = m_single_player;
            }
            else if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
            {
                *player = NULL;
            }
            else
            {
                *player = keyboard->getPlayer();
            }
            return keyboard;
        }
    }

    return NULL; // no appropriate binding found
}   // mapKeyboardInput

//-----------------------------------------------------------------------------
/** Helper method, only used internally. Takes care of analyzing gamepad
 *  input.
 *  \param[in]  type    Type of gamepad event (IT_STICKMOTION etc).
 *  \param[out] player  Which player this input belongs to (only set in
 *                      ASSIGN mode)
 *  \param[out] action  Which action is related to this input trigger
 *  \param      mode    Used to determine whether to determine menu actions
 *                      or game actions
 *  \return             The device to which this input belongs
 */

InputDevice *DeviceManager::mapGamepadInput(Input::InputType type,
                                             int device_id,
                                             int button_id,
                                             int axis_dir,
                                             int *value /* inout */,
                                             InputManager::InputDriverMode mode,
                                             StateManager::ActivePlayer **player /* out */,
                                             PlayerAction *action /* out */)
{
    GamePadDevice  *gPad = getGamePadFromIrrID(device_id);

    if (gPad != NULL)
    {
        if (gPad->processAndMapInput(type, button_id, mode, action, value))
        {
            if (m_single_player != NULL)
            {
                *player = m_single_player;

                // in single-player mode, assign the gamepad as needed
                if (gPad->getPlayer() != m_single_player) gPad->setPlayer(m_single_player);
            }
            else if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
            {
                *player = NULL;
            }
            else
            {
                *player = gPad->getPlayer();
            }
        }
        else gPad = NULL; // If no bind was found, return NULL
    }

    return gPad;
}   // mapGamepadInput

//-----------------------------------------------------------------------------

void DeviceManager::updateMultitouchDevice()
{
    if (m_multitouch_device == NULL)
        return;

    if (m_single_player != NULL)
    {
        // in single-player mode, assign the gamepad as needed
        if (m_multitouch_device->getPlayer() != m_single_player)
            m_multitouch_device->setPlayer(m_single_player);
    }
    else
    {
        m_multitouch_device->setPlayer(NULL);
    }
}   // updateMultitouchDevice

//-----------------------------------------------------------------------------

bool DeviceManager::translateInput( Input::InputType type,
                                    int device_id,
                                    int button_id,
                                    int axis_dir,
                                    int* value /* inout */,
                                    InputManager::InputDriverMode mode,
                                    StateManager::ActivePlayer** player /* out */,
                                    PlayerAction* action /* out */ )
{
    if (GUIEngine::getCurrentScreen() != NULL)
    {
        GUIEngine::getCurrentScreen()->filterInput(type, device_id, button_id, axis_dir, *value);
    }

    InputDevice *device = NULL;

    // If the input event matches a bind on an input device, get a pointer to the device
    switch (type)
    {
        case Input::IT_KEYBOARD:
            device = mapKeyboardInput(button_id, mode, player, action);
            // If the action is not recognised, check if it's a fire key
            // that should be mapped to select
            if(!device && m_map_fire_to_select)
            {
                device = mapKeyboardInput(button_id, InputManager::INGAME, player,
                                          action);
                if(device && *action == PA_FIRE)
                    *action = PA_MENU_SELECT;
            }
            break;
        case Input::IT_STICKBUTTON:
        case Input::IT_STICKMOTION:
            device = mapGamepadInput(type, device_id, button_id, axis_dir, value,
                                     mode, player, action);
            if(!device && m_map_fire_to_select)
            {
                device = mapGamepadInput(type, device_id, button_id, axis_dir, value,
                                         InputManager::INGAME, player, action);
                if(device && *action == PA_FIRE)
                    *action = PA_MENU_SELECT;
            }
            break;
        default:
            break;
    };


    // Return true if input was successfully translated to an action and player
    if (device != NULL && abs(*value) > Input::MAX_VALUE/2)
    {
        m_latest_used_device = device;
    }

    return (device != NULL);
}   // translateInput

//-----------------------------------------------------------------------------
InputDevice* DeviceManager::getLatestUsedDevice()
{
    // If none, probably the user clicked or used enter; give keyboard by default

    if (m_latest_used_device == NULL)
    {
        //Log::info("DeviceManager", "No latest device, returning keyboard);
        return m_keyboards.get(0); // FIXME: is this right?
    }

    return m_latest_used_device;
}   // getLatestUsedDevice

// -----------------------------------------------------------------------------
void DeviceManager::clearLatestUsedDevice()
{
    m_latest_used_device = NULL;
}   // clearLatestUsedDevice

// -----------------------------------------------------------------------------
/** Loads the configuration from the user's input.xml file.
 */
bool DeviceManager::load()
{
    std::string filepath = file_manager->getUserConfigFile(INPUT_FILE_NAME);

    if(UserConfigParams::logMisc())
        Log::info("Device manager","Loading input.xml...");

    const XMLNode *input = file_manager->createXMLTree(filepath);

    if(!input)
    {
        if(UserConfigParams::logMisc())
            Log::warn("Device manager","No configuration file exists.");
        return false;
    }

    if(input->getName()!="input")
    {
        Log::warn("DeviceManager", "Invalid input.xml file - no input node.");
        delete input;
        return false;
    }

    int version=0;
    if(!input->get("version", &version) || version != INPUT_FILE_VERSION )
    {
        //I18N: shown when config file is too old
        GUIEngine::showMessage(_("Please re-configure your key bindings."));
        GUIEngine::showMessage(_("Your input config file is not compatible "
                                 "with this version of STK."));
        delete input;
        return false;
    }

    for(unsigned int i=0; i<input->getNumNodes(); i++)
    {
        const XMLNode *config = input->getNode(i);
        DeviceConfig *device_config = DeviceConfig::create(config);
        if(!device_config)
        {
            Log::warn("DeviceManager",
                      "Invalid node '%s' in input.xml - ignored.",
                      config->getName().c_str());
            continue;
        }
        if(config->getName()=="keyboard")
        {
            KeyboardConfig *kc = static_cast<KeyboardConfig*>(device_config);
            m_keyboard_configs.push_back(kc);
        }
        else if (config->getName()=="gamepad")
        {
            GamepadConfig *gc = static_cast<GamepadConfig*>(device_config);
            m_gamepad_configs.push_back(gc);
        }
    }   // for i < getNumNodes

    if (UserConfigParams::logMisc())
    {
        Log::info("Device manager",
                  "Found %d keyboard and %d gamepad configurations.",
                  m_keyboard_configs.size(), m_gamepad_configs.size());
    }

    // For Debugging....
    /*
    for (int n = 0; n < m_keyboard_configs.size(); n++)
        printf("Config #%d\n%s", n + 1, m_keyboard_configs[n].toString().c_str());

    for (int n = 0; n < m_gamepad_configs.size(); n++)
        printf("%s", m_gamepad_configs[n].toString().c_str());
    */

    delete input;

    return true;
}   // load

// -----------------------------------------------------------------------------
void DeviceManager::save()
{
    static std::string filepath = file_manager->getUserConfigFile(INPUT_FILE_NAME);
    if(UserConfigParams::logMisc()) Log::info("Device manager","Serializing input.xml...");


    std::ofstream configfile;
    configfile.open (filepath.c_str());

    if(!configfile.is_open())
    {
        Log::error("Device manager","Failed to open %s for writing, controls won't be saved",filepath.c_str());
        return;
    }


    configfile << "<input version=\"" << INPUT_FILE_VERSION << "\">\n\n";

    for(unsigned int n=0; n<m_keyboard_configs.size(); n++)
    {
        m_keyboard_configs[n].save(configfile);
    }
    for(unsigned int n=0; n<m_gamepad_configs.size(); n++)
    {
        m_gamepad_configs[n].save(configfile);
    }

    configfile << "</input>\n";
    configfile.close();
    if(UserConfigParams::logMisc()) Log::info("Device manager","Serialization complete.");
}   // save

// -----------------------------------------------------------------------------

KeyboardDevice* DeviceManager::getKeyboardFromBtnID(const int button_id)
{
    const int keyboard_amount = m_keyboards.size();
    for (int n=0; n<keyboard_amount; n++)
    {
        if (m_keyboards[n].getConfiguration()->hasBindingFor(button_id))
            return m_keyboards.get(n);
    }

    return NULL;
}   // getKeyboardFromButtonID

// -----------------------------------------------------------------------------

void DeviceManager::shutdown()
{
    m_gamepads.clearAndDeleteAll();
    m_keyboards.clearAndDeleteAll();
    m_gamepad_configs.clearAndDeleteAll();
    m_keyboard_configs.clearAndDeleteAll();
    delete m_multitouch_device;
    m_multitouch_device = NULL;
    m_latest_used_device = NULL;
}   // shutdown
