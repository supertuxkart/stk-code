
#include "states_screens/state_manager.hpp"
#include "input/input.hpp"
#include "input/input_device.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"

InputDevice::InputDevice()
{
    for(int n=0; n<PA_COUNT; n++)
    {
        m_default_bindings[n].id = -1;
        m_default_bindings[n].type = Input::IT_NONE;
        m_default_bindings[n].dir = Input::AD_NEUTRAL;
    }
    m_player = NULL;
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
void InputDevice::serialize(std::ofstream& stream)
{
    if (m_type == DT_KEYBOARD) stream << "<keyboard>\n\n";
    else if (m_type == DT_GAMEPAD) stream << "<gamepad name=\"" << m_name.c_str() << "\" >\n\n";
    else std::cerr << "Warning, unknown input device type, skipping it\n";

   // stream << "owner=\"" << m_player << "\">\n\n";

    
    for(int n=0; n<PA_COUNT; n++)
    {
        stream << "    <action name=\"" << KartActionStrings[n] <<  "\" id=\""
            << m_default_bindings[n].id << "\" event=\"" << m_default_bindings[n].type << "\" ";

        if (m_type == DT_GAMEPAD) stream << "direction=\"" << m_default_bindings[n].dir << "\"";

        stream << "/>\n";
    }

    if (m_type == DT_KEYBOARD) stream << "\n</keyboard>\n\n\n";
    else if (m_type == DT_GAMEPAD) stream << "\n</gamepad>\n\n\n";
}
// -----------------------------------------------------------------------------
bool InputDevice::deserializeAction(irr::io::IrrXMLReader* xml)
{
    // ---- read name
    const char* name_string = xml->getAttributeValue("name");
    if(name_string == NULL) return false;

    int binding_id = -1;

    for(int n=0; n<PA_COUNT; n++)
    {
        if(strcmp(name_string,KartActionStrings[n].c_str()) == 0)
        {
            binding_id = n;
            break;
        }
    }
    if(binding_id == -1)
    {
        std::cerr << "Unknown action type : " << name_string << std::endl;
        return false;
    }

    // ---- read id
    const char* id_string = xml->getAttributeValue("id");
    if(id_string == NULL) return false;
    const int id = atoi(id_string);

    // ---- read event type
    const char* event_string = xml->getAttributeValue("event");
    if(event_string == NULL) return false;
    const int event_id = atoi(event_string);

    m_default_bindings[binding_id].id = id;
    m_default_bindings[binding_id].type = (Input::InputType)event_id;


    // ---- read axis direction
    const char* dir_string = xml->getAttributeValue("direction");
    if(dir_string != NULL)
    {
        const int dir = atoi(dir_string);
        m_default_bindings[binding_id].dir = (Input::AxisDirection)dir;
    }

    return true;

}
// -----------------------------------------------------------------------------
std::string InputDevice::getBindingAsString(const PlayerAction action) const
{
    return Input::getInputAsString(m_default_bindings[action].type, m_default_bindings[action].id, m_default_bindings[action].dir);
}

#if 0
#pragma mark -
#pragma mark Keyboard
#endif

// -----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice()
{
    m_type = DT_KEYBOARD;
}
// -----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice(irr::io::IrrXMLReader* xml)
{
    m_type = DT_KEYBOARD;

}
// -----------------------------------------------------------------------------
void KeyboardDevice::loadDefaults()
{
    m_default_bindings[PA_NITRO].id = KEY_KEY_N;
    m_default_bindings[PA_ACCEL].id = KEY_UP;
    m_default_bindings[PA_BRAKE].id = KEY_DOWN;
    m_default_bindings[PA_LEFT].id = KEY_LEFT;
    m_default_bindings[PA_RIGHT].id = KEY_RIGHT;
    m_default_bindings[PA_DRIFT].id = KEY_KEY_V;
    m_default_bindings[PA_RESCUE].id = KEY_BACK;
    m_default_bindings[PA_FIRE].id = KEY_SPACE;
    m_default_bindings[PA_LOOK_BACK].id = KEY_KEY_B ;

    m_default_bindings[PA_NITRO].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_ACCEL].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_BRAKE].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_LEFT].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_RIGHT].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_DRIFT].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_RESCUE].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_FIRE].type = Input::IT_KEYBOARD;
    m_default_bindings[PA_LOOK_BACK].type = Input::IT_KEYBOARD;
}
// -----------------------------------------------------------------------------
void KeyboardDevice::editBinding(PlayerAction action, int key_id)
{
    m_default_bindings[action].id = key_id;
}
// -----------------------------------------------------------------------------
/** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
bool KeyboardDevice::hasBinding(const int key_id, PlayerAction* action /* out */) const
{
    for(int n=0; n<PA_COUNT; n++)
    {
        if(m_default_bindings[n].id == key_id)
        {
            *action = (PlayerAction)n;
            return true;
        }
    }// next device

    return false;
}
// -----------------------------------------------------------------------------


#if 0
#pragma mark -
#pragma mark gamepad
#endif

/**
 * Creates a GamePade device from a config file. Note that this device will not yet be ready to be used,
 * it must first be detected to be connected by irrLicht, to be properly initialized
 */
GamePadDevice::GamePadDevice(irr::io::IrrXMLReader* xml)
{
    m_type = DT_GAMEPAD;
    m_prevAxisDirections = NULL;
    m_deadzone = DEADZONE_JOYSTICK;

    const char* name_string = xml->getAttributeValue("name");
    if(name_string == NULL)
    {
        std::cerr << "Warning, joystick without name in config file, making it undetectable\n";
    }
    else m_name = name_string;
    m_index = -1; // Set to -1 so we can establish when a device ID has been associated
    
    for(int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; n++)
        m_buttonPressed[n] = false;
}
// -----------------------------------------------------------------------------
/** Constructor for GamePadDevice from a connected gamepad for which no configuration existed
* (defaults will be used)
 *  \param sdlIndex Index of stick.
 */
GamePadDevice::GamePadDevice(const int irrIndex, const std::string name, const int axis_count, const int btnAmount)
{
    m_type = DT_GAMEPAD;
    m_deadzone = DEADZONE_JOYSTICK;
    m_prevAxisDirections = NULL;

    open(irrIndex, name, axis_count, btnAmount);
    m_name = name;

    loadDefaults();
    
    for(int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; n++)
        m_buttonPressed[n] = false;
}   // GamePadDevice
// -----------------------------------------------------------------------------
void GamePadDevice::open(const int irrIndex, const std::string name, const int axis_count, const int btnCount)
{
    m_axis_count = axis_count;
    m_prevAxisDirections = new Input::AxisDirection[axis_count];
    m_button_count = btnCount;
    
    std::cout << "(i) This gamepad has " << axis_count << " axes and " << m_button_count << " buttons\n";

    for (int i = 0; i < axis_count; i++)
        m_prevAxisDirections[i] = Input::AD_NEUTRAL;

    m_index = irrIndex;
}
// -----------------------------------------------------------------------------
void GamePadDevice::loadDefaults()
{
    // buttons
    m_default_bindings[PA_FIRE].type = Input::IT_STICKBUTTON;
    m_default_bindings[PA_FIRE].id = 0;

    m_default_bindings[PA_NITRO].type = Input::IT_STICKBUTTON;
    m_default_bindings[PA_NITRO].id = 1;

    m_default_bindings[PA_DRIFT].type = Input::IT_STICKBUTTON;
    m_default_bindings[PA_DRIFT].id = 2;

    m_default_bindings[PA_RESCUE].type = Input::IT_STICKBUTTON;
    m_default_bindings[PA_RESCUE].id = 3;

    m_default_bindings[PA_LOOK_BACK].type = Input::IT_STICKBUTTON;
    m_default_bindings[PA_LOOK_BACK].id = 4;

    // axes
    m_default_bindings[PA_ACCEL].type = Input::IT_STICKMOTION;
    m_default_bindings[PA_ACCEL].id = 1;
    m_default_bindings[PA_ACCEL].dir = Input::AD_NEGATIVE;

    m_default_bindings[PA_BRAKE].type = Input::IT_STICKMOTION;
    m_default_bindings[PA_BRAKE].id = 1;
    m_default_bindings[PA_BRAKE].dir = Input::AD_POSITIVE;

    m_default_bindings[PA_LEFT].type = Input::IT_STICKMOTION;
    m_default_bindings[PA_LEFT].id = 0;
    m_default_bindings[PA_LEFT].dir = Input::AD_NEGATIVE;

    m_default_bindings[PA_RIGHT].type = Input::IT_STICKMOTION;
    m_default_bindings[PA_RIGHT].id = 0;
    m_default_bindings[PA_RIGHT].dir = Input::AD_POSITIVE;


    /*
     TODO - mappings for clear/enter/leave ?

     set(GA_CLEAR_MAPPING,
     Input(Input::IT_STICKBUTTON, 0, 2));

     set(GA_ENTER,
     Input(Input::IT_STICKBUTTON, 0, 0),

     set(GA_LEAVE,
     Input(Input::IT_STICKBUTTON, 0, 1),
     */
}
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
void GamePadDevice::editBinding(const PlayerAction action, const Input::InputType type, const int id, Input::AxisDirection direction)
{
    m_default_bindings[action].type = type;
    m_default_bindings[action].id = id;
    m_default_bindings[action].dir = direction;
}
// -----------------------------------------------------------------------------
void GamePadDevice::resetAxisDirection(const int axis, Input::AxisDirection direction, ActivePlayer* player)
{
    if(!StateManager::get()->isGameState()) return; // ignore this while in menus

    PlayerKart* pk = player->getKart();
    if (pk == NULL)
    {
        std::cerr << "Error, trying to reset axis for an unknown player\n";
        return;
    }
    
    for(int n=0; n<PA_COUNT; n++)
    {
        if(m_default_bindings[n].id == axis && m_default_bindings[n].dir == direction)
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

        // find corresponding action and return it
        for(int n=0; n<PA_COUNT; n++)
        {
            if(m_default_bindings[n].type == type && m_default_bindings[n].id == id)
            {
                if(m_default_bindings[n].dir == Input::AD_NEGATIVE && value < 0)
                {
                    *action = (PlayerAction)n;
                    return true;
                }
                else if(m_default_bindings[n].dir == Input::AD_POSITIVE && value > 0)
                {
                    *action = (PlayerAction)n;
                    return true;
                }
            }
        }// next device

    }
    else if(type == Input::IT_STICKBUTTON)
    {
        // find corresponding action and return it
        for(int n=0; n<PA_COUNT; n++)
        {
            if(m_default_bindings[n].type == type && m_default_bindings[n].id == id)
            {
                *action = (PlayerAction)n;
                return true;
            }
        }// next device
    }

    return false;
}
// -----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
    delete[] m_prevAxisDirections;

    // FIXME - any need to close devices?
}   // ~GamePadDevice
