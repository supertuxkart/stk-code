//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 
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

#include <string>
#include "guiengine/screen.hpp"
#include "states_screens/state_manager.hpp"

namespace GUIEngine
{
    class Widget;
    class BubbleWidget;
}
class InputDevice;
class PlayerKartWidget;
class KartHoverListener;

/**
  * \brief screen where players can choose their kart
  * \ingroup states_screens
  */
class KartSelectionScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<KartSelectionScreen>
{
    friend class KartHoverListener;
    friend class PlayerNameSpinner;
    friend class FocusDispatcher;
    
    /** Contains the custom widget shown for every player. (ref only since we're adding them to a
      * Screen, and the Screen will take ownership of these widgets)
      */
    ptr_vector<PlayerKartWidget, REF> m_kart_widgets;
    
    friend class GUIEngine::ScreenSingleton<KartSelectionScreen>;
    friend class PlayerKartWidget;
    
    bool m_multiplayer;
    
    KartSelectionScreen();
    
    /** Stores whether any player confirmed their choice; then, some things are "frozen", for instance
      * the selected kart group tab
      */
    bool m_player_confirmed;
    
    PlayerKartWidget* m_removed_widget;
    
    /** Message shown in multiplayer mode */
    GUIEngine::BubbleWidget* m_multiplayer_message;
    
    /** Called when all players selected their kart */
    void allPlayersDone();
    
    /** Called when number/order of karts changed, so that all will keep an up-to-date ID */
    void renumberKarts();

    /** Checks identities chosen by players, making sure no duplicates are used.
      * \return Whether all choices are ok
      */
    bool validateIdentChoices();
    
    /** Checks karts chosen by players, making sure no duplicates are used.
      * \return Whether all choices are ok
      */
    bool validateKartChoices();
    
    /** Fill the ribbon with the karts from the currently selected group */
    void setKartsFromCurrentGroup();
    
    void playerConfirm(const int playerID);
    
public:
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();
    
    void setMultiplayer(bool multiplayer);
    
    /** \brief Called when a player hits 'fire'/'select' on his device to join the game */
    bool playerJoin(InputDevice* device, bool firstPlayer);
    
    /**
      * \brief Called when a player hits 'rescue'/'cancel' on his device to leave the game
      * \return true if event was handled succesfully
      */
    bool playerQuit(StateManager::ActivePlayer* player);
    
     /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();
    
     /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown();
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt, irr::video::IVideoDriver*);

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded();
};
