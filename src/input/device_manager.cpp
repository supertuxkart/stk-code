#include "input/device_manager.hpp"

#include <iostream>
#include <fstream>

#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "io/file_manager.hpp"

#define INPUT_MODE_DEBUG 0

const char* INPUT_FILE_NAME = "input.xml";

DeviceManager::DeviceManager()
{
    m_latest_used_device = NULL;
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
    
    const int keyboard_amount = m_keyboard_configs.size();
    for (int n=0; n<keyboard_amount; n++)
    {
        m_keyboards.push_back(new KeyboardDevice(m_keyboard_configs.get(n)));
        // TODO: Detect keyboard presence, if there is no keyboard, this should be false
        m_keyboard_configs.get(n)->setInUse(n == 0);
    }
    
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
    
#if INPUT_MODE_DEBUG
    if (assignMode == NO_ASSIGN) std::cout << "====== DeviceManager::setAssignMode(NO_ASSIGN) ======\n";
    if (assignMode == ASSIGN) std::cout << "====== DeviceManager::setAssignMode(ASSIGN) ======\n";
    if (assignMode == DETECT_NEW) std::cout << "====== DeviceManager::setAssignMode(DETECT_NEW) ======\n";
#endif
    
    // when going back to no-assign mode, do some cleanup
    if (assignMode == NO_ASSIGN)
    {
        for (int i=0; i < m_gamepads.size(); i++)
        {
            m_gamepads[i].setPlayer(NULL);
        }
        for (int i=0; i < m_keyboards.size(); i++)
        {
            m_keyboards[i].setPlayer(NULL);
        }
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
    m_keyboards.push_back(d);
}
// -----------------------------------------------------------------------------
void DeviceManager::addEmptyKeyboard()
{
    KeyboardConfig* newConf = new KeyboardConfig();
    m_keyboard_configs.push_back(newConf);
    m_keyboards.push_back( new KeyboardDevice(newConf) );
}

// -----------------------------------------------------------------------------
void DeviceManager::addGamepad(GamePadDevice* d)
{
    m_gamepads.push_back(d);
}
// -----------------------------------------------------------------------------

InputDevice* DeviceManager::mapKeyboardInput( int btnID,
                                              ActivePlayer **player /* out */,
                                              PlayerAction *action /* out */ )
{
    const int keyboard_amount = m_keyboards.size();
    
    //std::cout << "mapKeyboardInput " << btnID << " to " << keyboard_amount << " keyboards\n";
    
    for (int n=0; n<keyboard_amount; n++)
    {
        KeyboardDevice *keyboard = m_keyboards.get(n);

        if (keyboard->hasBinding(btnID, action))
        {
            //std::cout << "   binding found in keyboard #"  << (n+1) << "; action is " << KartActionStrings[*action] << "\n";
            if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
            {
                *player = NULL;
            }
            else
            {
                *player = keyboard->m_player;
            }
            return keyboard;
        }
    }
    
    return NULL; // no appropriate binding found
}
//-----------------------------------------------------------------------------

InputDevice *DeviceManager::mapGamepadInput( Input::InputType type,
                                             int deviceID,
                                             int btnID,
                                             int axisDir,
                                             int value,
                                             ActivePlayer **player /* out */,
                                             PlayerAction *action /* out */)
{
    GamePadDevice  *gPad = getGamePadFromIrrID(deviceID);

    if (gPad != NULL) 
    {
        if (gPad->hasBinding(type, btnID, value, gPad->getPlayer(), action))
        {
            if (m_assign_mode == NO_ASSIGN) // Don't set the player in NO_ASSIGN mode
            {
                *player = NULL;
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

bool DeviceManager::translateInput( Input::InputType type,
                                    int deviceID,
                                    int btnID,
                                    int axisDir,
                                    int value,
                                    ActivePlayer** player /* out */,
                                    PlayerAction* action /* out */ )
{
    InputDevice *device = NULL;

    // If the input event matches a bind on an input device, get a pointer to the device
    switch (type)
    {
        case Input::IT_KEYBOARD:
            device = mapKeyboardInput(btnID, player, action);
            break;
        case Input::IT_STICKBUTTON:
        case Input::IT_STICKMOTION:
            device = mapGamepadInput(type, deviceID, btnID, axisDir, value, player, action);
            break;
        default:
            break;
    };

    //if (*player != NULL) std::cout << "btn " << btnID << " belongs to player " << (*player)->m_id << std::endl;
    
    /*
    if (device != NULL)
    {
        std::cout << "   binding found; action is " << KartActionStrings[*action] << "\n";
    }*/
    
    // Return true if input was successfully translated to an action and player
    if (device != NULL && abs(value) > Input::MAX_VALUE/2)
    {
        //std::cout<< "========== Setting latest device " << (device->getType() == DT_KEYBOARD ? "keyboard" : "gamepad")
        //  << " #" << deviceID << " button=" << btnID << " value=" << value << " ==========\n";

        m_latest_used_device = device;
    }
    return (device != NULL);
}
//-----------------------------------------------------------------------------
InputDevice* DeviceManager::getLatestUsedDevice()
{
    // If none, probably the user clicked or used enter; give keyboard by default
    
    if (m_latest_used_device == NULL)
    {   
        //std::cout<< "========== No latest device, returning keyboard ==========\n";
        return m_keyboards.get(0); // FIXME: is this right?
    }
    
    return m_latest_used_device;
}
// -----------------------------------------------------------------------------
bool DeviceManager::deserialize()
{
    static std::string filepath = file_manager->getConfigDir() + "/" + INPUT_FILE_NAME;
    
    printf("Deserializing input.xml...\n");

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
    static std::string filepath = file_manager->getConfigDir() + "/" + INPUT_FILE_NAME;
    printf("Serializing input.xml...\n");

    
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

KeyboardDevice* DeviceManager::getKeyboardFromBtnID(const int btnID)
{
    const int keyboard_amount = m_keyboards.size();
    for (int n=0; n<keyboard_amount; n++)
    {
        if (m_keyboards[n].getConfiguration()->hasBindingFor(btnID)) return m_keyboards.get(n);
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------

void DeviceManager::shutdown()
{
    m_gamepads.clearAndDeleteAll();
    m_keyboards.clearAndDeleteAll();
    m_gamepad_configs.clearAndDeleteAll();
    m_keyboard_configs.clearAndDeleteAll();
    m_latest_used_device = NULL;
}
