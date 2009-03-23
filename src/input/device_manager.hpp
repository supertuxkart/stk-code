#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "ptr_vector.hpp"
#include "input/input_device.hpp"

class DeviceManager
{
    ptr_vector<KeyboardDevice, HOLD> m_keyboards;
    ptr_vector<GamePadDevice, HOLD> m_gamepads;
    
    unsigned int m_keyboard_amount;
    unsigned int m_gamepad_amount;
    
public:
    DeviceManager();
    
    void add(KeyboardDevice* d);
    void add(GamePadDevice* d);
    
    /** Given some input, finds to which device it belongs and, using the corresponding device object,
        maps this input to the corresponding player and game action. returns false if player/action could not be set */
    bool mapInputToPlayerAndAction( Input::InputType type, int id0, int id1, int id2,
                                   int* player /* out */, PlayerAction* action /* out */ );
};


#endif