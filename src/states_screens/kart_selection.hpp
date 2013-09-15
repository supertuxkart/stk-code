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

#ifndef KART_SELECTION_INCLUDED
#define KART_SELECTION_INCLUDED

#include <string>
#include "guiengine/screen.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include <IGUIImage.h>

namespace GUIEngine
{
    class Widget;
    class BubbleWidget;
    enum EventPropagation;
}
namespace Online
{
    class User;
}
class InputDevice;
class PlayerKartWidget;
class KartHoverListener;

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
    PtrVector<PlayerKartWidget, REF> m_kart_widgets;

    friend class GUIEngine::ScreenSingleton<KartSelectionScreen>;
    friend class PlayerKartWidget;

    bool m_multiplayer;

    /** Whether this screen is being visited from overworld or not */
    bool m_from_overworld;

    bool m_go_to_overworld_next;

    bool m_must_delete_on_back; //!< To delete the screen if back is pressed

    KartSelectionScreen(const char* filename);

    /** Stores whether any player confirmed their choice; then, some things
      * are "frozen", for instance the selected kart group tab
      */
    bool m_game_master_confirmed;

    PlayerKartWidget* m_removed_widget;

    /** Message shown in multiplayer mode */
    GUIEngine::BubbleWidget* m_multiplayer_message;

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
    /** updates model of a kart widget, to have the good selection when the user validates */
    void updateKartWidgetModel(uint8_t widget_id,
                const std::string& selection,
                const irr::core::stringw& selectionText);

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
    bool playerJoin(InputDevice* device, bool firstPlayer);

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
    virtual void onUpdate(float dt, irr::video::IVideoDriver*) OVERRIDE;

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
//! PlayerNameSpinner :
/** A small extension to the spinner widget to add features like player ID
 *  management or badging */
class PlayerNameSpinner : public GUIEngine::SpinnerWidget
{
    int m_playerID;
    bool m_incorrect;
    irr::gui::IGUIImage* m_red_mark_widget;
    KartSelectionScreen* m_parent;

    //virtual EventPropagation focused(const int m_playerID) ;

public:
    PlayerNameSpinner(KartSelectionScreen* parent, const int playerID);
    // ------------------------------------------------------------------------
    void setID(const int m_playerID);
    // ------------------------------------------------------------------------
    /** Add a red mark on the spinner to mean "invalid choice" */
    void markAsIncorrect();

    // ------------------------------------------------------------------------
    /** Remove any red mark set with 'markAsIncorrect' */
    void markAsCorrect();
};

/** A widget representing the kart selection for a player (i.e. the player's
 *  number, name, the kart view, the kart's name) */
class PlayerKartWidget : public GUIEngine::Widget,
    public GUIEngine::SpinnerWidget::ISpinnerConfirmListener
{
    /** Whether this player confirmed their selection */
    bool m_ready;

    /** widget coordinates */
    int player_id_x, player_id_y, player_id_w, player_id_h;
    int player_name_x, player_name_y, player_name_w, player_name_h;
    int model_x, model_y, model_w, model_h;
    int kart_name_x, kart_name_y, kart_name_w, kart_name_h;

    /** A reserved ID for this widget if any, -1 otherwise.  (If no ID is
     *  reserved, widget will not be in the regular tabbing order */
    int m_irrlicht_widget_ID;

    /** For animation purposes (see method 'move') */
    int target_x, target_y, target_w, target_h;
    float x_speed, y_speed, w_speed, h_speed;

    /** Object representing this player */
    StateManager::ActivePlayer* m_associatedPlayer; // local info
    int m_playerID;
    Online::Profile* m_associated_user; // network info

    /** Internal name of the spinner; useful to interpret spinner events,
     *  which contain the name of the activated object */
    std::string spinnerID;

#ifdef DEBUG
    long m_magic_number;
#endif

public:

    LEAK_CHECK()

    /** Sub-widgets created by this widget */
    //LabelWidget* m_player_ID_label;
    PlayerNameSpinner* m_player_ident_spinner;
    GUIEngine::ModelViewWidget* m_model_view;
    GUIEngine::LabelWidget* m_kart_name;

    KartSelectionScreen* m_parent_screen;

    irr::gui::IGUIStaticText* m_ready_text;

    //LabelWidget *getPlayerIDLabel() {return m_player_ID_label;}
    core::stringw deviceName;
    std::string m_kartInternalName;

    bool m_not_updated_yet;

    PlayerKartWidget(KartSelectionScreen* parent,
                     StateManager::ActivePlayer* associatedPlayer,
                     Online::Profile* associatedUser,
                     core::recti area, const int playerID,
                     std::string kartGroup,
                     const int irrlichtWidgetID=-1);
    // ------------------------------------------------------------------------

    ~PlayerKartWidget();

    // ------------------------------------------------------------------------
    /** Called when players are renumbered (changes the player ID) */
    void setPlayerID(const int newPlayerID);

    // ------------------------------------------------------------------------
    /** Returns the ID of this player */
    int getPlayerID() const;

    // ------------------------------------------------------------------------
    /** Add the widgets to the current screen */
    virtual void add();

    // ------------------------------------------------------------------------
    /** Get the associated ActivePlayer object*/
    StateManager::ActivePlayer* getAssociatedPlayer();

    // ------------------------------------------------------------------------
    /** Starts a 'move/resize' animation, by simply passing destination coords.
     *  The animation will then occur on each call to 'onUpdate'. */
    void move(const int x, const int y, const int w, const int h);

    // ------------------------------------------------------------------------
    /** Call when player confirmed his identity and kart */
    void markAsReady();

    // ------------------------------------------------------------------------
    /** \return Whether this player confirmed his kart and indent selection */
    bool isReady();

    // -------------------------------------------------------------------------
    /** Updates the animation (moving/shrinking/etc.) */
    void onUpdate(float delta);

    // -------------------------------------------------------------------------
    /** Event callback */
    virtual GUIEngine::EventPropagation transmitEvent(
        GUIEngine::Widget* w,
        const std::string& originator,
        const int m_playerID);

    // -------------------------------------------------------------------------
    /** Sets the size of the widget as a whole, and placed children widgets
     * inside itself */
    void setSize(const int x, const int y, const int w, const int h);

    // -------------------------------------------------------------------------

    /** Sets which kart was selected for this player */
    void setKartInternalName(const std::string& whichKart);

    // -------------------------------------------------------------------------

    const std::string& getKartInternalName() const;

    // -------------------------------------------------------------------------

    /** \brief Event callback from ISpinnerConfirmListener */
    virtual GUIEngine::EventPropagation onSpinnerConfirmed();
};   // PlayerKartWidget

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
