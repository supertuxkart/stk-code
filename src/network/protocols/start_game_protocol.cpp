#include "network/protocols/start_game_protocol.hpp"

#include "network/network_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/game_setup.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "online/current_online_user.hpp"

StartGameProtocol::StartGameProtocol(GameSetup* game_setup) :
        Protocol(NULL, PROTOCOL_START_GAME)
{
    m_game_setup = game_setup;
    std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        m_player_states.insert(std::pair<NetworkPlayerProfile*, STATE>(players[i], LOADING));
    }
    m_ready_count = 0;
    m_ready = NULL;
}

StartGameProtocol::~StartGameProtocol()
{
}

void StartGameProtocol::notifyEvent(Event* event)
{
    if (event->data.size() < 5)
    {
        Log::error("StartGameProtocol", "Too short message.");
        return;
    }
    uint32_t token = event->data.gui32(0);
    uint8_t ready = event->data.gui8(4);
    STKPeer* peer = (*(event->peer));
    if (peer->getClientServerToken() != token)
    {
        Log::error("StartGameProtocol", "Bad token received.");
        return;
    }
    if (m_listener->isServer() && ready) // on server, player is ready
    {
        m_player_states[peer->getPlayerProfile()] = READY;
        m_ready_count++;
        if (m_ready_count == m_game_setup->getPlayerCount())
        {
            // everybody ready, synchronize
            SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(m_listener->getProtocol(PROTOCOL_SYNCHRONIZATION));
            if (protocol)
            {
                protocol->startCountdown(m_ready, 5000); // 5 seconds countdown
                Log::info("StartGameProtocol", "All players ready, starting countdown.");
                m_state = READY;
            }
            else
                Log::error("StartGameProtocol", "The Synchronization protocol hasn't been started.");
        }
    }
    // on the client, we shouldn't even receive messages.
}

void StartGameProtocol::setup()
{
    m_state = NONE;
}

void StartGameProtocol::update()
{
    if (m_state == NONE)
    {
        // if no synchronization protocol exists, create one
        SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(m_listener->getProtocol(PROTOCOL_SYNCHRONIZATION));
        if (!protocol)
            m_listener->requestStart(new SynchronizationProtocol());
        // race startup sequence


        race_manager->setNumKarts(m_game_setup->getPlayerCount());
        race_manager->setNumPlayers(m_game_setup->getPlayerCount());
        race_manager->setNumLocalPlayers(1);
        std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
        for (unsigned int i = 0; i < players.size(); i++)
        {
            NetworkPlayerProfile* profile = players[i];
            RemoteKartInfo rki(profile->race_id, profile->kart_name, profile->user_profile->getUserName(), profile->race_id);
            rki.setGlobalPlayerId(profile->race_id);
            rki.setLocalPlayerId(profile->race_id);
            rki.setHostId(profile->race_id);
            race_manager->setPlayerKart(i, rki);

            PlayerProfile* profileToUse = unlock_manager->getCurrentPlayer();
            InputDevice* device = NULL;
            if (players[i]->user_profile == CurrentOnlineUser::get())
                device =  input_manager->getDeviceList()->getKeyboard(0);
            int new_player_id = StateManager::get()->createActivePlayer( profileToUse, device );
            // self config
        }
        Log::info("StartGameProtocol", "Players config ready. Starting single race now.");
        race_manager->startSingleRace("jungle", 1, false);
        m_state = LOADING;
/*
        KartSelectionScreen* s = KartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(false);
        StateManager::get()->pushScreen( s );*/
    }
    else if (m_state == LOADING)
    {

    }
    else if (m_state == READY)
    {
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
        m_listener->sendMessage(this, ns, true);
    }
    else // on the server
    {
    }
}

void StartGameProtocol::onReadyChange(bool* start)
{
    m_ready = start;
}
