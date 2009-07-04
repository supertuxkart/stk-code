#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "input/input_device.hpp"
#include "utils/ptr_vector.hpp"

class DeviceManager
{
    ptr_vector<KeyboardDevice, HOLD> m_keyboards;
    ptr_vector<GamePadDevice, HOLD> m_gamepads;
    
    core::array<SJoystickInfo> m_irrlicht_gamepads;
    
    unsigned int m_keyboard_amount;
    unsigned int m_gamepad_amount;
    
    InputDevice* m_latest_used_device;
    
    bool m_no_assign_mode;
    
public:
    DeviceManager();
    
    void add(KeyboardDevice* d);
    void add(GamePadDevice* d);
    
    int getGamePadAmount() const                            { return m_gamepad_amount; }
    GamePadDevice* getGamePad(const int i)                  { return m_gamepads.get(i); }
    GamePadDevice* getGamePadFromIrrID(const int i);
    InputDevice* getLatestUsedDevice();

    /**
      * The device manager starts in "no-assign" mode, which means no input configuration is associated
      * to any player. So all devices will react. This is used in menus before player set-up is done.
      * Switching back to no-assign mode will also clear anything in devices that was associated with
      * players in assign mode.
      */
    bool noAssignMode() const { return m_no_assign_mode; }
    void setNoAssignMode(const bool noAssignMode);
    
    int getKeyboardAmount() const                           { return m_keyboard_amount; }
    KeyboardDevice* getKeyboard(const int i)                { return m_keyboards.get(i); }
        
    /** Given some input, finds to which device it belongs and, using the corresponding device object,
        maps this input to the corresponding player and game action. returns false if player/action could not be set */
    bool mapInputToPlayerAndAction( Input::InputType type, int id0, int id1, int id2, int value, const bool programaticallyGenerated,
                                   int* player /* out */, PlayerAction* action /* out */ );
    
    void serialize();
    bool deserialize();
    
    /* returns whether a new gamepad was detected */
    bool initGamePadSupport();

    bool checkForGamePad(const int sdl_id);
};


#endif

