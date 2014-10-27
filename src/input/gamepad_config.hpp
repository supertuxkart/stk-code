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

#ifndef HEADER_GAMEPD_CONFIG_HPP
#define HEADER_GAMEPD_CONFIG_HPP

#include "input/binding.hpp"
#include "input/device_config.hpp"
#include "input/input.hpp"
#include "utils/no_copy.hpp"

#include <iosfwd>
#include <irrString.h>
#include <string>

//==== G A M E P A D C O N F I G ===============================================

/**
  * \brief specialisation of DeviceConfig for gamepad type devices
  * \ingroup config
  */
class GamepadConfig : public DeviceConfig
{

private:
    /** Number of axis this device has. */
    int         m_axis_count;

    /** Number of buttons this device has. */
    int         m_button_count;

    int m_deadzone;

public:

    irr::core::stringw toString     ();

    virtual void save(std::ofstream& stream);
    void        setDefaultBinds     ();
    GamepadConfig           (const XMLNode *config);
    GamepadConfig           (const std::string     &name,
                             const int              axis_count=0,
                             const int              button_ount=0);
    virtual bool load(const XMLNode *config);
    // ------------------------------------------------------------------------
    /** Sets the number of buttons this device has. */
    void setNumberOfButtons(int count) { m_button_count = count; }

    // ------------------------------------------------------------------------
    /** Sets the number of axis this device has. */
    void setNumberOfAxis(int count) { m_axis_count = count; }
    //        ~GamepadConfig();

    // ------------------------------------------------------------------------
    /** Returns the type of this configuration. */
    virtual DeviceConfig::DeviceConfigType getType() const
    {
        return DeviceConfig::DEVICE_CONFIG_TYPE_GAMEPAD;
    }   // getType
};

#endif
