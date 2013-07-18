#ifndef STOP_SERVER_HPP
#define STOP_SERVER_HPP

#include "network/protocol.hpp"


/*! \brief Removes the server info from the database
 */

class StopServer : public Protocol
{
    public:
        StopServer();
        virtual ~StopServer();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
        enum STATE
        {
            NONE,
            DONE,
            EXITING
        };
        STATE m_state;
};

#endif // STOP_SERVER_HPP
