//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014  SuperTuxKart Team
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

#include <assert.h>  // assert()
#include <angelscript.h>
#include "io/file_manager.hpp"
#include <iostream>  // cout
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "script_engine.hpp"
#include "scriptstdstring.hpp"
#include "scriptvec3.hpp"
#include <string.h>  // strstr()
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track.hpp"



using namespace Scripting;

namespace Scripting
{
//Constructor, creates a new Scripting Engine using AngelScript
ScriptEngine::ScriptEngine()
{
    // Create the script engine
    m_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if( m_engine == 0 )
    {
        std::cout << "Failed to create script engine." << std::endl;
    }
    // Configure the script engine with all the functions, 
    // and variables that the script should be able to use.
    configureEngine(m_engine);
}
ScriptEngine::~ScriptEngine()
{
    // Release the engine
    m_engine->Release();
}

/** Get Script By it's file name
*  \param string scriptname = name of script to get
*  \return      The corresponding script
*/
std::string getScript(std::string scriptName)
{
    std::string script_dir = file_manager->getAsset(FileManager::SCRIPT, "");
    script_dir += World::getWorld()->getTrack()->getIdent() + "/";
    if (scriptName != "update" && scriptName != "collisions" && scriptName!="start") scriptName = "triggers";
    script_dir += scriptName + ".as";
    FILE *f = fopen(script_dir.c_str(), "rb");
    if( f == 0 )
    {
        std::cout << "Failed to open the script file " + scriptName + ".as" << std::endl;
    }

    // Determine the size of the file   
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Read the entire file
    std::string script;
    script.resize(len);
    int c = fread(&script[0], len, 1, f);
    fclose(f);
    if( c == 0 ) 
    {
        std::cout << "Failed to load script file." << std::endl;
    }
    return script;
}
//-----------------------------------------------------------------------------
/** runs the specified script
*  \param ident scriptName = name of script to run
*/
void ScriptEngine::runScript(std::string scriptName)
{
    return; // Scripting disabled for now

    // TODO: this code seems to fetch the script from disk and compile it on every execution?
    // A cache should be created.

    int r;
    // Compile the script code
    r = compileScript(m_engine,scriptName);
    if( r < 0 )
    {
        m_engine->Release();
        return;
    }

    // Create a context that will execute the script.
    asIScriptContext *ctx = m_engine->CreateContext();
    if( ctx == 0 ) 
    {
        std::cout << "Failed to create the context." << std::endl;
        m_engine->Release();
        return;
    }

    if( r < 0 )
    {
        std::cout << "Failed to set the line callback function." << std::endl;
        ctx->Release();
        m_engine->Release();
        return;
    }

    // Find the function for the function we want to execute.
    //This is how you call a normal function with arguments
    //asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void func(arg1Type, arg2Type)");
    asIScriptFunction *func;
    if (scriptName =="collisions")
    {
        func = Scripting::Physics::registerScriptCallbacks(m_engine);
    }
    else if (scriptName == "update")
    {
        func = Scripting::Track::registerUpdateScriptCallbacks(m_engine);
    }
    else if (scriptName == "start")
    {
        func = Scripting::Track::registerStartScriptCallbacks(m_engine);
    }
    else
    {
        //trigger type can have different names
        func = Scripting::Track::registerScriptCallbacks(m_engine , scriptName);
    }
    if( func == 0 )
    {
        std::cout << "The required function was not found." << std::endl;
        ctx->Release();
        m_engine->Release();
        return;
    }

    // Prepare the script context with the function we wish to execute. Prepare()
    // must be called on the context before each new script function that will be
    // executed. Note, that if you intend to execute the same function several 
    // times, it might be a good idea to store the function returned by 
    // GetFunctionByDecl(), so that this relatively slow call can be skipped.
    r = ctx->Prepare(func);
    if( r < 0 ) 
    {
        std::cout << "Failed to prepare the context." << std::endl;
        ctx->Release();
        m_engine->Release();
        return;
    }

    // Here, we can pass parameters to the script functions. 
    //ctx->setArgType(index, value);
    //for example : ctx->SetArgFloat(0, 3.14159265359f);


    // Execute the function
    r = ctx->Execute();
    if( r != asEXECUTION_FINISHED )
    {
        // The execution didn't finish as we had planned. Determine why.
        if( r == asEXECUTION_ABORTED )
            std::cout << "The script was aborted before it could finish. Probably it timed out." << std::endl;
        else if( r == asEXECUTION_EXCEPTION )
        {
            std::cout << "The script ended with an exception." << std::endl;

            // Write some information about the script exception
            asIScriptFunction *func = ctx->GetExceptionFunction();
            std::cout << "func: " << func->GetDeclaration() << std::endl;
            std::cout << "modl: " << func->GetModuleName() << std::endl;
            std::cout << "sect: " << func->GetScriptSectionName() << std::endl;
            std::cout << "line: " << ctx->GetExceptionLineNumber() << std::endl;
            std::cout << "desc: " << ctx->GetExceptionString() << std::endl;
        }
        else
            std::cout << "The script ended for some unforeseen reason (" << r << ")." << std::endl;
    }
    else
    {
        // Retrieve the return value from the context here (for scripts that return values)
        // <type> returnValue = ctx->getReturnType(); for example
        //float returnValue = ctx->GetReturnFloat();
    }

    // We must release the contexts when no longer using them
    ctx->Release();

}


//-----------------------------------------------------------------------------
/** Configures the script engine by binding functions, enums
*  \param asIScriptEngine engine = engine to configure
*/
void ScriptEngine::configureEngine(asIScriptEngine *engine)
{
    int r;

    // Register the script string type
    RegisterStdString(engine); //register std::string
    RegisterVec3(engine);      //register Vec3

    Scripting::Track::registerScriptFunctions(m_engine);

    Scripting::Track::registerScriptEnums(m_engine);

    Scripting::Kart::registerScriptFunctions(m_engine);

    Scripting::Physics::registerScriptFunctions(m_engine);

    
    // It is possible to register the functions, properties, and types in 
    // configuration groups as well. When compiling the scripts it can then
    // be defined which configuration groups should be available for that
    // script. If necessary a configuration group can also be removed from
    // the engine, so that the engine configuration could be changed 
    // without having to recompile all the scripts.
}

//-----------------------------------------------------------------------------



int ScriptEngine::compileScript(asIScriptEngine *engine, std::string scriptName)
{
    int r;

    std::string script = getScript(scriptName);
    // Add the script sections that will be compiled into executable code.
    // If we want to combine more than one file into the same script, then 
    // we can call AddScriptSection() several times for the same module and
    // the script engine will treat them all as if they were one. The script
    // section name, will allow us to localize any errors in the script code.
    asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
    r = mod->AddScriptSection("script", &script[0], script.size());
    if( r < 0 ) 
    {
        std::cout << "AddScriptSection() failed" << std::endl;
        return -1;
    }
    
    // Compile the script. If there are any compiler messages they will
    // be written to the message stream that we set right after creating the 
    // script engine. If there are no errors, and no warnings, nothing will
    // be written to the stream.
    r = mod->Build();
    if( r < 0 )
    {
        std::cout << "Build() failed" << std::endl;
        return -1;
    }

    // The engine doesn't keep a copy of the script sections after Build() has
    // returned. So if the script needs to be recompiled, then all the script
    // sections must be added again.

    // If we want to have several scripts executing at different times but 
    // that have no direct relation with each other, then we can compile them
    // into separate script modules. Each module uses their own namespace and 
    // scope, so function names, and global variables will not conflict with
    // each other.

    return 0;
}

}
