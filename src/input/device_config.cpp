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


#include "input/device_config.hpp"

#include "io/xml_node.hpp"
#include "utils/log.hpp"

#include <SKeyMap.h>

#include <assert.h>

using namespace irr;

DeviceConfig::DeviceConfig(DeviceConfigType type)
{
    m_type    = type;
    m_enabled = true;
    m_plugged = 0;
}   // DeviceConfig

// ------------------------------------------------------------------------
/** Get a user-readable string describing the bound action.
 */
irr::core::stringw DeviceConfig::getBindingAsString (const PlayerAction action) const
{
    irr::core::stringw return_string = "";

    if ((action < PA_COUNT) && (action >= 0))
    {
        return_string = m_bindings[action].getAsString();
    }

    return return_string;
}   // getBindingAsString

//------------------------------------------------------------------------------
/** Get an internal unique string describing the bound action.
 *  \param action The action for which to get the string.
 */
irr::core::stringw DeviceConfig::getMappingIdString (const PlayerAction action) const
{
    irr::core::stringw return_string = "";

    if ((action < PA_COUNT) && (action >= 0))
    {
        const Input::InputType type = m_bindings[action].getType();
        const int id = m_bindings[action].getId();
        const Input::AxisDirection dir = m_bindings[action].getDirection();
        const Input::AxisRange range = m_bindings[action].getRange();

        switch (type)
        {
            case Input::IT_KEYBOARD:
                return_string += "keyb_";
                return_string += id;
                break;

            case Input::IT_STICKMOTION:
                return_string += "stkmo_";
                return_string += id;
                return_string += "$";
                return_string += dir;
                return_string += "$";
                return_string += range;
                break;

            case Input::IT_STICKBUTTON:
                return_string += "stkbt_";
                return_string += id;
                break;

            case Input::IT_MOUSEMOTION:
                return_string += "mousemo_";
                return_string += id;
                return_string += "$";
                return_string += dir;
                break;

            case Input::IT_MOUSEBUTTON:
                return_string += "mousebtn_";
                return_string += id;
                break;

            case Input::IT_NONE:
                return_string += "none";
                break;

            default:
                assert(false);
                return_string += type;
                return_string += "_";
                return_string += id;
                return_string += "$";
                return_string += dir;
        }
    }

    return return_string;
}   // getMappingIdString

//------------------------------------------------------------------------------

irr::core::stringw DeviceConfig::toString ()
{
    irr::core::stringw return_string = "";
    for (int n = 0; n < PA_COUNT; n++)
    {
        return_string += KartActionStrings[n].c_str();
        return_string += ": ";
        return_string += m_bindings[n].getAsString();
        return_string += "\n";
    }
    return return_string;
}   // toString

//------------------------------------------------------------------------------
/** Sets the bindings for an action.
 *  \param action The action to be bound.
 *  \param type Input type (stick button, stick motion, ...).
 *  \param id An id for this binding.
 *  \param direction In which direction the stick is moved.
 *  \param 
 */
void DeviceConfig::setBinding ( const PlayerAction      action,
                                const Input::InputType  type,
                                const int               id,
                                Input::AxisDirection    direction,
                                Input::AxisRange        range,
                                wchar_t                 character)
{
    m_bindings[action].set(type, id, direction, range, character);
}   // setBinding

//------------------------------------------------------------------------------
/** Searches for a game actions associated with the given input event.
 * \note               Don't call this directly unless you are KeyboardDevice or
 *                     GamepadDevice.
 * \param[out] action  the result, only set if method returned true.
 * \return             whether finding an action associated to this input was
 *                     successful.
 */
bool DeviceConfig::getGameAction(Input::InputType    type,
                                 const int           id,
                                 int*                value, /* inout */
                                 PlayerAction*       action /* out */ )
{
    return doGetAction(type, id, value, PA_FIRST_GAME_ACTION,
                       PA_LAST_GAME_ACTION, action);
}   // getGameAction

//------------------------------------------------------------------------------
/** Searches for a game actions associated with the given input event.
 *  \note Don't call this directly unless you are KeyboardDevice or
 *        GamepadDevice.
 *  \param[out] action  The result, only set if method returned true.
 *  \return             Whether finding an action associated to this input
 *                      was successful
 */
bool DeviceConfig::getMenuAction(Input::InputType    type,
                                 const int           id,
                                 int*                value,
                                 PlayerAction*       action /* out */ )
{
    return doGetAction(type, id, value, PA_FIRST_MENU_ACTION,
                       PA_LAST_MENU_ACTION, action);
}   // getMenuAction

//------------------------------------------------------------------------------
/** internal helper method for DeviceConfig::getGameAction and 
 *  DeviceConfig::getMenuAction
 */
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
}   // doGetAction

//------------------------------------------------------------------------------
/** Saves the configuration to a file.
 *  \param stream The stream to save to.
 */
void DeviceConfig::save (std::ofstream& stream)
{
    for(int n = 0; n < PA_COUNT; n++) // Start at 0?
    {
        stream << "    "
               << "<action "
               << "name=\""      << KartActionStrings[n] << "\" ";
        m_bindings[n].save(stream);
        stream   << "/>\n";
    }
}   // save

//------------------------------------------------------------------------------
/** Reads a device configuration from input.xml.
 *  \param config The XML Node of the configuration.
 *  \return False if an error occurred.
 */
bool DeviceConfig::load(const XMLNode *config)
{
    bool error = false;
    for(unsigned int i=0; i<config->getNumNodes(); i++)
    {
        const XMLNode *action = config->getNode(i);
        if(action->getName()!="action")
        {
            Log::warn("DeviceConfig", "Invalid configuration '%s' - ignored.");
            continue;
        }
        std::string name;
        action->get("name", &name);
        int binding_id = -1;
        for (int i = 0; i < PA_COUNT; i++)
        {
            if (name==KartActionStrings[i])
            {
                binding_id = i;
                break;
            }
        }
        if (binding_id == -1)
        {
            Log::warn("DeviceConfig",
                      "DeviceConfig::deserializeAction : action '%s' is unknown.",
                      name.c_str());
            error=true;
        }

        if(!m_bindings[binding_id].load(action))
        {
            Log::error("Device manager",
                       "Ignoring an ill-formed keyboard action in input config.");
            error=true;
        }
    }   // for i in nodes
    return !error;
}   // load

// ---------------------------------------------------------------------------

