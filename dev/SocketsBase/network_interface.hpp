#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "singleton.hpp"
#include "types.hpp"
#include "network_manager.hpp"

#include <stdint.h>
#include <pthread.h>
#include <string>


class NetworkInterface : public Singleton<NetworkInterface>
{
    friend class Singleton<NetworkInterface>;
    public:
        
        void initNetwork(bool server);
        
    protected:
        // protected functions
        NetworkInterface();
        virtual ~NetworkInterface();
        
};

#endif // NETWORK_INTERFACE_H
