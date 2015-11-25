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
        std::pair<NetworkPlayerProfile*, STATE> player_state(players[i], LOADING);
        m_player_states.insert(player_state);
    }
    m_ready_count = 0;
}   // StartGameProtocol

// ----------------------------------------------------------------------------
StartGameProtocol::~StartGameProtocol()
{
}

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
        }
    }
    else // on the client, we shouldn't even receive messages.
    {
        Log::error("StartGameProtocol", "Received a message with bad format.");
    }
    return true;
}

// ----------------------------------------------------------------------------
void StartGameProtocol::setup()
{
    m_state = NONE;
    m_ready_count = 0;
    m_ready = false;
    Log::info("SynchronizationProtocol", "Ready !");
}

// ----------------------------------------------------------------------------
bool compareKarts(NetworkPlayerProfile* a, NetworkPlayerProfile* b)
{ 
    return (a->getPlayerID() < b->getPlayerID()); 
}   // compareKarts

// ----------------------------------------------------------------------------
void StartGameProtocol::update()
{
    if (m_state == NONE)
    {
        // if no synchronization protocol exists, create one
        Protocol *p = new SynchronizationProtocol();
        p->requestStart();
        Log::info("StartGameProtocol", "SynchronizationProtocol started.");

        // Race startup sequence
        // ---------------------
        // builds it and starts
        NetworkWorld::getInstance<NetworkWorld>()->start();
        race_manager->setNumKarts(m_game_setup->getPlayerCount());
        race_manager->setNumPlayers(m_game_setup->getPlayerCount());
        race_manager->setNumLocalPlayers(1);
        std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
        std::sort(players.begin(), players.end(), compareKarts);
        // have to add self first
        for (unsigned int i = 0; i < players.size(); i++)
        {
            bool is_me = (players[i]->getOnlineProfile() ==
                          PlayerManager::getCurrentOnlineProfile());
            if (is_me)
            {
                NetworkPlayerProfile* profile = players[i];
                RemoteKartInfo rki(profile->getPlayerID(), profile->getKartName(),
                                   profile->getOnlineProfile()->getUserName().c_str(),
                                   profile->getPlayerID(), !is_me);
                rki.setPerPlayerDifficulty(profile->getPerPlayerDifficulty());
                rki.setGlobalPlayerId(profile->getPlayerID());
                rki.setLocalPlayerId(is_me?0:1);
                rki.setHostId(profile->getPlayerID());
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
                                  ->createActivePlayer( profile_to_use, device);
                StateManager::ActivePlayer *ap = 
                    StateManager::get()->getActivePlayer(new_player_id);
                device->setPlayer(ap);
                input_manager->getDeviceManager()->setSinglePlayer(ap);

                race_manager->setPlayerKart(i, rki);
                race_manager->setLocalKartInfo(new_player_id, profile->getKartName());
                // self config
                Log::info("StartGameProtocol", "Self player device added.");
                NetworkWorld::getInstance()->m_self_kart = profile->getKartName();
                break;
            }
        }
        for (unsigned int i = 0; i < players.size(); i++)
        {
            bool is_me = (players[i]->getOnlineProfile() ==
                          PlayerManager::getCurrentOnlineProfile());
            NetworkPlayerProfile* profile = players[i];
            RemoteKartInfo rki(profile->getPlayerID(), profile->getKartName(),
                               profile->getOnlineProfile()->getUserName(),
                               profile->getPlayerID(), !is_me);
            rki.setPerPlayerDifficulty(profile->getPerPlayerDifficulty());
            rki.setGlobalPlayerId(profile->getPlayerID());
            // on the server, the race id must be the local one.
            rki.setLocalPlayerId(NetworkConfig::get()->isServer() 
                                 ? profile->getPlayerID()
                                 : (is_me ? 0 : 1)                         );
            rki.setHostId(profile->getPlayerID());
            Log::info("StartGameProtocol",
                      "Creating kart %s for Player#%d with race_id %d",
                      profile->getKartName().c_str(), i,
                      profile->getPlayerID());
            if (!is_me)
            {
                StateManager::get()->createActivePlayer( NULL, NULL );

                race_manager->setPlayerKart(i, rki);
            }
        }
        race_manager->computeRandomKartList();
        Log::info("StartGameProtocol", "Player configuration ready.");
        m_state = SYNCHRONIZATION_WAIT;
/*
        KartSelectionScreen* s = KartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(false);
        s->push();*/
    }
    else if (m_state == SYNCHRONIZATION_WAIT)
    {
        Protocol *p = ProtocolManager::getInstance()
                                     ->getProtocol(PROTOCOL_SYNCHRONIZATION);
        SynchronizationProtocol* protocol = 
                                    static_cast<SynchronizationProtocol*>(p);
        if (protocol)
        {
            // now the synchronization protocol exists.
            Log::info("StartGameProtocol", "Starting the race loading.");
            race_manager->startSingleRace("jungle", 1, false);
            World::getWorld()->setNetworkWorld(true);
            m_state = LOADING;
        }
    }
    else if (m_state == LOADING)
    {
        if (m_ready)
        {
            m_state = READY;
        }
    }
    else if (m_state == READY)
    {
        // set karts into the network game setup
        STKHost::get()->getGameSetup()->bindKartsToProfiles();
        m_state = EXITING;
        requestTerminate();
    }
}   // update

// ----------------------------------------------------------------------------
void StartGameProtocol::ready()
{
    // On clients this means the loading is finished
    if (!NetworkConfig::get()->isServer())
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
    else // on the server
    {
    }
}   // ready

