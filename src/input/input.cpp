//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2008 Robert Schuster <robertschuster@fsfe.org>
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


#include "input/input.hpp"

#include <irrlicht.h>
using namespace irr;

#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

// -----------------------------------------------------------------------------

irr::core::stringw Input::getInputAsString(const Input::InputType type, const int id,
                                           const Input::AxisDirection dir)
{
    irr::core::stringw s;
    
    switch (type)
    {
        case Input::IT_NONE:
            //I18N: Unbound key binding
            s = _("[none]");
            break;
        case Input::IT_KEYBOARD:
            s = "?";
            
            switch(id)
        {
            case KEY_LBUTTON : 
                s = "left mouse button";
                break;
            case KEY_RBUTTON :
                s = "right mouse button";
                break;
            case KEY_CANCEL :   
                s = "cancel";
                break;
            case KEY_MBUTTON :  
                s = "middle mouse button";
                break;
            case KEY_XBUTTON1 : 
                s = "X1 mouse button";
                break;
            case KEY_XBUTTON2 : 
                s = "X2 mouse button";
                break;
            case KEY_BACK : 
                s = "backspace";
                break;
            case KEY_TAB :  
                s = "tab";
                break;
            case KEY_CLEAR :    
                s = "clear";
                break;
            case KEY_RETURN :   
                s = "return";
                break;
            case KEY_SHIFT :
                s = "shift";
                break;
            case KEY_CONTROL :  
                s = "control";
                break;
            case KEY_MENU : 
                s = "alt/menu";
                break;
            case KEY_PAUSE :
                s = "pause";
                break;
            case KEY_CAPITAL :  
                s = "caps lock";
                break;
            case KEY_KANA : 
                s = "kana";
                break;
            //case KEY_HANGUEL :    
            //case KEY_HANGUL :
            //    s = "hangul";
                break;
            case KEY_JUNJA :    
                s = "junja";
                break;
            case KEY_FINAL :    
                s = "final";
                break;
            //case KEY_HANJA :  
            //    s = "hanja";
            //    break;
            //case KEY_KANJI :  
            //    s = "kanji";
            //    break;
            case KEY_ESCAPE :   
                s = "escape";
                break;
            case KEY_CONVERT :  
                s = "convert";
                break;
            case KEY_NONCONVERT :   
                s = "nonconvert";
                break;
            case KEY_ACCEPT :   
                s = "accept";
                break;
            case KEY_MODECHANGE :   
                s = "modechange";
                break;
            case KEY_SPACE :    
                s = "space";
                break;
            case KEY_PRIOR :    
                s = "page up";
                break;
            case KEY_NEXT : 
                s = "page down";
                break;
            case KEY_END :
                s = "end";
                break;
            case KEY_HOME : 
                s = "home";
                break;
            case KEY_LEFT : 
                s = "left";
                break;
            case KEY_UP :   
                s = "up";
                break;
            case KEY_RIGHT :
                s = "right";
                break;
            case KEY_DOWN : 
                s = "down";
                break;
            case KEY_SELECT :   
                s = "select";
                break;
            case KEY_PRINT :    
                s = "print";
                break;
            case KEY_EXECUT :   
                s = "exec";
                break;
            case KEY_SNAPSHOT :
                s = "print screen";
                break;
            case KEY_INSERT :
                s = "insert";
                break;
            case KEY_DELETE :   
                s = "delete";
                break;
            case KEY_HELP : 
                s = "help";
                break;
            case KEY_KEY_0 :    
                s = "0";
                break;
            case KEY_KEY_1 :    
                s = "1";
                break;
            case KEY_KEY_2 :
                s = "2";
                break;
            case KEY_KEY_3 :    
                s = "3";
                break;
            case KEY_KEY_4 :
                s = "4";
                break;
            case KEY_KEY_5 :    
                s = "5";
                break;
            case KEY_KEY_6 :    
                s = "6";
                break;
            case KEY_KEY_7 :    
                s = "7";
                break;
            case KEY_KEY_8 :    
                s = "8";
                break;
            case KEY_KEY_9 :    
                s = "9";
                break;
            case KEY_KEY_A :    
                s = "A";
                break;
            case KEY_KEY_B :    
                s = "B";
                break;
            case KEY_KEY_C :    
                s = "C";
                break;
            case KEY_KEY_D :    
                s = "D";
                break;
            case KEY_KEY_E :    
                s = "E";
                break;
            case KEY_KEY_F :    
                s = "F";
                break;
            case KEY_KEY_G :    
                s = "G";
                break;
            case KEY_KEY_H :
                s = "H";
                break;
            case KEY_KEY_I :    
                s = "I";
                break;
            case KEY_KEY_J :    
                s = "J";
                break;
            case KEY_KEY_K :    
                s = "K";
                break;
            case KEY_KEY_L :    
                s = "L";
                break;
            case KEY_KEY_M :    
                s = "M";
                break;
            case KEY_KEY_N :    
                s = "N";
                break;
            case KEY_KEY_O :    
                s = "O";
                break;
            case KEY_KEY_P :    
                s = "P";
                break;
            case KEY_KEY_Q :    
                s = "Q";
                break;
            case KEY_KEY_R :    
                s = "R";
                break;
            case KEY_KEY_S :    
                s = "S";
                break;
            case KEY_KEY_T :    
                s = "T";
                break;
            case KEY_KEY_U :    
                s = "U";
                break;
            case KEY_KEY_V :    
                s = "V";
                break;
            case KEY_KEY_W :    
                s = "W";
                break;
            case KEY_KEY_X :
                s = "X";
                break;
            case KEY_KEY_Y :    
                s = "Y";
                break;
            case KEY_KEY_Z :    
                s = "Z";
                break;
            case KEY_LWIN : 
                s = "Left Logo";
                break;
            case KEY_RWIN : 
                s = "Right Logo";
                break;
            case KEY_APPS : 
                s = "apps";
                break;
            case KEY_SLEEP :    
                s = "sleep";
                break;
            case KEY_NUMPAD0 :  
                s = "numpad 0";
                break;
            case KEY_NUMPAD1 :  
                s = "numpad 1";
                break;
            case KEY_NUMPAD2 :  
                s = "numpad 2";
                break;
            case KEY_NUMPAD3 :  
                s = "numpad 3";
                break;
            case KEY_NUMPAD4 :
                s = "numpad 4";
                break;
            case KEY_NUMPAD5 :
                s = "numpad 5";
                break;
            case KEY_NUMPAD6 :  
                s = "numpad 6";
                break;
            case KEY_NUMPAD7 :  
                s = "numpad 7";
                break;
            case KEY_NUMPAD8 :  
                s = "numpad 8";
                break;
            case KEY_NUMPAD9 :  
                s = "numpad 9";
                break;
            case KEY_MULTIPLY : 
                s = "*";
                break;
            case KEY_ADD :  
                s = "+";
                break;
            case KEY_SEPARATOR :    
                s = "separator";
                break;
            case KEY_SUBTRACT : 
                s = "- (subtract)";
                break;
            case KEY_DECIMAL :  
                s = "decimal";
                break;
            case KEY_DIVIDE :   
                s = "/ (divide)";
                break;
            case KEY_F1 :   
                s = "F1";
                break;
            case KEY_F2 :   
                s = "F2";
                break;
            case KEY_F3 :   
                s = "F3";
                break;
            case KEY_F4 :   
                s = "F4";
                break;
            case KEY_F5 :   
                s = "F5";
                break;
            case KEY_F6 :   
                s = "F6";
                break;
            case KEY_F7 :   
                s = "F7";
                break;
            case KEY_F8 :   
                s = "F8";
                break;
            case KEY_F9 :
                s = "F9";
                break;
            case KEY_F10 :  
                s = "F10";
                break;
            case KEY_F11 :  
                s = "F11";
                break;
            case KEY_F12 :  
                s = "F12";
                break;
            case KEY_F13 :  
                s = "F13";
                break;
            case KEY_F14 :  
                s = "F14";
                break;
            case KEY_F15 :  
                s = "F15";
                break;
            case KEY_F16 :  
                s = "F16";
                break;
            case KEY_F17 :  
                s = "F17";
                break;
            case KEY_F18 :  
                s = "F18";
                break;
            case KEY_F19 :  
                s = "F19";
                break;
            case KEY_F20 :  
                s = "F20";
                break;
            case KEY_F21 :  
                s = "F21";
                break;
            case KEY_F22 :
                s = "F22";
                break;
            case KEY_F23 :  
                s = "F23";
                break;
            case KEY_F24 :  
                s = "F24";
                break;
            case KEY_NUMLOCK :  
                s = "num lock";
                break;
            case KEY_SCROLL :   
                s = "scroll lock";
                break;
            case KEY_LSHIFT :
                s = "left shift";
                break;
            case KEY_RSHIFT :   
                s = "right shift";
                break;
            case KEY_LCONTROL : 
                s = "left control";
                break;
            case KEY_RCONTROL : 
                s = "right control";
                break;
            case KEY_LMENU :    
                s = "left menu";
                break;
            case KEY_RMENU :    
                s = "right menu";
                break;
            case KEY_PLUS : 
                s = "+";
                break;
            case KEY_COMMA :
                s = ",";
                break;
            case KEY_MINUS :
                s = "-";
                break;
            case KEY_PERIOD :
                s = ".";
                break;
            case KEY_ATTN : 
                s = "attn";
                break;
            case KEY_CRSEL :
                s = "crsel";
                break;
            case KEY_EXSEL :
                s = "exsel";
                break;
            case KEY_EREOF :    
                s = "ereof";
                break;
            case KEY_PLAY : 
                s = "play";
                break;
            case KEY_ZOOM :
                s = "zoom";
                break;
            case KEY_PA1 :  
                s = "pa1";
                break;
            case KEY_OEM_CLEAR:
                s = "oem clear";
                break;
        }
            
            break;
        case Input::IT_STICKMOTION:
            //I18N: to appear in input configuration screen, for gamepad axes
            s = StringUtils::insertValues( _("Axis %d %s"), id, (dir == Input::AD_NEGATIVE) ? L"-" : L"+");
            break;
        case Input::IT_STICKBUTTON:
            //I18N: to appear in input configuration screen, for gamepad buttons
            s = StringUtils::insertValues( _("Gamepad button %d"), (id+1));
            break;
        case Input::IT_STICKHAT:
            //I18N: to appear in input configuration screen, for gamepad hats
            s = StringUtils::insertValues( _("Gamepad hat %d"), (id+1));
            break;
        case Input::IT_MOUSEBUTTON:
            //I18N: to appear in input configuration screen, for mouse (might not be used at all)
            s = StringUtils::insertValues( _("Mouse button %d"), (id+1));
            break;
        case Input::IT_MOUSEMOTION: // FIXME : I don't reckon this is used at all
            //I18N: to appear in input configuration screen, for mouse (might not be used at all)
            s = StringUtils::insertValues( _("Mouse axis %d %s"),
                                           (id+1), 
                                           (dir == Input::AD_NEGATIVE) 
                                           ? '-': '+'                        );
            break;
        default:
            s = "?";
    }
    
    return s;
}   // GetKeyAsString
