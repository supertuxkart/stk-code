#ifndef REQUEST_CONNECTION_HPP
#define REQUEST_CONNECTION_HPP

#include "network/protocol.hpp"

class RequestConnection : public Protocol
{
    public:
        RequestConnection(uint32_t server_id);
        virtual ~RequestConnection();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
        uint32_t m_server_id;

        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;

};

#endif // REQUEST_CONNECTION_HPP
