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

#include "script_utils.hpp"

#include "animations/three_d_animation.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "scriptengine/script_engine.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"

#include <angelscript.h>

#include <assert.h>
#include <iostream> //debug

/** \cond DOXYGEN_IGNORE */
namespace Scripting
{
    /** \endcond */

    namespace Utils
    {
        /** \addtogroup Scripting
        * @{
        */
        /** \addtogroup Utils
        * @{
        */
        // TODO: build these variations with variadic templates?

        /** Replaces placeholders with values. Note, in angelscript, omit the trailing number.
          * e.g. Utils::insertValues("Hello %s !", "world");
          */
        std::string insertValues(std::string* format_string, std::string* arg1)
        {
            irr::core::stringw out = 
                StringUtils::insertValues(StringUtils::utf8ToWide(*format_string),
                                          StringUtils::utf8ToWide(*arg1));

            return StringUtils::wideToUtf8(out);
        }

        /** Replaces placeholders with values. Note, in angelscript, omit the trailing number.
        * e.g. Utils::insertValues("Hello %s %s !", "John", "Doe");
        */
        std::string insertValues(std::string* format_string, std::string* arg1, std::string* arg2)
        {
            irr::core::stringw out = 
                StringUtils::insertValues(StringUtils::utf8ToWide(*format_string),
                                          StringUtils::utf8ToWide(*arg1),
                                          StringUtils::utf8ToWide(*arg2));

            return StringUtils::wideToUtf8(out);
        }

        /** Replaces placeholders with values. Note, in angelscript, omit the trailing number.
          * e.g. Utils::insertValues("Hello %s %s %s !", "Mr", "John", "Doe");
          */
        std::string insertValues(std::string* format_string, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            irr::core::stringw out =
                StringUtils::insertValues(StringUtils::utf8ToWide(*format_string),
                                          StringUtils::utf8ToWide(*arg1),
                                          StringUtils::utf8ToWide(*arg2),
                                          StringUtils::utf8ToWide(*arg3));

            return StringUtils::wideToUtf8(out);
        }

        /** Replaces placeholders with values. Note, in angelscript, omit the trailing number.
          * e.g. Utils::insertValues("%s %s %s %s !", "Hello", "Mr", "John", "Doe");
          */
        std::string insertValues(std::string* format_string, std::string* arg1,
                                 std::string* arg2, std::string* arg3,
                                 std::string* arg4)
        {
            irr::core::stringw out =
                StringUtils::insertValues(StringUtils::utf8ToWide(*format_string),
                                          StringUtils::utf8ToWide(*arg1),
                                          StringUtils::utf8ToWide(*arg2),
                                          StringUtils::utf8ToWide(*arg3),
                                          StringUtils::utf8ToWide(*arg4));

            return StringUtils::wideToUtf8(out);
        }

        /** Runs the script function specified by the given string */
        void runScript(const std::string* str)
        {
            ScriptEngine::getInstance()->runFunction(true, *str);
        }

        /** Generate a random integer value */
        int randomInt(int min, int maxExclusive)
        {
            return min + (rand() % (maxExclusive - min));
        }

        /** Generate a random floating-point value */
        float randomFloat(int min, int maxExclusive)
        {
            int val = min * 100 + (rand() % ((maxExclusive - min) * 100));
            return val / 100.0f;
        }

        /** Call a function after the specified delay */
        void setTimeout(const std::string* callback_name, float delay)
        {
            ScriptEngine::getInstance()->addPendingTimeout(delay, *callback_name);
        }

        /** Call a method from the given object after the specified delay */
        void setTimeoutDelegate(asIScriptFunction* obj, float delay)
        {
            ScriptEngine::getInstance()->addPendingTimeout(delay, obj);
        }

        /** Log to the console */
        void logInfo(std::string* log)
        {
            Log::info("Script", "%s", log->c_str());
        }

        /** Log warning to the console */
        void logWarning(std::string* log)
        {
            Log::warn("Script", "%s", log->c_str());
        }

        /** Log error to the console */
        void logError(std::string* log)
        {
            Log::error("Script", "%s", log->c_str());
        }
        /** @}*/
        /** @}*/

        // UNDOCUMENTED PROXIES : Use proxies to have different signatures, then redirect to the
        // documented function whose name is exposed in angelscript (these proxies exist so that
        // angelscript can properly resolve overloads, but doxygen can still generate the right docs
        /** \cond DOXYGEN_IGNORE */
        std::string proxy_insertValues1(std::string* formatString, std::string* arg1)
        {
            return insertValues(formatString, arg1);
        }
        std::string proxy_insertValues2(std::string* formatString, std::string* arg1, std::string* arg2)
        {
            return insertValues(formatString, arg1, arg2);
        }
        std::string proxy_insertValues3(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3)
        {
            return insertValues(formatString, arg1, arg2, arg3);
        }
        std::string proxy_insertValues4(std::string* formatString, std::string* arg1, std::string* arg2,
            std::string* arg3, std::string* arg4)
        {
            return insertValues(formatString, arg1, arg2, arg3, arg4);
        }
        /** \endcond */

        void registerScriptFunctions(asIScriptEngine *engine)
        {
            int r; // of type asERetCodes
            engine->SetDefaultNamespace("Utils");

            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in)", asFUNCTION(proxy_insertValues1), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in)", asFUNCTION(proxy_insertValues2), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in)", asFUNCTION(proxy_insertValues3), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in, const string &in)", asFUNCTION(proxy_insertValues4), asCALL_CDECL); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("void runScript(string &in)", asFUNCTION(runScript), asCALL_CDECL); assert(r >= 0);

            r = engine->RegisterGlobalFunction("int randomInt(int, int)", asFUNCTION(randomInt), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("float randomFloat(int, int)", asFUNCTION(randomFloat), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void setTimeout(const string &in, float)", asFUNCTION(setTimeout), asCALL_CDECL); assert(r >= 0);
            
            r = engine->RegisterFuncdef("void TimeoutCallback()"); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void setTimeoutDelegate(TimeoutCallback@, float)", asFUNCTION(setTimeoutDelegate), asCALL_CDECL); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void logInfo(const string &in)", asFUNCTION(logInfo), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void logWarning(const string &in)", asFUNCTION(logWarning), asCALL_CDECL); assert(r >= 0);
            r = engine->RegisterGlobalFunction("void logError(const string &in)", asFUNCTION(logError), asCALL_CDECL); assert(r >= 0);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
