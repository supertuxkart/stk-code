#include "gui/engine.hpp"
#include "gui/screen.hpp"
#include "gui/skin.hpp"
#include "gui/widget.hpp"
#include <iostream>

namespace GUIEngine
{

IGUIEnvironment* g_env;
IGUISkin* g_skin;
IGUIFont* g_font;
IrrlichtDevice* g_device;
irr::video::IVideoDriver* g_driver;

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

std::vector<Screen*> g_loaded_screens;
Screen* g_current_screen = NULL;
    
void switchToScreen(const char* screen_name)
{
    // clean what was left by the previous screen
    g_env->clear();
    g_current_screen = NULL;
    Widget::resetIDCounters();
    
    // check if we already loaded this screen
    const int screen_amount = g_loaded_screens.size();
    for(int n=0; n<screen_amount; n++)
    {
        if((*g_loaded_screens[n]) == screen_name)
        {
            g_current_screen = g_loaded_screens[n];
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
}
// -----------------------------------------------------------------------------
Screen* getCurrentScreen()
{
    assert(g_current_screen != NULL);
    return g_current_screen;
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
	g_font = g_env->getFont("fonthaettenschweiler.bmp");
	if (g_font) g_skin->setFont(g_font);
    
	//g_skin->setFont(g_env->getBuiltInFont(), EGDF_TOOLTIP);
}
// -----------------------------------------------------------------------------
void transmitEvent(Widget* widget, std::string& name)
{
    assert(g_event_callback != NULL);
    g_event_callback(widget, name);
}
// -----------------------------------------------------------------------------    
void render()
{
    g_env->drawAll();
}

}
