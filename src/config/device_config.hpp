#ifndef DEVICE_CONFIG_HPP
#define DEVICE_CONFIG_HPP

#include <string>
#include "input/input.hpp"
#include <iostream>
#include <fstream>
#include "io/xml_node.hpp"
#include <irrString.h>

/**
  * \ingroup config
  */
struct KeyBinding
{
    
    Input::InputType        type;
    int                     id;
    Input::AxisDirection    dir;
    
};


//==== D E V I C E C O N F I G =================================================

/**
  * \brief contains the key bindings information related to one input device
  * \ingroup config
  */
class DeviceConfig
{
private:
    
    KeyBinding  m_bindings[PA_COUNT];
    bool        m_inuse;  //!< Is there a device connected to the system which uses this config?
    
protected:
    
    std::string m_name;
    
public:
    
    std::string getName()           const { return m_name; };
    irr::core::stringw getBindingAsString  (const PlayerAction action) const;
    irr::core::stringw toString     ();
    
    void        serialize           (std::ofstream& stream);
    bool        deserializeAction   (irr::io::IrrXMLReader* xml);
    
    void        setBinding          (const PlayerAction     action,
                                     const Input::InputType type,
                                     const int              id,
                                     Input::AxisDirection   direction = Input::AD_NEUTRAL);
    
    void        setInUse            (bool inuse) {m_inuse = inuse;}
    bool        isInUse            () {return m_inuse;}
    
    /**
      * Don't call this directly unless you are KeyboardDevice or GamepadDevice
      * \param[out] action  the result, only set if method returned true
      * \return             whether finding an action associated to this input was successful
      */
    bool        getAction           (Input::InputType       type, 
                                     const int              id,
                                     const int              value,
                                     PlayerAction*          action /* out */);
    KeyBinding  getBinding          (int i) {return m_bindings[i];}
    
    bool hasBindingFor(const int buttonID) const;
    
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
