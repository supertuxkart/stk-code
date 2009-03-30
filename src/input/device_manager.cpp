
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
    else if(type == Input::IT_STICKBUTTON)
    {
        return false;
    }
    else if(type == Input::IT_STICKMOTION)
    {
        std::cout << "stick motion, ID=" <<id0 << " axis=" << id1 << " value=" << value  << std::endl;
        for(unsigned int n=0; n<m_gamepad_amount; n++)
        {
            if( /*m_gamepads[n].m_index == id0 &&*/ m_gamepads[n].hasBinding(id1 /* axis */, value, *player, action /* out */) )
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
                    keyboard_device = new KeyboardDevice();
                    
                    // TODO - read name and owner attributes
                    
                    reading_now = KEYBOARD;
                }
                else if (strcmp("gamepad", xml->getNodeName()) == 0)
                {
                    // TODO
                    //gamepad_device = new GamePadDevice();
                    
                    // TODO - read name and owner attributes
                    
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
                        // TODO
                        //if(!gamepad_device->deserializeAction(xml))
                        //    std::cerr << "Ignoring an ill-formed action in input config\n";
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
                    // TODO
                    // add(gamepad_device);
                    reading_now = GAMEPAD;
                }
            }
                break;
                
            default: break;
                
        } // end switch
    } // end while
    
    return true;
}

void DeviceManager::serialize()
{
    static std::string filepath = file_manager->getHomeDir() + "/input.config";
    
    std::ofstream configfile;
    configfile.open (filepath.c_str());
    
    if(!configfile.is_open())
    {
        std::cerr << "Failed to open " << filepath.c_str() << " for writing, controls won't be saved\n";
        return;
    }
    
    
    for(unsigned int n=0; n<m_keyboard_amount; n++)
    {
        m_keyboards[n].serialize(configfile);
    }
    for(unsigned int n=0; n<m_gamepad_amount; n++)
    {
        m_gamepads[n].serialize(configfile);
    }
    
    configfile.close();    
}
