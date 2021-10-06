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

#ifndef HEADER_GAMEPD_CONFIG_HPP
#define HEADER_GAMEPD_CONFIG_HPP

#include "input/binding.hpp"
#include "input/device_config.hpp"
#include "input/input.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"

#include <iosfwd>
#include <irrString.h>
#include <map>
#include <set>
#include <string>
#include <string>
#include <tuple>

using namespace irr;
class SDLController;

/** \brief specialisation of DeviceConfig for gamepad type devices
 *  \ingroup config
 */
class GamepadConfig : public DeviceConfig
{

private:
    /** Number of axis this device has. */
    int m_axis_count;

    /** Number of hat this device has. */
    int m_hat_count;

    /** Number of buttons this device has. */
    int m_button_count;

    /** Deadzone of this gamepad. */
    int m_deadzone;

    /** If set to true, map any analog axis from x in [0,1] to x^x --> at
     *  values close to 0 the joystick will react less sensitive. */
    bool m_desensitize;

    bool m_use_force_feedback;
    int m_auto_center;

    std::map<std::tuple<int, Input::AxisDirection>, int> m_sdl_mapping;

    std::set<int> m_digital_axes;

    bool getMappingTuple(const std::string& rhs,
                         std::tuple<int, Input::AxisDirection>& t);
    void setBindingFromTuple(const PlayerAction action,
                             std::tuple<int, Input::AxisDirection>& t);
public:

             GamepadConfig           ();
             GamepadConfig(const std::string &name,
                           const int          axis_count=0,
                           const int          button_ount=0);
    virtual ~GamepadConfig() {}

    core::stringw toString();

    virtual void save(std::ofstream& stream) OVERRIDE;
    void        setDefaultBinds     ();
    virtual core::stringw getBindingAsString(const PlayerAction action) const OVERRIDE;
    virtual bool load(const XMLNode *config) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns if this device uses analog axes. */
    virtual bool isAnalog(Input::InputType type, int id) const OVERRIDE;

    // ------------------------------------------------------------------------
    /** Returns true if this device should desensitize its input at values
     *  close to 0 (to avoid 'oversteering'). */
    virtual bool desensitize() const OVERRIDE { return m_desensitize;}

    // ------------------------------------------------------------------------
    /** Returns the number of buttons in this configuration. */
    virtual int getNumberOfButtons() const OVERRIDE { return m_button_count; }

    // ------------------------------------------------------------------------
    /** Sets the number of buttons this device has. */
    void setNumberOfButtons(int count) { m_button_count = count; }

    // ------------------------------------------------------------------------
    /** Returns the number of axis of this configufation. */
    virtual int getNumberOfAxes() const OVERRIDE { return m_axis_count; }

    // ------------------------------------------------------------------------
    /** Sets the number of axis this device has. */
    void setNumberOfAxis(int count) { m_axis_count = count; }

    // ------------------------------------------------------------------------
    /** Return deadzone of this configuration. */
    int getDeadzone() const { return m_deadzone; }
    // ------------------------------------------------------------------------
    virtual bool isGamePad()  const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool isKeyboard() const OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    void initSDLController(const std::string& mapping, int buttons, int axes,
                           int hats);
    // ------------------------------------------------------------------------
    void initSDLMapping();
    // ------------------------------------------------------------------------
    bool useForceFeedback() const             { return m_use_force_feedback; }
    // ------------------------------------------------------------------------
    void setForceFeedback(bool val)            { m_use_force_feedback = val; }
    // ------------------------------------------------------------------------
    int getAutoCenterStrength() const                { return m_auto_center; }
    // ------------------------------------------------------------------------
    void setAutoCenter(bool val)               { m_auto_center = val; }
};   // class GamepadConfig

#endif
