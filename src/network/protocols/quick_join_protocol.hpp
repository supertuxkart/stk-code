#ifndef QUICK_JOIN_PROTOCOL_HPP
#define QUICK_JOIN_PROTOCOL_HPP

#include "network/protocol.hpp"


class QuickJoinProtocol : public Protocol
{
    public:
        QuickJoinProtocol(CallbackObject* callback_object, uint32_t* server_id);
        virtual ~QuickJoinProtocol();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
    protected:
        uint32_t* m_server_id;
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // QUICK_JOIN_PROTOCOL_HPP
