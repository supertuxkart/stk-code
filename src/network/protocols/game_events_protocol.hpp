#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"

class AbstractKart;
class Item;

class GameEventsProtocol : public Protocol
{
public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event);
    virtual void setup();
    virtual void update();
    void collectedItem(Item* item, AbstractKart* kart);
    // ------------------------------------------------------------------------
    virtual void asynchronousUpdate() {}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) { return false; }


};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
