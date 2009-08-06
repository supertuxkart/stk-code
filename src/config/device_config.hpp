#ifndef DEVICE_CONFIG_HPP
#define DEVICE_CONFIG_HPP

#include <string>
#include "input/input.hpp"
#include <iostream>
#include <fstream>
#include "io/xml_node.hpp"


struct KeyBinding
{

    Input::InputType        type;
    int                     id;
    Input::AxisDirection    dir;

};


//==== D E V I C E C O N F I G =================================================

class DeviceConfig
{
    private:

        KeyBinding  m_bindings[PA_COUNT];

    protected:

    public:

        std::string getBindingAsString  (const PlayerAction action) const;
        std::string toString            ();

        void        serialize           (std::ofstream& stream);
        bool        deserializeAction   (irr::io::IrrXMLReader* xml);

        void        setBinding          (const PlayerAction     action,
                                         const Input::InputType type,
                                         const int              id,
                                         Input::AxisDirection   direction = Input::AD_NEUTRAL);

        bool        getBinding          (Input::InputType       type, 
                                         const int              id,
                                         const int              value,
                                         PlayerAction*          action);

};

//==== K E Y B O A R D C O N F I G =============================================

class KeyboardConfig : public DeviceConfig
{
    private:

    protected:

    public:

        void        setDefaultBinds     ();
        void        serialize           (std::ofstream& stream);

//        KeyboardConfig                  (irr::io::IrrXMLReader* xml);
        KeyboardConfig                  ();
};


//==== G A M E P A D C O N F I G ===============================================

class GamepadConfig : public DeviceConfig
{

    private:

        std::string m_name;
        int         m_axis_count;
        int         m_button_count;

    public:

        std::string toString            ();
        std::string getName()           const { return m_name; };
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
