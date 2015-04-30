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

namespace Scripting
{
    namespace GUI
    {
        //getter for key binding for player action enums
        void getKeyBinding(asIScriptGeneric *gen)
        {
            int Enum_value = (int)gen->GetArgDWord(0);
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            DeviceConfig* config = device->getConfiguration();
            irr::core::stringw control;
            PlayerAction ScriptAction = (PlayerAction)Enum_value;
            control = config->getBindingAsString(ScriptAction);
            std::string key = std::string(irr::core::stringc(control).c_str());
            void *key_pointer = &key;
            gen->SetReturnObject(key_pointer);
        }

        // Displays the message specified in displayMessage( string message ) within the script
        void displayMessage(asIScriptGeneric *gen)
        {
            std::string *input = (std::string*)gen->GetArgAddress(0);
            irr::core::stringw out = StringUtils::utf8_to_wide(input->c_str());
            new TutorialMessageDialog((out), true);
        }


        void translate(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);

            irr::core::stringw out = translations->fribidize(translations->w_gettext(input->c_str()));

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void insertValues1(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);

            irr::core::stringw out = StringUtils::insertValues(StringUtils::utf8_to_wide(input->c_str()),
                StringUtils::utf8_to_wide(arg1->c_str()));

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void insertValues2(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);
            std::string *arg2 = (std::string*)gen->GetArgAddress(2);

            irr::core::stringw out = StringUtils::insertValues(StringUtils::utf8_to_wide(input->c_str()),
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()));

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void insertValues3(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);
            std::string *arg2 = (std::string*)gen->GetArgAddress(2);
            std::string *arg3 = (std::string*)gen->GetArgAddress(3);

            irr::core::stringw out = StringUtils::insertValues(StringUtils::utf8_to_wide(input->c_str()),
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()),
                StringUtils::utf8_to_wide(arg3->c_str()));

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void translateAndInsertValues1(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);

            irr::core::stringw out = translations->w_gettext(input->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()));

            out = translations->fribidize(out);

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void translateAndInsertValues2(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);
            std::string *arg2 = (std::string*)gen->GetArgAddress(2);

            irr::core::stringw out = translations->w_gettext(input->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()));

            out = translations->fribidize(out);

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }

        void translateAndInsertValues3(asIScriptGeneric *gen)
        {
            // Get the arguments
            std::string *input = (std::string*)gen->GetArgAddress(0);
            std::string *arg1 = (std::string*)gen->GetArgAddress(1);
            std::string *arg2 = (std::string*)gen->GetArgAddress(2);
            std::string *arg3 = (std::string*)gen->GetArgAddress(3);

            irr::core::stringw out = translations->w_gettext(input->c_str());

            out = StringUtils::insertValues(out,
                StringUtils::utf8_to_wide(arg1->c_str()),
                StringUtils::utf8_to_wide(arg2->c_str()),
                StringUtils::utf8_to_wide(arg3->c_str()));

            out = translations->fribidize(out);

            // Return the string
            new(gen->GetAddressOfReturnLocation()) std::string(StringUtils::wide_to_utf8(out.c_str()));
        }
        
        void scriptLogInfo(asIScriptGeneric *gen)
        {
            std::string input = *(std::string*)gen->GetArgAddress(0);
            Log::info("Script", "%s", input.c_str());
        }

        void scriptLogWarning(asIScriptGeneric *gen)
        {
            std::string input = *(std::string*)gen->GetArgAddress(0);
            Log::warn("Script", "%s", input.c_str());
        }

        void scriptLogError(asIScriptGeneric *gen)
        {
            std::string input = *(std::string*)gen->GetArgAddress(0);
            Log::error("Script", "%s", input.c_str());
        }

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes
            engine->SetDefaultNamespace("GUI");
            r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(displayMessage), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string getKeyBinding(int input)", asFUNCTION(getKeyBinding), asCALL_GENERIC); assert(r >= 0);

            r = engine->RegisterGlobalFunction("string translate(const string &in)", asFUNCTION(translate), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in)", asFUNCTION(translateAndInsertValues1), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in)", asFUNCTION(translateAndInsertValues2), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string translate(const string &in, const string &in, const string &in, const string &in)", asFUNCTION(translateAndInsertValues3), asCALL_GENERIC); assert(r >= 0);
            
            engine->SetDefaultNamespace("Utils");
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in)", asFUNCTION(insertValues1), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in)", asFUNCTION(insertValues2), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in)", asFUNCTION(insertValues3), asCALL_GENERIC); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void logInfo(const string &in)", asFUNCTION(scriptLogInfo), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void logWarning(const string &in)", asFUNCTION(scriptLogWarning), asCALL_GENERIC); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void logError(const string &in)", asFUNCTION(scriptLogError), asCALL_GENERIC); assert(r >= 0);
        }

        void registerScriptEnums(asIScriptEngine *engine)
        {
            engine->SetDefaultNamespace("GUI");
            engine->RegisterEnum("PA");
            engine->RegisterEnumValue("PA", "STEER_LEFT", PA_STEER_LEFT);
            engine->RegisterEnumValue("PA", "STEER_RIGHT", PA_STEER_RIGHT);
            engine->RegisterEnumValue("PA", "ACCEL", PA_ACCEL);
            engine->RegisterEnumValue("PA", "BRAKE", PA_BRAKE);
            engine->RegisterEnumValue("PA", "NITRO", PA_NITRO);
            engine->RegisterEnumValue("PA", "DRIFT", PA_DRIFT);
            engine->RegisterEnumValue("PA", "RESCUE", PA_RESCUE);
            engine->RegisterEnumValue("PA", "FIRE", PA_FIRE);
            engine->RegisterEnumValue("PA", "LOOK_BACK", PA_LOOK_BACK);
            engine->RegisterEnumValue("PA", "PAUSE_RACE", PA_PAUSE_RACE);
            engine->RegisterEnumValue("PA", "MENU_UP", PA_MENU_UP);
            engine->RegisterEnumValue("PA", "MENU_DOWN", PA_MENU_DOWN);
            engine->RegisterEnumValue("PA", "MENU_LEFT", PA_MENU_LEFT);
            engine->RegisterEnumValue("PA", "MENU_RIGHT", PA_MENU_RIGHT);
            engine->RegisterEnumValue("PA", "MENU_SELECT", PA_MENU_SELECT);
            engine->RegisterEnumValue("PA", "MENU_CANCEL", PA_MENU_CANCEL);
        }
    }
}
