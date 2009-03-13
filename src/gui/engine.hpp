#ifndef HEADER_ENGINE_HPP
#define HEADER_ENGINE_HPP

#include <irrlicht.h>
#include <string>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace GUIEngine
{
    class Screen;
    class Widget;
    
    extern IrrlichtDevice* getDevice();
    extern IGUIEnvironment* getGUIEnv();
    extern IVideoDriver* getDriver();
    extern IGUIFont* getFont();

    void init(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, void (*eventCallback)(Widget* widget, std::string& name) );
    void switchToScreen(const char* );
    Screen* getCurrentScreen();
    
    void render();
    void transmitEvent(Widget* widget, std::string& name);
     
}

#endif