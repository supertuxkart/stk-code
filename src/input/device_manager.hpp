#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "input/input_device.hpp"
#include "config/device_config.hpp"
#include "utils/ptr_vector.hpp"

enum PlayerAssignMode
{
    NO_ASSIGN,  //!< react to all devices
    DETECT_NEW, //!< notify the manager when an inactive device is being asked to activate with fire
    ASSIGN      //!< only react to assigned devices
};

/**
 * This class holds the list of all known devices (ands their configurations), as well as the list
 * of currently plugged (used) devices. It thus takes care of finding to which device any given
 * input belongs, and what action each keypress is bound to, if any (and, since each device is associated
 * to a player, it also finds which player triggered this action)
 * These input mapping capabilities should *only* be used through the InputManager, not directly.
 *
 * The device manager starts in "no-assign" mode, which means no input configuration is associated
 * to any player. So all devices will react. This is used in menus before player set-up is done.
 * Switching back to no-assign mode will also clear anything in devices that was associated with
 * players in assign mode.
 */
class DeviceManager
{
private:

    ptr_vector<KeyboardDevice, HOLD>    m_keyboards;
    ptr_vector<GamePadDevice, HOLD>     m_gamepads;
    ptr_vector<KeyboardConfig, HOLD>    m_keyboard_configs;
    ptr_vector<GamepadConfig, HOLD>     m_gamepad_configs;
    core::array<SJoystickInfo>          m_irrlicht_gamepads;
    InputDevice*                        m_latest_used_device;
    PlayerAssignMode                    m_assign_mode;

    /**
      * Helper method, only used internally. Takes care of analyzing gamepad input.
      *
      * \param[out]  player     Which player this input belongs to (only set in ASSIGN mode)
      * \param[out]  action     Which action is related to this input trigger
      * \return                 The device to which this input belongs
      */
    InputDevice *mapGamepadInput      ( Input::InputType type,
                                        int deviceID,
                                        int btnID,
                                        int axisDir,
                                        int value,
                                        ActivePlayer **player /* out */,
                                        PlayerAction *action /* out */);

    /**
     * Helper method, only used internally. Takes care of analyzing keyboard input.
     *
     * \param[out]  player     Which player this input belongs to (only set in ASSIGN mode)
     * \param[out]  action     Which action is related to this input trigger
     * \return                 The device to which this input belongs
     */
    InputDevice *mapKeyboardInput     ( int btnID,
                                        ActivePlayer **player /* out */,
                                        PlayerAction *action /* out */);

    bool deserialize();
    void shutdown();

public:


    DeviceManager();
        
    // ---- Assign mode ----
    PlayerAssignMode    getAssignMode() const               { return m_assign_mode; }
    void                setAssignMode(const PlayerAssignMode assignMode);

    // ---- Gamepads ----
    void addGamepad(GamePadDevice* d);
    int getGamePadAmount() const                            { return m_gamepads.size(); }
    int getGamePadConfigAmount() const                      { return m_gamepad_configs.size(); }
    GamePadDevice*      getGamePad(const int i)             { return m_gamepads.get(i); }
    GamepadConfig*      getGamepadConfig(const int i)       { return m_gamepad_configs.get(i); }
    GamePadDevice*      getGamePadFromIrrID(const int i);
    void                clearGamepads()                     { m_gamepads.clearAndDeleteAll();  }
    /** Returns the keyboard that has a binding for this button, or NULL if none */
    bool                getConfigForGamepad(const int sdl_id, GamepadConfig **config);    
    
    // ---- Keyboard(s) ----
    void addKeyboard(KeyboardDevice* d);
    int getKeyboardConfigAmount() const                     { return m_keyboard_configs.size(); }
    KeyboardDevice*     getKeyboard(const int i)            { return m_keyboards.get(i); }
    KeyboardConfig*     getKeyboardConfig(const int i)      { return m_keyboard_configs.get(i); }
    void                clearKeyboard()                     { m_keyboards.clearAndDeleteAll(); }
    KeyboardDevice*     getKeyboardFromBtnID(const int btnID);



       
    /** Given some input, finds to which device it belongs and, using the corresponding device object,
      * maps this input to the corresponding player and game action. returns false if player/action could not be set.
      * Special case : can return true but set action to PA_FIRST if the input was used but is not associated to an
      * action and a player
      */
    bool translateInput( Input::InputType type,
                         int deviceID,
                         int btnID,
                         int axisDir,
                         int value,
                         ActivePlayer** player /* out */,
                         PlayerAction* action /* out */ );

    InputDevice*        getLatestUsedDevice();
    bool initialize();
    void serialize();
};


#endif

