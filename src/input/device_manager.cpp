#include "input/device_manager.hpp"

#include <iostream>
#include <fstream>

#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "io/file_manager.hpp"

DeviceManager::DeviceManager()
{
    m_latest_used_device = NULL;
    m_keyboard = NULL;
    m_assign_mode = NO_ASSIGN;
}

// -----------------------------------------------------------------------------
bool DeviceManager::initialize()
{
    GamepadConfig           *gamepadConfig = NULL;
    GamePadDevice           *gamepadDevice = NULL;
    bool created = false;
    int numGamepads;


    // Shutdown in case the device manager is being re-initialized
    shutdown();

    printf("Initializing Device Manager\n");
    printf("---------------------------\n");

    deserialize();

    // Assign a configuration to the keyboard, or create one if we haven't yet
    printf("Initializing keyboard support.\n");
    if (m_keyboard_configs.size() == 0)
    {
        printf("No keyboard configuration exists, creating one.\n");
        m_keyboard_configs.push_back(new KeyboardConfig());
        created = true;
    }
    m_keyboard = new KeyboardDevice(m_keyboard_configs.get(0));
    // TODO: Detect keyboard presence, if there is no keyboard, this should be false
    m_keyboard_configs.get(0)->setInUse(true);

    printf("Initializing gamepad support.\n");

    irr_driver->getDevice()->activateJoysticks(m_irrlicht_gamepads);
    numGamepads = m_irrlicht_gamepads.size();    
    printf("Irrlicht reports %d gamepads are attached to the system.\n", numGamepads);

    // Create GamePadDevice for each physical gamepad and find a GamepadConfig to match
    for (int id = 0; id < numGamepads; id++)
    {
        printf("#%d: %s detected...", id, m_irrlicht_gamepads[id].Name.c_str());
        // Returns true if new configuration was created
        if (getConfigForGamepad(id, &gamepadConfig) == true)
        {
            printf("creating new configuration.\n");
            created = true;
        }
        else
        {
            printf("using existing configuration.\n");
        }

        gamepadConfig->setInUse(true);
        gamepadDevice = new GamePadDevice( id, 
                                           m_irrlicht_gamepads[id].Name.c_str(),
                                           m_irrlicht_gamepads[id].Axes,
                                           m_irrlicht_gamepads[id].Buttons,
                                           gamepadConfig );
        addGamepad(gamepadDevice);
    } // end for

    if (created) serialize();
    return created;
}
// -----------------------------------------------------------------------------

void DeviceManager::setAssignMode(const PlayerAssignMode assignMode)
{
    m_assign_mode = assignMode;
    
    // when going back to no-assign mode, do some cleanup
    if(assignMode == NO_ASSIGN)
    {
        for(int i=0; i < m_gamepads.size(); i++)
        {
            m_gamepads[i].setPlayer(NULL);
        }
        m_keyboard->setPlayer(NULL);
    }
}
// -----------------------------------------------------------------------------
GamePadDevice* DeviceManager::getGamePadFromIrrID(const int id)
{
    GamePadDevice *gamepad = NULL;

    for(int i = 0; i < m_gamepads.size(); i++)
    {
        if(m_gamepads[i].m_index == id)
        {

            gamepad = m_gamepads.get(i);
        }
    }
    return gamepad;
}
// -----------------------------------------------------------------------------
/**
 * Check if we already have a config object for gamepad 'irr_id' as reported by irrLicht
 * If no, create one. Returns true if new configuration was created, otherwise false.
 */
bool DeviceManager::getConfigForGamepad(const int irr_id, GamepadConfig **config)
{
    bool found = false;
    bool configCreated = false;
    std::string name = m_irrlicht_gamepads[irr_id].Name.c_str();
    
    // Find appropriate configuration
    for(int n=0; n < m_gamepad_configs.size(); n++)
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
        *config = new GamepadConfig( m_irrlicht_gamepads[irr_id].Name.c_str(), 
                                     m_irrlicht_gamepads[irr_id].Axes, 
                                     m_irrlicht_gamepads[irr_id].Buttons );

        // Add new config to list
        m_gamepad_configs.push_back(*config);
        configCreated = true;
    }

    return configCreated;
}
// -----------------------------------------------------------------------------
void DeviceManager::addKeyboard(KeyboardDevice* d)
{
    m_keyboard = d;
}
// -----------------------------------------------------------------------------
void DeviceManager::addGamepad(GamePadDevice* d)
{
    m_gamepads.push_back(d);
}
// -----------------------------------------------------------------------------

InputDevice *DeviceManager::mapKeyboardInput( int deviceID,
                                              int btnID,
                                              const bool progGen,
                                              ActivePlayer **player,
                                              PlayerAction *action )
{
    InputDevice *device = m_keyboard;

    if (m_keyboard->hasBinding(btnID, action))
    {
        if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
        {
            *player = NULL;
            if (!progGen) m_latest_used_device = m_keyboard;
        }
        else *player = m_keyboard->m_player;
    }
    else device = NULL; // If no appropriate bind was found, return NULL

    return device;
}
//-----------------------------------------------------------------------------

InputDevice *DeviceManager::mapGamepadInput( Input::InputType type,
                                             int deviceID,
                                             int btnID,
                                             int axisDir,
                                             int value,
                                             const bool progGen,
                                             ActivePlayer **player,
                                             PlayerAction *action )
{
    GamePadDevice  *gPad = getGamePadFromIrrID(deviceID);

    if (gPad != NULL) 
    {
        if (gPad->hasBinding(type, btnID, value, NULL, action))
        {
            if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
            {
                *player = NULL;
                // IT_STICKMOTION happens all the time, don't consider it discrete input
                if ((!progGen) && (type == Input::IT_STICKBUTTON)) m_latest_used_device = gPad;
            }
            else 
            {
                *player = gPad->m_player;
            }
        }
        else gPad = NULL; // If no bind was found, return NULL
    }

    return gPad;
}
//-----------------------------------------------------------------------------

// Formerly mapInputToPlayerAndAction(), broken down to be more readable

bool DeviceManager::translateInput( Input::InputType type,
                                    int deviceID,
                                    int btnID,
                                    int axisDir,
                                    int value,
                                    const bool programaticallyGenerated,
                                    ActivePlayer** player /* out */,
                                    PlayerAction* action /* out */ )
{
    InputDevice *device = NULL;

    // If the input event matches a bind on an input device, get a pointer to the device
    switch (type)
    {
        case Input::IT_KEYBOARD:
            device = mapKeyboardInput(deviceID, btnID, programaticallyGenerated, player, action);
            break;
        case Input::IT_STICKBUTTON:
            device = mapGamepadInput(type, deviceID, btnID, axisDir, value, programaticallyGenerated, player, action);
            break;
        case Input::IT_STICKMOTION:
            device = mapGamepadInput(type, deviceID, btnID, axisDir, value, programaticallyGenerated, player, action);
            break;
        default:
            break;
    };

    // If a matching device was found
    if (device != NULL)
    {
        // Handle internal events


/*  FIXME: only call when in kart selection screen
        if ((*player != NULL) && (*action == PA_RESCUE))
        {
            KartSelectionScreen::playerPressedRescue( *player );
            *action = PA_FIRST; // FIXME: action set to PA_FIRST if handled internally (too hackish)
        }
*/

        if ((*player == NULL) && (*action == PA_FIRE) && (m_assign_mode == DETECT_NEW))
        {
            KartSelectionScreen::firePressedOnNewDevice( device );
            *action = PA_FIRST;
        }
    }

    // Return true if input was successfully translated to an action and player
    return (device != NULL);
}
//-----------------------------------------------------------------------------
InputDevice* DeviceManager::getLatestUsedDevice()
{
    // If none, probably the user clicked or used enter; give keyboard by default
    
    if (m_latest_used_device == NULL )
    {    
        return m_keyboard;
    }
    
    return m_latest_used_device;
}
// -----------------------------------------------------------------------------
bool DeviceManager::deserialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    
    printf("Deserializing input.config...\n");

    if(!file_manager->fileExists(filepath))
    {
        printf("Warning: no configuration file exists.\n");
    }
    else
    {
        irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( filepath.c_str() );
    
        const int GAMEPAD = 1;
        const int KEYBOARD = 2;
        const int NOTHING = 3;
    
        int reading_now = NOTHING;
    
        KeyboardConfig* keyboard_config = NULL;
        GamepadConfig* gamepad_config = NULL;
    
    
        // parse XML file
        while(xml && xml->read())
        {
            switch(xml->getNodeType())
            {
                case irr::io::EXN_TEXT:
                    break;
                    
                case irr::io::EXN_ELEMENT:
                {
                    if (strcmp("keyboard", xml->getNodeName()) == 0)
                    {
                        keyboard_config = new KeyboardConfig();
                        reading_now = KEYBOARD;
                    }
                    else if (strcmp("gamepad", xml->getNodeName()) == 0)
                    {
                        gamepad_config = new GamepadConfig(xml);
                        reading_now = GAMEPAD;
                    }
                    else if (strcmp("action", xml->getNodeName()) == 0)
                    {
                        if(reading_now == KEYBOARD) 
                        {
                            if(keyboard_config != NULL)
                                if(!keyboard_config->deserializeAction(xml))
                                    std::cerr << "Ignoring an ill-formed action in input config.\n";
                        }
                        else if(reading_now == GAMEPAD) 
                        {
                            if(gamepad_config != NULL)
                                if(!gamepad_config->deserializeAction(xml))
                                    std::cerr << "Ignoring an ill-formed action in input config.\n";
                        }
                        else std::cerr << "Warning: An action is placed in an unexpected area in the input config file.\n";
                    }
                }
                break;
                // ---- section ending
                case irr::io::EXN_ELEMENT_END:
                {
                    if (strcmp("keyboard", xml->getNodeName()) == 0)
                    {
                        m_keyboard_configs.push_back(keyboard_config);
                        reading_now = NOTHING;
                    }
                    else if (strcmp("gamepad", xml->getNodeName()) == 0)
                    {
                        m_gamepad_configs.push_back(gamepad_config);
                        reading_now = NOTHING;
                    }
                }
                    break;
                    
                default: break;
                
            } // end switch
        } // end while

        printf("Found %d keyboard and %d gamepad configurations.\n", m_keyboard_configs.size(), m_gamepad_configs.size());
        // For Debugging....
        /*
        for (int n = 0; n < m_keyboard_configs.size(); n++)
            printf("Config #%d\n%s", n + 1, m_keyboard_configs[n].toString().c_str());

        for (int n = 0; n < m_gamepad_configs.size(); n++)
            printf("%s", m_gamepad_configs[n].toString().c_str());
        */
    }

    return true;
}
// -----------------------------------------------------------------------------
void DeviceManager::serialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    user_config->CheckAndCreateDir();
    printf("Serializing input.config...\n");

    
    std::ofstream configfile;
    configfile.open (filepath.c_str());
    
    if(!configfile.is_open())
    {
        std::cerr << "Failed to open " << filepath.c_str() << " for writing, controls won't be saved\n";
        return;
    }
    
    
    configfile << "<input>\n\n";
    
    for(int n=0; n<m_keyboard_configs.size(); n++)
    {
        m_keyboard_configs[n].serialize(configfile);
    }
    for(int n=0; n<m_gamepad_configs.size(); n++)
    {
        m_gamepad_configs[n].serialize(configfile);
    }
    
    configfile << "</input>\n";
    configfile.close(); 
    printf("Serialization complete.\n\n");
}


// -----------------------------------------------------------------------------

void DeviceManager::shutdown()
{
    m_gamepads.clearAndDeleteAll();
    m_gamepad_configs.clearAndDeleteAll();
    m_keyboard_configs.clearAndDeleteAll();
    m_latest_used_device = NULL;
    if (m_keyboard != NULL)
    {
        delete m_keyboard;
        m_keyboard = NULL;
    }
}

