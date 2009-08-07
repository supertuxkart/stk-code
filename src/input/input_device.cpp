
#include "states_screens/state_manager.hpp"
#include "config/device_config.hpp"
#include "input/input.hpp"
#include "input/input_device.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"


InputDevice::InputDevice()
{
    m_player = NULL;
    m_configuration = NULL;
}
// -----------------------------------------------------------------------------
/**
  * Sets which players uses this device; or pass NULL to say no player uses it. 
  */
void InputDevice::setPlayer(ActivePlayer* owner)
{
    m_player = owner;
}

// -----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice(KeyboardConfig *configuration)
{
    m_configuration = configuration;
    m_type = DT_KEYBOARD;
}
// -----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice()
{
    m_configuration = new KeyboardConfig();
    m_type = DT_KEYBOARD;
}

// -----------------------------------------------------------------------------
bool KeyboardDevice::hasBinding(const int id, PlayerAction* action)
{
    return m_configuration->getAction(Input::IT_KEYBOARD, id, 0, action);
}
// -----------------------------------------------------------------------------


#if 0
#pragma mark -
#pragma mark gamepad
#endif


/** Constructor for GamePadDevice from a connected gamepad for which no configuration existed
* (defaults will be used)
 *  \param sdlIndex Index of stick.
 */
GamePadDevice::GamePadDevice(const int irrIndex, const std::string name, const int axis_count, const int btnAmount, GamepadConfig *configuration)
{
    m_type                  = DT_GAMEPAD;
    m_deadzone              = DEADZONE_JOYSTICK;
    m_prevAxisDirections    = NULL;
    m_configuration         = configuration;
    m_axis_count            = axis_count;
    m_prevAxisDirections    = new Input::AxisDirection[axis_count];
    m_button_count          = btnAmount;
    m_index                 = irrIndex;    
    m_name                  = name;
    
    for (int i = 0; i < axis_count; i++)
        m_prevAxisDirections[i] = Input::AD_NEUTRAL;

    for(int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; n++)
        m_buttonPressed[n] = false;
}   // GamePadDevice
// -----------------------------------------------------------------------------

bool GamePadDevice::isButtonPressed(const int i)
{
    return m_buttonPressed[i];
}
void GamePadDevice::setButtonPressed(const int i, bool isButtonPressed)
{
    m_buttonPressed[i] = isButtonPressed;
}
// -----------------------------------------------------------------------------

void GamePadDevice::resetAxisDirection(const int axis, Input::AxisDirection direction, ActivePlayer* player)
{
    KeyBinding bind;
    if(!StateManager::get()->isGameState()) return; // ignore this while in menus

    PlayerKart* pk = player->getKart();
    if (pk == NULL)
    {
        std::cerr << "Error, trying to reset axis for an unknown player\n";
        return;
    }
    
    for(int n=0; n<PA_COUNT; n++)
    {
        bind = m_configuration->getBinding(n);
        if(bind.id == axis && bind.dir == direction)
        {
            pk->action((PlayerAction)n, 0);
            return;
        }
    }

}
// -----------------------------------------------------------------------------

/**
  * Player ID can either be a player ID or -1. If -1, the method only returns whether a binding exists for this player.
  * If it's a player name, it also handles axis resets, direction changes, etc.
  */


bool GamePadDevice::hasBinding(Input::InputType type, const int id, const int value, ActivePlayer* player, PlayerAction* action /* out */)
{
    bool success = false;
    if(m_prevAxisDirections == NULL) return false; // device not open
    
    if(type == Input::IT_STICKMOTION)
    {
        if(id >= m_axis_count) return false; // this gamepad doesn't even have that many axes

        if (player != NULL)
        {
            // going to negative from positive
            if (value < 0 && m_prevAxisDirections[id] == Input::AD_POSITIVE)
            {
                //  set positive id to 0
                resetAxisDirection(id, Input::AD_POSITIVE, player);
            }
            // going to positive from negative
            else if (value > 0 && m_prevAxisDirections[id] == Input::AD_NEGATIVE)
            {
                //  set negative id to 0
                resetAxisDirection(id, Input::AD_NEGATIVE, player);
            }
        }

        if(value > 0) m_prevAxisDirections[id] = Input::AD_POSITIVE;
        else if(value < 0) m_prevAxisDirections[id] = Input::AD_NEGATIVE;

        // check if within deadzone
        if(value > -m_deadzone && value < m_deadzone && player != NULL)
        {
            // Axis stands still: This is reported once for digital axes and
            // can be called multipled times for analog ones. Uses the
            // previous direction in which the id was triggered to
            // determine which one has to be brought into the released
            // state. This allows us to regard two directions of an id
            // as completely independent input variants (as if they where
            // two buttons).

            if(m_prevAxisDirections[id] == Input::AD_NEGATIVE)
            {
                // set negative id to 0
                resetAxisDirection(id, Input::AD_NEGATIVE, player);
            }
            else if(m_prevAxisDirections[id] == Input::AD_POSITIVE)
            {
                // set positive id to 0
                resetAxisDirection(id, Input::AD_POSITIVE, player);
            }
            m_prevAxisDirections[id] = Input::AD_NEUTRAL;

            return false;
        }
    }

    if (m_configuration != NULL)
    {
        success = m_configuration->getAction(type, id, value, action);
    }
    else
    {
        printf("hasBinding() called on improperly initialized GamePadDevice\n");
        abort();
    }

    return success;
}
// -----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
    delete[] m_prevAxisDirections;

    // FIXME - any need to close devices?
}   // ~GamePadDevice
