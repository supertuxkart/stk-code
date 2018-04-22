#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class Item;

class GameEventsProtocol : public Protocol
{
public:
    enum GameEventType : uint8_t
    {
        GE_ITEM_COLLECTED     = 1,
        GE_KART_FINISHED_RACE = 2,
        GE_PLAYER_DISCONNECT = 3
    };   // GameEventType
private:
    void eliminatePlayer(const NetworkString &ns);

public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event) OVERRIDE;
    void collectedItem(Item* item, AbstractKart* kart);
    void collectedItem(const NetworkString &ns);
    void kartFinishedRace(AbstractKart *kart, float time);
    void kartFinishedRace(const NetworkString &ns);
    virtual void setup() OVERRIDE {}
    virtual void update(int ticks) OVERRIDE {};
    virtual void asynchronousUpdate() OVERRIDE{}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE 
    {
        return false;
    }   // notifyEventAsynchronous


};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
