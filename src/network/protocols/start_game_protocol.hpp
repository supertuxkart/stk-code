#ifndef START_GAME_PROTOCOL_HPP
#define START_GAME_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

#include <map>

class GameSetup;
class NetworkPlayerProfile;

/** This protocol runs on both server and clients.
 */
class StartGameProtocol : public Protocol
{
private:
    /** State for the finite state machine, and also for
     *  remote clients. */
    enum STATE { NONE, SYNCHRONIZATION_WAIT, 
                 LOADING, READY, EXITING };

    /** State of the finite state machine. */
    STATE m_state;

    /** Keeps the state for all clients. */
    std::map<uint8_t, STATE> m_player_states;

    /** Stores a handy pointer to the game setup structure. */
    GameSetup* m_game_setup;

    /** Counts how many clients have reported to be ready (which is then
     *  used to trigger the next state one all clients are ready). */
    int m_ready_count;

    bool m_ready;

    void startRace();

public:
             StartGameProtocol(GameSetup* game_setup);
    virtual ~StartGameProtocol();

    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    void ready();
    virtual void asynchronousUpdate() OVERRIDE {}

};   // class StartGameProtocol

#endif // START_GAME_PROTOCOL_HPP
