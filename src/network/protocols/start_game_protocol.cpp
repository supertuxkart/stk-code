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
#include "network/network_world.hpp"
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
    // builds it and starts
    NetworkWorld::getInstance<NetworkWorld>()->start();
    race_manager->setNumKarts(m_game_setup->getPlayerCount());
    race_manager->setNumPlayers(m_game_setup->getPlayerCount());
    // setNumPlayers by default sets number of local players to
    // number of players - so avoid this to keep the original number:
    race_manager->setNumPlayers(1, 
                         /*local players*/race_manager->getNumLocalPlayers());

    // Create the kart information for the race manager:
    // -------------------------------------------------
    std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        NetworkPlayerProfile* profile = players[i];
        bool is_me =  profile->getGlobalPlayerId()
                   == STKHost::get()->getGameSetup()->getLocalMasterID();
        RemoteKartInfo rki(profile->getGlobalPlayerId(),
                           profile->getKartName(),
                           profile->getName(),
                           /*hostid*/profile->getGlobalPlayerId(),
                           !is_me);
        rki.setPerPlayerDifficulty(profile->getPerPlayerDifficulty());
        rki.setLocalPlayerId(0);
        // FIXME: for now (only one local player) the global player id
        // can be used as host id.
        rki.setHostId(profile->getGlobalPlayerId());

        // Inform the race manager about the data for this kart.
        race_manager->setPlayerKart(i, rki);

        if(is_me)
        {
            PlayerProfile* profile_to_use = PlayerManager::getCurrentPlayer();
            assert(profile_to_use);
            InputDevice* device = input_manager->getDeviceManager()
                                ->getLatestUsedDevice();
            int new_player_id = 0;

            // more than one player, we're the first
            if (StateManager::get()->getActivePlayers().size() >= 1)
                new_player_id = 0;
            else
                new_player_id = StateManager::get()
                              ->createActivePlayer(profile_to_use, device);
            StateManager::ActivePlayer *ap =
                           StateManager::get()->getActivePlayer(new_player_id);
            device->setPlayer(ap);
            input_manager->getDeviceManager()->setSinglePlayer(ap);
            race_manager->setLocalKartInfo(new_player_id,
                                           profile->getKartName());
            NetworkWorld::getInstance()->setSelfKart(profile->getKartName());
        }   // if is_me
        else
        {
            StateManager::get()->createActivePlayer( NULL, NULL );
            race_manager->setPlayerKart(i, rki);
        }
    }   // for i in players

    race_manager->computeRandomKartList();
    Log::info("StartGameProtocol", "Player configuration ready.");
    m_state = SYNCHRONIZATION_WAIT;

}   // setup

// ----------------------------------------------------------------------------
bool StartGameProtocol::notifyEventAsynchronous(Event* event)
{
    const NetworkString &data = event->data();
    if (data.size() < 5)
    {
        Log::error("StartGameProtocol", "Too short message.");
        return true;
    }
    uint32_t token = data.gui32();
    uint8_t ready = data.gui8(4);
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
        NetworkString ns(5);
        ns.ai32(STKHost::get()->getPeers()[0]->getClientServerToken()).ai8(1);
        Log::info("StartGameProtocol", "Player ready, notifying server.");
        sendMessage(ns, true);
        m_state = READY;
        m_ready = true;
        return;
    }
}   // ready

