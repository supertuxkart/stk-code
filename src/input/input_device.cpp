
#include "input/input_device.hpp"

InputDevice::InputDevice()
{
    for(int n=0; n<PA_COUNT; n++)
    {
        m_bindings[n].id0 = 0;
        m_bindings[n].id1 = 0;
        m_bindings[n].id2 = 0;
    }
}

KeyboardDevice::KeyboardDevice()
{
}

void KeyboardDevice::loadDefaults()
{
    m_bindings[PA_NITRO].id0 = SDLK_SPACE;
    m_bindings[PA_ACCEL].id0 = SDLK_UP;
    m_bindings[PA_BRAKE].id0 = SDLK_DOWN;
    m_bindings[PA_LEFT].id0 = SDLK_LEFT;
    m_bindings[PA_RIGHT].id0 = SDLK_RIGHT;
    m_bindings[PA_DRIFT].id0 = SDLK_LSHIFT;
    m_bindings[PA_RESCUE].id0 = SDLK_ESCAPE;
    m_bindings[PA_FIRE].id0 = SDLK_LALT;
    m_bindings[PA_LOOK_BACK].id0 = SDLK_b;
}

/** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
bool KeyboardDevice::hasBinding(int id0, int id1, int id2, PlayerAction* action /* out */)
{
    for(int n=0; n<PA_COUNT; n++)
    {
        if(m_bindings[n].id0 == id0)
        {
            *action = (PlayerAction)n;
            return true;
        }
    }// next device
    
    return false;
}

// -----------------------------------------------------------------------------
/** Constructor for GamePadDevice.
 *  \param sdlIndex Index of stick.
 */
GamePadDevice::GamePadDevice(int sdlIndex)
{
    m_type = DT_GAMEPAD;
    
    
    m_sdlJoystick = SDL_JoystickOpen(sdlIndex);
    
    m_id = SDL_JoystickName(sdlIndex);
    
    const int count = SDL_JoystickNumAxes(m_sdlJoystick);
    m_prevAxisDirections = new Input::AxisDirection[count];
    
    for (int i = 0; i < count; i++)
        m_prevAxisDirections[i] = Input::AD_NEUTRAL;
    
    m_deadzone = DEADZONE_JOYSTICK;
    
    m_index = -1;
}   // GamePadDevice

// -----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
    delete m_prevAxisDirections;
    
    SDL_JoystickClose(m_sdlJoystick);
}   // ~GamePadDevice
