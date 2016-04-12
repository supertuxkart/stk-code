/*
AngelCode Scripting Library
Copyright (c) 2003-2015 Andreas Jonsson

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

#include "scriptstdstring.hpp"
#include <assert.h> // assert()
#include <sstream>  // std::stringstream
#include <string.h> // strstr()
#include <stdio.h>	// sprintf()
#include <stdlib.h> // strtod()
#ifndef __psp2__
	#include <locale.h> // setlocale()
#endif
#include <map>      // std::map

using namespace std;

BEGIN_AS_NAMESPACE

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)

#if AS_USE_STRINGPOOL == 1

// By keeping the literal strings in a pool the application
// performance is improved as there are less string copies created.

// The string pool will be kept as user data in the engine. We'll
// need a specific type to identify the string pool user data.
// We just define a number here that we assume nobody else is using for
// object type user data. The add-ons have reserved the numbers 1000
// through 1999 for this purpose, so we should be fine.
const asPWORD STRING_POOL = 1001;

// This global static variable is placed here rather than locally within the
// StringFactory, due to memory leak detectors that don't see the deallocation
// of global variables. By placing the variable globally it will be initialized
// before the memory leak detector starts, thus it won't report the missing
// deallocation. An example of this the Marmalade leak detector initialized with
// IwGxInit() and finished with IwGxTerminate().
static const string emptyString;

static const string &StringFactory(asUINT length, const char *s)
{
	// Each engine instance has its own string pool
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx == 0 )
	{
		// The string factory can only be called from a script
		assert( ctx );
		return emptyString;
	}
	asIScriptEngine *engine = ctx->GetEngine();

	// TODO: runtime optimize: Use unordered_map if C++11 is supported, i.e. MSVC10+, gcc 4.?+
	map<const char *, string> *pool = reinterpret_cast< map<const char *, string>* >(engine->GetUserData(STRING_POOL));

	if( !pool )
	{
		// The string pool hasn't been created yet, so we'll create it now
		asAcquireExclusiveLock();

		// Make sure the string pool wasn't created while we were waiting for the lock
		pool = reinterpret_cast< map<const char *, string>* >(engine->GetUserData(STRING_POOL));
		if( !pool )
		{
			#if defined(__S3E__)
			pool = new map<const char *, string>;
			#else
			pool = new (nothrow) map<const char *, string>;
			#endif
			if( pool == 0 )
			{
				ctx->SetException("Out of memory");
				asReleaseExclusiveLock();
				return emptyString;
			}
			engine->SetUserData(pool, STRING_POOL);
		}

		asReleaseExclusiveLock();
	}

	// We can't let other threads modify the pool while we query it
	asAcquireSharedLock();

	// First check if a string object hasn't been created already
	map<const char *, string>::iterator it;
	it = pool->find(s);
	if( it != pool->end() )
	{
		asReleaseSharedLock();
		return it->second;
	}

	asReleaseSharedLock();

	// Acquire an exclusive lock so we can add the new string to the pool
	asAcquireExclusiveLock();

	// Make sure the string wasn't created while we were waiting for the exclusive lock
	it = pool->find(s);
	if( it == pool->end() )
	{
		// Create a new string object
		it = pool->insert(map<const char *, string>::value_type(s, string(s, length))).first;
	}

	asReleaseExclusiveLock();
	return it->second;
}

static void CleanupEngineStringPool(asIScriptEngine *engine)
{
	map<const char *, string> *pool = reinterpret_cast< map<const char *, string>* >(engine->GetUserData(STRING_POOL));
	if( pool )
		delete pool;
}

#else
static string StringFactory(asUINT length, const char *s)
{
	return string(s, length);
}
#endif

static void ConstructString(string *thisPointer)
{
	new(thisPointer) string();
}

static void CopyConstructString(const string &other, string *thisPointer)
{
	new(thisPointer) string(other);
}

static void DestructString(string *thisPointer)
{
	thisPointer->~string();
}

static string &AddAssignStringToString(const string &str, string &dest)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration.
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	dest += str;
	return dest;
}

// bool string::isEmpty()
// bool string::empty() // if AS_USE_STLNAMES == 1
static bool StringIsEmpty(const string &str)
{
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	return str.empty();
}

static string &AssignUInt64ToString(asQWORD i, string &dest)
{
	ostringstream stream;
	stream << i;
	dest = stream.str();
	return dest;
}

static string &AddAssignUInt64ToString(asQWORD i, string &dest)
{
	ostringstream stream;
	stream << i;
	dest += stream.str();
	return dest;
}

static string AddStringUInt64(const string &str, asQWORD i)
{
	ostringstream stream;
	stream << i;
	return str + stream.str();
}

static string AddInt64String(asINT64 i, const string &str)
{
	ostringstream stream;
	stream << i;
	return stream.str() + str;
}

static string &AssignInt64ToString(asINT64 i, string &dest)
{
	ostringstream stream;
	stream << i;
	dest = stream.str();
	return dest;
}

static string &AddAssignInt64ToString(asINT64 i, string &dest)
{
	ostringstream stream;
	stream << i;
	dest += stream.str();
	return dest;
}

static string AddStringInt64(const string &str, asINT64 i)
{
	ostringstream stream;
	stream << i;
	return str + stream.str();
}

static string AddUInt64String(asQWORD i, const string &str)
{
	ostringstream stream;
	stream << i;
	return stream.str() + str;
}

static string &AssignDoubleToString(double f, string &dest)
{
	ostringstream stream;
	stream << f;
	dest = stream.str();
	return dest;
}

static string &AddAssignDoubleToString(double f, string &dest)
{
	ostringstream stream;
	stream << f;
	dest += stream.str();
	return dest;
}

static string &AssignFloatToString(float f, string &dest)
{
	ostringstream stream;
	stream << f;
	dest = stream.str();
	return dest;
}

static string &AddAssignFloatToString(float f, string &dest)
{
	ostringstream stream;
	stream << f;
	dest += stream.str();
	return dest;
}

static string &AssignBoolToString(bool b, string &dest)
{
	ostringstream stream;
	stream << (b ? "true" : "false");
	dest = stream.str();
	return dest;
}

static string &AddAssignBoolToString(bool b, string &dest)
{
	ostringstream stream;
	stream << (b ? "true" : "false");
	dest += stream.str();
	return dest;
}

static string AddStringDouble(const string &str, double f)
{
	ostringstream stream;
	stream << f;
	return str + stream.str();
}

static string AddDoubleString(double f, const string &str)
{
	ostringstream stream;
	stream << f;
	return stream.str() + str;
}

static string AddStringFloat(const string &str, float f)
{
	ostringstream stream;
	stream << f;
	return str + stream.str();
}

static string AddFloatString(float f, const string &str)
{
	ostringstream stream;
	stream << f;
	return stream.str() + str;
}

static string AddStringBool(const string &str, bool b)
{
	ostringstream stream;
	stream << (b ? "true" : "false");
	return str + stream.str();
}

static string AddBoolString(bool b, const string &str)
{
	ostringstream stream;
	stream << (b ? "true" : "false");
	return stream.str() + str;
}

static char *StringCharAt(unsigned int i, string &str)
{
	if( i >= str.size() )
	{
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		// Return a null pointer
		return 0;
	}

	return &str[i];
}

// AngelScript signature:
// int string::opCmp(const string &in) const
static int StringCmp(const string &a, const string &b)
{
	int cmp = 0;
	if( a < b ) cmp = -1;
	else if( a > b ) cmp = 1;
	return cmp;
}

// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findFirst(const string &in sub, uint start = 0) const
static int StringFindFirst(const string &sub, asUINT start, const string &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find(sub, start);
}

// This function returns the index of the last position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findLast(const string &in sub, int start = -1) const
static int StringFindLast(const string &sub, int start, const string &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.rfind(sub, (size_t)start);
}

// AngelScript signature:
// uint string::length() const
static asUINT StringLength(const string &str)
{
	// We don't register the method directly because the return type changes between 32bit and 64bit platforms
	return (asUINT)str.length();
}


// AngelScript signature:
// void string::resize(uint l)
static void StringResize(asUINT l, string &str)
{
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	str.resize(l);
}

// AngelScript signature:
// string formatInt(int64 val, const string &in options, uint width)
static string formatInt(asINT64 value, const string &options, asUINT width)
{
	bool leftJustify = options.find("l") != string::npos;
	bool padWithZero = options.find("0") != string::npos;
	bool alwaysSign  = options.find("+") != string::npos;
	bool spaceOnSign = options.find(" ") != string::npos;
	bool hexSmall    = options.find("h") != string::npos;
	bool hexLarge    = options.find("H") != string::npos;

	string fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

#ifdef _WIN32
	fmt += "*I64";
#else
#ifdef _LP64
	fmt += "*l";
#else
	fmt += "*ll";
#endif
#endif

	if( hexSmall ) fmt += "x";
	else if( hexLarge ) fmt += "X";
	else fmt += "d";

	string buf;
	buf.resize(width+30);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// string formatFloat(double val, const string &in options, uint width, uint precision)
static string formatFloat(double value, const string &options, asUINT width, asUINT precision)
{
	bool leftJustify = options.find("l") != string::npos;
	bool padWithZero = options.find("0") != string::npos;
	bool alwaysSign  = options.find("+") != string::npos;
	bool spaceOnSign = options.find(" ") != string::npos;
	bool expSmall    = options.find("e") != string::npos;
	bool expLarge    = options.find("E") != string::npos;

	string fmt = "%";
	if( leftJustify ) fmt += "-";
	if( alwaysSign ) fmt += "+";
	if( spaceOnSign ) fmt += " ";
	if( padWithZero ) fmt += "0";

	fmt += "*.*";

	if( expSmall ) fmt += "e";
	else if( expLarge ) fmt += "E";
	else fmt += "f";

	string buf;
	buf.resize(width+precision+50);
#if _MSC_VER >= 1400 && !defined(__S3E__)
	// MSVC 8.0 / 2005 or newer
	sprintf_s(&buf[0], buf.size(), fmt.c_str(), width, precision, value);
#else
	sprintf(&buf[0], fmt.c_str(), width, precision, value);
#endif
	buf.resize(strlen(&buf[0]));

	return buf;
}

// AngelScript signature:
// int64 parseInt(const string &in val, uint base = 10, uint &out byteCount = 0)
static asINT64 parseInt(const string &val, asUINT base, asUINT *byteCount)
{
	// Only accept base 10 and 16
	if( base != 10 && base != 16 )
	{
		if( byteCount ) *byteCount = 0;
		return 0;
	}

	const char *end = &val[0];

	// Determine the sign
	bool sign = false;
	if( *end == '-' )
	{
		sign = true;
		end++;
	}
	else if( *end == '+' )
		end++;

	asINT64 res = 0;
	if( base == 10 )
	{
		while( *end >= '0' && *end <= '9' )
		{
			res *= 10;
			res += *end++ - '0';
		}
	}
	else if( base == 16 )
	{
		while( (*end >= '0' && *end <= '9') ||
		       (*end >= 'a' && *end <= 'f') ||
		       (*end >= 'A' && *end <= 'F') )
		{
			res *= 16;
			if( *end >= '0' && *end <= '9' )
				res += *end++ - '0';
			else if( *end >= 'a' && *end <= 'f' )
				res += *end++ - 'a' + 10;
			else if( *end >= 'A' && *end <= 'F' )
				res += *end++ - 'A' + 10;
		}
	}

	if( byteCount )
		*byteCount = asUINT(size_t(end - val.c_str()));

	if( sign )
		res = -res;

	return res;
}

// AngelScript signature:
// double parseFloat(const string &in val, uint &out byteCount = 0)
double parseFloat(const string &val, asUINT *byteCount)
{
	char *end;

    // WinCE doesn't have setlocale. Some quick testing on my current platform
    // still manages to parse the numbers such as "3.14" even if the decimal for the
    // locale is ",".
#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Set the locale to C so that we are guaranteed to parse the float value correctly
	char *orig = setlocale(LC_NUMERIC, 0);
	setlocale(LC_NUMERIC, "C");
#endif

	double res = strtod(val.c_str(), &end);

#if !defined(_WIN32_WCE) && !defined(ANDROID) && !defined(__psp2__)
	// Restore the locale
	setlocale(LC_NUMERIC, orig);
#endif

	if( byteCount )
		*byteCount = asUINT(size_t(end - val.c_str()));

	return res;
}

// This function returns a string containing the substring of the input string
// determined by the starting index and count of characters.
//
// AngelScript signature:
// string string::substr(uint start = 0, int count = -1) const
static string StringSubString(asUINT start, int count, const string &str)
{
	// Check for out-of-bounds
	string ret;
	if( start < str.length() && count != 0 )
		ret = str.substr(start, count);

	return ret;
}

// String equality comparison.
// Returns true iff lhs is equal to rhs.
//
// For some reason gcc 4.7 has difficulties resolving the
// asFUNCTIONPR(operator==, (const string &, const string &)
// makro, so this wrapper was introduced as work around.
static bool StringEquals(const std::string& lhs, const std::string& rhs)
{
    return lhs == rhs;
}

void RegisterStdString_Native(asIScriptEngine *engine)
{
	int r = 0;
	UNUSED_VAR(r);

	// Register the string type
#if AS_CAN_USE_CPP11
	// With C++11 it is possible to use asGetTypeTraits to automatically determine the correct flags to use
	r = engine->RegisterObjectType("string", sizeof(string), asOBJ_VALUE | asGetTypeTraits<string>()); assert( r >= 0 );
#else
	r = engine->RegisterObjectType("string", sizeof(string), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
#endif

#if AS_USE_STRINGPOOL == 1
	// Register the string factory
	r = engine->RegisterStringFactory("const string &", asFUNCTION(StringFactory), asCALL_CDECL); assert( r >= 0 );

	// Register the cleanup callback for the string pool
	engine->SetEngineUserDataCleanupCallback(CleanupEngineStringPool, STRING_POOL);
#else
	// Register the string factory
	r = engine->RegisterStringFactory("string", asFUNCTION(StringFactory), asCALL_CDECL); assert( r >= 0 );
#endif

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructString),  asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asMETHODPR(string, operator =, (const string&), string&), asCALL_THISCALL); assert( r >= 0 );
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
//	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asMETHODPR(string, operator+=, (const string&), string&), asCALL_THISCALL); assert( r >= 0 );

	// Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
	r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTIONPR(StringEquals, (const string &, const string &), bool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int opCmp(const string &in) const", asFUNCTION(StringCmp), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTIONPR(operator +, (const string &, const string &), string), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	// The string length can be accessed through methods or through virtual property
	r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void resize(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint get_length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void set_length(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
//	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asMETHOD(string, empty), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	// Note that we don't register the operator[] directly, as it doesn't do bounds checking
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddStringDouble), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDoubleString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddStringFloat), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloatString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddStringInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt64ToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddStringUInt64), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt64String), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBoolToString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddStringBool), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBoolString), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Utilities
	r = engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findFirst(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findLast(const string &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("string formatInt(int64 val, const string &in options, uint width = 0)", asFUNCTION(formatInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatFloat(double val, const string &in options, uint width = 0, uint precision = 0)", asFUNCTION(formatFloat), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double parseFloat(const string &in, uint &out byteCount = 0)", asFUNCTION(parseFloat), asCALL_CDECL); assert(r >= 0);

#if AS_USE_STLNAMES == 1
	// Same as length
	r = engine->RegisterObjectMethod("string", "uint size() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as isEmpty
	r = engine->RegisterObjectMethod("string", "bool empty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findFirst
	r = engine->RegisterObjectMethod("string", "int find(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	// Same as findLast
	r = engine->RegisterObjectMethod("string", "int rfind(const string &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST); assert( r >= 0 );
#endif

	// TODO: Implement the following
	// findFirstOf
	// findLastOf
	// findFirstNotOf
	// findLastNotOf
	// findAndReplace - replaces a text found in the string
	// replaceRange - replaces a range of bytes in the string
	// trim/trimLeft/trimRight
	// multiply/times/opMul/opMul_r - takes the string and multiplies it n times, e.g. "-".multiply(5) returns "-----"
}

#if AS_USE_STRINGPOOL == 1
static void StringFactoryGeneric(asIScriptGeneric *gen)
{
  asUINT length = gen->GetArgDWord(0);
  const char *s = (const char*)gen->GetArgAddress(1);

  // Return a reference to a string
  gen->SetReturnAddress(const_cast<string*>(&StringFactory(length, s)));
}
#else
static void StringFactoryGeneric(asIScriptGeneric *gen)
{
  asUINT length = gen->GetArgDWord(0);
  const char *s = (const char*)gen->GetArgAddress(1);

  // Return a string value
  new (gen->GetAddressOfReturnLocation()) string(StringFactory(length, s));
}
#endif

static void ConstructStringGeneric(asIScriptGeneric * gen)
{
  new (gen->GetObject()) string();
}

static void CopyConstructStringGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetArgObject(0));
  new (gen->GetObject()) string(*a);
}

static void DestructStringGeneric(asIScriptGeneric * gen)
{
  string * ptr = static_cast<string *>(gen->GetObject());
  ptr->~string();
}

static void AssignStringGeneric(asIScriptGeneric *gen)
{
  string * a = static_cast<string *>(gen->GetArgObject(0));
  string * self = static_cast<string *>(gen->GetObject());
  *self = *a;
  gen->SetReturnAddress(self);
}

static void AddAssignStringGeneric(asIScriptGeneric *gen)
{
  string * a = static_cast<string *>(gen->GetArgObject(0));
  string * self = static_cast<string *>(gen->GetObject());
  *self += *a;
  gen->SetReturnAddress(self);
}

static void StringEqualsGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  string * b = static_cast<string *>(gen->GetArgAddress(0));
  *(bool*)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringCmpGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  string * b = static_cast<string *>(gen->GetArgAddress(0));

  int cmp = 0;
  if( *a < *b ) cmp = -1;
  else if( *a > *b ) cmp = 1;

  *(int*)gen->GetAddressOfReturnLocation() = cmp;
}

static void StringAddGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  string * b = static_cast<string *>(gen->GetArgAddress(0));
  string ret_val = *a + *b;
  gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric * gen)
{
  string * self = static_cast<string *>(gen->GetObject());
  *static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) = (asUINT)self->length();
}

static void StringIsEmptyGeneric(asIScriptGeneric * gen)
{
  string * self = reinterpret_cast<string *>(gen->GetObject());
  *reinterpret_cast<bool *>(gen->GetAddressOfReturnLocation()) = StringIsEmpty(*self);
}

static void StringResizeGeneric(asIScriptGeneric * gen)
{
  string * self = static_cast<string *>(gen->GetObject());
  self->resize(*static_cast<asUINT *>(gen->GetAddressOfArg(0)));
}

static void StringFindFirst_Generic(asIScriptGeneric * gen)
{
	string *find = reinterpret_cast<string*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string *self = reinterpret_cast<string *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirst(*find, start, *self);
}

static void StringFindLast_Generic(asIScriptGeneric * gen)
{
	string *find = reinterpret_cast<string*>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	string *self = reinterpret_cast<string *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLast(*find, start, *self);
}

static void formatInt_Generic(asIScriptGeneric * gen)
{
	asINT64 val = gen->GetArgQWord(0);
	string *options = reinterpret_cast<string*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new(gen->GetAddressOfReturnLocation()) string(formatInt(val, *options, width));
}

static void formatFloat_Generic(asIScriptGeneric *gen)
{
	double val = gen->GetArgDouble(0);
	string *options = reinterpret_cast<string*>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	asUINT precision = gen->GetArgDWord(3);
	new(gen->GetAddressOfReturnLocation()) string(formatFloat(val, *options, width, precision));
}

static void parseInt_Generic(asIScriptGeneric *gen)
{
	string *str = reinterpret_cast<string*>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseInt(*str,base,byteCount));
}

static void parseFloat_Generic(asIScriptGeneric *gen)
{
	string *str = reinterpret_cast<string*>(gen->GetArgAddress(0));
	asUINT *byteCount = reinterpret_cast<asUINT*>(gen->GetArgAddress(1));
	gen->SetReturnDouble(parseFloat(*str,byteCount));
}

static void StringCharAtGeneric(asIScriptGeneric * gen)
{
  unsigned int index = gen->GetArgDWord(0);
  string * self = static_cast<string *>(gen->GetObject());

  if (index >= self->size())
  {
    // Set a script exception
    asIScriptContext *ctx = asGetActiveContext();
    ctx->SetException("Out of range");

    gen->SetReturnAddress(0);
  }
  else
  {
    gen->SetReturnAddress(&(self->operator [](index)));
  }
}

static void AssignInt2StringGeneric(asIScriptGeneric *gen)
{
	asINT64 *a = static_cast<asINT64*>(gen->GetAddressOfArg(0));
	string *self = static_cast<string*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str();
	gen->SetReturnAddress(self);
}

static void AssignUInt2StringGeneric(asIScriptGeneric *gen)
{
	asQWORD *a = static_cast<asQWORD*>(gen->GetAddressOfArg(0));
	string *self = static_cast<string*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str();
	gen->SetReturnAddress(self);
}

static void AssignDouble2StringGeneric(asIScriptGeneric *gen)
{
	double *a = static_cast<double*>(gen->GetAddressOfArg(0));
	string *self = static_cast<string*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str();
	gen->SetReturnAddress(self);
}

static void AssignFloat2StringGeneric(asIScriptGeneric *gen)
{
	float *a = static_cast<float*>(gen->GetAddressOfArg(0));
	string *self = static_cast<string*>(gen->GetObject());
	std::stringstream sstr;
	sstr << *a;
	*self = sstr.str();
	gen->SetReturnAddress(self);
}

static void AssignBool2StringGeneric(asIScriptGeneric *gen)
{
	bool *a = static_cast<bool*>(gen->GetAddressOfArg(0));
	string *self = static_cast<string*>(gen->GetObject());
	std::stringstream sstr;
	sstr << (*a ? "true" : "false");
	*self = sstr.str();
	gen->SetReturnAddress(self);
}

static void AddAssignDouble2StringGeneric(asIScriptGeneric * gen)
{
  double * a = static_cast<double *>(gen->GetAddressOfArg(0));
  string * self = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a;
  *self += sstr.str();
  gen->SetReturnAddress(self);
}

static void AddAssignFloat2StringGeneric(asIScriptGeneric * gen)
{
  float * a = static_cast<float *>(gen->GetAddressOfArg(0));
  string * self = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a;
  *self += sstr.str();
  gen->SetReturnAddress(self);
}

static void AddAssignInt2StringGeneric(asIScriptGeneric * gen)
{
  asINT64 * a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
  string * self = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a;
  *self += sstr.str();
  gen->SetReturnAddress(self);
}

static void AddAssignUInt2StringGeneric(asIScriptGeneric * gen)
{
  asQWORD * a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
  string * self = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a;
  *self += sstr.str();
  gen->SetReturnAddress(self);
}

static void AddAssignBool2StringGeneric(asIScriptGeneric * gen)
{
  bool * a = static_cast<bool *>(gen->GetAddressOfArg(0));
  string * self = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << (*a ? "true" : "false");
  *self += sstr.str();
  gen->SetReturnAddress(self);
}

static void AddString2DoubleGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  double * b = static_cast<double *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  float * b = static_cast<float *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  asINT64 * b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  asQWORD * b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric * gen)
{
  string * a = static_cast<string *>(gen->GetObject());
  bool * b = static_cast<bool *>(gen->GetAddressOfArg(0));
  std::stringstream sstr;
  sstr << *a << (*b ? "true" : "false");
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric * gen)
{
  double* a = static_cast<double *>(gen->GetAddressOfArg(0));
  string * b = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric * gen)
{
  float* a = static_cast<float *>(gen->GetAddressOfArg(0));
  string * b = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric * gen)
{
  asINT64* a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
  string * b = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric * gen)
{
  asQWORD* a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
  string * b = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << *a << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric * gen)
{
  bool* a = static_cast<bool *>(gen->GetAddressOfArg(0));
  string * b = static_cast<string *>(gen->GetObject());
  std::stringstream sstr;
  sstr << (*a ? "true" : "false") << *b;
  std::string ret_val = sstr.str();
  gen->SetReturnObject(&ret_val);
}

static void StringSubString_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    string *str   = (string*)gen->GetObject();
    asUINT  start = *(int*)gen->GetAddressOfArg(0);
    int     count = *(int*)gen->GetAddressOfArg(1);

	// Return the substring
    new(gen->GetAddressOfReturnLocation()) string(StringSubString(start, count, *str));
}

void RegisterStdString_Generic(asIScriptEngine *engine)
{
	int r = 0;
	UNUSED_VAR(r);

	// Register the string type
	r = engine->RegisterObjectType("string", sizeof(string), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );

#if AS_USE_STRINGPOOL == 1
	// Register the string factory
	r = engine->RegisterStringFactory("const string &", asFUNCTION(StringFactoryGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the cleanup callback for the string pool
	engine->SetEngineUserDataCleanupCallback(CleanupEngineStringPool, STRING_POOL);
#else
	// Register the string factory
	r = engine->RegisterStringFactory("string", asFUNCTION(StringFactoryGeneric), asCALL_GENERIC); assert( r >= 0 );
#endif

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructStringGeneric),  asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asFUNCTION(AssignStringGeneric),    asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTION(StringEqualsGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int opCmp(const string &in) const", asFUNCTION(StringCmpGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTION(StringAddGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the object methods
	r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void resize(uint)",   asFUNCTION(StringResizeGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "uint get_length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "void set_length(uint)", asFUNCTION(StringResizeGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmptyGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC); assert( r >= 0 );

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddString2DoubleGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDouble2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddString2FloatGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloat2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddString2IntGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddString2UIntGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddString2BoolGeneric), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBool2StringGeneric), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findFirst(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "int findLast(const string &in, int start = -1) const", asFUNCTION(StringFindLast_Generic), asCALL_GENERIC); assert( r >= 0 );

	r = engine->RegisterGlobalFunction("string formatInt(int64 val, const string &in options, uint width = 0)", asFUNCTION(formatInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatFloat(double val, const string &in options, uint width = 0, uint precision = 0)", asFUNCTION(formatFloat_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt_Generic), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterGlobalFunction("double parseFloat(const string &in, uint &out byteCount = 0)", asFUNCTION(parseFloat_Generic), asCALL_GENERIC); assert(r >= 0);
}

void RegisterStdString(asIScriptEngine * engine)
{
	if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
		RegisterStdString_Generic(engine);
	else
		RegisterStdString_Native(engine);
}

END_AS_NAMESPACE




