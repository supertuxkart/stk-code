//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 Marianne Gagnon
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
    stream << "id=\""        << m_id        << "\" "
           << "event=\""     << m_type      << "\" "
           << "character=\"" << m_character << "\" ";

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

    m_character = 0;
    // XMLNode only supports stringw, not wchar_t*
    core::stringw s;
    action->get("character", &s);
    if(s.size()>0)
        m_character = s[0];

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
            if(m_character)
                s[0]=m_character;

            switch(m_id)
            {
#ifdef WIN32
            // Windows codes certain special keys, which have atm no defined
            // value in irr::KEY_*. So for now hardcode the values.
            // FIXME: what happens with international keyboards? E.g. A [
            // might be an Umlaut on a German keyboard. How do we get
            // the character to display in this case??
            case 186: s=";";  break;
            case 191: s="/";  break;
            case 192: s="`";  break;
            case 219: s="[";  break;
            case 220: s="\\"; break;
            case 221: s="]";  break;
            case 222: s="'";  break;
#endif
            case irr::KEY_LBUTTON    : s = "left mouse button";  break;
            case irr::KEY_RBUTTON    : s = "right mouse button"; break;
            case irr::KEY_CANCEL     : s = "cancel"; break;
            case irr::KEY_MBUTTON    : s = "middle mouse button"; break;
            case irr::KEY_XBUTTON1   : s = "X1 mouse button"; break;
            case irr::KEY_XBUTTON2   : s = "X2 mouse button"; break;
            case irr::KEY_BACK       : s = "backspace"; break;
            case irr::KEY_TAB        : s = "tab"; break;
            case irr::KEY_CLEAR      : s = "clear"; break;
            case irr::KEY_RETURN     : s = "return"; break;
            case irr::KEY_SHIFT      : s = "shift"; break;
            case irr::KEY_CONTROL    : s = "control"; break;
            case irr::KEY_MENU       : s = "alt/menu"; break;
            case irr::KEY_PAUSE      : s = "pause"; break;
            case irr::KEY_CAPITAL    : s = "caps lock"; break;
            case irr::KEY_KANA       : s = "kana"; break;
            case irr::KEY_JUNJA      : s = "junja"; break;
            case irr::KEY_FINAL      : s = "final"; break;
            case irr::KEY_ESCAPE     : s = "escape"; break;
            case irr::KEY_CONVERT    : s = "convert"; break;
            case irr::KEY_NONCONVERT : s = "nonconvert"; break;
            case irr::KEY_ACCEPT     : s = "accept"; break;
            case irr::KEY_MODECHANGE : s = "modechange"; break;
            case irr::KEY_SPACE      : s = "space"; break;
            case irr::KEY_PRIOR      : s = "page up"; break;
            case irr::KEY_NEXT       : s = "page down"; break;
            case irr::KEY_END        : s = "end"; break;
            case irr::KEY_HOME       : s = "home"; break;
            case irr::KEY_LEFT       : s = "left"; break;
            case irr::KEY_UP         : s = "up"; break;
            case irr::KEY_RIGHT      : s = "right"; break;
            case irr::KEY_DOWN       : s = "down"; break;
            case irr::KEY_SELECT     : s = "select"; break;
            case irr::KEY_PRINT      : s = "print"; break;
            case irr::KEY_EXECUT     : s = "exec"; break;
            case irr::KEY_SNAPSHOT   : s = "print screen"; break;
            case irr::KEY_INSERT     : s = "insert"; break;
            case irr::KEY_DELETE     : s = "delete"; break;
            case irr::KEY_HELP       : s = "help"; break;
            case irr::KEY_KEY_0      : s = "0"; break;
            case irr::KEY_KEY_1      : s = "1"; break;
            case irr::KEY_KEY_2      : s = "2"; break;
            case irr::KEY_KEY_3      : s = "3"; break;
            case irr::KEY_KEY_4      : s = "4"; break;
            case irr::KEY_KEY_5      : s = "5"; break;
            case irr::KEY_KEY_6      : s = "6"; break;
            case irr::KEY_KEY_7      : s = "7"; break;
            case irr::KEY_KEY_8      : s = "8"; break;
            case irr::KEY_KEY_9      : s = "9"; break;
            case irr::KEY_KEY_A      : s = "A"; break;
            case irr::KEY_KEY_B      : s = "B"; break;
            case irr::KEY_KEY_C      : s = "C"; break;
            case irr::KEY_KEY_D      : s = "D"; break;
            case irr::KEY_KEY_E      : s = "E"; break;
            case irr::KEY_KEY_F      : s = "F"; break;
            case irr::KEY_KEY_G      : s = "G"; break;
            case irr::KEY_KEY_H      : s = "H"; break;
            case irr::KEY_KEY_I      : s = "I"; break;
            case irr::KEY_KEY_J      : s = "J"; break;
            case irr::KEY_KEY_K      : s = "K"; break;
            case irr::KEY_KEY_L      : s = "L"; break;
            case irr::KEY_KEY_M      : s = "M"; break;
            case irr::KEY_KEY_N      : s = "N"; break;
            case irr::KEY_KEY_O      : s = "O"; break;
            case irr::KEY_KEY_P      : s = "P"; break;
            case irr::KEY_KEY_Q      : s = "Q"; break;
            case irr::KEY_KEY_R      : s = "R"; break;
            case irr::KEY_KEY_S      : s = "S"; break;
            case irr::KEY_KEY_T      : s = "T"; break;
            case irr::KEY_KEY_U      : s = "U"; break;
            case irr::KEY_KEY_V      : s = "V"; break;
            case irr::KEY_KEY_W      : s = "W"; break;
            case irr::KEY_KEY_X      : s = "X"; break;
            case irr::KEY_KEY_Y      : s = "Y"; break;
            case irr::KEY_KEY_Z      : s = "Z"; break;
            case irr::KEY_LWIN       : s = "Left Logo"; break;
            case irr::KEY_RWIN       : s = "Right Logo"; break;
            case irr::KEY_APPS       : s = "apps"; break;
            case irr::KEY_SLEEP      : s = "sleep"; break;
            case irr::KEY_NUMPAD0    : s = "numpad 0"; break;
            case irr::KEY_NUMPAD1    : s = "numpad 1"; break;
            case irr::KEY_NUMPAD2    : s = "numpad 2"; break;
            case irr::KEY_NUMPAD3    : s = "numpad 3"; break;
            case irr::KEY_NUMPAD4    : s = "numpad 4"; break;
            case irr::KEY_NUMPAD5    : s = "numpad 5"; break;
            case irr::KEY_NUMPAD6    : s = "numpad 6"; break;
            case irr::KEY_NUMPAD7    : s = "numpad 7"; break;
            case irr::KEY_NUMPAD8    : s = "numpad 8"; break;
            case irr::KEY_NUMPAD9    : s = "numpad 9"; break;
            case irr::KEY_MULTIPLY   : s = "*"; break;
            case irr::KEY_ADD        : s = "+"; break;
            case irr::KEY_SEPARATOR  : s = "separator"; break;
            case irr::KEY_SUBTRACT   : s = "- (subtract)"; break;
            case irr::KEY_DECIMAL    : s = "decimal"; break;
            case irr::KEY_DIVIDE     : s = "/ (divide)"; break;
            case irr::KEY_F1         : s = "F1"; break;
            case irr::KEY_F2         : s = "F2"; break;
            case irr::KEY_F3         : s = "F3"; break;
            case irr::KEY_F4         : s = "F4"; break;
            case irr::KEY_F5         : s = "F5"; break;
            case irr::KEY_F6         : s = "F6"; break;
            case irr::KEY_F7         : s = "F7"; break;
            case irr::KEY_F8         : s = "F8"; break;
            case irr::KEY_F9         : s = "F9"; break;
            case irr::KEY_F10        : s = "F10"; break;
            case irr::KEY_F11        : s = "F11"; break;
            case irr::KEY_F12        : s = "F12"; break;
            case irr::KEY_F13        : s = "F13"; break;
            case irr::KEY_F14        : s = "F14"; break;
            case irr::KEY_F15        : s = "F15"; break;
            case irr::KEY_F16        : s = "F16"; break;
            case irr::KEY_F17        : s = "F17"; break;
            case irr::KEY_F18        : s = "F18"; break;
            case irr::KEY_F19        : s = "F19"; break;
            case irr::KEY_F20        : s = "F20"; break;
            case irr::KEY_F21        : s = "F21"; break;
            case irr::KEY_F22        : s = "F22"; break;
            case irr::KEY_F23        : s = "F23"; break;
            case irr::KEY_F24        : s = "F24"; break;
            case irr::KEY_NUMLOCK    : s = "num lock"; break;
            case irr::KEY_SCROLL     : s = "scroll lock"; break;
            case irr::KEY_LSHIFT     : s = "left shift"; break;
            case irr::KEY_RSHIFT     : s = "right shift"; break;
            case irr::KEY_LCONTROL   : s = "left control"; break;
            case irr::KEY_RCONTROL   : s = "right control"; break;
            case irr::KEY_LMENU      : s = "left menu"; break;
            case irr::KEY_RMENU      : s = "right menu"; break;
            case irr::KEY_PLUS       : s = "+"; break;
            case irr::KEY_COMMA      : s = ","; break;
            case irr::KEY_MINUS      : s = "-"; break;
            case irr::KEY_PERIOD     : s = "."; break;
            case irr::KEY_ATTN       : s = "attn"; break;
            case irr::KEY_CRSEL      : s = "crsel"; break;
            case irr::KEY_EXSEL      : s = "exsel"; break;
            case irr::KEY_EREOF      : s = "ereof"; break;
            case irr::KEY_PLAY       : s = "play"; break;
            case irr::KEY_ZOOM       : s = "zoom"; break;
            case irr::KEY_PA1        : s = "pa1"; break;
            case irr::KEY_OEM_CLEAR  : s = "oem clear"; break;
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
