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
    m_assign_mode = NO_ASSIGN;
}
// -----------------------------------------------------------------------------
bool DeviceManager::initGamePadSupport()
{
    GamepadConfig           *gamepadConfig = NULL;
    GamePadDevice           *gamepadDevice = NULL;
    bool created = false;
    int numGamepads;

    printf("================================================================================\n");
    printf("Initializing Gamepad Support\n");
    printf("================================================================================\n\n");

    irr_driver->getDevice()->activateJoysticks(m_irrlicht_gamepads);
    numGamepads = m_irrlicht_gamepads.size();    

    // Create GamePadDevice for each physical gamepad and attach a configuration
    for (int id = 0; id < numGamepads; id++)
    {
        printf("#%d: %s detected...", id, m_irrlicht_gamepads[id].Name.c_str());
        // Returns true if new configuration was created
        if (getGamepadConfig(id, &gamepadConfig) == true)
        {
            printf("creating new configuration.\n");
            created = true;
        }
        else
        {
            printf("using existing configuration.\n");
        }

        gamepadDevice = new GamePadDevice( id, 
                                           m_irrlicht_gamepads[id].Name.c_str(),
                                           m_irrlicht_gamepads[id].Axes,
                                           m_irrlicht_gamepads[id].Buttons,
                                           gamepadConfig );

        addGamepad(gamepadDevice);
        
    } // end for
    printf("Gamepad support initialization complete.\n\n");

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
bool DeviceManager::getGamepadConfig(const int irr_id, GamepadConfig **config)
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
/*
    m_keyboards.push_back(d);
    m_keyboard_amount = m_keyboards.size();
*/
}
// -----------------------------------------------------------------------------
void DeviceManager::addGamepad(GamePadDevice* d)
{
    m_gamepads.push_back(d);
//    m_gamepad_amount = m_gamepads.size();
}
// -----------------------------------------------------------------------------

// TODO: rewrite this
bool DeviceManager::mapInputToPlayerAndAction( Input::InputType type, int deviceID, int btnID, int axisDir, int value,
                                              const bool programaticallyGenerated, ActivePlayer** player /* out */,
                                              PlayerAction* action /* out */ )
{
    if(m_assign_mode == NO_ASSIGN)
    {
        *player = NULL;
    }

    
    if(type == Input::IT_KEYBOARD)
    {
        if( m_keyboard->hasBinding(btnID, action) )
        {
            // We found which device was triggered.
                         
            if(m_assign_mode == NO_ASSIGN)
            {
                // In no-assign mode, simply keep track of which device is used
                if(!programaticallyGenerated) m_latest_used_device = m_keyboard;
                 
                //if(programaticallyGenerated) std::cout << "devieManager ignores programatical event\n";
            }
            else
            {
                // In assign mode, find to which active player this binding belongs
                if (m_keyboard->m_player != NULL)
                {
                    *player = m_keyboard->m_player;
                            
                    if (m_assign_mode == DETECT_NEW && *action == PA_RESCUE)
                    {
                        if (value > Input::MAX_VALUE/2) KartSelectionScreen::playerPressedRescue( *player );
                        *action = PA_FIRST; // FIXME : returning PA_FIRST is quite a hackish way to tell input was handled internally
                    }
                    return true;
                }
                else
                {
                    // no active player has this binding. if we want to check for new players trying to join,
                    // check now
                    if (m_assign_mode == DETECT_NEW)
                    {
                        PlayerAction localaction = PA_FIRST; // none
                        if (m_keyboard->hasBinding(btnID, &localaction))
                        {
                            if(localaction == PA_FIRE)
                            {
                                if (value > Input::MAX_VALUE/2)
                                    KartSelectionScreen::firePressedOnNewDevice( m_keyboard );
                            }
                                
                            *action = PA_FIRST; // FIXME : returning PA_FIRST is quite a hackish way to tell input was handled internally
                            return true;
                        }
                    } // end if assign_mode == DETECT_NEW
                }
                  
                return false;
            } // end if/else NO_ASSIGN mode
                       
            
            return true;
        }
        return false;
    }
    else if(type == Input::IT_MOUSEBUTTON)
    {
        return false;
    }
    else if(type == Input::IT_STICKBUTTON || type == Input::IT_STICKMOTION)
    {

        GamePadDevice* gamepad = getGamePadFromIrrID(deviceID);

        if (gamepad == NULL) {
            // Prevent null pointer crash
            *player = NULL;
            return false;
        }
        
        if(m_assign_mode == NO_ASSIGN)
        {
            if(gamepad->hasBinding(type, btnID /* axis or button */, value, *player, action /* out */) )
            {
                // In no-assign mode, simply keep track of which device is used.
                // Only assign for buttons, since axes may send some small values even when
                // not actively used by user
                if(!programaticallyGenerated && type == Input::IT_STICKBUTTON)
                {
                    m_latest_used_device = gamepad;
                }
                
                return true;
            }
        }
        else
        {
            if(gamepad->m_player != NULL)
            {
                *player = gamepad->m_player;
            }
            else
            {
                
                // no active player has this binding. if we want to check for new players trying to join,
                // check now
                if (m_assign_mode == DETECT_NEW)
                {
                    for(int n=0; n < m_gamepads.size(); n++)
                    {
                        PlayerAction localaction = PA_FIRST; // none
                        if (m_gamepads[n].hasBinding(type, btnID, value, NULL, &localaction) && localaction == PA_FIRE)
                        {
                            if (value > Input::MAX_VALUE/2)
                                KartSelectionScreen::firePressedOnNewDevice( m_gamepads.get(n) );
                            *action = PA_FIRST;
                            return true;
                        }
                    } // end for
                }
                
                return false; // no player mapped to this device
            }
                
            
            if(gamepad->hasBinding(type, btnID /* axis or button */, value, *player, action /* out */) )
            {
                if (m_assign_mode == DETECT_NEW && *action == PA_RESCUE)
                {
                    if (value > Input::MAX_VALUE/2) KartSelectionScreen::playerPressedRescue( *player );
                    *action = PA_FIRST; // FIXME : returning PA_FIRST is quite a hackish way to tell input was handled internally
                }
                
                return true;
            }
        }
        
        return false;
    }
    else
    {
        return false;
    }
    
    return false;
}
// -----------------------------------------------------------------------------
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
    
    if(!file_manager->fileExists(filepath)) return false;
    
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( filepath.c_str() );
    
    const int GAMEPAD = 1;
    const int KEYBOARD = 2;
    const int NOTHING = 3;
    
    int reading_now = NOTHING;
    
    KeyboardConfig* keyboard_config = NULL;
    GamepadConfig* gamepad_config = NULL;
    
    printf("================================================================================\n");
    printf("Deserializing input.config\n");
    printf("================================================================================\n\n");

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

    // For Debugging....
    printf("Keyboard Configs:\n");
    for (int n = 0; n < m_keyboard_configs.size(); n++)
        printf("%s\n", m_keyboard_configs[n].toString().c_str());
    printf("Gamepad Configs:\n");
    for (int n = 0; n < m_gamepad_configs.size(); n++)
        printf("%s\n", m_gamepad_configs[n].toString().c_str());

    if (m_keyboard_configs.size() == 0)
    {
        printf("No keyboard configuration exists, creating one.\n");
        m_keyboard_configs.push_back(new KeyboardConfig());
    }

    m_keyboard = new KeyboardDevice(m_keyboard_configs.get(0));
    printf("Deserialization completed.\n\n");

    return true;
}
// -----------------------------------------------------------------------------
void DeviceManager::serialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    user_config->CheckAndCreateDir();
    printf("Saving Gamepad & Keyboard Configuration\n");

    
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
}
