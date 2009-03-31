
#include "input/device_manager.hpp"
#include "io/file_manager.hpp"
#include <iostream>
#include <fstream>


DeviceManager::DeviceManager()
{
    m_keyboard_amount = 0;
    m_gamepad_amount = 0;
}

void DeviceManager::add(KeyboardDevice* d)
{
    m_keyboards.push_back(d);
    m_keyboard_amount = m_keyboards.size();
}

void DeviceManager::add(GamePadDevice* d)
{
    m_gamepads.push_back(d);
    m_gamepad_amount = m_gamepads.size();
}

bool DeviceManager::mapInputToPlayerAndAction( Input::InputType type, int id0, int id1, int id2, int value,
                                              int* player /* out */, PlayerAction* action /* out */ )
{
    // TODO - auto-detect player ID from device
    *player = 0;
    
    if(type == Input::IT_KEYBOARD)
    {
        for(unsigned int n=0; n<m_keyboard_amount; n++)
        {
            if( m_keyboards[n].hasBinding(id0, action) ) return true;
        }
        return false;
    }
    else if(type == Input::IT_MOUSEBUTTON)
    {
        return false;
    }
    else if(type == Input::IT_STICKBUTTON || type == Input::IT_STICKMOTION)
    {
        // std::cout << "stick motion, ID=" <<id0 << " axis=" << id1 << " value=" << value  << std::endl;
        for(unsigned int n=0; n<m_gamepad_amount; n++)
        {
            if(m_gamepads[n].hasBinding(type, id1 /* axis or button */, value, *player, action /* out */) )
                return true;
        }
        return false;
    }
    else
    {
        return false;
    }
    
    return false;
}

bool DeviceManager::deserialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    
    if(!file_manager->fileExists(filepath)) return false;
    
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( filepath.c_str() );
    
    const int GAMEPAD = 1;
    const int KEYBOARD = 2;
    const int NOTHING = 3;
    
    int reading_now = NOTHING;
    
    KeyboardDevice* keyboard_device;
    GamePadDevice* gamepad_device;
    
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
                    keyboard_device = new KeyboardDevice(xml);
                    reading_now = KEYBOARD;
                }
                else if (strcmp("gamepad", xml->getNodeName()) == 0)
                {
                    gamepad_device = new GamePadDevice(xml);
                    reading_now = GAMEPAD;
                }
                else if (strcmp("action", xml->getNodeName()) == 0)
                {
                    if(reading_now == KEYBOARD) 
                    {
                        if(!keyboard_device->deserializeAction(xml))
                            std::cerr << "Ignoring an ill-formed action in input config\n";
                    }
                    else if(reading_now == GAMEPAD) 
                    {
                        if(!gamepad_device->deserializeAction(xml))
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
                    add(keyboard_device);
                    reading_now = NOTHING;
                }
                else if (strcmp("gamepad", xml->getNodeName()) == 0)
                {
                    add(gamepad_device);
                    reading_now = GAMEPAD;
                }
            }
                break;
                
            default: break;
                
        } // end switch
    } // end while
    
    return true;
}

/**
  * Check if we already have a config object for joystick 'sdl_id' as reported by SDL
  * If yes, 'open' the gamepad instance and returns false. If no, create on and return true.
  */
bool DeviceManager::checkForGamePad(const int sdl_id)
{
    std::string name = SDL_JoystickName(sdl_id);
    
    std::cout << "trying to find gamepad " << name.c_str() << std::endl;
    
    for(unsigned int n=0; n<m_gamepad_amount; n++)
    {
        std::cout << "  (checking...) gamepad " << n << " is named " << m_gamepads[n].m_name.c_str() << std::endl;
        
        if(m_gamepads[n].m_name == name)
        {
            std::cout << "--> that's the one\n";
            m_gamepads[n].open(sdl_id);
            return false;
        }
    }

    std::cout << "couldn't find this joystick, so creating a new one" << std::endl;
    add(new GamePadDevice(sdl_id));
    return true;

}
    
void DeviceManager::serialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    std::cout << "writing " << filepath.c_str() << std::endl;
    
    std::ofstream configfile;
    configfile.open (filepath.c_str());
    
    if(!configfile.is_open())
    {
        std::cerr << "Failed to open " << filepath.c_str() << " for writing, controls won't be saved\n";
        return;
    }
    
    
    configfile << "<input>\n\n";
    
    for(unsigned int n=0; n<m_keyboard_amount; n++)
    {
        m_keyboards[n].serialize(configfile);
    }
    for(unsigned int n=0; n<m_gamepad_amount; n++)
    {
        m_gamepads[n].serialize(configfile);
    }
    
    configfile << "</input>\n";
    
    configfile.close();    
}
