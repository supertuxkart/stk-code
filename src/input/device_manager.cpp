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
    printf("================================================================================\n");
    printf("Parsing Keyboard & Gamepad Configurations\n");
    printf("================================================================================\n\n");
    deserialize();

    m_keyboard = new KeyboardDevice(m_keyboard_configs.get(0));
    printf("Keyboard Configs:\n");
    for (int n = 0; n < m_keyboard_configs.size(); n++)
        printf("%s\n", m_keyboard_configs[n].toString().c_str());
    printf("Gamepad Configs:\n");
    for (int n = 0; n < m_gamepad_configs.size(); n++)
        printf("%s\n", m_gamepad_configs[n].toString().c_str());
    

    m_keyboard_amount = 0;
    m_gamepad_amount = 0;
    m_latest_used_device = NULL;
    m_assign_mode = NO_ASSIGN;
}
// -----------------------------------------------------------------------------
bool DeviceManager::initGamePadSupport()
{
    GamepadConfig           *gamepadConfig;
    bool something_new_to_write = true;

    // Prepare a list of connected joysticks.
    std::cout << "====================\nGamePad/Joystick detection and configuration\n====================\n";
    
    irr_driver->getDevice()->activateJoysticks(m_irrlicht_gamepads);
    const int numSticks = m_irrlicht_gamepads.size();    
    
    std::cout << "irrLicht detects " << numSticks << " gamepads" << std::endl;

    for (int i = 0; i < numSticks; i++)
    {
        std::cout << m_irrlicht_gamepads[i].Name.c_str() << " : "
        << m_irrlicht_gamepads[i].Axes << " axes , "
        << m_irrlicht_gamepads[i].Buttons << " buttons\n";
        gamepadConfig = getGamepadConfig(i);
        add(new GamePadDevice(i, m_irrlicht_gamepads[i].Name.c_str(), m_irrlicht_gamepads[i].Axes, m_irrlicht_gamepads[i].Buttons, gamepadConfig));
    }
    
    std::cout << "====================\n";
    
    return something_new_to_write;
}
// -----------------------------------------------------------------------------

void DeviceManager::setAssignMode(const PlayerAssignMode assignMode)
{
    m_assign_mode = assignMode;
    
    // when going back to no-assign mode, do some cleanup
    if(assignMode == NO_ASSIGN)
    {
        for(unsigned int i=0; i<m_gamepad_amount; i++)
        {
            m_gamepads[i].setPlayer(NULL);
        }
        m_keyboard->setPlayer(NULL);
    }
}
// -----------------------------------------------------------------------------
GamePadDevice* DeviceManager::getGamePadFromIrrID(const int id)
{
    for(unsigned int i=0; i<m_gamepad_amount; i++)
    {
        if(m_gamepads[i].m_index == id)
        {

            return m_gamepads.get(i);
        }
    }
    return NULL;
}
// -----------------------------------------------------------------------------
/**
 * Check if we already have a config object for gamepad 'irr_id' as reported by irrLicht
 * If yes, return a pointer to the configuration. If no, create one. Returns pointer.
 */
GamepadConfig *DeviceManager::getGamepadConfig(const int irr_id)
{
    std::string name = m_irrlicht_gamepads[irr_id].Name.c_str();
    GamepadConfig *config = NULL;
    
    std::cout << "trying to find gamepad configuration" << name.c_str() << std::endl;
    
    for(int n=0; n<m_gamepad_configs.size(); n++)
    {
        std::cout << "  (checking...) I remember that gamepad configuration #" << n << " is named " << m_gamepad_configs[n].getName().c_str() << std::endl;
        
        // FIXME - don't check only name, but also number of axes and buttons?
        // Only assign device IDs to gamepads which have not yet been assigned a device ID
        if(m_gamepad_configs[n].getName() == name)
        {
            std::cout << "--> that's the one currently connected\n";
            config = m_gamepad_configs.get(n);
        }
    }
    
    if (config == NULL)
    {
        std::cout << "couldn't find a configuration for this gamepad, creating one" << std::endl;
        config = new GamepadConfig(m_irrlicht_gamepads[irr_id].Name.c_str(), m_irrlicht_gamepads[irr_id].Axes, m_irrlicht_gamepads[irr_id].Buttons);
        m_gamepad_configs.push_back(config);
    }
    return config;
}
// -----------------------------------------------------------------------------
void DeviceManager::add(KeyboardDevice* d)
{
    m_keyboard = d;
/*
    m_keyboards.push_back(d);
    m_keyboard_amount = m_keyboards.size();
*/
}
// -----------------------------------------------------------------------------
void DeviceManager::add(GamePadDevice* d)
{
    m_gamepads.push_back(d);
    m_gamepad_amount = m_gamepads.size();
}
// -----------------------------------------------------------------------------
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
                    for(unsigned int n=0; n<m_gamepad_amount; n++)
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
                                std::cerr << "Ignoring an ill-formed action in input config\n";
                    }
                    else if(reading_now == GAMEPAD) 
                    {
                        if(gamepad_config != NULL)
                            if(!gamepad_config->deserializeAction(xml))
                                std::cerr << "Ignoring an ill-formed action in input config\n";
                    }
                    else std::cerr << "Warning: An action is placed in an unexpected area in the input config file\n";
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

    if (m_keyboard_configs.size() == 0)
        m_keyboard_configs.push_back(new KeyboardConfig());

    return true;
}
// -----------------------------------------------------------------------------
void DeviceManager::serialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    user_config->CheckAndCreateDir();
    
    std::cout << "writing " << filepath.c_str() << std::endl;
    
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
