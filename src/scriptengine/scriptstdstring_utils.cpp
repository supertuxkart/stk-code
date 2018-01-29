/*
   AngelCode Scripting Library
   Copyright (c) 2003-2017 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/

#include <assert.h>
#include "scriptstdstring.hpp"
#include "scriptarray.hpp"
#include <stdio.h>
#include <string.h>

using namespace std;

BEGIN_AS_NAMESPACE

// This function takes an input string and splits it into parts by looking
// for a specified delimiter. Example:
//
// string str = "A|B||D";
// array<string>@ array = str.split("|");
//
// The resulting array has the following elements:
//
// {"A", "B", "", "D"}
//
// AngelScript signature:
// array<string>@ string::split(const string &in delim) const
static CScriptArray *StringSplit(const string &delim, const string &str)
{
    // Obtain a pointer to the engine
    asIScriptContext *ctx = asGetActiveContext();
    asIScriptEngine *engine = ctx->GetEngine();

    // TODO: This should only be done once
    // TODO: This assumes that CScriptArray was already registered
    asITypeInfo *arrayType = engine->GetTypeInfoByDecl("array<string>");

    // Create the array object
    CScriptArray *array = CScriptArray::Create(arrayType);

    // Find the existence of the delimiter in the input string
    int pos = 0, prev = 0, count = 0;
    while( (pos = (int)str.find(delim, prev)) != (int)string::npos )
    {
        // Add the part to the array
        array->Resize(array->GetSize()+1);
        ((string*)array->At(count))->assign(&str[prev], pos-prev);

        // Find the next part
        count++;
        prev = pos + (int)delim.length();
    }

    // Add the remaining part
    array->Resize(array->GetSize()+1);
    ((string*)array->At(count))->assign(&str[prev]);

    return array;
}

static void StringSplit_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    string *str   = (string*)gen->GetObject();
    string *delim = *(string**)gen->GetAddressOfArg(0);

    // Return the array by handle
    *(CScriptArray**)gen->GetAddressOfReturnLocation() = StringSplit(*delim, *str);
}



// This function takes as input an array of string handles as well as a
// delimiter and concatenates the array elements into one delimited string.
// Example:
//
// array<string> array = {"A", "B", "", "D"};
// string str = join(array, "|");
//
// The resulting string is:
//
// "A|B||D"
//
// AngelScript signature:
// string join(const array<string> &in array, const string &in delim)
static string StringJoin(const CScriptArray &array, const string &delim)
{
    // Create the new string
    string str = "";
    if( array.GetSize() )
    {
        int n;
        for( n = 0; n < (int)array.GetSize() - 1; n++ )
        {
            str += *(string*)array.At(n);
            str += delim;
        }

        // Add the last part
        str += *(string*)array.At(n);
    }

    return str;
}

static void StringJoin_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(0);
    string *delim = *(string**)gen->GetAddressOfArg(1);

    // Return the string
    new(gen->GetAddressOfReturnLocation()) string(StringJoin(*array, *delim));
}

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterStdStringUtils(asIScriptEngine *engine)
{
    int r;

    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
    {
        r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
    }
    else
    {
        r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit), asCALL_CDECL_OBJLAST); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin), asCALL_CDECL); assert(r >= 0);
    }
}

END_AS_NAMESPACE
