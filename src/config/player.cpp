
#include "config/player.hpp"

#include "karts/player_kart.hpp"
#include "modes/world.hpp"

ActivePlayer::ActivePlayer(PlayerProfile* player, InputDevice *device)
{
    m_player = player;
    m_device = NULL;
    setDevice(device);
}  // ActivePlayer

// ----------------------------------------------------------------------------
ActivePlayer::~ActivePlayer()
{
    setDevice(NULL);
}   // ~ActivePlayer

// ----------------------------------------------------------------------------
PlayerProfile* ActivePlayer::getProfile()
{
    return m_player;
}   // getProfile

// ----------------------------------------------------------------------------
void ActivePlayer::setPlayerProfile(PlayerProfile* player)
{
    m_player = player;
}   // setPlayerProfile

// ----------------------------------------------------------------------------
InputDevice* ActivePlayer::getDevice() const
{
    return m_device;
}   // getDevice

// ----------------------------------------------------------------------------
void ActivePlayer::setDevice(InputDevice* device)
{
    // unset player from previous device he was assigned to, if any
    if (m_device != NULL) m_device->setPlayer(NULL);
    
    m_device = device;
    
    // inform the devce of its new owner
    if (device != NULL) device->setPlayer(this);
}   // setDevice

// ----------------------------------------------------------------------------
PlayerKart* ActivePlayer::getKart()
{    
    World *world = World::getWorld();
    const int num_karts = world->getNumKarts();
    for (int p=0; p<num_karts; p++)
    {
        Kart *kart = world->getKart(p);
        if(kart->isPlayerKart() && ((PlayerKart*)kart)->getPlayer() == this)
        {
            return (PlayerKart*)kart;
        }
    }
    
    std::cout << "ActivePlayer::getKart() failed to find player named " << m_player->getName() << std::endl;
    return NULL;
}   // getKart
