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
    ActivePlayer* m_player;
    DeviceConfig* m_configuration;

public:
    std::string m_name; // if device has a name; unused for keyboards since AFAIK we can't tell keyboards apart
    
    InputDevice();
    void setConfiguration(DeviceConfig *config) {m_configuration = config;}
    DeviceConfig *getConfiguration() {return m_configuration;}

    DeviceType getType() const { return m_type; };
    
    void setPlayer(ActivePlayer* owner);
    ActivePlayer *getPlayer() {return m_player;}
    
    /**
      * returns a human-readable string for the key binded with the given action
      */
};

class KeyboardDevice : public InputDevice
{
public:

    
    KeyboardDevice();
    KeyboardDevice(KeyboardConfig *configuration);
    
    /**
     * Checks if this key belongs to this device. if yes, sets action and returns true; otherwise returns false
     *
     * \param      id      ID of the key that was pressed   
     * \param[out] action  The action associated to this input (only check this value if method returned true)
     */
    bool hasBinding(const int id, PlayerAction* action);
    
};

class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction, ActivePlayer* player);
    bool m_buttonPressed[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];

public:
    Input::AxisDirection *m_prevAxisDirections;
    int                   m_deadzone;
    int                   m_index;
    int                   m_axis_count;
    int                   m_button_count;
        
    GamePadDevice(const int irrIndex, const std::string name, const int axis_number, const int btnAmount, GamepadConfig *configuration);
    ~GamePadDevice();
    
    bool isButtonPressed(const int i);
    void setButtonPressed(const int i, bool isButtonPressed);
    
    /**
     * Checks if this key belongs to this device. if yes, sets action and returns true; otherwise returns false.
     *
     * \param      player  Only passed to know where to send 'axis reset's when necessary
     * \param      id      ID of the key that was pressed or of the axis that was triggered (depending on
     *                     the value of the 'type' parameter)
     * \param[out] action  The action associated to this input (only check this value if method returned true)
     */
    bool hasBinding(Input::InputType type, const int id, const int value, ActivePlayer* player, PlayerAction* action);

};


#endif

