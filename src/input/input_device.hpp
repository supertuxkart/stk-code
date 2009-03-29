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
    Input::InputType type;
    
    // key for keyboards, axis for gamepads
    int id;
    
    Input::AxisDirection dir; // for gamepads
};

class InputDevice
{
protected:
    DeviceType m_type;
    
    KeyBinding m_bindings[PA_COUNT];

    std::string m_player;
    
public:
    InputDevice();
    
    DeviceType getType() const { return m_type; };
};

class KeyboardDevice : public InputDevice
{
public:
    KeyboardDevice();
    
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
    bool hasBinding(const int key_id, PlayerAction* action /* out */) const;
    
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
        
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
    bool hasBinding(const int axis, const int value, PlayerAction* action /* out */) const;
    
    void loadDefaults();
    
    GamePadDevice(int sdlIndex);
    ~GamePadDevice();
};   // Stickinfo


#endif