
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "gui/widget.hpp"
#include "gui/screen.hpp"
#include "sdldrv.hpp"
#include "graphics/irr_driver.hpp"

#include <vector>

namespace StateManager
{
    void showTrackSelectionScreen()
    {
        GUIEngine::switchToScreen("tracks.stkgui");
        GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>(GUIEngine::getCurrentScreen()->getWidget("tracks"));
        assert( w != NULL );
        
        // FIXME - should be called only once, not on every show?
        w->addItem("Track 1","t1","track1.png");
        w->addItem("Track 2","t2","track2.png");
        w->addItem("Track 3","t3","track3.png");
        w->addItem("Track 4","t4","track4.png");
        w->addItem("Track 5","t5","track5.png");
        w->addItem("Track 6","t6","track6.png");
        w->addItem("Track 7","t7","track7.png");
        w->addItem("Track 8","t8","track8.png");
        w->updateItemDisplay();
    }
    
    void eventCallback(GUIEngine::Widget* widget, std::string& name)
    {
        std::cout << "event!! " << name.c_str() << std::endl;
        
        if(name == "menu_toprow")
        {
            std::string selection = ((GUIEngine::RibbonWidget*)widget)->getSelectionName().c_str();
            if(selection == "new") GUIEngine::switchToScreen("racesetup.stkgui");
            if(selection == "challenges")
            {
                GUIEngine::switchToScreen("karts.stkgui");
                GUIEngine::RibbonGridWidget* w = dynamic_cast<GUIEngine::RibbonGridWidget*>(GUIEngine::getCurrentScreen()->getWidget("karts"));
                assert( w != NULL );
                
                // FIXME - should be called only once, not on every show?
                w->addItem("Gnu","k1","gnu.png");
                w->addItem("Wilber","k2","gnu.png");
                w->addItem("Tux","k3","gnu.png");
                w->addItem("Puffy","k4","gnu.png");
                w->addItem("Hexley","k5","gnu.png");
                w->addItem("Sushi","k6","gnu.png");
                w->addItem("Nolok","k7","gnu.png");
                w->addItem("Mozilla","k8","gnu.png");
                w->updateItemDisplay();
                
                GUIEngine::SpinnerWidget* w2 = dynamic_cast<GUIEngine::SpinnerWidget*>(GUIEngine::getCurrentScreen()->getWidget("player"));
                assert( w2 != NULL );
                w2->addLabel("Hiker");
                w2->addLabel("Auria");
                w2->addLabel("Conso");
                w2->addLabel("MiniBjorn");
            }
        }
        if(name == "gamemode")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            if(w->getSelectionName() == "normal")
            {
                showTrackSelectionScreen();
            }
        }
        if(name == "tracks")
        {
            GUIEngine::RibbonGridWidget* w2 = dynamic_cast<GUIEngine::RibbonGridWidget*>(widget);
            if(w2 != NULL)
                std::cout << "Clicked on track " << w2->getSelectionName().c_str() << std::endl;
        }
        if(name == "gps")
        {
            GUIEngine::RibbonWidget* w = dynamic_cast<GUIEngine::RibbonWidget*>(widget);
            if(w != NULL)
                std::cout << "Clicked on GrandPrix " << w->getSelectionName().c_str() << std::endl;
        }    
    }
    
    void initGUI()
    {
        IrrlichtDevice* device = irr_driver->getDevice();
        video::IVideoDriver* driver = device->getVideoDriver();
        GUIEngine::init(device, driver, &eventCallback);
    }
    
    std::vector<std::string> g_menu_stack;
    static bool g_game_mode = false;
    
    void pushMenu(std::string name)
    {
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
    }
    
    void popMenu()
    {
        g_menu_stack.pop_back();
        g_game_mode = g_menu_stack[g_menu_stack.size()-1] == "race";
        GUIEngine::switchToScreen(g_menu_stack[g_menu_stack.size()-1].c_str());
    }
    
    void resetAndGoToMenu(std::string name)
    {
        g_menu_stack.clear();
        g_menu_stack.push_back(name);
        g_game_mode = false;
        GUIEngine::switchToScreen(name.c_str());
    }
    
    void enterGameState()
    {
        g_menu_stack.clear();
        g_menu_stack.push_back("race");
        g_game_mode = true;
        GUIEngine::clear();
        inputDriver->setMode(SDLDriver::INGAME);
    }
    
    bool isGameState()
    {
        return g_game_mode;
    }
    
}