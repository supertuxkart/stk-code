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


#include "input/gamepad_config.hpp"

#include "io/xml_node.hpp"
#include "utils/log.hpp"

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
    m_deadzone     = 2000;
    m_is_analog    = true;
    m_desensitize  = false;
    setDefaultBinds();
}   // GamepadConfig

//------------------------------------------------------------------------------

GamepadConfig::GamepadConfig() : DeviceConfig()
{
    m_axis_count   = 0;
    m_button_count = 0;
    m_deadzone     = 2000;
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
}   // setDefaultBinds

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
