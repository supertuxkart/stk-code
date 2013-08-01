#ifndef STOP_SERVER_HPP
#define STOP_SERVER_HPP

#include "network/protocol.hpp"
#include "online/request.hpp"

/*! \brief Removes the server info from the database
 */

class StopServer : public Protocol
{
    public:
        StopServer();
        virtual ~StopServer();

        virtual bool notifyEventAsynchronous(Event* event);
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
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

#endif // STOP_SERVER_HPP
