#include "network/network_world.hpp"

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
    World::getWorld()->updateWorld(dt);
}
