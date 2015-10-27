#ifndef CONTROLLER_EVENTS_PROTOCOL_HPP
#define CONTROLLER_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"

#include "input/input.hpp"

class Controller;
class STKPeer;

class ControllerEventsProtocol : public Protocol
{
    protected:
        std::vector<std::pair<Controller*, STKPeer*> > m_controllers;
        uint32_t m_self_controller_index;

    public:
        ControllerEventsProtocol();
        virtual ~ControllerEventsProtocol();

        virtual bool notifyEventAsynchronous(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {}

        void controllerAction(Controller* controller, PlayerAction action, int value);

};

#endif // CONTROLLER_EVENTS_PROTOCOL_HPP
