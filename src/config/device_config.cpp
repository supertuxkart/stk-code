//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <assert.h>

#include "config/device_config.hpp"
#include "utils/log.hpp"
#include <SKeyMap.h>
using namespace irr;

//==== D E V I C E C O N F I G =================================================

irr::core::stringw DeviceConfig::getBindingAsString (const PlayerAction action) const
{
    irr::core::stringw returnString = "";

    if ((action < PA_COUNT) && (action >= 0))
    {
        returnString = m_bindings[action].getAsString();
    }

    return returnString;
}

//------------------------------------------------------------------------------

irr::core::stringw DeviceConfig::getMappingIdString (const PlayerAction action) const
{
    irr::core::stringw returnString = "";

    if ((action < PA_COUNT) && (action >= 0))
    {
        const Input::InputType type = m_bindings[action].getType();
        const int id = m_bindings[action].getId();
        const Input::AxisDirection dir = m_bindings[action].getDirection();
        const Input::AxisRange range = m_bindings[action].getRange();

        switch (type)
        {
            case Input::IT_KEYBOARD:
                returnString += "keyb_";
                returnString += id;
                break;

            case Input::IT_STICKMOTION:
                returnString += "stkmo_";
                returnString += id;
                returnString += "$";
                returnString += dir;
                returnString += "$";
                returnString += range;
                break;

            case Input::IT_STICKBUTTON:
                returnString += "stkbt_";
                returnString += id;
                break;

            case Input::IT_MOUSEMOTION:
                returnString += "mousemo_";
                returnString += id;
                returnString += "$";
                returnString += dir;
                break;

            case Input::IT_MOUSEBUTTON:
                returnString += "mousebtn_";
                returnString += id;
                break;

            case Input::IT_NONE:
                returnString += "none";
                break;

            default:
                assert(false);
                returnString += type;
                returnString += "_";
                returnString += id;
                returnString += "$";
                returnString += dir;
        }
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
        returnString += m_bindings[n].getAsString();
        returnString += "\n";
    }
    return returnString;
}

//------------------------------------------------------------------------------

void DeviceConfig::setBinding ( const PlayerAction      action,
                                const Input::InputType  type,
                                const int               id,
                                Input::AxisDirection    direction,
                                Input::AxisRange        range,
                                wchar_t                 character)
{
    m_bindings[action].set(type, id, direction, range, character);
}

//------------------------------------------------------------------------------

// Don't call this directly unless you are KeyboardDevice or GamepadDevice
bool DeviceConfig::getGameAction(Input::InputType    type,
                                 const int           id,
                                 int*                value, /* inout */
                                 PlayerAction*       action /* out */ )
{
    return doGetAction(type, id, value, PA_FIRST_GAME_ACTION, PA_LAST_GAME_ACTION, action);
}

//------------------------------------------------------------------------------

// Don't call this directly unless you are KeyboardDevice or GamepadDevice
bool DeviceConfig::getMenuAction(Input::InputType    type,
                                 const int           id,
                                 int*                value,
                                 PlayerAction*       action /* out */ )
{
    return doGetAction(type, id, value, PA_FIRST_MENU_ACTION, PA_LAST_MENU_ACTION, action);
}

//------------------------------------------------------------------------------

bool DeviceConfig::doGetAction(Input::InputType    type,
                               const int           id,
                               int*                value, /* inout */
                               const PlayerAction  firstActionToCheck,
                               const PlayerAction  lastActionToCheck,
                               PlayerAction*       action /* out */ )
{
    if (!m_enabled) return false;

    bool success = false;
    int  n;

    for (n = firstActionToCheck; ((n <= lastActionToCheck) && (!success)); n++)
    {
        if ((m_bindings[n].getType() == type) && (m_bindings[n].getId() == id))
        {

            if (type == Input::IT_STICKMOTION)
            {
                if(m_bindings[n].getRange() == Input::AR_HALF)
                {
                    if ( ((m_bindings[n].getDirection() == Input::AD_POSITIVE)
                           && (*value > 0))                                      ||
                         ((m_bindings[n].getDirection() == Input::AD_NEGATIVE)
                           && (*value < 0))                                        )
                    {
                        success = true;
                       *action = (PlayerAction)n;
                    }
                }
                else
                {
                    if ( ((m_bindings[n].getDirection() == Input::AD_POSITIVE)
                           && (*value != -Input::MAX_VALUE))                     ||
                         ((m_bindings[n].getDirection() == Input::AD_NEGATIVE)
                           && (*value != Input::MAX_VALUE))                        )
                    {
                        success = true;
                        *action = (PlayerAction)n;
                        if(m_bindings[n].getDirection() == Input::AD_NEGATIVE)
                            *value = -*value;
                        *value = (*value + Input::MAX_VALUE) / 2;
                    }
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
               << "name=\""      << KartActionStrings[n] << "\" ";
        m_bindings[n].serialize(stream);
        stream   << "/>\n";
    }
}   // serialize

//------------------------------------------------------------------------------

bool DeviceConfig::deserializeAction(irr::io::IrrXMLReader* xml)
{
    int                  binding_id = -1;

    // Never hurts to check ;)
    if (xml == NULL)
    {
        Log::error("DeviceConfig", "Null pointer (DeviceConfig::deserializeAction)");
        return false;
    }

    // Read tags from XML
    const char *name_string     = xml->getAttributeValue("name");
    // Try to determine action # for verbose action name
    for (int i = 0; i < PA_COUNT; i++)
    {
        if (strcmp(name_string, KartActionStrings[i].c_str()) == 0)
        {
            binding_id = i;
            break;
        }
    }
    if(binding_id==-1)
    {
        Log::warn("DeviceConfig", "DeviceConfig::deserializeAction : action '%s' is unknown.",
                  name_string);
        return false;
    }

    return m_bindings[binding_id].deserialize(xml);
}   // deserializeAction


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
    setBinding(PA_STEER_LEFT,  Input::IT_KEYBOARD, KEY_LEFT);
    setBinding(PA_STEER_RIGHT, Input::IT_KEYBOARD, KEY_RIGHT);
    setBinding(PA_DRIFT,       Input::IT_KEYBOARD, KEY_KEY_V);
    setBinding(PA_RESCUE,      Input::IT_KEYBOARD, KEY_BACK);
    setBinding(PA_FIRE,        Input::IT_KEYBOARD, KEY_SPACE);
    setBinding(PA_LOOK_BACK,   Input::IT_KEYBOARD, KEY_KEY_B);
    setBinding(PA_PAUSE_RACE,  Input::IT_KEYBOARD, KEY_ESCAPE);

    setBinding(PA_MENU_UP,     Input::IT_KEYBOARD, KEY_UP);
    setBinding(PA_MENU_DOWN,   Input::IT_KEYBOARD, KEY_DOWN);
    setBinding(PA_MENU_LEFT,   Input::IT_KEYBOARD, KEY_LEFT);
    setBinding(PA_MENU_RIGHT,  Input::IT_KEYBOARD, KEY_RIGHT);
    setBinding(PA_MENU_SELECT, Input::IT_KEYBOARD, KEY_RETURN);
    setBinding(PA_MENU_CANCEL, Input::IT_KEYBOARD, KEY_BACK);
}

//------------------------------------------------------------------------------

KeyboardConfig::KeyboardConfig() : DeviceConfig(DEVICE_CONFIG_TYPE_KEYBOARD)
{
    m_name = "Keyboard";
    m_plugged = 1;
    setDefaultBinds();
}

//==== G A M E P A D C O N F I G ===============================================

void GamepadConfig::serialize (std::ofstream& stream)
{
    stream << "<gamepad name =\"" << m_name.c_str() << "\" enabled=\""
           << (m_enabled ? "true" : "false") << "\">\n\n";
    DeviceConfig::serialize(stream);
    stream << "</gamepad>\n\n\n";
}

//------------------------------------------------------------------------------

void GamepadConfig::setDefaultBinds ()
{
    setBinding(PA_STEER_LEFT,   Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    setBinding(PA_STEER_RIGHT,  Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    setBinding(PA_ACCEL,        Input::IT_STICKMOTION, 1, Input::AD_NEGATIVE);
    setBinding(PA_BRAKE,        Input::IT_STICKMOTION, 1, Input::AD_POSITIVE);
    setBinding(PA_FIRE,         Input::IT_STICKBUTTON, 0);
    setBinding(PA_NITRO,        Input::IT_STICKBUTTON, 1);
    setBinding(PA_DRIFT,        Input::IT_STICKBUTTON, 2);
    setBinding(PA_RESCUE,       Input::IT_STICKBUTTON, 3);
    setBinding(PA_LOOK_BACK,    Input::IT_STICKBUTTON, 4);
    setBinding(PA_PAUSE_RACE,   Input::IT_STICKBUTTON, 5);

    setBinding(PA_MENU_UP,      Input::IT_STICKMOTION, 1, Input::AD_NEGATIVE);
    setBinding(PA_MENU_DOWN,    Input::IT_STICKMOTION, 1, Input::AD_POSITIVE);
    setBinding(PA_MENU_LEFT,    Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    setBinding(PA_MENU_RIGHT,   Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    setBinding(PA_MENU_SELECT,  Input::IT_STICKBUTTON, 0);
    setBinding(PA_MENU_CANCEL,  Input::IT_STICKBUTTON, 3);
}

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig   ( const std::string     &name,
                                 const int              axis_count,
                                 const int              button_count )
             : DeviceConfig( DEVICE_CONFIG_TYPE_GAMEPAD )
{
    m_name         = name;
    m_axis_count   = axis_count;
    m_button_count = button_count;
    m_plugged      = 0;
    setDefaultBinds();
}

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig(irr::io::IrrXMLReader* xml) : DeviceConfig( DEVICE_CONFIG_TYPE_GAMEPAD )
{
    const char* name_string = xml->getAttributeValue("name");
    if(name_string == NULL)
        Log::error("DeviceConfig", "Unnamed joystick in config file.");
    else
        m_name = name_string;

    const char* enabled_string = xml->getAttributeValue("enabled");
    if (enabled_string != NULL)
        m_enabled = (strcmp(enabled_string, "true") == 0);
    else
        m_enabled = true;

    m_plugged = 0;
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

//------------------------------------------------------------------------------

bool DeviceConfig::hasBindingFor(const int button_id) const
{
    for (int n=0; n<PA_COUNT; n++)
    {
        if (m_bindings[n].getId() == button_id) return true;
    }
    return false;
}

//------------------------------------------------------------------------------

bool DeviceConfig::hasBindingFor(const int button_id, PlayerAction from, PlayerAction to) const
{
    for (int n=from; n<=to; n++)
    {
        if (m_bindings[n].getId() == button_id) return true;
    }
    return false;
}
