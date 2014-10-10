//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 SuperTuxKart-Team
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

#ifndef HEADER_ABSTRACT_STATE_MANAGER_HPP
#define HEADER_ABSTRACT_STATE_MANAGER_HPP

#include <vector>
#include <string>
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "utils/leak_check.hpp"

/**
  * \ingroup guiengine
  */
namespace GUIEngine
{
    class Widget;
    class Screen;

    /**
      * \ingroup guiengine
      */
    enum GameState
    {
        MENU,
        GAME,
        INGAME_MENU,
        /** Dummy GameState e. g. for parameters. */
        CURRENT = MENU | GAME | INGAME_MENU
    };   // GameState

    /**
     * \brief Abstract base class you must override from to use the GUI engine
     * \ingroup guiengine
     */
    class AbstractStateManager
    {
    protected:
        /**
         * Whether we are in game mode
         */
        GameState m_game_mode;

        /**
         *  This stack will contain menu names (e.g. main.stkgui),
         *  and/or 'race'.
         */
        std::vector<std::string> m_menu_stack;

        void pushMenu(std::string name);

        void setGameState(GameState state);

    public:

        LEAK_CHECK()

        /** inits an AbstractStateManager is MENU state */
        AbstractStateManager();

        virtual ~AbstractStateManager() { }

        /** \brief adds a menu at the top of the screens stack */
        void pushScreen(Screen* screen);

        /** \brief replaces the menu at the top of the screens stack
          * (i.e. pops the topmost screen and adds this one instead, but
          * without displaying the second-topmost menu of the stack
          * in-between)
          */
        void replaceTopMostScreen(Screen* screen, GUIEngine::GameState gameState = GUIEngine::CURRENT);

        /**
          * \brief removes the menu at the top of the screens stack
          * If the stack becomes empty after performing the pop (i.e. if it
          * contained only one item prior to the call), the game is aborted.
          * In other cases, the second-topmost screen is displayed.
          */
        void popMenu();

        /**
          * \brief clears the menu stack and starts afresh with a new stack
          *  containing only the given screen
          */
        void resetAndGoToScreen(Screen* screen);

        /**
         * \brief Sets the whole menu stack.
         * Only the topmost screen will be inited/shown, but others remain
         * under for cases where the user wants to go back.
         * \param screens an array containing the menus that should go into
         *  stack. The first item will be the bottom item in the stack, the
         *  last item will be the stack top. Array must be NULL-terminated.
         */
        void resetAndSetStack(Screen* screens[]);

        /**
          * \brief call to make the state manager enter game mode.
          * Causes the menu stack to be cleared; all widgets shown on screen
          * are removed
          */
        void enterGameState();

        /** \return the current state of the game */
        GameState getGameState();

        /** \brief to be called after e.g. a resolution switch */
        void reshowTopMostMenu();

        template<typename T>
        void hardResetAndGoToScreen()
        {
            if (m_game_mode != GAME) GUIEngine::getCurrentScreen()->tearDown();
            m_menu_stack.clear();

            GUIEngine::clearScreenCache();

            T* instance = T::getInstance();

            m_menu_stack.push_back(instance->getName());
            setGameState(MENU);

            switchToScreen(instance->getName().c_str());
            getCurrentScreen()->init();

            onTopMostScreenChanged();
        }

        /*  ***********************************
         * methods to override in children *
         *********************************** */

        /**
          * \brief callback invoked whenever escape was pressed (or any
          *  similar cancel operation)
          */
        virtual void escapePressed() = 0;

        /**
          * \brief callback invoked when game mode changes (e.g. goes from
          *  "menu" to "in-game")
          */
        virtual void onGameStateChange(GameState new_state) = 0;

        /**
          * \brief callback invoked when the stack is emptied (all menus are
          * popped out). This is essentially a request to close the
          * application (since a game can't run without a state)
          */
        virtual void onStackEmptied() = 0;

        virtual void onTopMostScreenChanged() = 0;

        // --------------------------------------------------------------------
        /** Returns the number of screens on the stack. Is used to decide
         *  if exiting a screen would cause STK to end or not. */
        unsigned int getMenuStackSize() const { return m_menu_stack.size(); }
    };   // Class AbstractStateManager

}   // GUIEngine
#endif
