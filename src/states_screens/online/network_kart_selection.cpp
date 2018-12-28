//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "states_screens/online/network_kart_selection.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "input/device_manager.hpp"
#include "network/network_config.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/tracks_screen.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::init()
{
    assert(!NetworkConfig::get()->isAddingNetworkPlayers());
    m_multiplayer = NetworkConfig::get()->getNetworkPlayers().size() != 1;
    KartSelectionScreen::init();

    m_timer = getWidget<GUIEngine::ProgressBarWidget>("timer");
    m_timer->showLabel(false);
    updateProgressBarText();

    // change the back button image (because it makes the game quit)
    IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
    back_button->setImage("gui/icons/main_quit.png");

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert(w != NULL);
    for (auto& p : NetworkConfig::get()->getNetworkPlayers())
    {
        joinPlayer(std::get<0>(p), std::get<1>(p));
        if (std::get<2>(p) == PLAYER_DIFFICULTY_HANDICAP)
        {
            m_kart_widgets.get(m_kart_widgets.size() -1)
                ->enableHandicapForNetwork();
        }
        w->updateItemDisplay();
        if (!w->setSelection(UserConfigParams::m_default_kart, 0, true))
        {
            // if kart from config not found, select the first instead
            w->setSelection(0, 0, true);
        }
    }
}   // init

// ----------------------------------------------------------------------------
/** Called once per frame. Updates the timer display.
 *  \param dt Time step size.
 */
void NetworkKartSelectionScreen::onUpdate(float dt)
{
    KartSelectionScreen::onUpdate(dt);
    updateProgressBarText();
}   // onUpdate

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::allPlayersDone()
{
    input_manager->setMasterPlayerOnly(true);

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert(tabs != NULL);

    std::string selected_kart_group =
        tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    UserConfigParams::m_last_used_kart_group = selected_kart_group;

    const PtrVector<StateManager::ActivePlayer, HOLD>& players =
        StateManager::get()->getActivePlayers();
    for (unsigned int n = 0; n < players.size(); n++)
    {
        StateManager::get()->getActivePlayer(n)->getProfile()
            ->incrementUseFrequency();
    }

    const uint8_t kart_count = (uint8_t)m_kart_widgets.size();
    NetworkString kart(PROTOCOL_LOBBY_ROOM);
    kart.addUInt8(LobbyProtocol::LE_KART_SELECTION).addUInt8(kart_count);
    for (unsigned n = 0; n < kart_count; n++)
    {
        // If server recieve an invalid name, it will auto correct to a random
        // kart
        kart.encodeString(m_kart_widgets[n].m_kart_internal_name);
    }
    STKHost::get()->sendToServer(&kart, true);

    // ---- Switch to assign mode
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
    TracksScreen::getInstance()->setNetworkTracks();
    TracksScreen::getInstance()->push();

}   // allPlayersDone

// ----------------------------------------------------------------------------
bool NetworkKartSelectionScreen::onEscapePressed()
{
    // then remove the lobby screen (you left the server)
    StateManager::get()->popMenu();
    STKHost::get()->shutdown();
    return true; // remove the screen
}   // onEscapePressed

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::updateProgressBarText()
{
    if (auto lp = LobbyProtocol::get<LobbyProtocol>())
    {
        float new_value =
            lp->getRemainingVotingTime() / lp->getMaxVotingTime();
        if (new_value < 0.0f)
            new_value = 0.0f;
        m_timer->setValue(new_value * 100.0f);
        int remaining_time = (int)(lp->getRemainingVotingTime());
        if (remaining_time < 0)
            remaining_time = 0;
        //I18N: In kart screen, show before the voting period in network ends.
        core::stringw message = _("Remaining time: %d", remaining_time);
        m_timer->setText(message);
    }
}   // updateProgressBarText
