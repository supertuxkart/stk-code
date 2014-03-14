#ifndef QUICK_JOIN_PROTOCOL_HPP
#define QUICK_JOIN_PROTOCOL_HPP

#include "network/protocol.hpp"

namespace Online { class XMLRequest; }

class QuickJoinProtocol : public Protocol
{
    public:
        QuickJoinProtocol(CallbackObject* callback_object, uint32_t* server_id);
        virtual ~QuickJoinProtocol();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
        uint32_t* m_server_id;
        Online::XMLRequest* m_request;
        enum STATE
        {
            NONE,
            REQUEST_PENDING,
            DONE,
            EXITING
        };
        STATE m_state;
};

#endif // QUICK_JOIN_PROTOCOL_HPP
