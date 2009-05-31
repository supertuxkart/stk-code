#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

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
    std::string m_name; // if device has a name; unused for keyboards since AFAIK we can't tell keyboards apart
    
    InputDevice();
    
    DeviceType getType() const { return m_type; };
    
    /**
      * returns a human-readable string for the key binded with the given action
      */
    std::string getBindingAsString(const PlayerAction action) const;

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
    
    void editBinding(PlayerAction action, int key_id);
    
    void loadDefaults();
};

class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction, const int player);
    bool m_buttonPressed[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];
public:
    int                   m_deadzone;
    int                   m_index;
    int                   m_axis_count;
    int                   m_button_count;
    Input::AxisDirection *m_prevAxisDirections;
        
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false.
     The 'player' id passed is simply to know where to send 'axis reset's when necessary*/
    bool hasBinding(Input::InputType type, const int id, const int value, const int player, PlayerAction* action /* out */);
    
    void editBinding(const PlayerAction action, const Input::InputType type, const int id,
                     Input::AxisDirection direction=Input::AD_NEUTRAL);
    
    void open(const int irrIndex, const std::string name, const int axis_count, const int btnCount);
    
    void loadDefaults();
    
    GamePadDevice(const int irrIndex, const std::string name, const int axis_number, const int btnAmount);
    GamePadDevice(irr::io::IrrXMLReader* xml);
    
    bool isButtonPressed(const int i);
    void setButtonPressed(const int i, bool isButtonPressed);
    
    ~GamePadDevice();
};


#endif

