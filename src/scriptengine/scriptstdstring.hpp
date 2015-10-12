//
// Script std::string
//
// This function registers the std::string type with AngelScript to be used as the default string type.
//
// The string type is registered as a value type, thus may have performance issues if a lot of 
// string operations are performed in the script. However, for relatively few operations, this should
// not cause any problem for most applications.
//

#ifndef SCRIPTSTDSTRING_H
#define SCRIPTSTDSTRING_H

// String must be included before angelscript.h to avoid some errors during
// compilation with GetObject function
#include <string>

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

//---------------------------
// Compilation settings
//

// The use of the string pool can improve performance quite drastically
// for scripts that work with a lot of literal string constants. 
//
//  1 = on
//  0 = off

#ifndef AS_USE_STRINGPOOL
#define AS_USE_STRINGPOOL 1
#endif

// Sometimes it may be desired to use the same method names as used by C++ STL.
// This may for example reduce time when converting code from script to C++ or
// back.
//
//  0 = off
//  1 = on

#ifndef AS_USE_STLNAMES
#define AS_USE_STLNAMES 0
#endif

BEGIN_AS_NAMESPACE

void RegisterStdString(asIScriptEngine *engine);
void RegisterStdStringUtils(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif
