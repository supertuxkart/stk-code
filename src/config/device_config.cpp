#include "config/device_config.hpp"

//==== D E V I C E C O N F I G =================================================

irr::core::stringw DeviceConfig::getBindingAsString (const PlayerAction action) const
{
    irr::core::stringw returnString = "";

    if ((action < PA_COUNT) && (action >= 0))
    {
        returnString += Input::getInputAsString(m_bindings[action].type, 
                                                m_bindings[action].id,
                                                m_bindings[action].dir);
    }

    return returnString;
}

//------------------------------------------------------------------------------

irr::core::stringw DeviceConfig::toString ()
{
    irr::core::stringw returnString = "";
    for (int n = 0; n < PA_COUNT; n++)
    {
        returnString += KartActionStrings[n].c_str();
        returnString += ": ";
        returnString += Input::getInputAsString(m_bindings[n].type, 
                                                m_bindings[n].id,
                                                m_bindings[n].dir);
        returnString += "\n";
    }
    return returnString;
}

//------------------------------------------------------------------------------

void DeviceConfig::setBinding ( const PlayerAction      action, 
                                const Input::InputType  type,
                                const int               id,
                                Input::AxisDirection    direction )
{
    m_bindings[action].type = type;
    m_bindings[action].id = id;
    m_bindings[action].dir = direction;
}

//------------------------------------------------------------------------------

// Don't call this directly unless you are KeyboardDevice or GamepadDevice
bool DeviceConfig::getAction  ( Input::InputType    type,
                                const int           id,
                                const int           value,
                                PlayerAction*       action )
{
    bool success = false;
    int  n;

    for (n = 0; ((n < PA_COUNT) && (!success)); n++)
    {
        if ((m_bindings[n].type == type) && (m_bindings[n].id == id))
        {

            if (type == Input::IT_STICKMOTION)
            {
                if ((m_bindings[n].dir == Input::AD_POSITIVE) && (value > 0) ||
                    (m_bindings[n].dir == Input::AD_NEGATIVE) && (value < 0))
                {
                    success = true;
                   *action = (PlayerAction)n;
                }
            }
            else
            {
                success = true;
               *action = (PlayerAction)n;
            }
        } 
    } // end for n

    return success;
}

//------------------------------------------------------------------------------

void DeviceConfig::serialize (std::ofstream& stream)
{
    for(int n = 0; n < PA_COUNT; n++) // Start at 0?
    {
        stream << "    "
               << "<action "
                   << "name=\""      << KartActionStrings[n] << "\" "
                   << "id=\""        << m_bindings[n].id     << "\" "
                   << "event=\""     << m_bindings[n].type   << "\" ";
        
        // Only serialize the direction for stick motions
        if (m_bindings[n].type == Input::IT_STICKMOTION)
        {
            stream << "direction=\"" << m_bindings[n].dir    << "\" ";
        }

        stream   << "/>\n";
    }
}

//------------------------------------------------------------------------------

bool DeviceConfig::deserializeAction(irr::io::IrrXMLReader* xml)
{
    bool                 success = false;
    int                  binding_id = -1;
    int                  id;
    Input::InputType     type;
    Input::AxisDirection dir;

    // Never hurts to check ;)
    if (xml == NULL)
    {
        fprintf(stderr, "Error: null pointer (DeviceConfig::deserializeAction)\n");
    }
    else
    {
    // Read tags from XML
        const char *name_string     = xml->getAttributeValue("name");
        const char *id_string       = xml->getAttributeValue("id");
        const char *event_string    = xml->getAttributeValue("event");
        const char *dir_string      = xml->getAttributeValue("direction");
    
        // Proceed only if neccesary tags were found
        if ((name_string != NULL) && (id_string != NULL) && 
            (event_string != NULL))
        {
            // Convert strings to string tags to integer types
            type = (Input::InputType)atoi(event_string);
            id = atoi(id_string);

            // Try to determine action # for verbose action name
            for (int n = 0; ((n < PA_COUNT) && (binding_id == -1)); n++)
            {
                if (strcmp(name_string, KartActionStrings[n].c_str()) == 0)
                    binding_id = n;
            }

            // If action # was found then store the bind
            if (binding_id != -1)
            {
                // If the action is not a stick motion (button or key)
                if (type != Input::IT_STICKMOTION)
                {
                    setBinding((PlayerAction)binding_id, type, id);
                    success = true;
                }

                // If the action is a stick motion & a direction is defined
                else if (dir_string != NULL)
                {
                    dir = (Input::AxisDirection)atoi(dir_string);
                    setBinding((PlayerAction)binding_id, type, id, dir);
                    success = true;
                }
                else
                {
                    printf("WARNING: IT_STICKMOTION without direction, ignoring.\n");
                }
            } // end if binding_id != -1
        } // end if name_string != NULL ...
    } // end if xml == NULL ... else

    return success;
}


//  KeyboardConfig & GamepadConfig classes really should be in a separate cpp
//  file but they are so small that we'll just leave them here for now.

//==== K E Y B O A R D C O N F I G =============================================

void KeyboardConfig::serialize (std::ofstream& stream)
{
    stream << "<keyboard>\n\n";
    DeviceConfig::serialize(stream);
    stream << "</keyboard>\n\n\n";
}

//------------------------------------------------------------------------------

void KeyboardConfig::setDefaultBinds()
{
    setBinding(PA_NITRO,       Input::IT_KEYBOARD, KEY_KEY_N);
    setBinding(PA_ACCEL,       Input::IT_KEYBOARD, KEY_UP);
    setBinding(PA_BRAKE,       Input::IT_KEYBOARD, KEY_DOWN);
    setBinding(PA_LEFT,        Input::IT_KEYBOARD, KEY_LEFT);
    setBinding(PA_RIGHT,       Input::IT_KEYBOARD, KEY_RIGHT);
    setBinding(PA_DRIFT,       Input::IT_KEYBOARD, KEY_KEY_V);
    setBinding(PA_RESCUE,      Input::IT_KEYBOARD, KEY_BACK);
    setBinding(PA_FIRE,        Input::IT_KEYBOARD, KEY_SPACE);
    setBinding(PA_LOOK_BACK,   Input::IT_KEYBOARD, KEY_KEY_B);
}

//------------------------------------------------------------------------------

KeyboardConfig::KeyboardConfig()
{
    m_name = "Keyboard";
    setInUse(true);
    setDefaultBinds();
}

//==== G A M E P A D C O N F I G ===============================================

void GamepadConfig::serialize (std::ofstream& stream)
{
    stream << "<gamepad name =\"" << m_name.c_str() << "\">\n\n";
    DeviceConfig::serialize(stream);
    stream << "</gamepad>\n\n\n";
}

//------------------------------------------------------------------------------

void GamepadConfig::setDefaultBinds ()
{
    setBinding(PA_LEFT,         Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    setBinding(PA_RIGHT,        Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    setBinding(PA_ACCEL,        Input::IT_STICKMOTION, 1, Input::AD_NEGATIVE);
    setBinding(PA_BRAKE,        Input::IT_STICKMOTION, 1, Input::AD_POSITIVE);
    setBinding(PA_FIRE,         Input::IT_STICKBUTTON, 0);
    setBinding(PA_NITRO,        Input::IT_STICKBUTTON, 1);
    setBinding(PA_DRIFT,        Input::IT_STICKBUTTON, 2);
    setBinding(PA_RESCUE,       Input::IT_STICKBUTTON, 3);
    setBinding(PA_LOOK_BACK,    Input::IT_STICKBUTTON, 4);
    //TODO - mappings for clear/enter/leave GA_CLEAR_MAPPING, GA_ENTER, GA_LEAVE?
}

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig   ( const std::string      name,
                                 const int              axis_count,
                                 const int              btnCount )
{
    m_name = name;
    m_axis_count = axis_count;
    m_button_count = btnCount;
    setInUse(false);
    setDefaultBinds();
}

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig(irr::io::IrrXMLReader* xml)
{
    const char* name_string = xml->getAttributeValue("name");
    if(name_string == NULL)
    {
        printf("ERROR: Unnamed joystick in config file\n");
    }
    else 
    {
        m_name = name_string;
    }
    setInUse(false);
    setDefaultBinds();
}

//------------------------------------------------------------------------------

irr::core::stringw GamepadConfig::toString ()
{
    irr::core::stringw returnString = "";
    returnString += getName().c_str();
    returnString += "\n";
    returnString += DeviceConfig::toString();
    return returnString;
}
