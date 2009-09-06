#ifndef HEADER_ABSTRACT_STATE_MANAGER_HPP
#define HEADER_ABSTRACT_STATE_MANAGER_HPP

#include <vector>
#include <string>

namespace GUIEngine
{
    class Widget;

    enum GameState
    {
        MENU,
        GAME,
        INGAME_MENU
    };
    
/**
  * Abstract base class you must override from to use the GUI engine
  */
class AbstractStateManager
{
protected:
    /**
      * Whether we are in game mode
      */
    GameState m_game_mode;
    
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
    
    GameState getGameState();
    
    void reshowTopMostMenu();

    /*  ***********************************
        * methods to override in children *
        *********************************** */
    
    /**
      * callback called whenever escape was pressed (or any similar cancel operation)
      */
    virtual void escapePressed() = 0;
    
    /**
      * Called every frame, to allow updating animations if there is any need.
      */
    virtual void onUpdate(float elpased_time) = 0;
    
    /**
     * will be called everytime sometimes happens.
     * Events are generally a widget state change. In this case, a pointer to the said widget is passed along its
     * name, so you get its new state and/or act. There are two special events, passed with a NULL widget, and which
     * bear the anmes "init" and "tearDown", called respectively when a screen is being made visible and when it's
     * being left, allowing for setup/clean-up.
     */
    virtual void eventCallback(Widget* widget, const std::string& name) = 0;
};

}
#endif
