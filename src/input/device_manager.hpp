#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "input/input_device.hpp"
#include "config/device_config.hpp"
#include "utils/ptr_vector.hpp"

enum PlayerAssignMode
{
    NO_ASSIGN,  // react to all devices
    DETECT_NEW, // notify the manager when an inactive device is being asked to activate with fire
    ASSIGN      // only react to assigned devices
};

class DeviceManager
{
private:

    KeyboardDevice                     *m_keyboard;
    ptr_vector<GamePadDevice, HOLD>     m_gamepads;
    ptr_vector<KeyboardConfig, HOLD>    m_keyboard_configs;
    ptr_vector<GamepadConfig, HOLD>     m_gamepad_configs;
    core::array<SJoystickInfo>          m_irrlicht_gamepads;
    InputDevice*                        m_latest_used_device;
    PlayerAssignMode                    m_assign_mode;

    // Helper functions, only used internally
    InputDevice *mapGamepadInput      ( Input::InputType type,
                                        int deviceID,
                                        int btnID,
                                        int axisDir,
                                        int value,
                                        const bool progGen,
                                        ActivePlayer **player,
                                        PlayerAction *action );

    InputDevice *mapKeyboardInput     ( int deviceID,
                                        int btnID,
                                        const bool progGen,
                                        ActivePlayer **player,
                                        PlayerAction *action );

    bool deserialize();
    void shutdown();

public:

    /**
      * The device manager starts in "no-assign" mode, which means no input configuration is associated
      * to any player. So all devices will react. This is used in menus before player set-up is done.
      * Switching back to no-assign mode will also clear anything in devices that was associated with
      * players in assign mode.
      */


    DeviceManager();
    
    void clearGamepads() { m_gamepads.clearAndDeleteAll(); }
    void clearKeyboard() {delete m_keyboard;}
    void addKeyboard(KeyboardDevice* d);
    void addGamepad(GamePadDevice* d);
    
    int getGamePadAmount() const                            { return m_gamepads.size(); }
    int getGamePadConfigAmount() const                      { return m_gamepad_configs.size(); }
    GamePadDevice*      getGamePad(const int i)             { return m_gamepads.get(i); }
    GamepadConfig*      getGamepadConfig(const int i)       { return m_gamepad_configs.get(i); }
    PlayerAssignMode    playerAssignMode() const            { return m_assign_mode; }
    KeyboardDevice*     getKeyboard(const int i)            { return m_keyboard; }
    GamePadDevice*      getGamePadFromIrrID(const int i);
    InputDevice*        getLatestUsedDevice();
    void                setAssignMode(const PlayerAssignMode assignMode);
    bool                getConfigForGamepad(const int sdl_id, GamepadConfig **config);    
       
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
                         const bool programaticallyGenerated,
                         ActivePlayer** player /* out */,
                         PlayerAction* action /* out */ );

    /* returns whether a new gamepad was detected */
    bool initialize();
    void serialize();
};


#endif

