//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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


#include "input/gamepad_config.hpp"

#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <SKeyMap.h>

#include <assert.h>

using namespace irr;


GamepadConfig::GamepadConfig( const std::string &name,
                              const int        axis_count,
                              const int        button_count )
             : DeviceConfig()
{
    setName(name);
    m_axis_count   = axis_count;
    m_button_count = button_count;
    m_deadzone     = 4096;
    m_is_analog    = true;
    m_desensitize  = false;
    setDefaultBinds();
    detectType();
}   // GamepadConfig

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig() : DeviceConfig()
{
    m_axis_count   = 0;
    m_button_count = 0;
    m_deadzone     = 4096;
    m_is_analog    = true;
    m_desensitize  = false;
    setDefaultBinds();
}   // GamepadConfig

//------------------------------------------------------------------------------
/** Loads this configuration from the given XML node.
 *  \param config The XML tree.
 *  \return False in case of an error.
 */
bool GamepadConfig::load(const XMLNode *config)
{
    config->get("deadzone",     &m_deadzone    );
    config->get("analog",       &m_is_analog   );
    config->get("desensitize",  &m_desensitize );
    bool ok = DeviceConfig::load(config);

    if(getName()=="")
    {
        Log::error("DeviceConfig", "Unnamed joystick in config file.");
        return false;
    }
    detectType();
    return ok;
}   // load

// ----------------------------------------------------------------------------
/** Saves the configuration to a file. It writes the name for a gamepad
 *  config, saves the device specific parameters, and calls
 *  DeviceConfig::save() to save the rest.
 *  \param stream The stream to save to.
 */
void GamepadConfig::save (std::ofstream& stream)
{
    stream << "<gamepad name =\"" << getName()
           << "\" deadzone=\""    << m_deadzone
           << "\" desensitize=\"" << m_desensitize
           << "\" analog=\""      << m_is_analog<<"\"\n";
    stream << "         ";
    DeviceConfig::save(stream);
    stream << "</gamepad>\n\n";
}   // save

//-----------------------------------------------------------------------------
/** Try to identify a gamepad type (e.g. 'xbox'), so that better defaults
 *  and button names can be set. Atm the gamepad name is used.
 */
void GamepadConfig::detectType()
{
    m_type = GP_UNIDENTIFIED;

    std::string lower = StringUtils::toLowerCase(getName());

    // xbox appears to be xbox 360
    if(lower.find("xbox")!=std::string::npos)
    {
        m_type = GP_XBOX360;
        return;
    }
    // The original xbox gamepad
    if(lower.find("x-box")!=std::string::npos)
    {
        m_type = GP_XBOX_ORIGINAL;
        return;
    }
}   // detectType

//------------------------------------------------------------------------------

void GamepadConfig::setDefaultBinds ()
{
    setBinding(PA_STEER_LEFT,   Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    setBinding(PA_STEER_RIGHT,  Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    setBinding(PA_ACCEL,        Input::IT_STICKBUTTON, 0, Input::AD_NEGATIVE);
    setBinding(PA_BRAKE,        Input::IT_STICKBUTTON, 3, Input::AD_POSITIVE);
    setBinding(PA_FIRE,         Input::IT_STICKBUTTON, 1);
    setBinding(PA_NITRO,        Input::IT_STICKBUTTON, 4);
    setBinding(PA_DRIFT,        Input::IT_STICKBUTTON, 5);
    setBinding(PA_RESCUE,       Input::IT_STICKBUTTON, 8);
    setBinding(PA_LOOK_BACK,    Input::IT_STICKBUTTON, 6);
    setBinding(PA_PAUSE_RACE,   Input::IT_STICKBUTTON, 9);

    setBinding(PA_MENU_UP,      Input::IT_STICKMOTION, 1, Input::AD_NEGATIVE);
    setBinding(PA_MENU_DOWN,    Input::IT_STICKMOTION, 1, Input::AD_POSITIVE);
    setBinding(PA_MENU_LEFT,    Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    setBinding(PA_MENU_RIGHT,   Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    setBinding(PA_MENU_SELECT,  Input::IT_STICKBUTTON, 0);
    setBinding(PA_MENU_CANCEL,  Input::IT_STICKBUTTON, 3);
}   // setDefaultBinds

//------------------------------------------------------------------------------
core::stringw GamepadConfig::getBindingAsString(const PlayerAction action) const
{
    // Default names if gamepad is not identified
    if(m_type==GP_UNIDENTIFIED) return DeviceConfig::getBindingAsString(action);
    const Binding &b = getBinding(action);
    int id = b.getId();
    Input::AxisDirection ad = b.getDirection();
    Input::InputType it = b.getType();

    // XBOX-360 controller
    // -------------------
    if(m_type==GP_XBOX_ORIGINAL)
    {
        // Handle only the differences to the xbox 360 controller, the rest
        // will 'fall trough' to the xbox 360 code below
        if(it==Input::IT_STICKBUTTON)
        {
            switch(id)
            {
                    // I18N: Name of the black button on xbox controller
            case 2: return _("Black");
            case 3: return "X";
            case 4: return "Y";
                    // I18N: Name of the white button on xbox controller
            case 5: return _("White");
            }
        }
        if(it==Input::IT_STICKMOTION)
        {
            switch(id)
            {
                    // I18N: name of buttons on gamepads
            case 2: return _("Left trigger");
                                                      // I18N: name of buttons on gamepads
            case 3: return (ad == Input::AD_POSITIVE) ? _("Right thumb right")
                                                      // I18N: name of buttons on gamepads
                                                      : _("Right thumb left");
            case                                      // I18N: name of buttons on gamepads
                4: return (ad == Input::AD_POSITIVE)  ? _("Right thumb down")
                                                      // I18N: name of buttons on gamepads
                                                      : _("Right thumb up");
                    // I18N: name of buttons on gamepads
            case 5: return _("Right trigger");
                                                      // I18N: name of buttons on gamepads
            case 6: return (ad == Input::AD_POSITIVE) ? _("DPad right")
                                                      // I18N: name of buttons on gamepads
                                                      : _("DPad left");
                                                      // I18N: name of buttons on gamepads
            case 7: return (ad == Input::AD_POSITIVE) ? _("DPad down")
                                                      // I18N: name of buttons on gamepads
                                                      : _("DPad up");
            }   // switch
        }   // stickmotion
    }   // xbox (original)
    if(m_type==GP_XBOX360 || m_type==GP_XBOX_ORIGINAL)
    {
        if(it==Input::IT_STICKBUTTON)
        {
            switch(id)
            {
            case 0: return "A";
            case 1: return "B";
            case 2: return "X";
            case 3: return "Y";
                    // I18N: name of buttons on gamepads
            case 4: return _("Left bumper");
                    // I18N: name of buttons on gamepads                
            case 5: return _("Right bumper");
                    // I18N: name of buttons on gamepads
            case 6: return _("Back");
                    // I18N: name of buttons on gamepads
            case 7: return _("Start");
                    // I18N: name of buttons on gamepads
            case 8: return _("Left thumb button");
                    // I18N: name of buttons on gamepads
            case 9: return _("Right thumb button");
            default: return DeviceConfig::getBindingAsString(action);
            }   // switch
        }   // if IT_STICKBUTTON
        if(it==Input::IT_STICKMOTION)
        {
            switch(id)
            {
                                                    // I18N: name of stick on gamepads
            case 0: return (ad==Input::AD_POSITIVE) ? _("Left thumb right")
                                                    // I18N: name of stick on gamepads
                                                    : _("Left thumb left");
                                                    // I18N: name of stick on gamepads
            case 1: return (ad==Input::AD_POSITIVE) ? _("Left thumb down")
                                                    // I18N: name of stick on gamepads
                                                    : _("Left thumb up");
                                                    // I18N: name of stick on gamepads
            case 2: return _("Left trigger");       // I18N: name of trigger on gamepads
            case 3: return (ad==Input::AD_POSITIVE) ? _("Right thumb down")
                                                    // I18N: name of stick on gamepads
                                                    : _("Right thumb up");
                                                    // I18N: name of stick on gamepads
            case 4: return (ad==Input::AD_POSITIVE) ? _("Right thumb right")
                                                    // I18N: name of stick on gamepads
                                                    : _("Right thumb left");
                                                    // I18N: name of buttons on gamepads
            case 5: return _("Right trigger");      // I18N: name of trigger on gamepads
            case Input::HAT_H_ID: return (ad == Input::AD_POSITIVE) ? _("DPad up")
                                                    // I18N: name of buttons on gamepads
                                                                    : _("DPad down");
                                                    // I18N: name of buttons on gamepads
            case Input::HAT_V_ID: return (ad == Input::AD_POSITIVE) ? _("DPad right")
                                                    // I18N: name of buttons on gamepads
                                                                    : _("DPad left");
            }   // switch
        }
    }   // xbox

    // Offer a fallback ... just in case
    Log::warn("GamepadConfig", "Missing action string for pad '%s' action '%d'",
              getName().c_str(), action);
    return DeviceConfig::getBindingAsString(action);
}   // getBindingAsString

//------------------------------------------------------------------------------
/** Converts the configuration to a string.
 */
irr::core::stringw GamepadConfig::toString()
{
    irr::core::stringw returnString = "";
    returnString += getName().c_str();
    returnString += "\n";
    returnString += DeviceConfig::toString();
    return returnString;
}   // toString

//------------------------------------------------------------------------------

bool DeviceConfig::hasBindingFor(const int button_id) const
{
    for (int n=0; n<PA_COUNT; n++)
    {
        if (m_bindings[n].getId() == button_id) return true;
    }
    return false;
}   // hasBindingFor

//------------------------------------------------------------------------------

bool DeviceConfig::hasBindingFor(const int button_id, PlayerAction from,
                                 PlayerAction to) const
{
    for (int n=from; n<=to; n++)
    {
        if (m_bindings[n].getId() == button_id) return true;
    }
    return false;
}   // hasBindingFor
