#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <vector>

class Packet
{
    public:
        Packet();
        virtual ~Packet();
        
        virtual std::string getData();
        
    protected:
        char*                   m_packetHeader;
        unsigned int            m_packetHeaderSize;
        unsigned int            m_packetSize;
        Packet*                 m_parent;
        std::vector<Packet*>    m_children;
};

#endif // PACKET_HPP
