#include "gui/engine.hpp"

#include <iostream>
#include <assert.h>
#include "gui/screen.hpp"
#include "gui/skin.hpp"
#include "gui/widget.hpp"
#include "io/file_manager.hpp"
#include "gui/state_manager.hpp"
#include "input/input_manager.hpp"

namespace GUIEngine
{
    IGUIEnvironment* g_env;
    IGUISkin* g_skin = NULL;
    IGUIFont* g_font;
    IrrlichtDevice* g_device;
    irr::video::IVideoDriver* g_driver;
    
    ptr_vector<Screen, HOLD> g_loaded_screens;
    Screen* g_current_screen = NULL;

    ITexture* bg_image = NULL;
    
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
            if(event.EventType == EET_GUI_EVENT || !StateManager::isGameState())
            {
                if(g_current_screen == NULL) return false;
                g_current_screen->OnEvent(event);
            }
            else
                input_manager->input(event);
            return false;
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
void free()
{
    if(g_skin != NULL) delete g_skin;
    g_skin = NULL;
    bg_image = NULL;
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
void transmitEvent(Widget* widget, std::string& name)
{
    assert(g_event_callback != NULL);
    g_event_callback(widget, name);
}
// -----------------------------------------------------------------------------    
void render(float elapsed_time)
{
    GUIEngine::dt = elapsed_time;
    
    // on one end, making these static is not too clean.
    // on another end, these variables are really only used locally,
    // and making them static avoids doing the same stupid computations every frame
    static core::rect<s32> dest;
    static core::rect<s32> source_area;
    
    if(bg_image == NULL)
    {
        int texture_w, texture_h;
        // TODO/FIXME? - user preferences still include a background image choice 
        bg_image = GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/background.jpg").c_str() );
        assert(bg_image != NULL);
        texture_w = bg_image->getSize().Width;
        texture_h = bg_image->getSize().Height;
        
        source_area = core::rect<s32>(0, 0, texture_w, texture_h);
        
        core::dimension2d<s32> frame_size = GUIEngine::getDriver()->getCurrentRenderTargetSize();
        const int screen_w = frame_size.Width;
        const int screen_h = frame_size.Height;
        
        // stretch image vertically to fit
        float ratio = (float)screen_h / texture_h;
        
        // check that with the vertical stretching, it still fits horizontally
        while(texture_w*ratio < screen_w) ratio += 0.1f;
        
        texture_w = (int)(texture_w*ratio);
        texture_h = (int)(texture_h*ratio);
        
        const int clipped_x_space = (texture_w - screen_w);
        
        dest = core::rect<s32>(-clipped_x_space/2, 0, screen_w+clipped_x_space/2, texture_h);
    }

    if(!StateManager::isGameState())
        GUIEngine::getDriver()->draw2DImage(bg_image, dest, source_area,
                                            0 /* no clipping */, 0, false /* alpha */);
    
    //GUIEngine::getDriver()->draw2DRectangle( SColor(255, 0, 150, 0), rect );
    
    g_env->drawAll();
}

}