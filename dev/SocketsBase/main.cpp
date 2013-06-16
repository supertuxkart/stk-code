#include <iostream>
#include <enet/enet.h>

#include "client_network_manager.hpp"
#include "server_network_manager.hpp"
#include "protocol_manager.hpp"
#include "protocols/get_public_address.hpp"
#include <stdio.h>
#include <string.h>

using namespace std;
ProtocolManager* protocolListener;

void* foo(void* data)
{
    while(1)
    {
        protocolListener->update();
    }
    return NULL;
}

int main()
{
    std::string answer;
    cout << "host or client:";
    cin >> answer;
    if (answer == "client")
    {
        ClientNetworkManager clt;
        clt.run();
        clt.connect(0x0100007f, 7000); // addr in little endian, real address is 7f 00 00 01 (127.0.0.1)
        
        protocolListener = new ProtocolManager();
        
        pthread_t* thrd = (pthread_t*)(malloc(sizeof(pthread_t)));
        pthread_create(thrd, NULL, foo, NULL);
        
        // start a retreive stun addr protocol
        Protocol* prt = new GetPublicAddress();
        protocolListener->runProtocol(prt);
        
        char buffer[256];
        while (1)
        {
            gets(buffer);
            if (strlen(buffer) == 0) { continue; }
            clt.sendPacket(buffer);
        }
    }
    else if (answer == "host")
    {
        ServerNetworkManager srv;
        srv.run();
        srv.start();
        //srv.protocolListener = new ProtocolManager();
        while(1){}
    }
    return 0;
}
