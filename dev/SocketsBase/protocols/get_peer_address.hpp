#ifndef GET_PEER_ADDRESS_HPP
#define GET_PEER_ADDRESS_HPP

#include "../protocol.hpp"

class GetPeerAddress : public Protocol
{
    public:
        GetPeerAddress(CallbackObject* callbackObject);
        virtual ~GetPeerAddress();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
        void setPeerName(std::string peerName);
    protected:
        std::string m_peerName;
        
        enum STATE 
        {
            NONE,
            DONE
        };
        STATE m_state;
        
};

#endif // GET_PEER_ADDRESS_HPP
