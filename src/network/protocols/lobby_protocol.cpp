//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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


#include "network/protocols/lobby_protocol.hpp"

#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "modes/world.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/controller_events_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/protocols/kart_update_protocol.hpp"
#include "network/protocols/latency_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"

LobbyProtocol *LobbyProtocol::m_lobby = NULL;

LobbyProtocol::LobbyProtocol(CallbackObject* callback_object)
                 : Protocol(PROTOCOL_LOBBY_ROOM, callback_object)
{
    m_game_setup = NULL;
}   // LobbyProtocol

// ----------------------------------------------------------------------------
LobbyProtocol::~LobbyProtocol()
{
}   // ~LobbyProtocol

//-----------------------------------------------------------------------------
/** Starts the sychronization protocol and the RaceEventManager. It then
 *  sets the player structures up, creates the active player, and loads
 *  the world.
 *  This is called on the client when the server informs them that
 *  the world can be loaded (LE_LOAD_WORLD) and on the server in state
 *  LOAD_WORLD (i.e. just after informing all clients).
 */
void LobbyProtocol::loadWorld()
{
    Log::info("LobbyProtocol", "Ready !");

    // Race startup sequence
    // ---------------------
    // This creates the network world.
    RaceEventManager::getInstance<RaceEventManager>()->start();

    // The number of karts includes the AI karts, which are not supported atm
    race_manager->setNumKarts(m_game_setup->getPlayerCount());

    // Set number of global and local players.
    race_manager->setNumPlayers(m_game_setup->getPlayerCount(),
                                m_game_setup->getNumLocalPlayers());

    // Create the kart information for the race manager:
    // -------------------------------------------------
    std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
    int local_player_id = 0;
    for (unsigned int i = 0; i < players.size(); i++)
    {
        NetworkPlayerProfile* profile = players[i];
        bool is_local = profile->isLocalPlayer();

        // All non-local players are created here. This means all players
        // on the server, and all non-local players on a client (the local
        // karts are created in the NetworkingLobby).
        if (!is_local)
        {
            // On the server no device or player profile is needed.
            StateManager::get()->createActivePlayer(NULL, NULL);
        }

        // Adjust the local player id so that local players have the numbers
        // 0 to num-1; and all other karts start with num. This way the local
        // players get the first ActivePlayers assigned (which have the 
        // corresponding device associated with it).
        RemoteKartInfo rki(is_local ? local_player_id
                                    : i - local_player_id
                                      + STKHost::get()->getGameSetup()->getNumLocalPlayers(),
                           profile->getKartName(),
                           profile->getName(),
                           profile->getHostId(),
                          !is_local);
        rki.setGlobalPlayerId(profile->getGlobalPlayerId());
        rki.setPerPlayerDifficulty(profile->getPerPlayerDifficulty());
        if (is_local)
        {
            rki.setLocalPlayerId(local_player_id);
            local_player_id++;
        }

        // Inform the race manager about the data for this kart.
        race_manager->setPlayerKart(i, rki);
    }   // for i in players

    // Make sure that if there is only a single local player this player can
    // use all input devices.
    StateManager::ActivePlayer *ap = race_manager->getNumLocalPlayers()>1
                                   ? NULL
                                   : StateManager::get()->getActivePlayer(0);

    input_manager->getDeviceManager()->setSinglePlayer(ap);

    Log::info("LobbyProtocol", "Player configuration ready.");

    // Load the actual world.
    m_game_setup->getRaceConfig()->loadWorld();
    World::getWorld()->setNetworkWorld(true);
    (new KartUpdateProtocol())->requestStart();
    (new ControllerEventsProtocol())->requestStart();
    (new GameEventsProtocol())->requestStart();

}   // loadWorld

// ----------------------------------------------------------------------------
/** Terminates the LatencyProtocol.
 */
void LobbyProtocol::terminateLatencyProtocol()
{
    Protocol *p = ProtocolManager::getInstance()
                ->getProtocol(PROTOCOL_SYNCHRONIZATION);
    LatencyProtocol *sp = dynamic_cast<LatencyProtocol*>(p);
    if (sp)
        sp->requestTerminate();
}   // stopLatencyProtocol
