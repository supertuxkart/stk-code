//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#ifndef KART_SELECTION_INCLUDED
#define KART_SELECTION_INCLUDED

#include "guiengine/screen.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/player_kart_widget.hpp"
#include "states_screens/state_manager.hpp"

namespace GUIEngine
{
    class Widget;
    class BubbleWidget;
    enum EventPropagation;
}
namespace Online
{
    class User;
    class OnlineProfile;
}

class FocusDispatcher;
class InputDevice;
class KartHoverListener;

extern int g_root_id;

/**
  * \brief screen where players can choose their kart
  * \ingroup states_screens
  */
class KartSelectionScreen : public GUIEngine::Screen
{
    friend class KartHoverListener;
    friend class PlayerNameSpinner;
    friend class FocusDispatcher;
protected:
    /** Contains the custom widget shown for every player. (ref only since
     *  we're adding them to a Screen, and the Screen will take ownership
     *  of these widgets)
     */
    PtrVector<GUIEngine::PlayerKartWidget, REF> m_kart_widgets;

    friend class GUIEngine::ScreenSingleton<KartSelectionScreen>;
    friend class GUIEngine::PlayerKartWidget;

    bool m_multiplayer;

    /** Whether this screen is being visited from overworld or not */
    bool m_from_overworld;

    bool m_go_to_overworld_next;

    bool m_must_delete_on_back; //!< To delete the screen if back is pressed

    /** Stores whether any player confirmed their choice; then, some things
      * are "frozen", for instance the selected kart group tab
      */
    bool m_game_master_confirmed;

    GUIEngine::PlayerKartWidget* m_removed_widget;

    /** Message shown in multiplayer mode */
    GUIEngine::BubbleWidget* m_multiplayer_message;

    FocusDispatcher  *m_dispatcher;

    KartSelectionScreen(const char* filename);

    /** Called when all players selected their kart */
    void allPlayersDone();

    /** Called when number/order of karts changed, so that all will keep
     *  an up-to-date ID */
    void renumberKarts();

    /** Checks identities chosen by players, making sure no duplicates are
      * used.
      * \return Whether all choices are ok
      */
    bool validateIdentChoices();

    /** Checks karts chosen by players, making sure no duplicates are used.
      * \return Whether all choices are ok
      */
    bool validateKartChoices();

    /** Fill the ribbon with the karts from the currently selected group */
    void setKartsFromCurrentGroup();

    virtual void playerConfirm(const int playerID);

    void updateKartStats(uint8_t widget_id,
                         const std::string& selection);

    /** updates model of a kart widget, to have the good selection when the
     *  user validates */
    void updateKartWidgetModel(int widget_id,
                const std::string& selection,
                const irr::core::stringw& selectionText);

    /** Adds a message to the screen which indicates that players must press fire to join. */
    void addMultiplayerMessage();

    /** Remove the multiplayer message. */
    void removeMultiplayerMessage();

    /** Stores a pointer to the current selection screen */
    static KartSelectionScreen* m_instance_ptr;
public:
    /** Returns the current instance */
    static KartSelectionScreen* getRunningInstance();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    void setMultiplayer(bool multiplayer);

    /** \brief Set whether this screen is being visited from overworld or not */
    void setFromOverworld(bool from_overworld) { m_from_overworld = from_overworld; }

    void setGoToOverworldNext() { m_go_to_overworld_next = true; }

    /** \brief Called when a player hits 'fire'/'select' on his device to
     *  join the game */
    bool joinPlayer(InputDevice* device);

    /**
      * \brief Called when a player hits 'rescue'/'cancel' on his device
      *  to leave the game
      * \return true if event was handled succesfully
      */
    bool playerQuit(StateManager::ActivePlayer* player);

     /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    virtual void beforeAddingWidget() OVERRIDE;

     /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;

    /** \brief implement optional callback from parent
     *  class GUIEngine::Screen */
    virtual void unloaded() OVERRIDE;

    /** \brief implement optional callback from parent
     *  class GUIEngine::Screen */
    virtual bool onEscapePressed() OVERRIDE;

};   // KartSelectionScreen

//!----------------------------------------------------------------------------
//! FocusDispatcher :
/** Currently, navigation for multiple players at the same time is implemented
    in a somewhat clunky way. An invisible "dispatcher" widget is added above
    kart icons. When a player moves up, he focuses the dispatcher, which in
    turn moves the selection to the appropriate spinner. "tabbing roots" are
    used to make navigation back down possible. (FIXME: maybe find a cleaner
    way?) */
class FocusDispatcher : public GUIEngine::Widget
{
protected:
    KartSelectionScreen* m_parent;
    int m_reserved_id;

    bool m_is_initialised;

public:

    LEAK_CHECK()

    // ------------------------------------------------------------------------
    FocusDispatcher(KartSelectionScreen* parent);
    // ------------------------------------------------------------------------
    void setRootID(const int reservedID);

    // ------------------------------------------------------------------------
    virtual void add();

    // ------------------------------------------------------------------------

    virtual GUIEngine::EventPropagation focused(const int playerID);
};   // FocusDispatcher

//!----------------------------------------------------------------------------
//! KartHoverListener :
class KartHoverListener : public GUIEngine::DynamicRibbonHoverListener
{
    KartSelectionScreen* m_parent;
public:
    unsigned int m_magic_number;

    KartHoverListener(KartSelectionScreen* parent);

    // ------------------------------------------------------------------------
    virtual ~KartHoverListener();

    // ------------------------------------------------------------------------
    void onSelectionChanged(GUIEngine::DynamicRibbonWidget* theWidget,
                            const std::string& selectionID,
                            const irr::core::stringw& selectionText,
                            const int playerID);
};   // KartHoverListener

#endif

