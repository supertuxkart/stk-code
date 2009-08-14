
#include "config/player.hpp"
#include "race/race_manager.hpp"
#include "modes/world.hpp"

ActivePlayer::ActivePlayer(PlayerProfile* player, InputDevice *device)
{
    m_player = player;
    m_device = NULL;
    setDevice(device);
}
ActivePlayer::~ActivePlayer()
{
    setDevice(NULL);
}

PlayerProfile* ActivePlayer::getProfile()
{
    return m_player;
}
void ActivePlayer::setPlayerProfile(PlayerProfile* player)
{
    m_player = player;
}

InputDevice* ActivePlayer::getDevice() const
{
    return m_device;
}

void ActivePlayer::setDevice(InputDevice* device)
{
    if (m_device != NULL) m_device->setPlayer(NULL);
    
    m_device = device;
    
    if(device != NULL) device->setPlayer(this);
}

PlayerKart* ActivePlayer::getKart()
{    
    const int amount = RaceManager::getWorld()->getCurrentNumLocalPlayers();
    for (int p=0; p<amount; p++)
    {
        if (RaceManager::getWorld()->getLocalPlayerKart(p)->getPlayer() == this)
        {
            return RaceManager::getWorld()->getLocalPlayerKart(p);
        }
    }
    
    std::cout << "ActivePlayer::getKart() failed to find player named " << m_player->getName() << std::endl;
    return NULL;
}
