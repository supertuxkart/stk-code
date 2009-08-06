#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <string>
#include "input/input.hpp"
#include "config/device_config.hpp"
#include <iostream>
#include <fstream>
#include "io/xml_node.hpp"

class ActivePlayer;

enum DeviceType
{
    DT_KEYBOARD,
    DT_GAMEPAD
};

class InputDevice
{
    friend class DeviceManager;
protected:
    DeviceType m_type;
    KeyBinding m_default_bindings[PA_COUNT];

    ActivePlayer* m_player;

public:
    std::string m_name; // if device has a name; unused for keyboards since AFAIK we can't tell keyboards apart
    
    InputDevice();
    
    DeviceType getType() const { return m_type; };
    
    void setPlayer(ActivePlayer* owner);
    
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
    KeyboardConfig  *m_configuration;

    KeyboardDevice();
    KeyboardDevice(KeyboardConfig *configuration);
    KeyboardDevice(irr::io::IrrXMLReader* xml);
    
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
    bool hasBinding(const int key_id, PlayerAction* action /* out */) const;
    
    void editBinding(PlayerAction action, int key_id);
    
    void loadDefaults();
};

class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction, ActivePlayer* player);
    bool m_buttonPressed[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];

public:
    GamepadConfig        *m_configuration;
    int                   m_deadzone;
    int                   m_index;
    int                   m_axis_count;
    int                   m_button_count;
    Input::AxisDirection *m_prevAxisDirections;
        
    /** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false.
     The 'player' id passed is simply to know where to send 'axis reset's when necessary*/
    bool hasBinding(Input::InputType type, const int id, const int value, ActivePlayer* player, PlayerAction* action /* out */);
    
    void editBinding(const PlayerAction action, const Input::InputType type, const int id,
                     Input::AxisDirection direction=Input::AD_NEUTRAL);
    
    void open(const int irrIndex, const std::string name, const int axis_count, const int btnCount);
    
    void loadDefaults();
    
    GamePadDevice(const int irrIndex, const std::string name, const int axis_number, const int btnAmount, GamepadConfig *configuration);
    GamePadDevice(irr::io::IrrXMLReader* xml);
    
    bool isButtonPressed(const int i);
    void setButtonPressed(const int i, bool isButtonPressed);
    
    ~GamePadDevice();
};


#endif

