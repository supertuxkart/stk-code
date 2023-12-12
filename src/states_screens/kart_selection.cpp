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

#include "states_screens/kart_selection.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include <ge_render_info.hpp>
#include "guiengine/message_queue.hpp"
#include "guiengine/widgets/bubble_widget.hpp"
#include "guiengine/widgets/kart_stats_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/player_name_spinner.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_characteristic.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/overworld.hpp"
#include "network/network_config.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#endif

using namespace GUIEngine;
using irr::core::stringw;


static const char RANDOM_KART_ID[] = "randomkart";
static const char ID_DONT_USE[] = "x";
// Use '/' as special character to avoid that someone creates
// a kart called 'locked'
static const char ID_LOCKED[] = "locked/";

KartSelectionScreen* KartSelectionScreen::m_instance_ptr = NULL;

int g_root_id;

/** Currently, navigation for multiple players at the same time is implemented
    in a somewhat clunky way. An invisible "dispatcher" widget is added above
    kart icons. When a player moves up, he focuses the dispatcher, which in
    turn moves the selection to the appropriate spinner. "tabbing roots" are
    used to make navigation back down possible. (FIXME: maybe find a cleaner
    way?) */

// ------------------------------------------------------------------------
FocusDispatcher::FocusDispatcher(KartSelectionScreen* parent) : Widget(WTYPE_BUTTON)
{
    m_parent = parent;
    m_supports_multiplayer = true;
    m_is_initialised = false;
    
    Widget* kartsAreaWidget = parent->getWidget("playerskarts");
    assert(kartsAreaWidget);

    m_x = 0;
    m_y = kartsAreaWidget->m_y;
    m_w = 1;
    m_h = 1;

    m_reserved_id = Widget::getNewNoFocusID();
}   // FocusDispatcher
// ------------------------------------------------------------------------
void FocusDispatcher::setRootID(const int reservedID)
{
    assert(reservedID != -1);

    m_reserved_id = reservedID;

    if (m_element != NULL)
    {
        m_element->setID(m_reserved_id);
    }

    m_is_initialised = true;
}   // setRootID

// ------------------------------------------------------------------------
void FocusDispatcher::add()
{
    core::rect<s32> widget_size(m_x, m_y, m_x + m_w, m_y + m_h);

    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, NULL,
                m_reserved_id,
                L"Dispatcher", L"");

    m_id = m_element->getID();
    m_element->setTabStop(true);
    m_element->setTabGroup(false);
    m_element->setTabOrder(m_id);
}

EventPropagation FocusDispatcher::focused(const int player_id)
{
    if (!m_is_initialised) return EVENT_LET;

    if(UserConfigParams::logGUI())
        Log::info("[KartSelectionScreen]", "FocusDispatcher focused by player %u",
                  player_id);

    // since this screen is multiplayer, redirect focus to the right widget
    const int amount = m_parent->m_kart_widgets.size();
    for (int n=0; n<amount; n++)
    {
        if (m_parent->m_kart_widgets[n].getPlayerID() == player_id)
        {
            // If player is done, don't do anything with focus
            if (m_parent->m_kart_widgets[n].isReady())
                return GUIEngine::EVENT_BLOCK;

            //std::cout << "--> Redirecting focus for player " << player_id
            //          << " from FocusDispatcher "  <<
            //             " (ID " << m_element->getID() <<
            //             ") to spinner " << n << " (ID " <<
            //             m_parent->m_kart_widgets[n].m_player_ident_spinner
            //             ->getIrrlichtElement()->getID() <<
            //             ")" << std::endl;

            m_parent->m_kart_widgets[n].m_player_ident_spinner
            ->setFocusForPlayer(player_id);


            return GUIEngine::EVENT_BLOCK;
        }
    }

    //Log::fatal("KartSelectionScreen", "The focus dispatcher can't"
    //    "find the widget for player %d!", player_id);
    return GUIEngine::EVENT_LET;
}   // focused

#if 0
#pragma mark -
#pragma mark KartHoverListener
#endif
// ============================================================================

KartHoverListener::KartHoverListener(KartSelectionScreen* parent)
{
    m_magic_number = 0xCAFEC001;
    m_parent = parent;
}   // KartHoverListener

// ------------------------------------------------------------------------
KartHoverListener::~KartHoverListener()
{
    assert(m_magic_number == 0xCAFEC001);
    m_magic_number = 0xDEADBEEF;
}   // ~KartHoverListener

// ------------------------------------------------------------------------
void KartHoverListener::onSelectionChanged(DynamicRibbonWidget* theWidget,
        const std::string& selectionID,
        const irr::core::stringw& selectionText,
        const int player_id)
{
    assert(m_magic_number == 0xCAFEC001);

    // Check if this player has a kart
    if (m_parent->m_kart_widgets.size() <= unsigned(player_id))
    {
        GUIEngine::focusNothingForPlayer(player_id);
        return;
    }

    // Don't allow changing the selection after confirming it
    if (m_parent->m_kart_widgets[player_id].isReady())
    {
        // discard events sent when putting back to the right kart
        if (selectionID ==
                m_parent->m_kart_widgets[player_id].m_kart_internal_name) return;

        DynamicRibbonWidget* w =
            m_parent->getWidget<DynamicRibbonWidget>("karts");
        assert(w != NULL);

        w->setSelection(m_parent->m_kart_widgets[player_id]
                        .m_kart_internal_name, player_id, true);
        return;
    }

    if (m_parent->m_kart_widgets[player_id].getKartInternalName() == selectionID)
        return; // already selected

    m_parent->updateKartWidgetModel(player_id, selectionID, selectionText,
        m_parent->m_kart_widgets[player_id].getAssociatedPlayer()->getProfile()
        ->getDefaultKartColor());
    m_parent->m_kart_widgets[player_id].setKartInternalName(selectionID);
    m_parent->updateKartStats(player_id, selectionID);
    m_parent->validateKartChoices();
}   // onSelectionChanged

#if 0
#pragma mark -
#pragma mark KartSelectionScreen
#endif

// ============================================================================

/** Small utility function that returns whether the two given players chose
 *  the same kart. The advantage of this function is that it can handle
 *  "random kart" selection. */
bool sameKart(const PlayerKartWidget& player1, const PlayerKartWidget& player2)
{
    return player1.getKartInternalName() == player2.getKartInternalName() &&
           player1.getKartInternalName() != RANDOM_KART_ID;
}

// ============================================================================

KartSelectionScreen::KartSelectionScreen(const char* filename) : Screen(filename)
{
    m_dispatcher           = NULL;
    m_removed_widget       = NULL;
    m_multiplayer_message  = NULL;
    m_from_overworld       = false;
    m_go_to_overworld_next = false;
}   // KartSelectionScreen

// ============================================================================

KartSelectionScreen* KartSelectionScreen::getRunningInstance()
{
    return m_instance_ptr;
}

// ----------------------------------------------------------------------------

void KartSelectionScreen::loadedFromFile()
{
    m_dispatcher          = new FocusDispatcher(this);
    m_first_widget        = m_dispatcher;
    m_game_master_confirmed    = false;
    m_multiplayer_message = NULL;
    // Dynamically add tabs
    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );

    m_last_widget = tabs;
}   // loadedFromFile

// ----------------------------------------------------------------------------

void KartSelectionScreen::beforeAddingWidget()
{
    if (useContinueButton())
    {
        getWidget("kartlist")->m_properties[GUIEngine::PROP_WIDTH] = "85%";
        getWidget("continue")->setVisible(true);
    }
    else
    {
        getWidget("kartlist")->m_properties[GUIEngine::PROP_WIDTH] = "100%";
        getWidget("continue")->setVisible(false);
    }
    // Remove dispatcher from m_widgets before calculateLayout otherwise a
    // dummy button is shown in kart screen
    bool removed_dispatcher = false;
    if (m_widgets.contains(m_dispatcher))
    {
        m_widgets.remove(m_dispatcher);
        removed_dispatcher = true;
    }
    calculateLayout();
    if (removed_dispatcher)
        m_widgets.push_back(m_dispatcher);

    // Dynamically add tabs
    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );

    m_last_widget = tabs;
    tabs->clearAllChildren();

    const std::vector<std::string>& groups =
        kart_properties_manager->getAllGroups();
    const int group_amount = (int)groups.size();

    // Add "All" group first
    if (group_amount > 1)
    {
        //I18N: name of the tab that will show karts from all groups
        tabs->addTextChild( _("All") , ALL_KART_GROUPS_ID);
    }

    // Make group names being picked up by gettext
#define FOR_GETTEXT_ONLY(x)
    //I18N: kart group name
    FOR_GETTEXT_ONLY( _("All") )
    //I18N: kart group name
    FOR_GETTEXT_ONLY( _("Standard") )
    //I18N: kart group name
    FOR_GETTEXT_ONLY( _("Add-Ons") )


    // Add other groups after
    for (int n=0; n<group_amount; n++)
    {
        if (groups[n] == "standard") // Fix capitalization (#4622)
            tabs->addTextChild( _("Standard") , groups[n]);
        else // Try to translate group names
            tabs->addTextChild( _(groups[n].c_str()) , groups[n]);
    } // for n<group_amount


    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );

    // Avoid too many items shown at the same time
    w->setItemCountHint(std::min((int)kart_properties_manager->getNumberOfKarts(), 20));
}   // beforeAddingWidget

// ----------------------------------------------------------------------------

void KartSelectionScreen::init()
{
#ifndef SERVER_ONLY
    GE::getGEConfig()->m_enable_draw_call_cache = true;
#endif
    m_instance_ptr = this;
    Screen::init();
    m_must_delete_on_back = false;

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );
    tabs->select(UserConfigParams::m_last_used_kart_group,
                 PLAYER_ID_GAME_MASTER);

    Widget* placeholder = getWidget("playerskarts");
    assert(placeholder != NULL);

    m_dispatcher->setRootID(placeholder->m_reserved_id);

    g_root_id = placeholder->m_reserved_id;
    if (!m_widgets.contains(m_dispatcher))
    {
        m_widgets.push_back(m_dispatcher);

        // this is only needed if the dispatcher wasn't already in
        // the list of widgets. If it already was, it was added along
        // other widgets.
        m_dispatcher->add();
    }

    m_game_master_confirmed = false;

    tabs->setActive(true);

    m_kart_widgets.clearAndDeleteAll();

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );
    // Only allow keyboard and gamepad to choose kart without continue button in
    // multitouch GUI, so mouse (touch) clicking can be used as previewing karts
    w->setEventCallbackActive(Input::IT_MOUSEBUTTON, !useContinueButton());

    KartHoverListener* karthoverListener = new KartHoverListener(this);
    w->registerHoverListener(karthoverListener);


    // Build kart list (it is built everytime, to account for .g. locking)
    setKartsFromCurrentGroup();

    /*

     TODO: Ultimately, it'd be nice to *not* clear m_kart_widgets so that
     when players return to the kart selection screen, it will appear as
     it did when they left (at least when returning from the track menu).
     Rebuilding the screen is a little tricky.

     */

    /*
    if (m_kart_widgets.size() > 0)
    {
        // trying to rebuild the screen
        for (int n = 0; n < m_kart_widgets.size(); n++)
        {
            PlayerKartWidget *pkw;
            pkw = m_kart_widgets.get(n);
            manualAddWidget(pkw);
            pkw->add();
        }

    }
    else */
    // For now this is what will happen
    input_manager->getDeviceManager()->setAssignMode(DETECT_NEW);
    // This flag will cause that a 'fire' event will be mapped to 'select' (if
    // 'fire' is not assigned to a GUI event). This is done to support the old
    // way of player joining by pressing 'fire' instead of 'select'.
    input_manager->getDeviceManager()->mapFireToSelect(true);

    if (!NetworkConfig::get()->isNetworking())
    {
        StateManager::get()->resetActivePlayers();
        if (!m_multiplayer)
        {
            joinPlayer(input_manager->getDeviceManager()->getLatestUsedDevice(),
                NULL/*player profile*/);
            w->updateItemDisplay();

            // Player 0 select default kart
            if (!w->setSelection(UserConfigParams::m_default_kart, 0, true))
            {
                // if kart from config not found, select the first instead
                w->setSelection(0, 0, true);
            }
        }
        else
        {
            // Add multiplayer message
            addMultiplayerMessage();
#ifdef MOBILE_STK
            MessageQueue::addStatic(MessageQueue::MT_GENERIC,
                _("Connect a keyboard or gamepad to play splitscreen multiplayer"));
#endif
        }
    }
}   // init

// ----------------------------------------------------------------------------

void KartSelectionScreen::tearDown()
{
#ifndef SERVER_ONLY
    GE::getGEConfig()->m_enable_draw_call_cache = false;
    GE::GEVulkanDriver* gevk = GE::getVKDriver();
    if (gevk)
        gevk->clearDrawCallsCache();
#endif
#ifdef MOBILE_STK
    if (m_multiplayer)
        MessageQueue::discardStatic();
#endif
    // Reset the 'map fire to select' option of the device manager
    input_manager->getDeviceManager()->mapFireToSelect(false);

    // if a removed widget is currently shrinking down, remove it upon leaving
    // the screen
    if (m_removed_widget != NULL)
    {
        manualRemoveWidget(m_removed_widget);
        delete m_removed_widget;
        m_removed_widget = NULL;
    }

    removeMultiplayerMessage();

    Screen::tearDown();
    m_kart_widgets.clearAndDeleteAll();

    if (m_must_delete_on_back)
        GUIEngine::removeScreen(this);

}   // tearDown

// ----------------------------------------------------------------------------

void KartSelectionScreen::unloaded()
{
    // This pointer is no longer valid (has been deleted along other widgets)
    m_dispatcher = NULL;
}

// ----------------------------------------------------------------------------
// Return true if event was handled successfully
bool KartSelectionScreen::joinPlayer(InputDevice* device, PlayerProfile* p)
{
#ifdef MOBILE_STK
    if (m_multiplayer)
        MessageQueue::discardStatic();
#endif
    bool first_player = m_kart_widgets.size() == 0;

    if (UserConfigParams::logGUI())
        Log::info("KartSelectionScreen",  "joinPlayer() invoked");
    if (!m_multiplayer && !first_player) return false;

    assert (m_dispatcher != NULL);

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    if (w == NULL)
    {
        Log::error("KartSelectionScreen", "joinPlayer(): Called outside of "
                  "kart selection screen.");
        return false;
    }
    else if (device == NULL)
    {
        if (!NetworkConfig::get()->isNetworkAIInstance())
        {
            Log::error("KartSelectionScreen", "joinPlayer(): Received null "
                    "device pointer");
        }
        return false;
    }

    if (StateManager::get()->activePlayerCount() >= MAX_PLAYER_COUNT)
    {
        Log::error("KartSelectionScreen", "Maximum number of players "
                  "reached");
        SFXManager::get()->quickSound( "anvil" );
        return false;
    }

    // ---- Create new active player
    PlayerProfile* profile_to_use = p == NULL ?
        PlayerManager::getCurrentPlayer() : p;

    // Make sure enough guest character exists. At this stage this player has
    // not been added, so the number of guests requested for the first player
    // is 0 --> forcing at least one real player.
    if (p == NULL)
    {
        PlayerManager::get()->createGuestPlayers(
            StateManager::get()->activePlayerCount());
    }
    if (!first_player && p == NULL)
    {
        // Give each player a different start profile
        const int num_active_players = StateManager::get()->activePlayerCount();
        profile_to_use = PlayerManager::get()->getPlayer(num_active_players);

        removeMultiplayerMessage();
    }

    const int new_player_id =
        StateManager::get()->createActivePlayer(profile_to_use, device);
    StateManager::ActivePlayer* aplayer =
        StateManager::get()->getActivePlayer(new_player_id);

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert(tabs != NULL);

    std::string selected_kart_group =
        tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    // ---- Get available area for karts
    // make a copy of the area, ands move it to be outside the screen
    Widget* kartsAreaWidget = getWidget("playerskarts");
    // start at the rightmost of the screen
    const int shift = irr_driver->getFrameSize().Width;
    core::recti kartsArea(kartsAreaWidget->m_x + shift,
                          kartsAreaWidget->m_y,
                          kartsAreaWidget->m_x + shift + kartsAreaWidget->m_w,
                          kartsAreaWidget->m_y + kartsAreaWidget->m_h);

    // ---- Create player/kart widget
    PlayerKartWidget* newPlayerWidget =
        new PlayerKartWidget(this, aplayer, kartsArea, m_kart_widgets.size(),
                             selected_kart_group);

    manualAddWidget(newPlayerWidget);
    m_kart_widgets.push_back(newPlayerWidget);

    newPlayerWidget->add();
    // From network kart selection, the player name is already defined
    if (p != NULL)
    {
        newPlayerWidget->getPlayerNameSpinner()->setActive(false);
        newPlayerWidget->getPlayerNameSpinner()->setCustomText(p->getName());
    }

    // ---- Divide screen space among all karts
    const int amount = m_kart_widgets.size();
    Widget* fullarea = getWidget("playerskarts");

    // in this special case, leave room for a message on the right
    if (m_multiplayer && first_player)
    {
        if (p == NULL)
            addMultiplayerMessage();
        const int splitWidth = fullarea->m_w / 2;
        m_kart_widgets[0].move( fullarea->m_x, fullarea->m_y, splitWidth,
                                fullarea->m_h );
    }
    else
    {
        const int splitWidth = fullarea->m_w / amount;

        for (int n=0; n<amount; n++)
        {
            m_kart_widgets[n].move( fullarea->m_x + splitWidth * n,
                                    fullarea->m_y, splitWidth, fullarea->m_h);
        }
    }

    // select something (anything) in the ribbon; by default, only the
    // game master has something selected. Thus, when a new player joins,
    // we need to select something for them
    w->setSelection(new_player_id, new_player_id, true);

    //newPlayerWidget->m_player_ident_spinner
    //               ->setFocusForPlayer(new_player_id);

    if (!m_multiplayer)
    {
        input_manager->getDeviceManager()->setSinglePlayer(StateManager::get()
                                                         ->getActivePlayer(0));
    }

    return true;
}   // joinPlayer

// -----------------------------------------------------------------------------

bool KartSelectionScreen::playerQuit(StateManager::ActivePlayer* player)
{
    int player_id = -1;

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    if (w == NULL)
    {
        Log::error("KartSelectionScreen", "playerQuit() called "
                  "outside of kart selection screen, "
                  "or the XML file for this screen was changed without "
                  "adapting the code accordingly");
        return false;
    }

    // If last player quits, return to main menu
    if (m_kart_widgets.size() <= 1)
    {
        StateManager::get()->escapePressed();
        return true;
    }

    std::map<PlayerKartWidget*, std::string> selections;

    // Find the player ID associated to this player
    for (unsigned int n=0; n<m_kart_widgets.size(); n++)
    {
        if (m_kart_widgets[n].getAssociatedPlayer() == player)
        {
            // Check that this player has not already confirmed,
            // then they can't back out
            if (m_kart_widgets[n].isReady())
            {
                SFXManager::get()->quickSound( "anvil" );
                return true;
            }

            player_id = n;
        }
        else
        {
            selections[m_kart_widgets.get(n)] =
                m_kart_widgets[n].getKartInternalName();
        }
    }
    if (player_id == -1)
    {
        Log::warn("KartSelectionScreen", "playerQuit cannot find "
                  "passed player");
        return false;
    }
    if(UserConfigParams::logGUI())
        Log::info("KartSelectionScreen", "playerQuit(%d)", player_id);

    // Just a cheap way to check if there is any discrepancy
    // between m_kart_widgets and the active player array
    assert( m_kart_widgets.size() == StateManager::get()->activePlayerCount());

    // unset selection of this player
    GUIEngine::focusNothingForPlayer(player_id);

    // delete a previous removed widget that didn't have time to fully shrink
    // yet.
    // TODO: handle multiple shrinking widgets gracefully?
    if (m_removed_widget != NULL)
    {
        manualRemoveWidget(m_removed_widget);
        delete m_removed_widget;
        m_removed_widget = NULL;
    }

    // keep the removed kart a while, for the 'disappear' animation
    // to take place
    m_removed_widget = m_kart_widgets.remove(player_id);

    // Tell the StateManager to remove this player
    StateManager::get()->removeActivePlayer(player_id);

    addMultiplayerMessage();

    // Karts count changed, maybe order too, so renumber them.
    renumberKarts();

    // Tell the removed widget to perform the shrinking animation (which will
    // be updated in onUpdate, and will stop when the widget has disappeared)
    Widget* fullarea = getWidget("playerskarts");
    m_removed_widget->move(m_removed_widget->m_x + m_removed_widget->m_w/2,
                           fullarea->m_y + fullarea->m_h, 0, 0);

    // update selections

    const unsigned int amount = m_kart_widgets.size();
    for (unsigned int n=0; n<amount; n++)
    {
        const std::string& selectedKart = selections[m_kart_widgets.get(n)];
        if (selectedKart.size() > 0)
        {
            //std::cout << m_kart_widgets[n].getAssociatedPlayer()
            //              ->getProfile()->getName() << " selected "
            //          << selectedKart.c_str() << "\n";
            const bool success = w->setSelection(selectedKart, n, true);
            if (!success)
            {
                Log::warn("KartSelectionScreen", "Failed to select kart %s"
                          " for player %u, what's going on??", selectedKart.c_str(),n);
            }
        }
    }


    // check if all players are ready
    bool allPlayersReady = true;
    for (unsigned int n=0; n<amount; n++)
    {
        if (!m_kart_widgets[n].isReady())
        {
            allPlayersReady = false;
            break;
        }
    }
    if (allPlayersReady && (!m_multiplayer || amount > 1)) allPlayersDone();

    return true;
}   // playerQuit

// ----------------------------------------------------------------------------

void KartSelectionScreen::onUpdate(float delta)
{
    // Dispatch the onUpdate event to each kart, so they can perform their
    // animation if any
    const int amount = m_kart_widgets.size();
    for (int n=0; n<amount; n++)
    {
        m_kart_widgets[n].onUpdate(delta);
    }

    // When a kart widget is removed, it's a kept a while, for the disappear
    // animation to take place
    if (m_removed_widget != NULL)
    {
        m_removed_widget->onUpdate(delta);

        if (m_removed_widget->m_w == 0 || m_removed_widget->m_h == 0)
        {
            // destruct when too small (for "disappear" effects)
            manualRemoveWidget(m_removed_widget);
            delete m_removed_widget;
            m_removed_widget = NULL;
        }
    }
}   // onUpdate

// ----------------------------------------------------------------------------

void KartSelectionScreen::playerConfirm(const int player_id)
{
    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert(w != NULL);
    const std::string selection = w->getSelectionIDString(player_id);
    if (StringUtils::startsWith(selection, ID_LOCKED) && !m_multiplayer)
    {
        unlock_manager->playLockSound();
        return;
    }

    if (m_kart_widgets[player_id].getKartInternalName().size() == 0 ||
        m_kart_widgets[player_id].getKartInternalName() == RibbonWidget::NO_ITEM_ID)
    {
        SFXManager::get()->quickSound( "anvil" );
        return;
    }

    const int amount = m_kart_widgets.size();

    // Check if we have enough karts for everybody. If there are more players
    // than karts then just allow duplicates
    const int available_kart_count = (int) w->getItems().size();
    const bool will_need_duplicates = (amount > available_kart_count);

    // make sure no other player selected the same identity or kart
    for (int n=0; n<amount; n++)
    {
        if (n == player_id) continue; // don't check a kart against itself

        const bool player_ready   = m_kart_widgets[n].isReady();
        const bool ident_conflict =
            !m_kart_widgets[n].getAssociatedPlayer()->getProfile()
            ->isGuestAccount() &&
            m_kart_widgets[n].getAssociatedPlayer()->getProfile() ==
            m_kart_widgets[player_id].getAssociatedPlayer()->getProfile();
        const bool kart_conflict  = sameKart(m_kart_widgets[n],
                                             m_kart_widgets[player_id]);

        if (player_ready && (ident_conflict || kart_conflict) &&
                !will_need_duplicates)
        {
            if (UserConfigParams::logGUI())
                Log::warn("KartSelectionScreen", "You can't select this identity "
                       "or kart, someone already took it!!");

            SFXManager::get()->quickSound( "anvil" );
            return;
        }

        // If two PlayerKart entries are associated to the same ActivePlayer,
        // something went wrong
        assert(m_kart_widgets[n].getAssociatedPlayer() !=
               m_kart_widgets[player_id].getAssociatedPlayer());
    }

    // Mark this player as ready to start
    m_kart_widgets[player_id].markAsReady();

    if (player_id == PLAYER_ID_GAME_MASTER)
    {
        m_game_master_confirmed = true;
        RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
        assert( tabs != NULL );
        tabs->setActive(false);
    }

    // validate choices to notify player of duplicates
    const bool names_ok = validateIdentChoices();
    const bool karts_ok = validateKartChoices();

    if (!names_ok || !karts_ok) return;

    // check if all players are ready
    bool allPlayersReady = true;
    for (int n=0; n<amount; n++)
    {
        if (!m_kart_widgets[n].isReady())
        {
            allPlayersReady = false;
            break;
        }
    }

    if (allPlayersReady && (!m_multiplayer || amount > 1)) allPlayersDone();
}   // playerConfirm

// ----------------------------------------------------------------------------

void KartSelectionScreen::updateKartStats(uint8_t widget_id,
                                          const std::string& selection)
{
    KartStatsWidget* w = m_kart_widgets[widget_id].m_kart_stats;
    assert(w != NULL);

    const KartProperties *kp =
                    kart_properties_manager->getKart(selection);
    NetworkConfig* nc = NetworkConfig::get();
    // Adjust for online addon karts
    if (kp && kp->isAddon() && nc->isNetworking() && nc->useTuxHitboxAddon() &&
        nc->getServerCapabilities().find(
        "real_addon_karts") == nc->getServerCapabilities().end())
        kp = kart_properties_manager->getKart("tux");
    if (kp != NULL)
    {
        w->setValues(kp, m_kart_widgets[widget_id].getHandicap());
        w->update(0);
    }
    else
    {
        w->hideAll();
        w->update(0);
    }
}

// ----------------------------------------------------------------------------
void KartSelectionScreen::updateKartWidgetModel(int widget_id,
                const std::string& selection,
                const irr::core::stringw& selectionText, float kart_color)
{
    // Update the displayed model
    ModelViewWidget* w3 = m_kart_widgets[widget_id].m_model_view;
    assert( w3 != NULL );

    // set the color of the name label
    m_kart_widgets[widget_id].m_kart_name->setColor(GUIEngine::getSkin()->getColor("text::neutral"));

    if (selection == RANDOM_KART_ID)
    {
        // Random kart
        scene::IMesh* model =
            ItemManager::getItemModel(Item::ITEM_BONUS_BOX);

        w3->clearModels();
        core::matrix4 model_location;
        model_location.setTranslation(core::vector3df(0.0f, -12.0f, 0.0f));
        model_location.setScale(core::vector3df(35.0f, 35.0f, 35.0f));
        w3->addModel(model, model_location);
        w3->update(0);
        m_kart_widgets[widget_id].m_kart_name
            ->setText( _("Random Kart"), false );
    }
    // selection contains the name of the kart, so check only for substr
    else if (StringUtils::startsWith(selection, ID_LOCKED) && !m_multiplayer)
    {
        w3->clearModels();
        core::matrix4 model_location;
        model_location.setScale(core::vector3df(15.0f, 15.0f, 15.0f));
        file_manager->pushTextureSearchPath
            (file_manager->getAsset(FileManager::MODEL,""), "models");
        w3->addModel(irr_driver->getAnimatedMesh(
            file_manager->getAsset(FileManager::MODEL, "chest.spm"))
            ->getMesh(20), model_location);
        file_manager->popTextureSearchPath();
        w3->update(0);

        if (m_multiplayer)
        {
            m_kart_widgets[widget_id].m_kart_name
            ->setText(_("Locked"), false );
        }
        else
        {
            m_kart_widgets[widget_id].m_kart_name
            ->setText(_("Locked : solve active challenges to gain access to more!"), false );
        }
    }
    else
    {
        const KartProperties *kp =
            kart_properties_manager->getKart(selection);
        if (kp != NULL)
        {
            const KartModel &kart_model = kp->getMasterKartModel();

            float scale = 35.0f;
            if (kart_model.getLength() > 1.45f)
            {
                // if kart is too long, size it down a bit so that it fits
                scale = 30.0f;
            }

            core::matrix4 model_location;
            model_location.setScale(core::vector3df(scale, scale, scale));
            w3->clearModels();
            const bool has_win_anime =
                UserConfigParams::m_animated_characters &&
                (((kart_model.getFrame(KartModel::AF_WIN_LOOP_START) > -1 ||
                kart_model.getFrame(KartModel::AF_WIN_START) > -1) &&
                kart_model.getFrame(KartModel::AF_WIN_END) > -1) ||
                (kart_model.getFrame(KartModel::AF_SELECTION_START) > -1 &&
                kart_model.getFrame(KartModel::AF_SELECTION_END) > -1));
            w3->addModel( kart_model.getModel(), model_location,
                has_win_anime ?
                kart_model.getFrame(KartModel::AF_SELECTION_START) > -1 ?
                kart_model.getFrame(KartModel::AF_SELECTION_START) :
                kart_model.getFrame(KartModel::AF_WIN_LOOP_START) > -1 ?
                kart_model.getFrame(KartModel::AF_WIN_LOOP_START) :
                kart_model.getFrame(KartModel::AF_WIN_START) :
                kart_model.getBaseFrame(),
                has_win_anime ?
                kart_model.getFrame(KartModel::AF_SELECTION_END) > -1 ?
                kart_model.getFrame(KartModel::AF_SELECTION_END) :
                kart_model.getFrame(KartModel::AF_WIN_END) :
                kart_model.getBaseFrame(),
                kart_model.getAnimationSpeed());

            w3->getModelViewRenderInfo()->setHue(kart_color);
            model_location.setScale(core::vector3df(1.0f, 1.0f, 1.0f));
            for (unsigned i = 0; i < 4; i++)
            {
                model_location.setTranslation(kart_model
                    .getWheelGraphicsPosition(i).toIrrVector());
                w3->addModel(kart_model.getWheelModel(i), model_location);
            }

            for (unsigned i = 0;
                 i < kart_model.getSpeedWeightedObjectsCount(); i++)
            {
                const SpeedWeightedObject& obj =
                    kart_model.getSpeedWeightedObject(i);
                core::matrix4 swol = obj.m_location;
                if (!obj.m_bone_name.empty())
                {
                    core::matrix4 inv =
                        kart_model.getInverseBoneMatrix(obj.m_bone_name);
                    swol = inv * obj.m_location;
                }
                w3->addModel(obj.m_model, swol, -1, -1, 0.0f, obj.m_bone_name);
            }
            //w3->update(0);

            m_kart_widgets[widget_id].m_kart_name
                ->setText( selectionText.c_str(), false );
        }
        else
            Log::warn("KartSelectionScreen", "could not "
                      "find a kart named '%s'",
                      selection.c_str());
    }
}

// ----------------------------------------------------------------------------
/**
 * Adds a message to the screen which indicates that players must press fire to join.
 */
void KartSelectionScreen::addMultiplayerMessage()
{
    Widget* fullarea = getWidget("playerskarts");
    const int splitWidth = fullarea->m_w / 2;
    int message_x = 0;
    if (m_kart_widgets.size() == 1)
        message_x = (int) (fullarea->m_x + splitWidth + splitWidth * 0.2f);
    else
        message_x = (int) (fullarea->m_x + splitWidth / 2 + splitWidth * 0.2f);

    if (m_kart_widgets.size() < 2 && m_multiplayer_message == NULL)
    {
        m_multiplayer_message = new BubbleWidget();
        m_multiplayer_message->m_properties[PROP_TEXT_ALIGN] = "center";
        m_multiplayer_message->setText(_("Everyone:\n"
            "Press the 'Select' button to join the game"));
        m_multiplayer_message->m_x = message_x;
        m_multiplayer_message->m_y = (int) (fullarea->m_y + fullarea->m_h * 0.3f);
        m_multiplayer_message->m_w = (int) (splitWidth * 0.6f);
        m_multiplayer_message->m_h = (int) (fullarea->m_h * 0.6f);
        m_multiplayer_message->setFocusable(false);
        m_multiplayer_message->add();
        manualAddWidget(m_multiplayer_message);
    }
    else if(m_multiplayer_message != NULL)
    {
        m_multiplayer_message->move(message_x, (int) (fullarea->m_y + fullarea->m_h * 0.3f),
                                    (int) (splitWidth * 0.6f), (int) (fullarea->m_h * 0.6f));
    }
}   // addMultiplayerMessage

// ----------------------------------------------------------------------------
/**
 * Remove the multiplayer message.
 */
void KartSelectionScreen::removeMultiplayerMessage()
{
    if (m_multiplayer_message != NULL)
    {
        manualRemoveWidget(m_multiplayer_message);
        m_multiplayer_message->getIrrlichtElement()->remove();
        m_multiplayer_message->elementRemoved();
        delete m_multiplayer_message;
        m_multiplayer_message = NULL;
    }
}   // removeMultiplayerMessage

// ----------------------------------------------------------------------------
/**
 * Callback handling events from the kart selection menu
 */
void KartSelectionScreen::eventCallback(Widget* widget,
                                        const std::string& name,
                                        const int player_id)
{
    // don't allow changing group after someone confirmed
    if (name == "kartgroups" && !m_game_master_confirmed)
    {
        RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
        assert(tabs != NULL);
        DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
        assert(w != NULL);

        setKartsFromCurrentGroup();

        const std::string &selected_kart_group =
            tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        UserConfigParams::m_last_used_kart_group = selected_kart_group;

        RandomGenerator random;

        const int num_players = m_kart_widgets.size();
        for (int n=0; n<num_players; n++)
        {
            // The game master is the one that can change the groups, leave
            // his focus on the tabs for others, remove focus from kart that
            // might no more exist in this tab.
            if (n != PLAYER_ID_GAME_MASTER)
                GUIEngine::focusNothingForPlayer(n);

            if (!m_kart_widgets[n].isReady())
            {
                // try to preserve the same kart for each player (except for
                // game master, since it's the one  that can change the
                // groups, so focus for this player must remain on the tabs)
                const std::string& selected_kart =
                    m_kart_widgets[n].getKartInternalName();
                if (!w->setSelection( selected_kart, n,
                                      n != PLAYER_ID_GAME_MASTER))
                {
                    // if we get here, it means one player "lost" his kart in
                    // the tab switch
                    if (UserConfigParams::logGUI())
                        Log::info("KartSelectionScreen", "Player %u"
                                  " lost their selection when switching tabs!!!",n);

                    // Select a random kart in this case
                    const int count = (int) w->getItems().size();
                    if (count > 0)
                    {
                        // FIXME: two players may be given the same kart by
                        // the use of random
                        const int random_id = random.get( count );

                        // select kart for players > 0 (player 0 is the one
                        // that can change the groups, so focus for player 0
                        // must remain on the tabs)
                        const bool success =
                            w->setSelection( random_id, n,
                                             n != PLAYER_ID_GAME_MASTER );
                        if (!success)
                            Log::warn("KartSelectionScreen",
                                      "setting kart of player %u failed");
                    }
                    else
                    {
                        Log::warn("KartSelectionScreen",  " 0 items "
                                  "in the ribbon");
                    }
                }
            }
        } // end for
    }
    else if (name == "karts")
    {
        if (m_kart_widgets.size() > unsigned(player_id))
            playerConfirm(player_id);
    }
    else if (name == "continue")
    {
        if (m_kart_widgets.size() > unsigned(player_id))
            playerConfirm(player_id);
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else
    {
        // Transmit to all subwidgets, maybe *they* care about this event
        const int amount = m_kart_widgets.size();
        for (int n=0; n<amount; n++)
        {
            m_kart_widgets[n].transmitEvent(widget, name, player_id);
        }

        // those events may mean that a player selection changed, so
        // validate again
        validateIdentChoices();
        validateKartChoices();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void KartSelectionScreen::setMultiplayer(bool multiplayer)
{
    m_multiplayer = multiplayer;
}   // setMultiplayer

// ----------------------------------------------------------------------------

bool KartSelectionScreen::onEscapePressed()
{
    m_go_to_overworld_next = false; // valid once
    m_must_delete_on_back = true; // delete the screen
    if (m_from_overworld)
    {
        m_from_overworld = false; // valid once
        OverWorld::enterOverWorld();
        return false;
    }
    else
    {
        return true;
    }
}

// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark KartSelectionScreen (private)
#endif

void KartSelectionScreen::allPlayersDone()
{
    input_manager->setMasterPlayerOnly(true);

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert(tabs != NULL);

    std::string selected_kart_group =
        tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    UserConfigParams::m_last_used_kart_group = selected_kart_group;

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );

    const PtrVector< StateManager::ActivePlayer, HOLD >& players =
        StateManager::get()->getActivePlayers();

    // ---- Print selection (for debugging purposes)
    if(UserConfigParams::logGUI())
    {
        Log::info("KartSelectionScreen", "players : %d",players.size());

        for (unsigned int n=0; n<players.size(); n++)
        {
            Log::info("KartSelectionScreen", "     Player %u is %s on %s",n,
                    core::stringc(
                          players[n].getConstProfile()->getName().c_str()).c_str(),
                    players[n].getDevice()->getName().c_str());
        }
    }

    for (unsigned int n=0; n<players.size(); n++)
    {
        StateManager::get()->getActivePlayer(n)->getProfile()
            ->incrementUseFrequency();
    }
    // ---- Give player info to race manager
    RaceManager::get()->setNumPlayers(players.size());

    // ---- Manage 'random kart' selection(s)
    RandomGenerator random;

    std::vector<ItemDescription> items = w->getItems();

    // remove the 'random' item itself
    const int item_count = (int) items.size();
    for (int n=0; n<item_count; n++)
    {
        if (items[n].m_code_name == RANDOM_KART_ID)
        {
            items[n].m_code_name = ID_DONT_USE;
            break;
        }
    }

    // pick random karts
    const int kart_count = m_kart_widgets.size();
    for (int n = 0; n < kart_count; n++)
    {
        std::string selected_kart = m_kart_widgets[n].m_kart_internal_name;

        if (selected_kart == RANDOM_KART_ID)
        {
            // don't select an already selected kart
            // to prevent infinite loop in case they are all locked
            int count = 0;
            bool done = false;
            do
            {
                int random_id = random.get(item_count);
                // valid kart if it can bt used, and is either not locked,
                // or it's a multiplayer race.
                if (items[random_id].m_code_name != ID_DONT_USE &&
                    (!StringUtils::startsWith(items[random_id].m_code_name, ID_LOCKED)
                    || m_multiplayer)                                                 )
                {
                    selected_kart = items[random_id].m_code_name;
                    done = true;
                }
                items[random_id].m_code_name = ID_DONT_USE;
                count++;
                if (count > 100) return;
            }
            while (!done);
        }
        else
        {
            // mark the item as taken
            for (int i=0; i<item_count; i++)
            {
                if (items[i].m_code_name ==
                        m_kart_widgets[n].m_kart_internal_name)
                {
                    items[i].m_code_name = ID_DONT_USE;
                    break;
                }
            }
        }
        
        if (n == PLAYER_ID_GAME_MASTER)
        {
            UserConfigParams::m_default_kart = selected_kart;
        }

        RaceManager::get()->setPlayerKart(n, selected_kart);

        // Set handicap if needed
        if (m_multiplayer && UserConfigParams::m_per_player_difficulty)
            RaceManager::get()->setPlayerHandicap(n, m_kart_widgets[n].getHandicap());
    }

    // ---- Switch to assign mode
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);

    StateManager::ActivePlayer *ap = m_multiplayer 
                                   ? NULL 
                                   : StateManager::get()->getActivePlayer(0);
    input_manager->getDeviceManager()->setSinglePlayer(ap);

    // ---- Go to next screen or return to overworld
    if (m_from_overworld || m_go_to_overworld_next)
    {
        m_from_overworld = false; // valid once
        m_go_to_overworld_next = false;
        OverWorld::enterOverWorld();
    }
    else
    {
        RaceSetupScreen::getInstance()->push();
    }
}   // allPlayersDone

// ----------------------------------------------------------------------------

bool KartSelectionScreen::validateIdentChoices()
{
    bool ok = true;

    const int amount = m_kart_widgets.size();

    // reset all marks, we'll re-add them next if errors are still there
    for (int n=0; n<amount; n++)
    {
        // first check if the player name widget is still there, it won't
        // be for those that confirmed
        if (m_kart_widgets[n].m_player_ident_spinner != NULL)
        {
            m_kart_widgets[n].m_player_ident_spinner->markAsCorrect();

            // verify internal consistency in debug mode
            if (m_multiplayer)
            {
                int spinner_value = m_kart_widgets[n].m_player_ident_spinner->getValue();
                if (UserConfigParams::m_per_player_difficulty)
                    spinner_value /= 2;
                assert(m_kart_widgets[n].getAssociatedPlayer()->getProfile() ==
                    PlayerManager::get()->getPlayer(spinner_value));
            }
        }
    }

    // perform actual checking
    for (int n=0; n<amount; n++)
    {
        // skip players that took a guest account, they can be many on the
        // same identity in this case
        if (m_kart_widgets[n].getAssociatedPlayer()->getProfile()
                ->isGuestAccount())
        {
            continue;
        }

        // check if another kart took the same identity as the current one
        for (int m=n+1; m<amount; m++)
        {

            // check if 2 players took the same name
            if (m_kart_widgets[n].getAssociatedPlayer()->getProfile() ==
                    m_kart_widgets[m].getAssociatedPlayer()->getProfile())
            {
                // two players took the same name. check if one is ready
                if (!m_kart_widgets[n].isReady() &&
                        m_kart_widgets[m].isReady())
                {
                    // player m is ready, so player n should not choose
                    // this name
                    m_kart_widgets[n].m_player_ident_spinner
                    ->markAsIncorrect();
                }
                else if (m_kart_widgets[n].isReady() &&
                         !m_kart_widgets[m].isReady())
                {
                    // player n is ready, so player m should not
                    // choose this name
                    m_kart_widgets[m].m_player_ident_spinner
                    ->markAsIncorrect();
                }
                else if (m_kart_widgets[n].isReady() &&
                         m_kart_widgets[m].isReady())
                {
                    // it should be impossible for two players to confirm
                    // they're ready with the same name
                    assert(false);
                }

                ok = false;
            }
        } // end for
    }

    return ok;
}   // validateIdentChoices

// -----------------------------------------------------------------------------

bool KartSelectionScreen::validateKartChoices()
{
    bool ok = true;

    const unsigned int amount = m_kart_widgets.size();

    // reset all marks, we'll re-add them next if errors are still there
    for (unsigned int n=0; n<amount; n++)
    {
        m_kart_widgets[n].m_model_view->unsetBadge(BAD_BADGE);
    }

    // Check if we have enough karts for everybody. If there are more
    // players than karts then just allow duplicates
    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );
    const unsigned int availableKartCount = (unsigned int)w->getItems().size();
    if (amount > availableKartCount) return true;

    // Check everyone for duplicates
    for (unsigned int n=0; n<amount; n++)
    {
        for (unsigned int m=n+1; m<amount; m++)
        {
            // check if 2 players took the same name
            if (sameKart(m_kart_widgets[n], m_kart_widgets[m]))
            {
                if (UserConfigParams::logGUI())
                {
                    Log::warn("KartSelectionScreen", "Kart conflict!!");
                    Log::warn("KartSelectionScreen", "    Player %u chose %s",n,
                              m_kart_widgets[n].getKartInternalName().c_str());
                    Log::warn("KartSelectionScreen", "    Player %u chose %s",m,
                              m_kart_widgets[m].getKartInternalName().c_str());
                }

                // two players took the same kart. check if one is ready
                if (!m_kart_widgets[n].isReady() &&
                        m_kart_widgets[m].isReady())
                {
                    if (UserConfigParams::logGUI())
                       Log::info("KartSelectionScreen", "    --> Setting red badge on player %u", n);

                    // player m is ready, so player n should not choose
                    // this name
                    m_kart_widgets[n].m_model_view->setBadge(BAD_BADGE);
                }
                else if (m_kart_widgets[n].isReady() &&
                         !m_kart_widgets[m].isReady())
                {
                    if (UserConfigParams::logGUI())
                        Log::info("KartSelectionScreen", "    --> Setting red badge on player %u",m);

                    // player n is ready, so player m should not
                    // choose this name
                    m_kart_widgets[m].m_model_view->setBadge(BAD_BADGE);
                }
                else if (m_kart_widgets[n].isReady() &&
                         m_kart_widgets[m].isReady())
                {
                    // it should be impossible for two players to confirm
                    // they're ready with the same kart
                    assert(false);
                }

                // we know it's not ok (but don't stop right now, all bad
                // ones need red badges)
                ok = false;
            }
        } // end for
    }

    return ok;

}   // validateKartChoices

// ----------------------------------------------------------------------------

void KartSelectionScreen::renumberKarts()
{
    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert( w != NULL );
    Widget* fullarea = getWidget("playerskarts");
    int splitWidth = fullarea->m_w / m_kart_widgets.size();
    if (m_kart_widgets.size() == 1)
        splitWidth /= 2;

    for (unsigned int n=0; n < m_kart_widgets.size(); n++)
    {
        m_kart_widgets[n].setPlayerID(n);
        m_kart_widgets[n].move( fullarea->m_x + splitWidth*n, fullarea->m_y,
                                splitWidth, fullarea->m_h );
    }

    w->updateItemDisplay();
}   // renumberKarts

// ----------------------------------------------------------------------------
PtrVector<const KartProperties, REF> KartSelectionScreen::getUsableKarts(
    const std::string& selected_kart_group)
{
    PtrVector<const KartProperties, REF> karts;
    for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* prop = kart_properties_manager->getKartById(i);
        // Ignore karts that are not in the selected group
        if((selected_kart_group != ALL_KART_GROUPS_ID &&
            !prop->isInGroup(selected_kart_group)) || isIgnored(prop->getIdent()))
            continue;
        karts.push_back(prop);
    }
    karts.insertionSort();
    return karts;
}   // getUsableKarts

// ----------------------------------------------------------------------------

void KartSelectionScreen::setKartsFromCurrentGroup()
{
    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert(tabs != NULL);

    std::string selected_kart_group =
        tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    UserConfigParams::m_last_used_kart_group = selected_kart_group;

    // This can happen if addons are removed so that also the previously
    // selected kart group is removed. In this case, select the
    // 'standard' group
    if (selected_kart_group != ALL_KART_GROUPS_ID &&
        !kart_properties_manager->getKartsInGroup(selected_kart_group).size())
    {
        selected_kart_group = DEFAULT_GROUP_NAME;
    }

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    w->clearItems();

    int usable_kart_count = 0;
    PtrVector<const KartProperties, REF> karts = getUsableKarts(selected_kart_group);

    if (karts.empty())
    {
        // In network this will happen if no addons kart on server
        PtrVector<const KartProperties, REF> new_karts =
            getUsableKarts(DEFAULT_GROUP_NAME);
        std::swap(karts.m_contents_vector, new_karts.m_contents_vector);
        tabs->select(DEFAULT_GROUP_NAME, PLAYER_ID_GAME_MASTER);
    }

    for(unsigned int i=0; i<karts.size(); i++)
    {
        const KartProperties* prop = karts.get(i);
        if (PlayerManager::getCurrentPlayer()->isLocked(prop->getIdent()) &&
            !m_multiplayer && !NetworkConfig::get()->isNetworking())
        {
            w->addItem(_("Locked : solve active challenges to gain access to more!"),
                       ID_LOCKED + prop->getIdent(),
                       prop->getAbsoluteIconFile(), LOCKED_BADGE,
                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            w->addItem(prop->getName(),
                       prop->getIdent(),
                       prop->getAbsoluteIconFile(), 0,
                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            usable_kart_count++;
        }
    }

    // add random
    if (usable_kart_count > 1)
    {
        w->addItem(_("Random Kart"), RANDOM_KART_ID, "/gui/icons/random_kart.png");
    }

    w->updateItemDisplay();
}

// ----------------------------------------------------------------------------
bool KartSelectionScreen::useContinueButton() const
{
#ifdef MOBILE_STK
    if (m_multiplayer)
        return false;
    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 &&
        irr_driver->getDevice()->supportsTouchDevice()) ||
        UserConfigParams::m_multitouch_active > 1;
    return multitouch_enabled;
#else
    return false;
#endif
}   // useContinueButton

#if 0
#pragma mark -
#endif

