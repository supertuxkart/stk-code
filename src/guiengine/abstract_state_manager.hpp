#ifndef HEADER_ABSTRACT_STATE_MANAGER_HPP
#define HEADER_ABSTRACT_STATE_MANAGER_HPP

#include <vector>
#include <string>

namespace GUIEngine
{
    class Widget;
    class Screen;

    enum GameState
    {
        MENU,
        GAME,
        CUTSCENE,
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
    
    void pushMenu(std::string name);
    void pushCutScene(std::string name);
    
public:
    AbstractStateManager();
    virtual ~AbstractStateManager() { }
    
    void pushScreen(Screen* screen);

    void replaceTopMostScreen(Screen* screen);
    void popMenu();
    void resetAndGoToScreen(Screen* screen);
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

};

}
#endif
