#include "protocol_manager.hpp"

#include "protocol.hpp"
#include <assert.h>

ProtocolManager::ProtocolManager() : ProtocolListener()
{
}

ProtocolManager::~ProtocolManager()
{
}

void ProtocolManager::messageReceived(uint8_t* data)
{
    assert(data);
    m_messagesToProcess.push_back(data);
}

void ProtocolManager::runProtocol(Protocol* protocol)
{
    m_protocols.push_back(protocol);
    protocol->setup();
    protocol->start();
}
void ProtocolManager::stopProtocol(Protocol* protocol)
{
    
}

void ProtocolManager::update()
{
    // before updating, notice protocols that they have received information
    int size = m_messagesToProcess.size();
    for (int i = 0; i < size; i++)
    {
        uint8_t* data = m_messagesToProcess.back();
        PROTOCOL_TYPE searchedProtocol = (PROTOCOL_TYPE)(data[0]);
        for (unsigned int i = 0; i < m_protocols.size() ; i++)
        {
            if (m_protocols[i]->getProtocolType() == searchedProtocol)
                m_protocols[i]->messageReceived(data+1);
        }
        m_messagesToProcess.pop_back();
    }
    // now update all protocols
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        m_protocols[i]->update();
    }
}
