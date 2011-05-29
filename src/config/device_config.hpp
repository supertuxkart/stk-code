//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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

#ifndef DEVICE_CONFIG_HPP
#define DEVICE_CONFIG_HPP

#include "input/binding.hpp"
#include "input/input.hpp"
#include "utils/no_copy.hpp"

#include <iosfwd>
#include <irrString.h>
#include <string>

/**
  * \ingroup config
  */

enum DeviceConfigType
{
    DEVICE_CONFIG_TYPE_GAMEPAD,
    DEVICE_CONFIG_TYPE_KEYBOARD
};


//==== D E V I C E C O N F I G =================================================

/**
  * \brief contains the key bindings information related to one input device
  * \ingroup config
  */
class DeviceConfig : public NoCopy
{
private:
    
    Binding  m_bindings[PA_COUNT];
    bool     m_plugged;  //!< Is there a device connected to the system which uses this config?
    
protected:
    
    bool     m_enabled;  //!< If set to false, this device will be ignored. Currently for gamepads only

    std::string m_name;
    
    DeviceConfigType m_type;
    
    DeviceConfig(DeviceConfigType type)
    {
        m_type = type;
        m_enabled = true;
    }
    
    /**
      * \brief internal helper method for DeviceConfig::getGameAction and DeviceConfig::getMenuAction
      */
    bool doGetAction(Input::InputType    type,
                     const int           id,
                     const int           value,
                     const PlayerAction  firstActionToCheck,
                     const PlayerAction  lastActionToCheck,
                     PlayerAction*       action /* out */ );
    
public:
    
    std::string        getName           () const { return m_name; };
    irr::core::stringw toString          ();
    DeviceConfigType   getType           () const { return m_type; }
    
    /** Get a user-readable string describing the bound action */
    irr::core::stringw getBindingAsString(const PlayerAction action) const;
    
    /** Get an internal unique string describing the bound action */
    irr::core::stringw getMappingIdString (const PlayerAction action) const;
    
    void        serialize           (std::ofstream& stream);
    bool        deserializeAction   (irr::io::IrrXMLReader* xml);
    
    void        setBinding          (const PlayerAction     action,
                                     const Input::InputType type,
                                     const int              id,
                                     Input::AxisDirection   direction = Input::AD_NEUTRAL,
                                     wchar_t                character=0);
    
    void        setPlugged            (bool plugged) {m_plugged = plugged;}
    bool        isPlugged           () {return m_plugged;}
    
    /**
      * \brief              Searches for a game actions associated with the given input event
      * \note               Don't call this directly unless you are KeyboardDevice or GamepadDevice
      * \param[out] action  the result, only set if method returned true
      * \return             whether finding an action associated to this input was successful
      */
    bool        getGameAction       (Input::InputType       type, 
                                     const int              id,
                                     const int              value,
                                     PlayerAction*          action /* out */);
    
    /**
      * \brief              Searches for a game actions associated with the given input event
      * \note Don't call this directly unless you are KeyboardDevice or GamepadDevice
      * \param[out] action  the result, only set if method returned true
      * \return             whether finding an action associated to this input was successful
      */
    bool        getMenuAction       (Input::InputType       type, 
                                     const int              id,
                                     const int              value,
                                     PlayerAction*          action /* out */);
    
    Binding&    getBinding          (int i) {return m_bindings[i];}
    
    bool hasBindingFor(const int buttonID) const;
    
    /** At this time only relevant for gamepads, keyboards are always enabled */
    bool isEnabled() const { return m_enabled; }
    
    void setEnabled(bool newValue) { m_enabled = newValue; }
};

//==== K E Y B O A R D C O N F I G =============================================

/**
  * \brief specialisation of DeviceConfig for keyboard type devices
  * \ingroup config
  */
class KeyboardConfig : public DeviceConfig
{

public:
    
    void        setDefaultBinds     ();
    void        serialize           (std::ofstream& stream);
    
    KeyboardConfig                  ();
};


//==== G A M E P A D C O N F I G ===============================================

/**
  * \brief specialisation of DeviceConfig for gamepad type devices
  * \ingroup config
  */
class GamepadConfig : public DeviceConfig
{
    
private:
    
    int         m_axis_count;
    int         m_button_count;
    
public:
    
    irr::core::stringw toString     ();
    int         getAxisCount()      const { return m_axis_count; };
    int         getButtonCount()    const { return m_button_count; };
    void        serialize           (std::ofstream& stream);
    void        setDefaultBinds     ();
    GamepadConfig           (irr::io::IrrXMLReader* xml);
    GamepadConfig           (const std::string      name,
                             const int              axis_count,
                             const int              btnCount);
    //        ~GamepadConfig();  
};

#endif
