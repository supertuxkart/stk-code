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

#include "states_screens/network_kart_selection.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widgets/player_kart_widget.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "network/stk_host.hpp"
#include "states_screens/server_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "states_screens/waiting_for_others.hpp"

static const char ID_LOCKED[] = "locked/";

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkKartSelectionScreen );

NetworkKartSelectionScreen::NetworkKartSelectionScreen()
                          : KartSelectionScreen("karts_online.stkgui")
{
}   // NetworkKartSelectionScreen

// ----------------------------------------------------------------------------
NetworkKartSelectionScreen::~NetworkKartSelectionScreen()
{
}   // ~NetworkKartSelectionScreen

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::init()
{
    m_multiplayer = false;
    KartSelectionScreen::init();

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );
    // Select standard kart group
    tabs->select( "standard", PLAYER_ID_GAME_MASTER);
    tabs->setActive(false);
    tabs->setVisible(false);

    // change the back button image (because it makes the game quit)
    IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
    back_button->setImage("gui/main_quit.png");

    // add a widget for each player except self (already exists):
    GameSetup* setup = STKHost::get()->getGameSetup();
    if (!setup)
    {
        Log::error("NetworkKartSelectionScreen",
                   "No network game setup registered.");
        return;
    }
    std::vector<NetworkPlayerProfile*> players = setup->getPlayers();

    Log::info("NKSS", "There are %d players", players.size());
    // ---- Get available area for karts
    // make a copy of the area, ands move it to be outside the screen
    Widget* kartsAreaWidget = getWidget("playerskarts");
    // start at the rightmost of the screen
    const int shift = irr_driver->getFrameSize().Width;
    core::recti kartsArea(kartsAreaWidget->m_x + shift,
                            kartsAreaWidget->m_y,
                            kartsAreaWidget->m_x + shift + kartsAreaWidget->m_w,
                            kartsAreaWidget->m_y + kartsAreaWidget->m_h);
    GameSetup *game_setup = STKHost::get()->getGameSetup();

    // FIXME: atm only adds the local master, split screen supports
    // needs to be added
    int player_id = game_setup->getLocalMasterID();
    m_id_mapping.clear();
    m_id_mapping.insert(m_id_mapping.begin(), player_id);

    const int amount = m_kart_widgets.size();
    Widget* fullarea = getWidget("playerskarts");

    const int splitWidth = fullarea->m_w / amount;

    for (int n=0; n<amount; n++)
    {
        m_kart_widgets[n].move( fullarea->m_x + splitWidth*n,
                                fullarea->m_y, splitWidth, fullarea->m_h);
    }

}   // init

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::playerConfirm(const int playerID)
{
    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert(w != NULL);
    const std::string selection = w->getSelectionIDString(playerID);
    if (StringUtils::startsWith(selection, ID_LOCKED))
    {
        unlock_manager->playLockSound();
        return;
    }

    if (playerID == PLAYER_ID_GAME_MASTER)
    {
        UserConfigParams::m_default_kart = selection;
    }

    if (m_kart_widgets[playerID].getKartInternalName().size() == 0)
    {
        SFXManager::get()->quickSound( "anvil" );
        return;
    }
    if(playerID == PLAYER_ID_GAME_MASTER) // self
    {
        ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
                ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
        // FIXME SPLITSCREEN: we need to supply the global player id of the 
        // player selecting the kart here. For now ... just vote the same kart
        // for each local player.
        std::vector<NetworkPlayerProfile*> players =
            STKHost::get()->getMyPlayerProfiles();
        for(unsigned int i=0; i<players.size(); i++)
        {
            protocol->requestKartSelection(players[i]->getGlobalPlayerId(),
                                           selection);
        }
    }
}   // playerConfirm

// ----------------------------------------------------------------------------
void NetworkKartSelectionScreen::playerSelected(uint8_t player_id,
                                                const std::string &kart_name)
{
    int widget_id = -1;
    for (unsigned int i = 0; i < m_id_mapping.size(); i++)
    {
        Log::info("NKSS", "Checking race id %d : mapped of %d is %d",
                   player_id, i, m_id_mapping[i]);
        if (m_id_mapping[i] == player_id)
            widget_id = i;
    }

    // This selection was for a remote kart, which is not shown
    // Just ignore it.
    if(widget_id==-1)
        return;

    KartSelectionScreen::updateKartWidgetModel(widget_id, kart_name,
                                       irr::core::stringw(kart_name.c_str()));
    KartSelectionScreen::updateKartStats(widget_id, kart_name);
    m_kart_widgets[widget_id].setKartInternalName(kart_name);
    m_kart_widgets[widget_id].markAsReady(); // mark player ready

    // If this is the authorised client, send the currently set race config
    // to the server.
    if(STKHost::get()->isAuthorisedToControl())
    {
        Protocol* protocol = ProtocolManager::getInstance()
                           ->getProtocol(PROTOCOL_LOBBY_ROOM);
        ClientLobbyRoomProtocol* clrp =
                           static_cast<ClientLobbyRoomProtocol*>(protocol);

        // FIXME: for now we submit a vote from the authorised user
        // for the various modes based on the settings in the race manager. 
        // This needs more/better gui elements (and some should be set when
        // defining the server).
        std::vector<NetworkPlayerProfile*> players =
                                         STKHost::get()->getMyPlayerProfiles();
        for(unsigned int i=0; i<players.size(); i++)
        {
            uint8_t id = players[i]->getGlobalPlayerId();
            clrp->voteMajor(id, race_manager->getMajorMode());
            clrp->voteMinor(id, race_manager->getMinorMode());
            clrp->voteReversed(id, race_manager->getReverseTrack());
            clrp->voteRaceCount(id, 1);
            clrp->voteLaps(id, 3);
        }
        //WaitingForOthersScreen::getInstance()->push();
        //return;
    }
    TracksScreen::getInstance()->setOfficalTrack(true);
    TracksScreen::getInstance()->push();
}   // playerSelected

// ----------------------------------------------------------------------------
bool NetworkKartSelectionScreen::onEscapePressed()
{
    // then remove the lobby screen (you left the server)
    StateManager::get()->popMenu();
    ServerSelection::getInstance()->refresh();
    // notify the server that we left
    ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
    if (protocol)
        protocol->leave();
    return true; // remove the screen
}   // onEscapePressed
