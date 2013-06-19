#ifndef CONNECT_TO_SERVER_HPP
#define CONNECT_TO_SERVER_HPP

#include "../protocol.hpp"
#include <string>

class ConnectToServer : public Protocol, public CallbackObject
{
    public:
        ConnectToServer(CallbackObject* callbackObject);
        virtual ~ConnectToServer();
        
        virtual void messageReceived(uint8_t* data);
        virtual void setup();
        virtual void update();
        
        void setServerAddress(uint32_t ip, uint16_t port);
        
    protected:
        uint32_t m_serverIp;
        uint16_t m_serverPort;
        
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // CONNECT_TO_SERVER_HPP
