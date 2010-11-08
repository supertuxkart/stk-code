#ifndef DEVICE_CONFIG_HPP
#define DEVICE_CONFIG_HPP

#include "input/input.hpp"
#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"

#include <fstream>
#include <iostream>
#include <irrString.h>
#include <string>

/**
  * \ingroup config
  */
struct KeyBinding
{
    
    Input::InputType        type;
    int                     id;
    Input::AxisDirection    dir;
    
};

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
    
    KeyBinding  m_bindings[PA_COUNT];
    bool        m_plugged;  //!< Is there a device connected to the system which uses this config?
    
protected:
    
    bool        m_enabled;  //!< If set to false, this device will be ignored. Currently for gamepads only

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
                                     Input::AxisDirection   direction = Input::AD_NEUTRAL);
    
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
    
    KeyBinding  getBinding          (int i) {return m_bindings[i];}
    
    bool hasBindingFor(const int buttonID) const;
    
    /** At this time only relevant for gamepads, keyboards are always enabled */
    bool isEnabled() const { return m_enabled; }
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
