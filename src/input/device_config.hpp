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

#ifndef HEADER_DEVICE_CONFIG_HPP
#define HEADER_DEVICE_CONFIG_HPP

#include "input/binding.hpp"
#include "input/input.hpp"
#include "utils/no_copy.hpp"

#include <assert.h>
#include <iosfwd>
#include <irrString.h>
#include <string>

/**
  * \ingroup config
  */

//==== D E V I C E C O N F I G =================================================

/**
  * \brief contains the key bindings information related to one input device
  * \ingroup config
  */
class DeviceConfig : public NoCopy
{

private:
    /** If set to false, this device will be ignored. 
     *  Currently for gamepads only. */
    bool m_enabled;

    /** How many devices connected to the system which uses this config? */
    int m_plugged; 

    /** Name of this configuratiom. */
    std::string m_name;

protected:

    Binding  m_bindings[PA_COUNT];

    DeviceConfig();

    bool doGetAction(Input::InputType    type,
                     const int           id,
                     int*                value, /* inout */
                     const PlayerAction  firstActionToCheck,
                     const PlayerAction  lastActionToCheck,
                     PlayerAction*       action /* out */ );
protected:
    /** Those two classes need to be able to call getGameAction. */
    friend class GamePadDevice;
    friend class KeyboardDevice;
    bool getGameAction(Input::InputType       type,
                       const int              id,
                       int*                   value, /* inout */
                       PlayerAction*          action /* out */);

public:

    virtual ~DeviceConfig() {}

    static DeviceConfig* create(const XMLNode *config);
    irr::core::stringw toString();
    bool hasBindingFor(const int buttonID) const;
    bool hasBindingFor(const int buttonID, PlayerAction from,
                       PlayerAction to) const;
    void setBinding(const PlayerAction     action,
                    const Input::InputType type,
                    const int              id,
                    Input::AxisDirection   direction = Input::AD_NEUTRAL,
                    Input::AxisRange       range     = Input::AR_HALF,
                    wchar_t                character=0);
    bool getMenuAction(Input::InputType       type,
                       const int              id,
                       int*                   value,
                       PlayerAction*          action /* out */);
    irr::core::stringw getMappingIdString (const PlayerAction action) const;
    virtual irr::core::stringw getBindingAsString(const PlayerAction action) const;
    virtual bool isGamePad()  const = 0;
    virtual bool isKeyboard() const = 0;

    virtual void save(std::ofstream& stream);
    virtual bool load(const XMLNode *config);

    // ------------------------------------------------------------------------
    /** Returns true if this device has analog axis, so that steering values
     *  will not be affected by time-full-steer delays. */
    virtual bool isAnalog() const { return false;}
    // ------------------------------------------------------------------------
    /** Returns true if this device should desensitize its input at values
     *  close to 0 (to avoid 'oversteering'). */
    virtual bool desensitize() const { return false;}
    // ------------------------------------------------------------------------
    /** Should only be called for gamepads, which has its own implementation.
     *  of this function. */
    virtual int getNumberOfButtons() const
    {
        assert(false); return 0;
    }   // getNumberOfButtons

    // ------------------------------------------------------------------------
    /** Should only be called for gamepads, which has its own implementation.
     *  of this function. */
    virtual int getNumberOfAxes() const
    {
        assert(false); return 0;
    }   // getNumberOfAxes

    // ------------------------------------------------------------------------
    /** Sets the name of this device. */
    void setName(const std::string &name) { m_name = name; }

    // ------------------------------------------------------------------------
    /** Returns the name for this device configuration. */
    const std::string& getName() const { return m_name; };

    // ------------------------------------------------------------------------
    /** Increase ref counter. */
    void setPlugged() { m_plugged++; }

    // ------------------------------------------------------------------------
    /** Returns if this config is sed by any devices. */
    bool isPlugged() const { return m_plugged > 0; }

    // ------------------------------------------------------------------------
    /** Returns the number of devices using this configuration. */
    int getNumberOfDevices() const { return m_plugged;     }

    // ------------------------------------------------------------------------
    /** Returns the binding of a given index. */
    const Binding& getBinding(int i) const {return m_bindings[i];}

    // ------------------------------------------------------------------------
    /** At this time only relevant for gamepads, keyboards are always enabled */
    bool isEnabled() const { return m_enabled; }

    // ------------------------------------------------------------------------
    /** Sets this config to be enabled or disabled. */
    void setEnabled(bool new_value) { m_enabled = new_value; }
};   // class DeviceConfig

#endif
