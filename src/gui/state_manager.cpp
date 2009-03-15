
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "sdldrv.hpp"
#include <vector>

namespace StateManager
{
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