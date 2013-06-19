#ifndef SHOW_PUBLIC_ADDRESS_HPP
#define SHOW_PUBLIC_ADDRESS_HPP

#include "../protocol.hpp"
#include <string>

class ShowPublicAddress : public Protocol
{
    public:
        ShowPublicAddress(CallbackObject* callbackObject);
        virtual ~ShowPublicAddress();
        
        virtual void messageReceived(uint8_t* data);
        virtual void setup();
        virtual void update();
        
        virtual void setUsername(std::string username);
        virtual void setPassword(std::string password);
        virtual void setPublicAddress(uint32_t ip, uint16_t port);
        
    protected:
        std::string m_username;
        std::string m_password;
        uint32_t m_publicIp;
        uint16_t m_publicPort;
        
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // HIDE_PUBLIC_ADDRESS_HPP
