
#include <iostream>  // cout
#include <assert.h>  // assert()
#include <string.h>  // strstr()
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#ifdef _LINUX_
	#include <sys/time.h>
	#include <stdio.h>
	#include <termios.h>
	#include <unistd.h>
#else
	//#include <conio.h>   // kbhit(), getch()
	//#include <windows.h> // timeGetTime()
#endif

#include "angelscript.h"
#include "script_engine.hpp"
#include "scriptstdstring.h"

#define UINT unsigned int 
typedef unsigned int DWORD;

using namespace irr;

// Linux does have a getch() function in the curses library, but it doesn't
// work like it does on DOS. So this does the same thing, with out the need
// of the curses library.
/*int getch() 
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );

	ch = getchar();

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}
*/
//#endif

// Function prototypes
int  RunApplication(std::string);
void ConfigureEngine(asIScriptEngine *engine);
int  CompileScript(asIScriptEngine *engine,std::string scriptName);
void PrintString(std::string &str);
void PrintString_Generic(asIScriptGeneric *gen);
//void timeGetTime_Generic(asIScriptGeneric *gen);
//void LineCallback(asIScriptContext *ctx, DWORD *timeOut);

ScriptEngineOne::ScriptEngineOne(){

}

// Displays the message specified in displayMessage( string message ) within the script
void dispmsg(asIScriptGeneric *gen){
	std::string *input = (std::string*)gen->GetArgAddress(0);
	irr::core::stringw msgtodisp;
	irr::core::stringw out = irr::core::stringw((*input).c_str()); //irr::core::stringw supported by message dialogs
	new TutorialMessageDialog((out),true); 
}

std::string ScriptEngineOne::doit(std::string scriptName)
{
	//displaymsg();
	fprintf(stderr, "inside engine");
	RunApplication(scriptName);

	// Wait until the user presses a key
	//std::cout << std::endl << "Press any key to quit." << std::endl;
	//while(!getch());

	return "wot";
}
void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING ) 
		type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION ) 
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}


int RunApplication(std::string scriptName)
{
	int r;
	std::cout<<scriptName;
	// Create the script engine
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if( engine == 0 )
	{
		std::cout << "Failed to create script engine." << std::endl;
		return -1;
	}

	// The script compiler will write any compiler messages to the callback.
	//engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

	// Configure the script engine with all the functions, 
	// and variables that the script should be able to use.
	ConfigureEngine(engine);
	
	// Compile the script code
	r = CompileScript(engine,scriptName);
	if( r < 0 )
	{
		engine->Release();
		return -1;
	}

	// Create a context that will execute the script.
	asIScriptContext *ctx = engine->CreateContext();
	if( ctx == 0 ) 
	{
		std::cout << "Failed to create the context." << std::endl;
		engine->Release();
		return -1;
	}

	// We don't want to allow the script to hang the application, e.g. with an
	// infinite loop, so we'll use the line callback function to set a timeout
	// that will abort the script after a certain time. Before executing the 
	// script the timeOut variable will be set to the time when the script must 
	// stop executing. 
	//DWORD timeOut;
	//r = ctx->SetLineCallback(asFUNCTION(LineCallback), &timeOut, asCALL_CDECL);
	if( r < 0 )
	{
		std::cout << "Failed to set the line callback function." << std::endl;
		ctx->Release();
		engine->Release();
		return -1;
	}

	// Find the function for the function we want to execute.
	//This is how you call a normal function with arguments
	//asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void onTrigger(float, float)");
	asIScriptFunction *func = engine->GetModule(0)->GetFunctionByDecl("void onTrigger()");
	if( func == 0 )
	{
		std::cout << "The function 'float calc(float, float)' was not found." << std::endl;
		ctx->Release();
		engine->Release();
		return -1;
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
		engine->Release();
		return -1;
	}

	// Now we need to pass the parameters to the script function. 
	//ctx->SetArgFloat(0, 3.14159265359f);
	//ctx->SetArgFloat(1, 2.71828182846f);


	// Execute the function
	std::cout << "Executing the script." << std::endl;
	std::cout << "---" << std::endl;
	r = ctx->Execute();
	std::cout << "---" << std::endl;
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
		// Retrieve the return value from the context
		//float returnValue = ctx->GetReturnFloat();
		//std::cout << "The script function returned: " << returnValue << std::endl;
	}

	// We must release the contexts when no longer using them
	ctx->Release();

	// Release the engine
	engine->Release();

	return 0;
}
void ConfigureEngine(asIScriptEngine *engine)
{
	int r;

	// Register the script string type
	// Look at the implementation for this function for more information  
	// on how to register a custom string type, and other object types.
	RegisterStdString(engine);

	if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		// Register the functions that the scripts will be allowed to use.
		// Note how the return code is validated with an assert(). This helps
		// us discover where a problem occurs, and doesn't pollute the code
		// with a lot of if's. If an error occurs in release mode it will
		// be caught when a script is being built, so it is not necessary
		// to do the verification here as well.
		r = engine->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(PrintString), asCALL_CDECL); assert( r >= 0 );
		
	}
	else
	{
		// Notice how the registration is almost identical to the above. 
		r = engine->RegisterGlobalFunction("void Print(string &in)", asFUNCTION(PrintString_Generic), asCALL_GENERIC); assert( r >= 0 );
		
	}
	r = engine->RegisterGlobalFunction("void displayMessage(string &in)", asFUNCTION(dispmsg), asCALL_GENERIC); assert(r>=0);

	// It is possible to register the functions, properties, and types in 
	// configuration groups as well. When compiling the scripts it then
	// be defined which configuration groups should be available for that
	// script. If necessary a configuration group can also be removed from
	// the engine, so that the engine configuration could be changed 
	// without having to recompile all the scripts.
}

int CompileScript(asIScriptEngine *engine, std::string scriptName)
{
	int r;

	// We will load the script from a file on the disk.
	std::string load_dir = "D:\\Github\\stk\\stk-code\\src\\scriptengine\\";
	//std::string load_dir = "D:\\Github\\stk\\stk-code\\src\\scriptengine\\";
	load_dir += scriptName + ".as";
	FILE *f = fopen(load_dir.c_str(), "rb");
	//FILE *f = fopen("D:\\Uni Torrents\\angelscript_2.28.1\\sdk\\samples\\tutorial\\bin\\script.as", "rb");
	//FILE *f = fopen("//media//New Volume//Uni Torrents//angelscript_2.28.1//sdk//samples//tutorial//bin//script.as", "rb");
	if( f == 0 )
	{
		std::cout << "Failed to open the script file 'script.as'." << std::endl;
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
void PrintString(std::string &str)
{
	std::cout << str;
}

// Function implementation with generic script interface
void PrintString_Generic(asIScriptGeneric *gen)
{
	std::string *str = (std::string*)gen->GetArgAddress(0);
	std::cout << *str;
}