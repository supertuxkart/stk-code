//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "input/binding.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "utils/log.hpp"

#include <SKeyMap.h>

/** Convert this binding to XML attributes. The full XML node is actually
 *  written by device_config, so we only have to add the attributes here.
 */
void Binding::save(std::ofstream& stream) const
{
    stream << "event=\"" << m_type << "\" ";
    stream << "id=\"" << m_id << "\" ";

    // Only serialize the direction and the range for stick motions
    if (m_type == Input::IT_STICKMOTION)
    {
        stream << "direction=\"" << m_dir    << "\" ";
        stream << "range=\""     << m_range    << "\" ";
    }
}   // save

// ----------------------------------------------------------------------------
bool Binding::load(const XMLNode *action)
{
    int n;
    if(!action->get("id", &m_id) || !action->get("event", &n)  )
    {
        Log::warn("Binding", "No id-string or event-string given - ignored.");
        return false;
    }
    m_type = (Input::InputType)n;
    // Default settings for button
    m_range = Input::AR_HALF;
    m_dir = Input::AD_NEUTRAL;

    // If the action is not a stick motion (button or key)
    if (m_type == Input::IT_STICKMOTION)
    {
        if(!action->get("direction", &n))
        {
            Log::warn("Binding", "IT_STICKMOTION without direction, ignoring.");
            return false;
        }
        m_dir = (Input::AxisDirection)n;

        if(!action->get("range", &n)) m_range = Input::AR_HALF;
        else                          m_range = (Input::AxisRange)n;

    }   // if m_type!=stickmotion
    return true;
}   // load

// ----------------------------------------------------------------------------
/** Returns a string representing this binding, which can be displayed on the
 *  screen.
 */
irr::core::stringw Binding::getAsString() const
{
    irr::core::stringw s;
    switch (m_type)
    {
        case Input::IT_NONE:
            //I18N: Unbound key binding
            s = _("[none]");
            break;
        case Input::IT_KEYBOARD:
            s = "?";
            switch(m_id)
            {
             //I18N: input configuration screen: mouse button
            case irr::IRR_KEY_LBUTTON    : s = _C("input_key", "Left Mouse Button");  break;
             //I18N: input configuration screen: mouse button
            case irr::IRR_KEY_RBUTTON    : s = _C("input_key", "Right Mouse Button"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CANCEL     : s = _C("input_key", "Cancel"); break;
             //I18N: input configuration screen: mouse button
            case irr::IRR_KEY_MBUTTON    : s = _C("input_key", "Middle Mouse Button"); break;
             //I18N: input configuration screen: mouse button
            case irr::IRR_KEY_XBUTTON1   : s = _C("input_key", "X1 Mouse Button"); break;
             //I18N: input configuration screen: mouse button
            case irr::IRR_KEY_XBUTTON2   : s = _C("input_key", "X2 Mouse Button"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_BACK       : s = _C("input_key", "Backspace"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_TAB        : s = _C("input_key", "Tab"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CLEAR      : s = _C("input_key", "Clear"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RETURN     : s = _C("input_key", "Return"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SHIFT      : s = _C("input_key", "Shift"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CONTROL    : s = _C("input_key", "Control"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_MENU       : s = _C("input_key", "Alt/Menu"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_PAUSE      : s = _C("input_key", "Pause"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CAPITAL    : s = _C("input_key", "Caps Lock"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_KANA       : s = _C("input_key", "Kana"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_JUNJA      : s = _C("input_key", "Junja"); break;
             //I18N: input configuration screen: keyboard key
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_FINAL      : s = _C("input_key", "Final"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_ESCAPE     : s = _C("input_key", "Escape"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CONVERT    : s = _C("input_key", "Convert"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NONCONVERT : s = _C("input_key", "Nonconvert"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_ACCEPT     : s = _C("input_key", "Accept"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_MODECHANGE : s = _C("input_key", "Modechange"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SPACE      : s = _C("input_key", "Space"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_PRIOR      : s = _C("input_key", "Page Up"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NEXT       : s = _C("input_key", "Page Down"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_END        : s = _C("input_key", "End"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_HOME       : s = _C("input_key", "Home"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_LEFT       : s = _C("input_key", "Left"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_UP         : s = _C("input_key", "Up"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RIGHT      : s = _C("input_key", "Right"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_DOWN       : s = _C("input_key", "Down"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SELECT     : s = _C("input_key", "Select"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_PRINT      : s = _C("input_key", "Print"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_EXECUT     : s = _C("input_key", "Exec"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SNAPSHOT   : s = _C("input_key", "Print Screen"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_INSERT     : s = _C("input_key", "Insert"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_DELETE     : s = _C("input_key", "Delete"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_HELP       : s = _C("input_key", "Help"); break;
            case irr::IRR_KEY_0          : s = "0"; break;
            case irr::IRR_KEY_1          : s = "1"; break;
            case irr::IRR_KEY_2          : s = "2"; break;
            case irr::IRR_KEY_3          : s = "3"; break;
            case irr::IRR_KEY_4          : s = "4"; break;
            case irr::IRR_KEY_5          : s = "5"; break;
            case irr::IRR_KEY_6          : s = "6"; break;
            case irr::IRR_KEY_7          : s = "7"; break;
            case irr::IRR_KEY_8          : s = "8"; break;
            case irr::IRR_KEY_9          : s = "9"; break;
            case irr::IRR_KEY_A          : s = "A"; break;
            case irr::IRR_KEY_B          : s = "B"; break;
            case irr::IRR_KEY_C          : s = "C"; break;
            case irr::IRR_KEY_D          : s = "D"; break;
            case irr::IRR_KEY_E          : s = "E"; break;
            case irr::IRR_KEY_F          : s = "F"; break;
            case irr::IRR_KEY_G          : s = "G"; break;
            case irr::IRR_KEY_H          : s = "H"; break;
            case irr::IRR_KEY_I          : s = "I"; break;
            case irr::IRR_KEY_J          : s = "J"; break;
            case irr::IRR_KEY_K          : s = "K"; break;
            case irr::IRR_KEY_L          : s = "L"; break;
            case irr::IRR_KEY_M          : s = "M"; break;
            case irr::IRR_KEY_N          : s = "N"; break;
            case irr::IRR_KEY_O          : s = "O"; break;
            case irr::IRR_KEY_P          : s = "P"; break;
            case irr::IRR_KEY_Q          : s = "Q"; break;
            case irr::IRR_KEY_R          : s = "R"; break;
            case irr::IRR_KEY_S          : s = "S"; break;
            case irr::IRR_KEY_T          : s = "T"; break;
            case irr::IRR_KEY_U          : s = "U"; break;
            case irr::IRR_KEY_V          : s = "V"; break;
            case irr::IRR_KEY_W          : s = "W"; break;
            case irr::IRR_KEY_X          : s = "X"; break;
            case irr::IRR_KEY_Y          : s = "Y"; break;
            case irr::IRR_KEY_Z          : s = "Z"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_LWIN       : s = _C("input_key", "Left Logo"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RWIN       : s = _C("input_key", "Right Logo"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_APPS       : s = _C("input_key", "Apps"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SLEEP      : s = _C("input_key", "Sleep"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD0    : s = _C("input_key", "Numpad 0"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD1    : s = _C("input_key", "Numpad 1"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD2    : s = _C("input_key", "Numpad 2"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD3    : s = _C("input_key", "Numpad 3"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD4    : s = _C("input_key", "Numpad 4"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD5    : s = _C("input_key", "Numpad 5"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD6    : s = _C("input_key", "Numpad 6"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD7    : s = _C("input_key", "Numpad 7"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD8    : s = _C("input_key", "Numpad 8"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMPAD9    : s = _C("input_key", "Numpad 9"); break;
            case irr::IRR_KEY_MULTIPLY   : s = "*"; break;
            case irr::IRR_KEY_ADD        : s = "+"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SEPARATOR  : s = _C("input_key", "Separator"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SUBTRACT   : s = _C("input_key", "- (Subtract)"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_DECIMAL    : s = _C("input_key", "Decimal"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_DIVIDE     : s = _C("input_key", "/ (Divide)"); break;
            case irr::IRR_KEY_F1         : s = "F1"; break;
            case irr::IRR_KEY_F2         : s = "F2"; break;
            case irr::IRR_KEY_F3         : s = "F3"; break;
            case irr::IRR_KEY_F4         : s = "F4"; break;
            case irr::IRR_KEY_F5         : s = "F5"; break;
            case irr::IRR_KEY_F6         : s = "F6"; break;
            case irr::IRR_KEY_F7         : s = "F7"; break;
            case irr::IRR_KEY_F8         : s = "F8"; break;
            case irr::IRR_KEY_F9         : s = "F9"; break;
            case irr::IRR_KEY_F10        : s = "F10"; break;
            case irr::IRR_KEY_F11        : s = "F11"; break;
            case irr::IRR_KEY_F12        : s = "F12"; break;
            case irr::IRR_KEY_F13        : s = "F13"; break;
            case irr::IRR_KEY_F14        : s = "F14"; break;
            case irr::IRR_KEY_F15        : s = "F15"; break;
            case irr::IRR_KEY_F16        : s = "F16"; break;
            case irr::IRR_KEY_F17        : s = "F17"; break;
            case irr::IRR_KEY_F18        : s = "F18"; break;
            case irr::IRR_KEY_F19        : s = "F19"; break;
            case irr::IRR_KEY_F20        : s = "F20"; break;
            case irr::IRR_KEY_F21        : s = "F21"; break;
            case irr::IRR_KEY_F22        : s = "F22"; break;
            case irr::IRR_KEY_F23        : s = "F23"; break;
            case irr::IRR_KEY_F24        : s = "F24"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_NUMLOCK    : s = _C("input_key", "Num Lock"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_SCROLL     : s = _C("input_key", "Scroll Lock"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_LSHIFT     : s = _C("input_key", "Left Shift"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RSHIFT     : s = _C("input_key", "Right Shift"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_LCONTROL   : s = _C("input_key", "Left Control"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RCONTROL   : s = _C("input_key", "Right Control"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_LMENU      : s = _C("input_key", "Left Menu"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_RMENU      : s = _C("input_key", "Right Menu"); break;
            case irr::IRR_KEY_PLUS       : s = "+"; break;
            case irr::IRR_KEY_COMMA      : s = ","; break;
            case irr::IRR_KEY_MINUS      : s = "-"; break;
            case irr::IRR_KEY_PERIOD     : s = "."; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_ATTN       : s = _C("input_key", "Attn"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_CRSEL      : s = _C("input_key", "Crsel"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_EXSEL      : s = _C("input_key", "Exsel"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_EREOF      : s = _C("input_key", "Ereof"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_PLAY       : s = _C("input_key", "Play"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_ZOOM       : s = _C("input_key", "Zoom"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_PA1        : s = _C("input_key", "Pa1"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_CLEAR  : s = _C("input_key", "Oem Clear"); break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_1      : s = ";"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_2      : s = "/"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_3      : s = "`"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_4      : s = "["; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_5      : s = "\\"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_6      : s = "]"; break;
             //I18N: input configuration screen: keyboard key
            case irr::IRR_KEY_OEM_7      : s = "'"; break;
            
            // for azerty layout
            case irr::IRR_KEY_AMPERSAND   : s = "&"; break;
            case irr::IRR_KEY_EACUTE      : s = "é"; break;
            case irr::IRR_KEY_QUOTEDBL    : s = "\""; break;
            case irr::IRR_KEY_PARENLEFT   : s = "("; break;
            case irr::IRR_KEY_EGRAVE      : s = "è"; break;
            case irr::IRR_KEY_CCEDILLA    : s = "ç"; break;
            case irr::IRR_KEY_AGRAVE      : s = "à"; break;
            case irr::IRR_KEY_PARENRIGHT  : s = ")"; break;
            case irr::IRR_KEY_UGRAVE      : s = "ù"; break;
            case irr::IRR_KEY_COLON       : s = ":"; break;
            case irr::IRR_KEY_DOLLAR      : s = "$"; break;
            case irr::IRR_KEY_EXCLAM      : s = "!"; break;
            case irr::IRR_KEY_TWOSUPERIOR : s = "²"; break;
            case irr::IRR_KEY_MU          : s = "µ"; break;
            case irr::IRR_KEY_SECTION     : s = "§"; break;
        }

            break;
        case Input::IT_STICKMOTION:

            if (m_id == Input::HAT_H_ID)
            {
                //I18N: to appear in input configuration screen, for gamepad hats
                s = _("Gamepad hat %d", (m_dir == Input::AD_NEGATIVE ? L"0-" : L"0+"));
            }
            else if (m_id == Input::HAT_V_ID)
            {
                //I18N: to appear in input configuration screen, for gamepad hats
                s = _("Gamepad hat %d", (m_dir == Input::AD_NEGATIVE ? L"1-" : L"1+"));
            }
            else
            {
                if (m_range == Input::AR_HALF)
                {
                    //I18N: to appear in input configuration screen, for gamepad axes
                    s = _("Axis %d %s", m_id, (m_dir == Input::AD_NEGATIVE) ? L"-" : L"+");
                }
                else
                {
                    if(m_dir == Input::AD_NEGATIVE)
                    {
                        //I18N: to appear in input configuration screen, for gamepad axes
                        s = _("Axis %d inverted", m_id);
                    }
                    else
                    {
                        //I18N: to appear in input configuration screen, for gamepad axes
                        s = _("Axis %d", m_id);
                    }
                }
            }
            break;
        case Input::IT_STICKBUTTON:
            //I18N: to appear in input configuration screen, for gamepad buttons
            s = ( _("Gamepad button %d", m_id+1));
            break;        case Input::IT_MOUSEBUTTON:
            //I18N: to appear in input configuration screen, for mouse (might not be used at all)
            s = _("Mouse button %d", (m_id+1));
            break;
        case Input::IT_MOUSEMOTION: // FIXME : I don't reckon this is used at all
            //I18N: to appear in input configuration screen, for mouse (might not be used at all)
            s = _("Mouse axis %d %s", (m_id+1), (m_dir == Input::AD_NEGATIVE) ? '-': '+');
            break;
        default:
            s = "?";
    }

    return s;
}   // GetKeyAsString
