#include <iostream>
#include <enet/enet.h>

#include "client_network_manager.hpp"
#include "server_network_manager.hpp"
#include "protocol_manager.hpp"
#include "protocols/get_public_address.hpp"
#include "http_functions.hpp"
#include "protocols/connect_to_server.hpp"
#include "protocols/hide_public_address.hpp"

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
    HTTP::init();
    
    std::string answer;
    cout << "host or client:";
    answer = "client";
    cin >> answer;
    if (answer == "client")
    {
        protocolListener = new ProtocolManager();
        ClientNetworkManager clt;
        clt.run();
        
        /// NICKNAME :
        std::string nickname;
        cout << "Nickname=";
        std::cin >> nickname;
        /// PASSWORD :
        std::string password;
        cout << "Password=";
        std::cin >> password;
        /// HOST NICKNAME : 
        std::string hostNickname;
        cout << "Host Nickname=";
        std::cin >> hostNickname;
        ConnectToServer* connectionProtocol = new ConnectToServer(&clt);
        connectionProtocol->setPassword(password);
        connectionProtocol->setUsername(nickname);
        connectionProtocol->setHostName(hostNickname);
        
        pthread_t* thrd = (pthread_t*)(malloc(sizeof(pthread_t)));
        pthread_create(thrd, NULL, foo, NULL);
        
        // start a retreive stun addr protocol
        Protocol* prt = new GetPublicAddress(connectionProtocol);
        prt->setListener(protocolListener);
        connectionProtocol->setListener(protocolListener);
        
        protocolListener->startProtocol(prt);

        //clt.connect(0x0100007f, 7000); // addr in little endian, real address is 7f 00 00 01 (127.0.0.1)
        bool* connected = &connectionProtocol->connected;
        std::string buffer;
        while (1)
        {
            cin >> buffer;
            if (buffer == "cmd=protocolsCount")
            {
                cout << protocolListener->runningProtocolsCount() << " protocols are running." << endl;
                continue;
            }
            if (buffer == "cmd=hideAddress")
            {
                HidePublicAddress* hideipv4 = new HidePublicAddress(NULL);
                hideipv4->setPassword(password);
                hideipv4->setNickname(nickname);
                protocolListener->startProtocol(hideipv4);
            }
            if (buffer == "cmd=login")
            {
                std::cout << "Username=";
                std::cin >> nickname;
                connectionProtocol->setUsername(nickname);
                std::cout << "Password=";
                std::cin >> password;
                connectionProtocol->setPassword(password);
                connectionProtocol->unpause();
            }
            if (buffer.size() == 0) { continue; }
            char buffer2[256];
            strcpy(buffer2, buffer.c_str());
            if (*connected)
            {
                clt.sendPacket(buffer2);
            }
        }
        
        
        enet_deinitialize();
    }
    else if (answer == "host")
    {
        ServerNetworkManager srv;
        srv.run();
        srv.start();
        srv.protocolListener = new ProtocolManager();
        
        GetPublicAddress
        
        while(1){}
    }
    return 0;
}
