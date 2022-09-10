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
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "input/device_manager.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/server.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/tracks_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::beforeAddingWidget()
{
    m_multiplayer = NetworkConfig::get()->getNetworkPlayers().size() != 1;
    KartSelectionScreen::beforeAddingWidget();
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::init()
{
    assert(!NetworkConfig::get()->isAddingNetworkPlayers());
    m_all_players_done = false;
    KartSelectionScreen::init();

    m_timer = getWidget<GUIEngine::ProgressBarWidget>("timer");
    m_timer->showLabel(false);
    if (m_live_join)
        m_timer->setVisible(false);
    else
    {
        m_timer->setVisible(true);
        updateProgressBarText();
    }

    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert(w != NULL);
    for (auto& p : NetworkConfig::get()->getNetworkPlayers())
    {
        joinPlayer(std::get<0>(p), std::get<1>(p));
        if (std::get<2>(p) == HANDICAP_MEDIUM)
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
    m_exit_timeout = std::numeric_limits<uint64_t>::max();
}   // init

// ----------------------------------------------------------------------------
/** Called once per frame. Updates the timer display.
 *  \param dt Time step size.
 */
void NetworkKartSelectionScreen::onUpdate(float dt)
{
    if (StkTime::getMonoTimeMs() > m_exit_timeout)
    {
        // Reset the screen to networking menu if failed to back to lobby
        STKHost::get()->shutdown();
        StateManager::get()->resetAndSetStack(
            NetworkConfig::get()->getResetScreens().data());
        NetworkConfig::get()->unsetNetworking();
        return;
    }

    KartSelectionScreen::onUpdate(dt);
    updateProgressBarText();
}   // onUpdate

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::allPlayersDone()
{
    m_all_players_done = true;
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
    if (m_live_join)
    {
        kart.setSynchronous(true);
        kart.addUInt8(LobbyProtocol::LE_LIVE_JOIN)
            // not spectator
            .addUInt8(0);
    }
    else
        kart.addUInt8(LobbyProtocol::LE_KART_SELECTION);
    kart.addUInt8(kart_count);
    for (unsigned n = 0; n < kart_count; n++)
    {
        // If server recieve an invalid name, it will auto correct to a random
        // kart
        kart.encodeString(m_kart_widgets[n].m_kart_internal_name);
    }

    NetworkConfig* nc = NetworkConfig::get();
    if (nc->getServerCapabilities().find(
        "real_addon_karts") != nc->getServerCapabilities().end())
    {
        for (unsigned n = 0; n < kart_count; n++)
        {
            KartData kart_data;
            if (nc->useTuxHitboxAddon())
            {
                const KartProperties* kp = kart_properties_manager
                    ->getKart(m_kart_widgets[n].m_kart_internal_name);
                if (kp && kp->isAddon())
                    kart_data = KartData(kp);
            }
            kart_data.encode(&kart);
        }
    }
    STKHost::get()->sendToServer(&kart, true);

    // ---- Switch to assign mode
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
    auto cl = LobbyProtocol::get<ClientLobby>();
    if (!m_live_join && cl && cl->serverEnabledTrackVoting())
    {
        TracksScreen::getInstance()->setNetworkTracks();
        TracksScreen::getInstance()->push();
    }
}   // allPlayersDone

// ----------------------------------------------------------------------------
bool NetworkKartSelectionScreen::onEscapePressed()
{
    if (!m_live_join)
    {
        /*auto cl = LobbyProtocol::get<ClientLobby>();
        if (m_all_players_done && cl && !cl->serverEnabledTrackVoting())
        {
            // TODO: Allow players to re-choose kart(s) if no track voting
            init();
            return false;
        }*/
        if (m_exit_timeout == std::numeric_limits<uint64_t>::max())
        {
            // Send go back lobby event to server with an exit timeout, so if
            // server doesn't react in time we exit the server
            m_exit_timeout = StkTime::getMonoTimeMs() + 5000;
            NetworkString back(PROTOCOL_LOBBY_ROOM);
            back.addUInt8(LobbyProtocol::LE_CLIENT_BACK_LOBBY);
            STKHost::get()->sendToServer(&back, true);
        }
        return false;
    }
    // Rewrite the previous server infos saved (game mode, chat lists...)
    NetworkingLobby::getInstance()->reloadServerInfos();
    if (auto cl = LobbyProtocol::get<ClientLobby>())
        NetworkingLobby::getInstance()->setHeader(cl->getJoinedServer()->getName());
    return true; // remove the screen
}   // onEscapePressed

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::updateProgressBarText()
{
    if (m_live_join)
        return;
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

// ----------------------------------------------------------------------------
bool NetworkKartSelectionScreen::isIgnored(const std::string& ident) const
{
    // Online addon kart use tux for hitbox in server so we can allow any
    // addon kart graphically, if live join is disabled
    if (NetworkConfig::get()->useTuxHitboxAddon() &&
        ident.find("addon_") != std::string::npos)
        return false;
    return m_available_karts.find(ident) == m_available_karts.end();
}   // isIgnored
