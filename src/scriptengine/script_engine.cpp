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

extern "C"
{
    #include <mcpp_lib.h>
}
#include <assert.h>
#include <angelscript.h>
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "scriptengine/aswrappedcall.hpp"
#include "scriptengine/script_audio.hpp"
#include "scriptengine/script_challenges.hpp"
#include "scriptengine/script_kart.hpp"
#include "scriptengine/script_engine.hpp"
#include "scriptengine/script_gui.hpp"
#include "scriptengine/script_physics.hpp"
#include "scriptengine/script_track.hpp"
#include "scriptengine/script_utils.hpp"
#include "scriptengine/scriptstdstring.hpp"
#include "scriptengine/scriptvec3.hpp"
#include "scriptengine/scriptarray.hpp"
#include <string.h>
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/profiler.hpp"


using namespace Scripting;

namespace Scripting
{
    const char* MODULE_ID_MAIN_SCRIPT_FILE = "main";

    void AngelScript_ErrorCallback (const asSMessageInfo *msg, void *param)
    {
        const char *type = "ERR ";
        if (msg->type == asMSGTYPE_WARNING)
            type = "WARN";
        else if (msg->type == asMSGTYPE_INFORMATION)
            type = "INFO";

        Log::warn("Scripting", "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
    }


    //Constructor, creates a new Scripting Engine using AngelScript
    ScriptEngine::ScriptEngine()
    {
        // Create the script engine
        m_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
        if (m_engine == NULL)
        {
            Log::fatal("Scripting", "Failed to create script engine.");
        }

        // The script compiler will write any compiler messages to the callback.
        m_engine->SetMessageCallback(asFUNCTION(AngelScript_ErrorCallback), 0, asCALL_CDECL);

        // Configure the script engine with all the functions, 
        // and variables that the script should be able to use.
        configureEngine(m_engine);
    }

    ScriptEngine::~ScriptEngine()
    {
        // Release the engine
        m_pending_timeouts.clearAndDeleteAll();
        m_engine->DiscardModule(MODULE_ID_MAIN_SCRIPT_FILE);
        m_engine->Release();
    }



    /** Get Script By it's file name
    *  \param string scriptname = name of script to get
    *  \return      The corresponding script
    */
    std::string getScript(std::string script_path)
    {
        std::string script_file = FileUtils::getPortableReadingPath(script_path);
        if (!file_manager->fileExists(script_file))
        {
#ifndef SERVER_ONLY
            Log::debug("Scripting", "File does not exist : %s", script_path.c_str());
#endif
            return "";
        }

        // libmcpp ignores the first argument (like real main which is the exe)
        std::string cmd1 = "mcpp";
        std::string int_version =
            StringUtils::toString(StringUtils::versionToInt(STK_VERSION));
        // Preprocessing (atm add stk version)
        std::string cmd2  = "-DSTK_VERSION=";
        cmd2 += int_version;
        // -P Don't output #line lines.
        std::string cmd3 = "-P";
        // -j Don't output the source line in diagnostics.
        std::string cmd4 = "-j";
        // -e <encoding>   Change the default multi-byte character encoding to one of:
        //    euc_jp, gb2312, ksc5601, big5, sjis, iso2022_jp, utf8.
        std::string cmd5 = "-e";
        std::string cmd6 = "utf8";
        std::string cmd7 = script_file;
        std::vector<char*> all_cmds;
        all_cmds.push_back(&cmd1[0]);
        all_cmds.push_back(&cmd2[0]);
        all_cmds.push_back(&cmd3[0]);
        all_cmds.push_back(&cmd4[0]);
        all_cmds.push_back(&cmd5[0]);
        all_cmds.push_back(&cmd6[0]);
        all_cmds.push_back(&cmd7[0]);
        mcpp_use_mem_buffers(1);
        mcpp_lib_main(all_cmds.size(), all_cmds.data());

        char* err = mcpp_get_mem_buffer((OUTDEST)1/*error buffer*/);
        bool has_error = false;
        if (err)
        {
            std::string total = err;
            auto errs = StringUtils::split(total, '\n');
            for (auto& e : errs)
            {
                if (e.find("warning: Converted [CR+LF] to [LF]") !=
                    std::string::npos)
                    continue;

                if (e.find("fatal:") != std::string::npos ||
                    e.find("error:") != std::string::npos)
                {
                    has_error = true;
                    Log::error("Scripting preprocessing", "%s", e.c_str());
                }
                else
                {
                    Log::warn("Scripting preprocessing", "%s", e.c_str());
                }
            }
        }

        std::string result;
        if (!has_error)
        {
            char* buf = mcpp_get_mem_buffer((OUTDEST)0/*output buffer*/);
            if (buf)
                result = buf;
        }

        // Calling this again causes the memory buffers to be freed.
        mcpp_use_mem_buffers(1);
        return result;
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::evalScript(std::string script_fragment)
    {
        script_fragment = "void evalScript_main() { \n" + script_fragment + "\n}";

        asIScriptModule* mod = m_engine->GetModule(MODULE_ID_MAIN_SCRIPT_FILE, asGM_ONLY_IF_EXISTS);

        asIScriptFunction* func;
        int r = mod->CompileFunction("eval", script_fragment.c_str(), 0, 0, &func);
        if (r < 0)
        {
            Log::error("Scripting", "evalScript: CompileFunction() failed");
            return;
        }

        asIScriptContext *ctx = m_engine->CreateContext();
        if (ctx == NULL)
        {
            Log::error("Scripting", "evalScript: Failed to create the context.");
            //m_engine->Release();
            return;
        }

        r = ctx->Prepare(func);
        if (r < 0)
        {
            Log::error("Scripting", "evalScript: Failed to prepare the context.");
            ctx->Release();
            return;
        }

        // Execute the function
        r = ctx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't finish as we had planned. Determine why.
            if (r == asEXECUTION_ABORTED)
            {
                Log::error("Scripting", "The script was aborted before it could finish. Probably it timed out.");
            }
            else if (r == asEXECUTION_EXCEPTION)
            {
                Log::error("Scripting", "The script ended with an exception.");
            }
            else
            {
                Log::error("Scripting", "The script ended for some unforeseen reason (%i)", r);
            }
        }

        ctx->Release();
        func->Release();
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::runDelegate(asIScriptFunction* delegate)
    {
        asIScriptContext *ctx = m_engine->CreateContext();
        if (ctx == NULL)
        {
            Log::error("Scripting", "runMethod: Failed to create the context.");
            //m_engine->Release();
            return;
        }

        int r = ctx->Prepare(delegate);
        if (r < 0)
        {
            Log::error("Scripting", "runMethod: Failed to prepare the context.");
            ctx->Release();
            return;
        }

        // Execute the function
        r = ctx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't finish as we had planned. Determine why.
            if (r == asEXECUTION_ABORTED)
            {
                Log::error("Scripting", "The script was aborted before it could finish. Probably it timed out.");
            }
            else if (r == asEXECUTION_EXCEPTION)
            {
                Log::error("Scripting", "The script ended with an exception : (line %i) %s",
                    ctx->GetExceptionLineNumber(), 
                    ctx->GetExceptionString());
            }
            else
            {
                Log::error("Scripting", "The script ended for some unforeseen reason (%i)", r);
            }
        }

        ctx->Release();
    }

    //-----------------------------------------------------------------------------
    
    /*
    void ScriptEngine::runMethod(asIScriptObject* obj, std::string methodName)
    {
        asITypeInfo* type = obj->GetObjectType();
        asIScriptFunction* method = type->GetMethodByName(methodName.c_str());
        if (method == NULL)
            Log::error("Scripting", ("runMethod: object does not implement method " + methodName).c_str());


        asIScriptContext *ctx = m_engine->CreateContext();
        if (ctx == NULL)
        {
            Log::error("Scripting", "runMethod: Failed to create the context.");
            //m_engine->Release();
            return;
        }

        int r = ctx->Prepare(method);
        if (r < 0)
        {
            Log::error("Scripting", "runMethod: Failed to prepare the context.");
            ctx->Release();
            return;
        }

        // Execute the function
        r = ctx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't finish as we had planned. Determine why.
            if (r == asEXECUTION_ABORTED)
            {
                Log::error("Scripting", "The script was aborted before it could finish. Probably it timed out.");
            }
            else if (r == asEXECUTION_EXCEPTION)
            {
                Log::error("Scripting", "The script ended with an exception.");
            }
            else
            {
                Log::error("Scripting", "The script ended for some unforeseen reason (%i)", r);
            }
        }

        ctx->Release();
    }
    */
    //-----------------------------------------------------------------------------

    /** runs the specified script
    *  \param string scriptName = name of script to run
    */
    void ScriptEngine::runFunction(bool warn_if_not_found, std::string function_name)
    {
        std::function<void(asIScriptContext*)> callback;
        std::function<void(asIScriptContext*)> get_return_value;
        runFunction(warn_if_not_found, function_name, callback, get_return_value);
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::runFunction(bool warn_if_not_found, std::string function_name,
        std::function<void(asIScriptContext*)> callback)
    {
        std::function<void(asIScriptContext*)> get_return_value;
        runFunction(warn_if_not_found, function_name, callback, get_return_value);
    }

    //-----------------------------------------------------------------------------

    /** runs the specified script
    *  \param string scriptName = name of script to run
    */
    void ScriptEngine::runFunction(bool warn_if_not_found, std::string function_name,
        std::function<void(asIScriptContext*)> callback,
        std::function<void(asIScriptContext*)> get_return_value)
    {
        int r; //int for error checking

        asIScriptFunction *func;

        // TODO: allow splitting in multiple files
        std::string script_filename = "scripting.as";
        auto cached_function = m_functions_cache.find(function_name);
        if (cached_function == m_functions_cache.end())
        {
            // Find the function for the function we want to execute.
            //      This is how you call a normal function with arguments
            //      asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void func(arg1Type, arg2Type)");
            asIScriptModule* module = m_engine->GetModule(MODULE_ID_MAIN_SCRIPT_FILE);

            if (module == NULL)
            {
#ifndef SERVER_ONLY
                if (warn_if_not_found)
                    Log::warn("Scripting", "Scripting function was not found : %s (module not found)", function_name.c_str());
                else
                    Log::debug("Scripting", "Scripting function was not found : %s (module not found)", function_name.c_str());
#endif
                m_functions_cache[function_name] = NULL; // remember that this function is unavailable
                return;
            }

            func = module->GetFunctionByDecl(function_name.c_str());
            
            if (func == NULL)
            {
#ifndef SERVER_ONLY
                if (warn_if_not_found)
                    Log::warn("Scripting", "Scripting function was not found : %s", function_name.c_str());
                else
                    Log::debug("Scripting", "Scripting function was not found : %s", function_name.c_str());
#endif
                m_functions_cache[function_name] = NULL; // remember that this function is unavailable
                return;
            }

            m_functions_cache[function_name] = func;
            func->AddRef();
        }
        else
        {
            // Script present in cache
            func = cached_function->second;
        }

        if (func == NULL)
        {
            if (warn_if_not_found)
                Log::warn("Scripting", "Scripting function was not found : %s", function_name.c_str());
            return; // function unavailable
        }

        // Create a context that will execute the script.
        asIScriptContext *ctx = m_engine->CreateContext();
        if (ctx == NULL)
        {
            Log::error("Scripting", "Failed to create the context.");
            //m_engine->Release();
            return;
        }

        // Prepare the script context with the function we wish to execute. Prepare()
        // must be called on the context before each new script function that will be
        // executed. Note, that if because we intend to execute the same function 
        // several times, we will store the function returned by 
        // GetFunctionByDecl(), so that this relatively slow call can be skipped.
        r = ctx->Prepare(func);
        if (r < 0)
        {
            Log::error("Scripting", "Failed to prepare the context.");
            ctx->Release();
            //m_engine->Release();
            return;
        }

        // Here, we can pass parameters to the script functions. 
        //ctx->setArgType(index, value);
        //for example : ctx->SetArgFloat(0, 3.14159265359f);

        if (callback)
            callback(ctx);

        // Execute the function
        r = ctx->Execute();
        if (r != asEXECUTION_FINISHED)
        {
            // The execution didn't finish as we had planned. Determine why.
            if (r == asEXECUTION_ABORTED)
            {
                Log::error("Scripting", "The script was aborted before it could finish. Probably it timed out.");
            }
            else if (r == asEXECUTION_EXCEPTION)
            {
                Log::error("Scripting", "The script ended with an exception.");

                // Write some information about the script exception
                //asIScriptFunction *func = ctx->GetExceptionFunction();
                //std::cout << "func: " << func->GetDeclaration() << std::endl;
                //std::cout << "modl: " << func->GetModuleName() << std::endl;
                //std::cout << "sect: " << func->GetScriptSectionName() << std::endl;
                //std::cout << "line: " << ctx->GetExceptionLineNumber() << std::endl;
                //std::cout << "desc: " << ctx->GetExceptionString() << std::endl;
            }
            else
            {
                Log::error("Scripting", "The script ended for some unforeseen reason (%i)", r);
            }
        }
        else
        {
            // Retrieve the return value from the context here (for scripts that return values)
            // <type> returnValue = ctx->getReturnType(); for example
            //float returnValue = ctx->GetReturnFloat();

            if (get_return_value)
                get_return_value(ctx);
        }

        // We must release the contexts when no longer using them
        ctx->Release();
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::cleanupCache()
    {
        for (auto curr : m_functions_cache)
        {
            if (curr.second != NULL)
                curr.second->Release();
        }
        m_functions_cache.clear();
        m_engine->DiscardModule(MODULE_ID_MAIN_SCRIPT_FILE);
    }

    //-----------------------------------------------------------------------------
    /** Configures the script engine by binding functions, enums
    *  \param asIScriptEngine engine = engine to configure
    */
    void ScriptEngine::configureEngine(asIScriptEngine *engine)
    {
        // Register the script string type
        RegisterStdString(engine); //register std::string
        RegisterVec3(engine);      //register Vec3
        RegisterScriptArray(engine, true);

        Scripting::Track::registerScriptFunctions(m_engine);
        Scripting::Challenges::registerScriptFunctions(m_engine);
        Scripting::Kart::registerScriptFunctions(m_engine);
        Scripting::Kart::registerScriptEnums(m_engine);
        Scripting::Physics::registerScriptFunctions(m_engine);
        Scripting::Utils::registerScriptFunctions(m_engine);
        Scripting::GUI::registerScriptEnums(m_engine);
        Scripting::GUI::registerScriptFunctions(m_engine);
        Scripting::Audio::registerScriptFunctions(m_engine);

        // It is possible to register the functions, properties, and types in 
        // configuration groups as well. When compiling the scripts it can then
        // be defined which configuration groups should be available for that
        // script. If necessary a configuration group can also be removed from
        // the engine, so that the engine configuration could be changed 
        // without having to recompile all the scripts.
    }

    //-----------------------------------------------------------------------------

    bool ScriptEngine::loadScript(std::string script_path, bool clear_previous)
    {
        int r;

        std::string script = getScript(script_path);
        if (script.size() == 0)
        {
            // No such file
            return false;
        }

        // Add the script sections that will be compiled into executable code.
        // If we want to combine more than one file into the same script, then 
        // we can call AddScriptSection() several times for the same module and
        // the script engine will treat them all as if they were one. The script
        // section name, will allow us to localize any errors in the script code.
        asIScriptModule *mod = m_engine->GetModule(MODULE_ID_MAIN_SCRIPT_FILE,
            clear_previous ? asGM_ALWAYS_CREATE : asGM_CREATE_IF_NOT_EXISTS);
        r = mod->AddScriptSection("script", &script[0], script.size());
        if (r < 0)
        {
            Log::error("Scripting", "AddScriptSection() failed");
            return false;
        }
    
        return true;
    }

    //-----------------------------------------------------------------------------

    bool ScriptEngine::compileLoadedScripts()
    {
        int r;
        asIScriptModule *mod = m_engine->GetModule(MODULE_ID_MAIN_SCRIPT_FILE, asGM_CREATE_IF_NOT_EXISTS);

        // Compile the script. If there are any compiler messages they will
        // be written to the message stream that we set right after creating the 
        // script engine. If there are no errors, and no warnings, nothing will
        // be written to the stream.
        r = mod->Build();
        if (r < 0)
        {
            Log::error("Scripting", "Build() failed");
            return false;
        }

        // The engine doesn't keep a copy of the script sections after Build() has
        // returned. So if the script needs to be recompiled, then all the script
        // sections must be added again.

        // If we want to have several scripts executing at different times but 
        // that have no direct relation with each other, then we can compile them
        // into separate script modules. Each module uses their own namespace and 
        // scope, so function names, and global variables will not conflict with
        // each other.

        return true;
    }

    //-----------------------------------------------------------------------------

    PendingTimeout::PendingTimeout(double time, asIScriptFunction* callback_delegate) 
    {
        m_time = time;
        m_callback_delegate = callback_delegate;
        
        #if ANGELSCRIPT_VERSION < 23300
        if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
        {
            callback_delegate->AddRef();
        }
        #endif
    }

    //-----------------------------------------------------------------------------

    PendingTimeout::~PendingTimeout()
    {
        if (m_callback_delegate != NULL)
        {
            m_callback_delegate->Release();
        }
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::addPendingTimeout(double time, const std::string& callback_name)
    {
        m_pending_timeouts.push_back(new PendingTimeout(time, callback_name));
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::addPendingTimeout(double time, asIScriptFunction* delegate)
    {
        m_pending_timeouts.push_back(new PendingTimeout(time, delegate));
    }

    //-----------------------------------------------------------------------------

    void ScriptEngine::update(float dt)
    {
        for (int i = m_pending_timeouts.size() - 1; i >= 0; i--)
        {
            PendingTimeout& curr = m_pending_timeouts[i];
            curr.m_time -= dt;
            if (curr.m_time <= 0.0)
            {
                if (curr.m_callback_delegate != NULL)
                {
                    runDelegate(curr.m_callback_delegate);
                }
                else
                {
                    runFunction(true, "void " + curr.m_callback_name + "()");
                }

                m_pending_timeouts.erase(i);
            }
        }
    }
}
