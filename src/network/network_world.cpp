#include "network/network_world.hpp"

#include "network/protocol_manager.hpp"
#include "network/protocols/synchronization_protocol.hpp"
#include "modes/world.hpp"

NetworkWorld::NetworkWorld()
{
    m_running = false;
}

NetworkWorld::~NetworkWorld()
{
}

void NetworkWorld::update(float dt)
{
    SynchronizationProtocol* protocol = static_cast<SynchronizationProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_SYNCHRONIZATION));
    if (protocol) // if this protocol exists, that's that we play online
    {
        Log::info("NetworkWorld", "Coutdown value is %d", protocol->getCountdown());
        if (protocol->getCountdown() > 0)
        {
            return;
        }
        else
        {
            // kill those two protocols
            ProtocolManager::getInstance()->requestStop(protocol);
            ProtocolManager::getInstance()->requestStop(ProtocolManager::getInstance()->getProtocol(PROTOCOL_START_GAME));
        }
    }
    World::getWorld()->updateWorld(dt);
}
