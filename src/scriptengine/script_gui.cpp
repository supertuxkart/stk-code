//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015  SuperTuxKart Team
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

#include "script_track.hpp"

#include "animations/three_d_animation.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"

#include <angelscript.h>
#include "scriptarray.hpp"

#include <assert.h>
#include <iostream> //debug

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
/** \endcond */

    namespace GUI
    {
        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup GUI
        * @{
        */

        /** Get the key bound to a player action (enum GUI::PlayerAction)*/
        std::string getKeyBinding(int Enum_value)
        {
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            DeviceConfig* config = device->getConfiguration();
            irr::core::stringw control;
            PlayerAction ScriptAction = (PlayerAction)Enum_value;
            control = config->getBindingAsString(ScriptAction);
            std::string key = std::string(irr::core::stringc(control).c_str());
            return key;
        }

        /** Show the specified message in a popup */
        void displayMessage(std::string* input)
        {
            irr::core::stringw out = StringUtils::utf8_to_wide(input->c_str());
            new TutorialMessageDialog((out), true);
        }

        /** Get translated version of string */
        std::string translate(std::string* input)
        {
            irr::core::stringw out = translations->fribidize(translations->w_gettext(input->c_str()));

            return StringUtils::wide_to_utf8(out.c_str());
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()));

            out = translations->fribidize(out);

            return StringUtils::wide_to_utf8(out.c_str());
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1, std::string* arg2)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()));

            out = translations->fribidize(out);

            return StringUtils::wide_to_utf8(out.c_str());
        }

        /** Translate string and insert values. e.g. GUI::translate("Hello %s !", "John") */
        std::string translate(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            irr::core::stringw out = translations->w_gettext(formatString->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()),
                StringUtils::utf8_to_wide(arg3->c_str()));

            out = translations->fribidize(out);

            return StringUtils::wide_to_utf8(out.c_str());
        }
        /** @}*/
        /** @}*/

        // UNDOCUMENTED PROXIES : Use proxies to have different signatures, then redirect to the
        // documented function whose name is exposed in angelscript (these proxies exist so that
        // angelscript can properly resolve overloads, but doxygen can still generate the right docs
        /** \cond DOXYGEN_IGNORE */
        std::string proxy_translate(std::string* formatString)
        {
            return translate(formatString);
        }

        std::string proxy_translateAndInsertValues1(std::string* formatString, std::string* arg1)
        {
            return translate(formatString, arg1);
        }

        std::string proxy_translateAndInsertValues2(std::string* formatString, std::string* arg1, std::string* arg2)
        {
            return translate(formatString, arg1, arg2);
        }

        std::string proxy_translateAndInsertValues3(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            return translate(formatString, arg1, arg2, arg3);
        }
        /** \endcond */

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes
            engine->SetDefaultNamespace("GUI");
            r = engine->RegisterGlobalFunction("void displayMessage(const string &in)", asFUNCTION(displayMessage), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string getKeyBinding(int input)", asFUNCTION(getKeyBinding), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in)", asFUNCTION(proxy_translate), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in)", asFUNCTION(proxy_translateAndInsertValues1), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in)", asFUNCTION(proxy_translateAndInsertValues2), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in, const string &in)", asFUNCTION(proxy_translateAndInsertValues3), asCALL_CDECL); assert(r >= 0);
        }

        void registerScriptEnums(asIScriptEngine *engine)
        {
            // TODO: document enum in doxygen-generated scripting docs
            engine->SetDefaultNamespace("GUI");
            engine->RegisterEnum("PlayerAction");
            engine->RegisterEnumValue("PlayerAction", "STEER_LEFT", PA_STEER_LEFT);
            engine->RegisterEnumValue("PlayerAction", "STEER_RIGHT", PA_STEER_RIGHT);
            engine->RegisterEnumValue("PlayerAction", "ACCEL", PA_ACCEL);
            engine->RegisterEnumValue("PlayerAction", "BRAKE", PA_BRAKE);
            engine->RegisterEnumValue("PlayerAction", "NITRO", PA_NITRO);
            engine->RegisterEnumValue("PlayerAction", "DRIFT", PA_DRIFT);
            engine->RegisterEnumValue("PlayerAction", "RESCUE", PA_RESCUE);
            engine->RegisterEnumValue("PlayerAction", "FIRE", PA_FIRE);
            engine->RegisterEnumValue("PlayerAction", "LOOK_BACK", PA_LOOK_BACK);
            engine->RegisterEnumValue("PlayerAction", "PAUSE_RACE", PA_PAUSE_RACE);
            engine->RegisterEnumValue("PlayerAction", "MENU_UP", PA_MENU_UP);
            engine->RegisterEnumValue("PlayerAction", "MENU_DOWN", PA_MENU_DOWN);
            engine->RegisterEnumValue("PlayerAction", "MENU_LEFT", PA_MENU_LEFT);
            engine->RegisterEnumValue("PlayerAction", "MENU_RIGHT", PA_MENU_RIGHT);
            engine->RegisterEnumValue("PlayerAction", "MENU_SELECT", PA_MENU_SELECT);
            engine->RegisterEnumValue("PlayerAction", "MENU_CANCEL", PA_MENU_CANCEL);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
