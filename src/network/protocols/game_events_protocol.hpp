#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"


class GameEventsProtocol : public Protocol
{
    public:
        GameEventsProtocol();
        virtual ~GameEventsProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {}

    protected:
};

#endif // GAME_EVENTS_PROTOCOL_HPP
