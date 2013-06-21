#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <stdint.h>

class CallbackObject
{
    public:
        CallbackObject() {}
        
};

class TransportAddress : public CallbackObject 
{
    public:
    TransportAddress() {}
    
    uint32_t ip;
    uint16_t port;
};

class PlayerLogin : public CallbackObject 
{
    public:
    PlayerLogin() {}
    
    std::string username;
    std::string password;
};


#endif // TYPES_HPP
