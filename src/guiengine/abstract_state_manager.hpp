#ifndef HEADER_ABSTRACT_STATE_MANAGER_HPP
#define HEADER_ABSTRACT_STATE_MANAGER_HPP

#include <vector>
#include <string>

namespace GUIEngine
{
    class Widget;

/**
  * Abstract base class you must override from to use the GUI engine
  */
class AbstractStateManager
{
protected:
    /**
      * Whether we are in game mode
      */
    bool m_game_mode;
    
    /**
     * This stack will contain menu names (e.g. main.stkgui), and/or 'race'.
     */
    std::vector<std::string> m_menu_stack;
    
public:
    AbstractStateManager();
    virtual ~AbstractStateManager() { }
    
    void pushMenu(std::string name);
    void replaceTopMostMenu(std::string name);
    void popMenu();
    void resetAndGoToMenu(std::string name);
    void enterGameState();
    bool isGameState();
    void reshowTopMostMenu();

    // method to override in children
    virtual void escapePressed() = 0;
    virtual void onUpdate(float elpased_time) = 0;
    virtual void eventCallback(Widget* widget, const std::string& name) = 0;
};

}
#endif
