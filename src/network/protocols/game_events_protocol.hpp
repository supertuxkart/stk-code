#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"

class AbstractKart;
class Item;

class GameEventsProtocol : public Protocol
{
private:
    enum GameEventType {
        GE_ITEM_COLLECTED     = 0x01,
        GE_KART_FINISHED_RACE = 0x02
    };   // GameEventType

public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event);
    virtual void setup();
    virtual void update();
    void collectedItem(Item* item, AbstractKart* kart);
    void collectedItem(const NetworkString &ns);
    void kartFinishedRace(AbstractKart *kart, float time);
    void kartFinishedRace(const NetworkString &ns);
    // ------------------------------------------------------------------------
    virtual void asynchronousUpdate() {}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) { return false; }


};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
