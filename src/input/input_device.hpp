#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <string>
#include <iostream>
#include <fstream>

#include "config/device_config.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "io/xml_node.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/no_copy.hpp"
/**
  * \brief Input device type
  * \ingroup input
  */
enum DeviceType
{
    DT_KEYBOARD,
    DT_GAMEPAD
};

/**
  * \brief base class for input devices
  * \ingroup input
  */
class InputDevice: public NoCopy
{
    friend class DeviceManager;
protected:
    DeviceType m_type;
    StateManager::ActivePlayer* m_player;
    DeviceConfig* m_configuration;

public:
    std::string m_name; //!< if device has a name; unused for keyboards since AFAIK we can't tell keyboards apart
    
    InputDevice();
    void setConfiguration(DeviceConfig *config) {m_configuration = config;}
    DeviceConfig *getConfiguration() {return m_configuration;}

    DeviceType getType() const { return m_type; };
    
    void setPlayer(StateManager::ActivePlayer* owner);
    StateManager::ActivePlayer *getPlayer() { return m_player; }
};

/**
  * \brief specialisation of InputDevice for keyboard type devices
  * \ingroup input
  */
class KeyboardDevice : public InputDevice
{
public:

    
    KeyboardDevice();
    KeyboardDevice(KeyboardConfig *configuration);
    
    /**
     * Checks if this key belongs to this device. if yes, sets action and returns true; otherwise returns false
     *
     * \param      id      ID of the key that was pressed   
     * \param      mode    Used to determine whether to bind menu actions or game actions
     * \param[out] action  The action associated to this input (only check this value if method returned true)
     */
    bool hasBinding(const int id, InputManager::InputDriverMode mode, PlayerAction* action);
    
};

/**
  * \brief specialisation of Inputdevice for gamepad type devices
  * \ingroup input
  */
class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction, StateManager::ActivePlayer* player);
    bool m_buttonPressed[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];

public:
    Input::AxisDirection *m_prevAxisDirections;
    int                   m_deadzone;
    int                   m_index;
    int                   m_axis_count;
    int                   m_button_count;
        
    /** Constructor for GamePadDevice from a connected gamepad for which no configuration existed
      * (defaults will be used)
      *  \param irrIndex Index of stick as given by irrLicht.
      */
    GamePadDevice(const int irrIndex, const std::string name, const int axis_number,
                  const int btnAmount, GamepadConfig *configuration);
    ~GamePadDevice();
    
    bool isButtonPressed(const int i);
    void setButtonPressed(const int i, bool isButtonPressed);
    
    /**
     * \return Checks if this key belongs to this device.
     * If yes, sets action and returns true; otherwise returns false.
     *
     * \param      player  Only passed to know where to send 'axis reset's when necessary
     * \param      id      ID of the key that was pressed or of the axis that was triggered (depending on
     *                     the value of the 'type' parameter)
     * \param      mode    Used to determine whether to map menu actions or game actions
     * \param[out] action  The action associated to this input (only check this value if method returned true)
     */
    bool hasBinding(Input::InputType type, const int id, const int value, InputManager::InputDriverMode mode,
                    StateManager::ActivePlayer* player, PlayerAction* action);

};


#endif

