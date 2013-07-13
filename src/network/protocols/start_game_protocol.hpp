#ifndef START_GAME_PROTOCOL_HPP
#define START_GAME_PROTOCOL_HPP

#include "network/protocol.hpp"

class GameSetup;
class NetworkPlayerProfile;

class StartGameProtocol : public Protocol
{
    protected:
        enum STATE { LOADING, READY };
        std::vector<std::pair<NetworkPlayerProfile*, STATE> > m_player_states;

        GameSetup* m_game_setup;

        STATE m_state;

    public:
        StartGameProtocol(GameSetup* game_setup);
        virtual ~StartGameProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();

};

#endif // START_GAME_PROTOCOL_HPP
