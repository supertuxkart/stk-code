#ifndef START_GAME_PROTOCOL_HPP
#define START_GAME_PROTOCOL_HPP

#include "network/protocol.hpp"
#include <map>

class GameSetup;
class NetworkPlayerProfile;

class StartGameProtocol : public Protocol
{
protected:
    enum STATE { NONE, SYNCHRONIZATION_WAIT, LOADING, READY, EXITING };
    std::map<NetworkPlayerProfile*, STATE> m_player_states;

    GameSetup* m_game_setup;
    int m_ready_count;
    double m_sending_time;

    STATE m_state;
    bool m_ready;

public:
             StartGameProtocol(GameSetup* game_setup);
    virtual ~StartGameProtocol();

    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup();
    virtual void update();
    void ready();
    virtual void asynchronousUpdate() {}

};   // class StartGameProtocol

#endif // START_GAME_PROTOCOL_HPP
