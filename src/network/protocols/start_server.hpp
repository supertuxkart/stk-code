#ifndef START_SERVER_HPP
#define START_SERVER_HPP

#include "network/protocol.hpp"
#include "online/request.hpp"

/*!
 *  This protocol tells to the database that the server is up and running,
 *  and shows online the public IP:port that stores the NetworkManager.
 */
class StartServer : public Protocol
{
    public:
        StartServer();
        virtual ~StartServer();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
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

#endif // START_SERVER_HPP
