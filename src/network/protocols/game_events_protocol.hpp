#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"


class GameEventsProtocol : public Protocol
{
    public:
        GameEventsProtocol();
        virtual ~GameEventsProtocol();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {}

    protected:
};

#endif // GAME_EVENTS_PROTOCOL_HPP
