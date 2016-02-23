#include "network/protocols/start_game_protocol.hpp"

#include "config/player_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/race_event_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "race/race_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

StartGameProtocol::StartGameProtocol(GameSetup* game_setup)
                 : Protocol(PROTOCOL_START_GAME)
{
    m_game_setup = game_setup;
    const std::vector<NetworkPlayerProfile*> &players = 
                                                    m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        m_player_states[ players[i] ] = LOADING;
    }
    m_ready_count = 0;
}   // StartGameProtocol

// ----------------------------------------------------------------------------
StartGameProtocol::~StartGameProtocol()
{
}   // ~StartGameProtocol

// ----------------------------------------------------------------------------
/** Setup the actual game. It first starts the SynchronisationProtocol,
 *  and then 
 */
void StartGameProtocol::setup()
{
    m_state       = NONE;
    m_ready_count = 0;
    m_ready       = false;
    Log::info("SynchronizationProtocol", "Ready !");

    Protocol *p = new SynchronizationProtocol();
    p->requestStart();
    Log::info("StartGameProtocol", "SynchronizationProtocol started.");

    // Race startup sequence
    // ---------------------
    // This creates the network world.
    RaceEventManager::getInstance<RaceEventManager>()->start();

    // The number of karts includes the AI karts, which are not supported atn
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
        if(!is_local)
        {
            // On the server no device or player profile is needed.
            StateManager::get()->createActivePlayer(NULL, NULL);
        }

        // Adjust the local player id so that local players have the numbers
        // 0 to num-1; and all other karts start with num. This way the local
        // players get the first ActivePlayers assigned (which have the 
        // corresponding device associated with it).
        RemoteKartInfo rki(is_local ? local_player_id
                                    : i-local_player_id+STKHost::get()->getGameSetup()->getNumLocalPlayers(),
                           profile->getKartName(),
                           profile->getName(),
                           profile->getHostId(),
                           !is_local);
        rki.setGlobalPlayerId(profile->getGlobalPlayerId());
        rki.setPerPlayerDifficulty(profile->getPerPlayerDifficulty());
        if(is_local)
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

    Log::info("StartGameProtocol", "Player configuration ready.");
    m_state = SYNCHRONIZATION_WAIT;

}   // setup

// ----------------------------------------------------------------------------
bool StartGameProtocol::notifyEventAsynchronous(Event* event)
{
    const NewNetworkString &data = event->data();
    if (data.size() < 5)
    {
        Log::error("StartGameProtocol", "Too short message.");
        return true;
    }
    uint32_t token = data.getUInt32();
    uint8_t ready = data.getUInt8(4);
    STKPeer* peer = event->getPeer();
    if (peer->getClientServerToken() != token)
    {
        Log::error("StartGameProtocol", "Bad token received.");
        return true;
    }
    if (NetworkConfig::get()->isServer() && ready) // on server, player is ready
    {
        Log::info("StartGameProtocol", "One of the players is ready.");
        m_player_states[peer->getPlayerProfile()] = READY;
        m_ready_count++;
        if (m_ready_count == m_game_setup->getPlayerCount())
        {
            // everybody ready, synchronize
            Protocol *p = ProtocolManager::getInstance()
                        ->getProtocol(PROTOCOL_SYNCHRONIZATION);
            SynchronizationProtocol* protocol = 
                                static_cast<SynchronizationProtocol*>(p);
            if (protocol)
            {
                protocol->startCountdown(5000); // 5 seconds countdown
                Log::info("StartGameProtocol",
                          "All players ready, starting countdown.");
                m_ready = true;
                return true;
            }
            else
                Log::error("StartGameProtocol",
                          "The Synchronization protocol hasn't been started.");
        }   // if m_ready_count == number of players
    }
    else // on the client, we shouldn't even receive messages.
    {
        Log::error("StartGameProtocol", "Received a message with bad format.");
    }
    return true;
}   // notifyEventAsynchronous

// ----------------------------------------------------------------------------
void StartGameProtocol::update()
{
    switch(m_state)
    {
        case SYNCHRONIZATION_WAIT:
        {
            // Wait till the synchronisation protocol is running
            Protocol *p = ProtocolManager::getInstance()
                        ->getProtocol(PROTOCOL_SYNCHRONIZATION);
            SynchronizationProtocol* protocol =
                              static_cast<SynchronizationProtocol*>(p);
            if (protocol)
            {
                // Now the synchronization protocol exists.
                Log::info("StartGameProtocol", "Starting the race loading.");
                // This will create the world instance,
                // i.e. load track and karts
                m_game_setup->getRaceConfig()->setRaceData();
                World::getWorld()->setNetworkWorld(true);
                m_state = LOADING;
            }
            break;
        }
        case LOADING:
        {
            if (m_ready) m_state = READY;
            break;
        }
        case READY:
        {
            // set karts into the network game setup
            STKHost::get()->getGameSetup()->bindKartsToProfiles();
            m_state = EXITING;
            requestTerminate();
            break;
        }
    }   // switch
}   // update

// ----------------------------------------------------------------------------
/** Callback from the race manager when the world was setup.
 */
void StartGameProtocol::ready()
{
    // On clients this means the loading is finished.
    // Inform the server that this client is ready.
    if (NetworkConfig::get()->isClient())
    {
        assert(STKHost::get()->getPeerCount() == 1);
        NewNetworkString *ns = getNetworkString(5);
        ns->setToken(STKHost::get()->getPeers()[0]->getClientServerToken());
        ns->addUInt8(1);
        Log::info("StartGameProtocol", "Player ready, notifying server.");
        sendMessage(*ns, /*reliable*/true);
        delete ns;
        m_state = READY;
        m_ready = true;
        return;
    }
}   // ready

