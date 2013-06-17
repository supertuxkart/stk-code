#ifndef STK_HOST_HPP
#define STK_HOST_HPP

#include <enet/enet.h>


class STKHost
{
    friend class STKPeer;
    public:
        enum HOST_TYPE
        {
            HOST_ANY       = 0,            
            HOST_BROADCAST = 0xFFFFFFFF,   
            PORT_ANY       = 0             
        };
    
        STKHost();
        virtual ~STKHost();
        
        static void* receive_data(void* self);
        
        void setupServer(uint32_t address, uint16_t port, int peerCount, int channelLimit, uint32_t maxIncomingBandwidth, uint32_t maxOutgoingBandwidth);
        void setupClient(int peerCount, int channelLimit, uint32_t maxIncomingBandwidth, uint32_t maxOutgoingBandwidth);
        
        void startListening();
        void stopListening();
        
        void sendRawPacket(uint8_t* data, int length, unsigned int dstIp, unsigned short dstPort);
        uint8_t* receiveRawPacket();
        uint8_t* receiveRawPacket(unsigned int dstIp, unsigned short dstPort);
        void broadcastPacket(char* data);
        
    protected:
        ENetHost* m_host;
        pthread_t* m_listeningThread;
        
};

#endif // STK_HOST_HPP
