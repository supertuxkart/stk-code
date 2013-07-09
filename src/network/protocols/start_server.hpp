#ifndef START_SERVER_HPP
#define START_SERVER_HPP

#include "network/protocol.hpp"

/*!
 *  This protocol tells to the database that the server is up and running, 
 *  and shows online the public IP:port that stores the NetworkManager.
 */
class StartServer : public Protocol
{
    public:
        StartServer();
        virtual ~StartServer();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
    protected:
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // START_SERVER_HPP
