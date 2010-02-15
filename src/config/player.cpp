
#include "config/player.hpp"

#include "karts/controller/player_controller.hpp"
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

