//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include <assert.h>
#include "scriptstdstring.hpp"
#include "scriptarray.hpp"
#include <stdio.h>
#include <string.h>
#include "utils/translation.hpp"

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
	asIObjectType *arrayType = engine->GetObjectTypeById(engine->GetTypeIdByDecl("array<string>"));

    // Create the array object
    CScriptArray *array = new CScriptArray(0, arrayType);

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

void translate(asIScriptGeneric *gen)
{
    // Get the arguments
    std::string *input = (std::string*)gen->GetArgAddress(0);

    irr::core::stringw out = translations->fribidize(translations->w_gettext(input->c_str()));

    // Return the string
    new(gen->GetAddressOfReturnLocation()) string(StringUtils::wide_to_utf8(out.c_str()));
}

void insertValues1(asIScriptGeneric *gen)
{
    // Get the arguments
    std::string *input = (std::string*)gen->GetArgAddress(0);
    std::string *arg1 = (std::string*)gen->GetArgAddress(1);

    irr::core::stringw out = StringUtils::insertValues(StringUtils::utf8_to_wide(input->c_str()),
        StringUtils::utf8_to_wide(arg1->c_str()));

    // Return the string
    new(gen->GetAddressOfReturnLocation()) string(StringUtils::wide_to_utf8(out.c_str()));
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
    new(gen->GetAddressOfReturnLocation()) string(StringUtils::wide_to_utf8(out.c_str()));
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
    new(gen->GetAddressOfReturnLocation()) string(StringUtils::wide_to_utf8(out.c_str()));
}

/*
void insertValues(asIScriptGeneric *gen)
{
    // Get the arguments
    std::string *input = (std::string*)gen->GetArgAddress(0);
    CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(1);

    int size = array->GetSize();
    vector<string> all_values;
    for (int i = 0; i < size; i++)
    {
        string* curr = (string*)array->At(i);
        all_values.push_back(curr);
    }

    StringUtils::insertValues(*input, all_values);

    irr::core::stringw out = translations->fribidize(translations->w_gettext(input->c_str()));

    // Return the string
    new(gen->GetAddressOfReturnLocation()) string(irr::core::stringc(out).c_str());
}
*/

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterStdStringUtils(asIScriptEngine *engine)
{
	int r;

	//if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
	//{
		//r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
		//r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string translate(const string &in)", asFUNCTION(translate), asCALL_GENERIC); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in)", asFUNCTION(insertValues1), asCALL_GENERIC); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in)", asFUNCTION(insertValues2), asCALL_GENERIC); assert(r >= 0);
        r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &in, const string &in, const string &in)", asFUNCTION(insertValues3), asCALL_GENERIC); assert(r >= 0);
    //}
	//else
	//{
	//	//r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit), asCALL_CDECL_OBJLAST); assert(r >= 0);
	//	//r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin), asCALL_CDECL); assert(r >= 0);
    //    r = engine->RegisterGlobalFunction("string translate(const string &in)", asFUNCTION(translate), asCALL_CDECL); assert(r >= 0);
    //    r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &arg1)", asFUNCTION(insertValues1), asCALL_CDECL); assert(r >= 0);
    //    r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &arg1, const string &arg2)", asFUNCTION(insertValues2), asCALL_CDECL); assert(r >= 0);
    //    r = engine->RegisterGlobalFunction("string insertValues(const string &in, const string &arg1, const string &arg2, const string &arg3)", asFUNCTION(insertValues3), asCALL_CDECL); assert(r >= 0);
	//}
}

END_AS_NAMESPACE
