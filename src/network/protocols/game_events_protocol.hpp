#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;

class GameEventsProtocol : public Protocol
{
public:
    enum GameEventType : uint8_t
    {
        GE_KART_FINISHED_RACE = 1,
        GE_STARTUP_BOOST = 2,
        GE_BATTLE_KART_SCORE = 3,
        GE_CTF_SCORED = 4,
        GE_RESET_BALL = 5,
        GE_PLAYER_GOAL = 6,
        GE_CHECK_LINE = 7
    };   // GameEventType
private:
    int m_last_finished_position;

    void eliminatePlayer(const NetworkString &ns);

public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event) OVERRIDE;
    void kartFinishedRace(AbstractKart *kart, float time);
    void kartFinishedRace(const NetworkString &ns);
    void sendStartupBoost(uint8_t kart_id);
    virtual void setup() OVERRIDE {}
    virtual void update(int ticks) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE
    {
        return false;
    }   // notifyEventAsynchronous

};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
