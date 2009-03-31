#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <SDL/SDL.h>
#include <string>
#include "input/input.hpp"
#include <iostream>
#include <fstream>
#include "io/xml_node.hpp"

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
    std::string m_name; // if device has a name; unused for keyboards since SDL can't tell keyboards apart
    
    InputDevice();
    
    DeviceType getType() const { return m_type; };
    
    void serialize(std::ofstream& stream);
    bool deserializeAction(irr::io::IrrXMLReader* xml);
};

class KeyboardDevice : public InputDevice
{
public:
    KeyboardDevice();
    KeyboardDevice(irr::io::IrrXMLReader* xml);
    
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
    bool hasBinding(const int key_id, PlayerAction* action /* out */) const;
    
    void loadDefaults();
};

class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction, const int player);
public:
    SDL_Joystick         *m_sdlJoystick;
    int                   m_deadzone;
    int                   m_index;
    Input::AxisDirection *m_prevAxisDirections;
        
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false.
     The 'player' id passed is simply to know where to send 'axis reset's when necessary*/
    bool hasBinding(const int axis, const int value, const int player, PlayerAction* action /* out */);
    
    void open(const int sdl_id);
    
    void loadDefaults();
    
    GamePadDevice(int sdlIndex);
    GamePadDevice(irr::io::IrrXMLReader* xml);
    
    ~GamePadDevice();
};


#endif

