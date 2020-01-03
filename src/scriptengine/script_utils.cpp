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
#include "network/crypto.hpp"
#include "network/network_config.hpp"
#include "scriptengine/aswrappedcall.hpp"
#include "scriptengine/script_engine.hpp"
#include "scriptengine/scriptarray.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

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

        /** Return a sha256 checksum of string in an array of integers of size 32 */
        CScriptArray* sha256(std::string* input)
        {
            asIScriptContext* ctx = asGetActiveContext();
            asIScriptEngine* engine = ctx->GetEngine();
            asITypeInfo* t = engine->GetTypeInfoByDecl("array<uint8>");
            auto result = Crypto::sha256(*input);
            CScriptArray* script_array = CScriptArray::Create(t, 32);
            for (unsigned int i = 0; i < 32; i++)
                script_array->SetValue(i, result.data() + i);
            return script_array;
        }

        /* Convert an integer to its hexadecimal value */
        std::string toHex(uint64_t num)
        {
            std::ostringstream output;
            output << std::hex << num;
            return output.str();
        }

        /* Return true if current game is networking (ie playing online),
         * where some limitation of scripting exists */
        bool isNetworking()
        {
            return NetworkConfig::get()->isNetworking();
        }

        /* Return a (STK) version string to its integer value */
        int versionToInt(const std::string* version)
        {
            return StringUtils::versionToInt(*version);
        }

        /* Return the current STK version in string */
        std::string getSTKVersion()
        {
            return STK_VERSION;
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
            engine->SetDefaultNamespace("Utils");
            
            bool mp = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
            asDWORD call_conv = mp ? asCALL_GENERIC : asCALL_CDECL;
            int r; // of type asERetCodes

            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_insertValues1) : asFUNCTION(proxy_insertValues1), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_insertValues2) : asFUNCTION(proxy_insertValues2), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_insertValues3) : asFUNCTION(proxy_insertValues3), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in, const string &in)", 
                                               mp ? WRAP_FN(proxy_insertValues4) : asFUNCTION(proxy_insertValues4), 
                                               call_conv); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("void runScript(string &in)", 
                                               mp ? WRAP_FN(runScript) : asFUNCTION(runScript), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("int randomInt(int, int)", 
                                               mp ? WRAP_FN(randomInt) : asFUNCTION(randomInt), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("int versionToInt(const string &in)",
                                               mp ? WRAP_FN(versionToInt) : asFUNCTION(versionToInt),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("string getSTKVersion()",
                                               mp ? WRAP_FN(getSTKVersion) : asFUNCTION(getSTKVersion),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("float randomFloat(int, int)", 
                                               mp ? WRAP_FN(randomFloat) : asFUNCTION(randomFloat),
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void setTimeout(const string &in, float)", 
                                               mp ? WRAP_FN(setTimeout) : asFUNCTION(setTimeout), 
                                               call_conv); assert(r >= 0);
            
            r = engine->RegisterFuncdef("void TimeoutCallback()"); assert(r >= 0);
            
            r = engine->RegisterGlobalFunction("void setTimeoutDelegate(TimeoutCallback@, float)", 
                                               mp ? WRAP_FN(setTimeoutDelegate) : asFUNCTION(setTimeoutDelegate), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("void logInfo(const string &in)", 
                                               mp ? WRAP_FN(logInfo) : asFUNCTION(logInfo), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void logWarning(const string &in)", 
                                               mp ? WRAP_FN(logWarning) : asFUNCTION(logWarning), 
                                               call_conv); assert(r >= 0);
                                               
            r = engine->RegisterGlobalFunction("void logError(const string &in)", 
                                               mp ? WRAP_FN(logError) : asFUNCTION(logError), 
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("bool isNetworking()",
                                               mp ? WRAP_FN(isNetworking) : asFUNCTION(isNetworking),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("array<uint8>@ sha256(const string &in)",
                                               mp ? WRAP_FN(sha256) : asFUNCTION(sha256),
                                               call_conv); assert(r >= 0);

            r = engine->RegisterGlobalFunction("string toHex(uint64 num)",
                                               mp ? WRAP_FN(toHex) : asFUNCTION(toHex),
                                               call_conv); assert(r >= 0);
        }
    }

/** \cond DOXYGEN_IGNORE */
}
/** \endcond */
