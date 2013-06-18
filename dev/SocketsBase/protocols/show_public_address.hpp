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
        virtual void start();
        virtual void pause();
        virtual void unpause();
        virtual void update();
        
        virtual void setNickname(std::string nickname);
        virtual void setPassword(std::string password);
    protected:
        std::string m_nickname;
        std::string m_password;
        
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // HIDE_PUBLIC_ADDRESS_HPP
