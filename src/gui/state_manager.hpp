#ifndef STATE_MANAGER_HPP
#define STATE_MANAGER_HPP

#include <string>

namespace StateManager
{
    void pushMenu(std::string name);
    void popMenu();
    void resetAndGoToMenu(std::string name);
    void enterGameState();
    bool isGameState();
}

#endif