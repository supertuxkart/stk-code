#ifndef REQUEST_CONNECTION_HPP
#define REQUEST_CONNECTION_HPP

#include "network/protocol.hpp"
#include "online/current_user.hpp"

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
        Online::CurrentUser::ServerJoinRequest* m_request;
        enum STATE
        {
            NONE,
            REQUEST_PENDING,
            DONE,
            EXITING
        };
        STATE m_state;

};

#endif // REQUEST_CONNECTION_HPP
