//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#include "gui/engine.hpp"

#include <iostream>
#include <assert.h>
#include "gui/screen.hpp"
#include "gui/skin.hpp"
#include "gui/widget.hpp"
#include "gui/credits.hpp"
#include "io/file_manager.hpp"
#include "gui/state_manager.hpp"
#include "input/input_manager.hpp"

namespace GUIEngine
{
    IGUIEnvironment* g_env;
    Skin* g_skin = NULL;
    IGUIFont* g_font;
    IrrlichtDevice* g_device;
    irr::video::IVideoDriver* g_driver;
    
    ptr_vector<Screen, HOLD> g_loaded_screens;
    Screen* g_current_screen = NULL;

    
    float dt = 0;
    
    float getLatestDt()
    {
        return dt;
    }
    
    class IrrlichtEventCore : public IEventReceiver
    {
    public:
        IrrlichtEventCore()
        {
        }
        ~IrrlichtEventCore()
        {
        }
        bool OnEvent (const SEvent &event)
        {
            if(event.EventType == EET_GUI_EVENT ||
               (!StateManager::isGameState() && event.EventType != EET_KEY_INPUT_EVENT && event.EventType != EET_JOYSTICK_INPUT_EVENT)
               )
            {
                if(g_current_screen == NULL) return false;
                g_current_screen->OnEvent(event);
                return false;
            }
            else
                return input_manager->input(event);
        }
    };
    IrrlichtEventCore* g_irrlicht_event_core = NULL;
// -----------------------------------------------------------------------------
IrrlichtDevice* getDevice()
{
    return g_device;
}
// -----------------------------------------------------------------------------
IGUIFont* getFont()
{
    return g_font;
}
// -----------------------------------------------------------------------------
IVideoDriver* getDriver()
{
    return g_driver;
}
// -----------------------------------------------------------------------------
IGUIEnvironment* getGUIEnv()
{
    return g_env;
}
// -----------------------------------------------------------------------------  
void clear()
{
    g_env->clear();
    g_current_screen = NULL;
}
// -----------------------------------------------------------------------------  
void cleanForGame()
{
    clear();
    if(g_irrlicht_event_core == NULL) g_irrlicht_event_core = new IrrlichtEventCore();
    g_device->setEventReceiver(g_irrlicht_event_core);
}
// -----------------------------------------------------------------------------  
void switchToScreen(const char* screen_name)
{    
    // clean what was left by the previous screen
    g_env->clear();
    if(g_current_screen != NULL) g_current_screen->elementsWereDeleted();
    g_current_screen = NULL;
    Widget::resetIDCounters();
    
    // check if we already loaded this screen
    const int screen_amount = g_loaded_screens.size();
    for(int n=0; n<screen_amount; n++)
    {
        if(g_loaded_screens[n].getName() == screen_name)
        {
            g_current_screen = g_loaded_screens.get(n);
            break;
        }
    }
    // screen not found in list of existing ones, so let's create it
    if(g_current_screen == NULL)
    {
        GUIEngine::Screen* new_screen = new GUIEngine::Screen(screen_name);
        g_loaded_screens.push_back(new_screen);
        g_current_screen = new_screen;
    }
    

    
    // show screen
    g_current_screen->addWidgets();
    
    // set event listener
    if(g_irrlicht_event_core == NULL) g_irrlicht_event_core = new IrrlichtEventCore();
    g_device->setEventReceiver(g_irrlicht_event_core);
    //g_env->setUserEventReceiver(g_irrlicht_event_core);

}
// -----------------------------------------------------------------------------
/** to be called after e.g. a resolution switch */
void reshowCurrentScreen()
{
    StateManager::reshowTopMostMenu();
    //g_current_screen->addWidgets();
}
// -----------------------------------------------------------------------------
Screen* getCurrentScreen()
{
    assert(g_current_screen != NULL);
    return g_current_screen;
}
// -----------------------------------------------------------------------------
void cleanUp()
{
    if(g_skin != NULL) delete g_skin;
    g_skin = NULL;
    g_loaded_screens.clearAndDeleteAll();
    
    g_current_screen = NULL;
    
    // nothing else to delete for now AFAIK, irrlicht will automatically kill everything along the device
}
// -----------------------------------------------------------------------------
void (*g_event_callback)(Widget* widget, std::string& name);
void init(IrrlichtDevice* device_a, IVideoDriver* driver_a, void (*eventCallback)(Widget* widget, std::string& name) )
{
    g_env = device_a->getGUIEnvironment();
    g_device = device_a;
    g_driver = driver_a;
    g_event_callback = eventCallback;
    
	/*
     To make the g_font a little bit nicer, we load an external g_font
     and set it as the new default g_font in the g_skin.
     To keep the standard g_font for tool tip text, we set it to
     the built-in g_font.
     */
    g_skin = new Skin(g_env->getSkin());
    g_env->setSkin(g_skin);
	//g_skin = g_env->getSkin();
	g_font = g_env->getFont( (file_manager->getGUIDir() + "/okolaks.xml").c_str() );
	if (g_font) g_skin->setFont(g_font);
    
	//g_skin->setFont(g_env->getBuiltInFont(), EGDF_TOOLTIP);
}
// -----------------------------------------------------------------------------
/** transmit event to user event callback (out of encapsulated GUI module) */
void transmitEvent(Widget* widget, std::string& name)
{
    assert(g_event_callback != NULL);
    g_event_callback(widget, name);
}
    
// -----------------------------------------------------------------------------    
void render(float elapsed_time)
{
    GUIEngine::dt = elapsed_time;
    
     // ---- menu drawing
    // draw background image and sections
    if(!StateManager::isGameState())
    {
        g_skin->drawBgImage();
        g_skin->renderSections();
    }
    
    // let irrLicht do the rest (the Skin object will be called for further render)
    g_env->drawAll();
    
    // ---- additionnal drawing
    if(!StateManager::isGameState() && getCurrentScreen()->getName() == "credits.stkgui")
    {
        Credits::getInstance()->render(elapsed_time);
    }
}

}
