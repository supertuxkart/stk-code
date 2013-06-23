#ifndef HIDE_PUBLIC_ADDRESS_HPP
#define HIDE_PUBLIC_ADDRESS_HPP

#include "../protocol.hpp"
#include <string>

class HidePublicAddress : public Protocol
{
    public:
        HidePublicAddress(CallbackObject* callbackObject);
        virtual ~HidePublicAddress();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
        virtual void setUsername(std::string username);
        virtual void setPassword(std::string password);
    protected:
        std::string m_username;
        std::string m_password;
        
        enum STATE
        {
            NONE,
            DONE
        };
        STATE m_state;
};

#endif // HIDE_PUBLIC_ADDRESS_HPP
