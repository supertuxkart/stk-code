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

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include <string>

//---------------------------
// Compilation settings
//

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
