#include "network/protocols/start_game_protocol.hpp"

#include "config/player_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/game_setup.hpp"
#include "network/network_world.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "online/online_profile.hpp"
#include "race/race_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

StartGameProtocol::StartGameProtocol(GameSetup* game_setup)
                 : Protocol(NULL, PROTOCOL_START_GAME)
{
    m_game_setup = game_setup;
    std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        m_player_states.insert(std::pair<NetworkPlayerProfile*, STATE>(players[i], LOADING));
    }
    m_ready_count = 0;
}

StartGameProtocol::~StartGameProtocol()
{
}

bool StartGameProtocol::notifyEventAsynchronous(Event* event)
{
    NetworkString data = event->data();
    if (data.size() < 5)
    {
        Log::error("StartGameProtocol", "Too short message.");
        return true;
    }
    uint32_t token = data.gui32();
    uint8_t ready = data.gui8(4);
    STKPeer* peer = (*(event->peer));
    if (peer->getClientServerToken() != token)
    {
        Log::error("StartGameProtocol", "Bad token received.");
        return true;
    }
    if (m_listener->isServer() && ready) // on server, player is ready
    {
        Log::info("StartGameProtocol", "One of the players is ready.");
        m_player_states[peer->getPlayerProfile()] = READY;
        m_ready_count++;
        if (m_ready_count == m_game_setup->getPlayerCount())
        {
            // everybody ready, synchronize
            SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(m_listener->getProtocol(PROTOCOL_SYNCHRONIZATION));
            if (protocol)
            {
                protocol->startCountdown(5000); // 5 seconds countdown
                Log::info("StartGameProtocol", "All players ready, starting countdown.");
                m_ready = true;
                return true;
            }
            else
                Log::error("StartGameProtocol", "The Synchronization protocol hasn't been started.");
        }
    }
    else // on the client, we shouldn't even receive messages.
    {
        Log::error("StartGameProtocol", "Received a message with bad format.");
    }
    return true;
}

void StartGameProtocol::setup()
{
    m_state = NONE;
    m_ready_count = 0;
    m_ready = false;
    Log::info("SynchronizationProtocol", "Ready !");
}

bool sort_karts (NetworkPlayerProfile* a, NetworkPlayerProfile* b)
{ return (a->race_id < b->race_id); }

void StartGameProtocol::update()
{
    if (m_state == NONE)
    {
        // if no synchronization protocol exists, create one
        m_listener->requestStart(new SynchronizationProtocol());
        Log::info("StartGameProtocol", "SynchronizationProtocol started.");
        // race startup sequence
        NetworkWorld::getInstance<NetworkWorld>()->start(); // builds it and starts
        race_manager->setNumKarts(m_game_setup->getPlayerCount());
        race_manager->setNumPlayers(m_game_setup->getPlayerCount());
        race_manager->setNumLocalPlayers(1);
        std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
        std::sort(players.begin(), players.end(), sort_karts);
        // have to add self first
        for (unsigned int i = 0; i < players.size(); i++)
        {
            bool is_me = (players[i]->user_profile ==
                          PlayerManager::getCurrentOnlineProfile());
            if (is_me)
            {
                NetworkPlayerProfile* profile = players[i];
                RemoteKartInfo rki(profile->race_id, profile->kart_name,
                    profile->user_profile->getUserName(), profile->race_id, !is_me);
                rki.setGlobalPlayerId(profile->race_id);
                rki.setLocalPlayerId(is_me?0:1);
                rki.setHostId(profile->race_id);
                PlayerProfile* profile_to_use = PlayerManager::getCurrentPlayer();
                assert(profile_to_use);
                InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
                int new_player_id = 0;
                if (StateManager::get()->getActivePlayers().size() >= 1) // more than one player, we're the first
                    new_player_id = 0;
                else
                    new_player_id = StateManager::get()->createActivePlayer( profile_to_use, device);
                device->setPlayer(StateManager::get()->getActivePlayer(new_player_id));
                input_manager->getDeviceManager()->setSinglePlayer(StateManager::get()->getActivePlayer(new_player_id));

                race_manager->setPlayerKart(i, rki);
                race_manager->setLocalKartInfo(new_player_id, profile->kart_name);
                Log::info("StartGameProtocol", "Self player device added.");            // self config
                NetworkWorld::getInstance()->m_self_kart = profile->kart_name;
            }
        }
        for (unsigned int i = 0; i < players.size(); i++)
        {
            bool is_me = (players[i]->user_profile ==
                          PlayerManager::getCurrentOnlineProfile());
            NetworkPlayerProfile* profile = players[i];
            RemoteKartInfo rki(profile->race_id, profile->kart_name,
                profile->user_profile->getUserName(), profile->race_id, !is_me);
            rki.setGlobalPlayerId(profile->race_id);
            // on the server, the race id must be the local one.
            rki.setLocalPlayerId(m_listener->isServer()?profile->race_id:(is_me?0:1));
            rki.setHostId(profile->race_id);
            Log::info("StartGameProtocol", "Creating kart %s for Player#%d with race_id %d", profile->kart_name.c_str(), i, profile->race_id);

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
        SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>
            (m_listener->getProtocol(PROTOCOL_SYNCHRONIZATION));
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
        NetworkManager::getInstance()->getGameSetup()->bindKartsToProfiles();
        m_state = EXITING;
        m_listener->requestTerminate(this);
    }
}

void StartGameProtocol::ready() // on clients, means the loading is finished
{
    if (!m_listener->isServer()) // if we're a client
    {
        assert(NetworkManager::getInstance()->getPeerCount() == 1);
        NetworkString ns;
        ns.ai32(NetworkManager::getInstance()->getPeers()[0]->getClientServerToken()).ai8(1);
        Log::info("StartGameProtocol", "Player ready, notifying server.");
        m_listener->sendMessage(this, ns, true);
        m_state = READY;
        m_ready = true;
        return;
    }
    else // on the server
    {
    }
}

