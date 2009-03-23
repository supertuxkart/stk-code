#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <SDL/SDL.h>
#include <string>
#include "input/input.hpp"

enum DeviceType
{
    DT_KEYBOARD,
    DT_GAMEPAD
};

struct KeyBinding
{
    int id0, id1, id2;
};

class InputDevice
{
protected:
    DeviceType m_type;
    
    KeyBinding m_bindings[PA_COUNT];

public:
    InputDevice();
    
    DeviceType getType() const { return m_type; };
};

class KeyboardDevice : public InputDevice
{
public:
    KeyboardDevice();
    
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
    bool hasBinding(int id0, int id1, int id2, PlayerAction* action /* out */);
    
    void loadDefaults();
};

class GamePadDevice : public InputDevice
{
public:
    SDL_Joystick         *m_sdlJoystick;
    std::string           m_id;
    int                   m_deadzone;
    int                   m_index;
    Input::AxisDirection *m_prevAxisDirections;
    
    GamePadDevice(int sdlIndex);
    ~GamePadDevice();
};   // Stickinfo


#endif