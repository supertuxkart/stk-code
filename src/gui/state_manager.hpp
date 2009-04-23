#ifndef STATE_MANAGER_HPP
#define STATE_MANAGER_HPP

#include <string>

namespace StateManager
{
    void initGUI();
    
    void pushMenu(std::string name);
    void replaceTopMostMenu(std::string name);
    void popMenu();
    void resetAndGoToMenu(std::string name);
    void enterGameState();
    bool isGameState();
    void reshowTopMostMenu();
    
    void escapePressed();
}

#endif
