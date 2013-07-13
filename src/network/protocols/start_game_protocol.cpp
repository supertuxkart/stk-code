#include "network/protocols/start_game_protocol.hpp"

#include "network/game_setup.hpp"

StartGameProtocol::StartGameProtocol(GameSetup* game_setup) :
        Protocol(NULL, PROTOCOL_START_GAME)
{
    m_game_setup = game_setup;
    std::vector<NetworkPlayerProfile*> players = m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        m_player_states.push_back(std::pair<NetworkPlayerProfile*, STATE>(players[i], LOADING));
    }
}

StartGameProtocol::~StartGameProtocol()
{
}

void StartGameProtocol::notifyEvent(Event* event)
{
}

void StartGameProtocol::setup()
{
    m_state = LOADING;
}

void StartGameProtocol::update()
{
    if (m_state == LOADING)
    {
    }
    else if (m_state == READY)
    {
    }
}
