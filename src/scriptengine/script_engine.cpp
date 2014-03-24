
#include <iostream>  // cout
#include <assert.h>  // assert()
#include <string.h>  // strstr()
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include <angelscript.h>
#include "script_engine.hpp"
#include "scriptstdstring.h"
#include "modes/world.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track.hpp"

using namespace irr;


asIScriptEngine *m_engine;
// Function prototypes for binding. TODO:put these in their right place
void configureEngine(asIScriptEngine *engine);
int  compileScript(asIScriptEngine *engine,std::string scriptName);
void printString(std::string &str);
void printString_Generic(asIScriptGeneric *gen);

ScriptEngine::ScriptEngine(){
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
ScriptEngine::~ScriptEngine(){
	// Release the engine
	m_engine->Release();
}
// Displays the message specified in displayMessage( string message ) within the script
void displayMessage(asIScriptGeneric *gen){
	std::string *input = (std::string*)gen->GetArgAddress(0);
	irr::core::stringw msgtodisp;
	irr::core::stringw out = irr::core::stringw((*input).c_str()); //irr::core::stringw supported by message dialogs
	new TutorialMessageDialog((out),true); 
}
void disableAnimation(asIScriptGeneric *gen){
		std::string *str = (std::string*)gen->GetArgAddress(0);
		World::getWorld()->getTrack()->getTrackObjectManager()->disable(*str);
}


void ScriptEngine::runScript(std::string scriptName)
{

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
	asIScriptFunction *func = m_engine->GetModule(0)->GetFunctionByDecl("void onTrigger()");
	if( func == 0 )
	{
		std::cout << "The function 'void onTrigger()' was not found." << std::endl;
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

	// Now we can pass parameters to the script function. 
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
void configureEngine(asIScriptEngine *engine)
{
	int r;

	// Register the script string type
	RegisterStdString(engine);

	if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		// Register the functions that the scripts will be allowed to use.
		// Note how the return code is validated with an assert(). This helps
		// us discover where a problem occurs, and doesn't pollute the code
		// with a lot of if's. If an error occurs in release mode it will
		// be caught when a script is being built, so it is not necessary
		// to do the verification here as well.
		r = engine->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(printString), asCALL_CDECL); assert( r >= 0 );
		
	}
	else
	{
		// Notice how the registration is almost identical to the above. 
		r = engine->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(printString_Generic), asCALL_GENERIC); assert( r >= 0 );
		
	}
	r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(displayMessage), asCALL_GENERIC); assert(r>=0);
	r = engine->RegisterGlobalFunction("void disableAnimation(string &in)", asFUNCTION(disableAnimation), asCALL_GENERIC); assert(r>=0);
	// It is possible to register the functions, properties, and types in 
	// configuration groups as well. When compiling the scripts it then
	// be defined which configuration groups should be available for that
	// script. If necessary a configuration group can also be removed from
	// the engine, so that the engine configuration could be changed 
	// without having to recompile all the scripts.
}

int compileScript(asIScriptEngine *engine, std::string scriptName)
{
	int r;

	// For now we will load the script directtly from a file on the disk.
	//TODO use filemanager to do this.
	std::string load_dir = "D:\\Github\\stk\\stk-code\\src\\scriptengine\\";
	//std::string load_dir = "//media//New Volume//Github//stk//stk-code//src//scriptengine//";
	load_dir += scriptName + ".as";
	FILE *f = fopen(load_dir.c_str(), "rb");
	if( f == 0 )
	{
		std::cout << "Failed to open the script file " + scriptName + ".as" << std::endl;
		return -1;
	}

	// Determine the size of the file	
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);

	// On Win32 it is possible to do the following instead
	// int len = _filelength(_fileno(f));

	// Read the entire file
	std::string script;
	script.resize(len);
	int c =	fread(&script[0], len, 1, f);
	fclose(f);
	if( c == 0 ) 
	{
		std::cout << "Failed to load script file." << std::endl;
		return -1;
	}
	//std::cout<<script<<std::endl;
	//script = "float calc(float a, float b){ return 23;}//asfafagadbsgsgsbfdxhbdhdhdfhdfbdfbdbfg";
	//len = script.size();
	// Add the script sections that will be compiled into executable code.
	// If we want to combine more than one file into the same script, then 
	// we can call AddScriptSection() several times for the same module and
	// the script engine will treat them all as if they were one. The script
	// section name, will allow us to localize any errors in the script code.
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	r = mod->AddScriptSection("script", &script[0], len);
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
	// into separate script modules. Each module use their own namespace and 
	// scope, so function names, and global variables will not conflict with
	// each other.

	return 0;
}


// Function implementation with native calling convention
void printString(std::string &str)
{
	std::cout << str;
}

// Function implementation with generic script interface
void printString_Generic(asIScriptGeneric *gen)
{
	std::string *str = (std::string*)gen->GetArgAddress(0);
	std::cout << *str;
}
