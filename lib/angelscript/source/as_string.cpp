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

#include "as_config.h"

#include <stdarg.h>		// va_list, va_start(), etc
#include <stdlib.h>		// strtod(), strtol()
#include <string.h>		// some compilers declare memcpy() here

#if !defined(AS_NO_MEMORY_H)
#include <memory.h>
#endif

#include "as_string.h"
#include "as_string_util.h"

asCString::asCString()
{
	length = 0;
	local[0] = 0;
}

// Copy constructor
asCString::asCString(const asCString &str)
{
	length = 0;
	local[0] = 0;

	Assign(str.AddressOf(), str.length);
}

#ifdef AS_CAN_USE_CPP11
asCString::asCString(asCString &&str)
{
	if( str.length <= 11 )
	{
		length = str.length;
		memcpy(local, str.local, length);
		local[length] = 0;
	}
	else
	{
		dynamic = str.dynamic;
		length = str.length;
	}

	str.dynamic = 0;
	str.length = 0;
}
#endif // c++11

asCString::asCString(const char *str, size_t len)
{
	length = 0;
	local[0] = 0;

	Assign(str, len);
}

asCString::asCString(const char *str)
{
	length = 0;
	local[0] = 0;

	size_t len = strlen(str);
	Assign(str, len);
}

asCString::asCString(char ch)
{
	length = 0;
	local[0] = 0;

	Assign(&ch, 1);
}

asCString::~asCString()
{
	if( length > 11 && dynamic )
	{
		asDELETEARRAY(dynamic);
	}
}

char *asCString::AddressOf()
{
	if( length <= 11 )
		return local;
	else
		return dynamic;
}

const char *asCString::AddressOf() const
{
	if( length <= 11 )
		return local;
	else
		return dynamic;
}

void asCString::SetLength(size_t len)
{
	Allocate(len, true);
}

void asCString::Allocate(size_t len, bool keepData)
{
	// If we stored the capacity of the dynamically allocated buffer it would be possible
	// to save some memory allocations if a string decreases in size then increases again,
	// but this would require extra bytes in the string object itself, or a decrease of 
	// the static buffer, which in turn would mean extra memory is needed. I've tested each
	// of these options, and it turned out that the current choice is what best balanced
	// the number of allocations against the size of the allocations.

	if( len > 11 && len > length )
	{
		// Allocate a new dynamic buffer if the new one is larger than the old
		char *buf = asNEWARRAY(char,len+1);
		if( buf == 0 )
		{
			// Out of memory. Return without modifying anything
			return;
		}

		if( keepData )
		{
			int l = (int)len < (int)length ? (int)len : (int)length;
			memcpy(buf, AddressOf(), l);
		}

		if( length > 11 )
		{
			asDELETEARRAY(dynamic);
		}

		dynamic = buf;
	}
	else if( len <= 11 && length > 11 )
	{
		// Free the dynamic buffer, since it is no longer needed
		char *buf = dynamic;
		if( keepData )
		{
			memcpy(&local, buf, len);
		}
		asDELETEARRAY(buf);
	}

	length = (int)len;

	// Make sure the buffer is null terminated
	AddressOf()[length] = 0;
}

void asCString::Assign(const char *str, size_t len)
{
	Allocate(len, false);

	// Copy the string
	memcpy(AddressOf(), str, length);
	AddressOf()[length] = 0;
}

asCString &asCString::operator =(const char *str)
{
	size_t len = str ? strlen(str) : 0;
	Assign(str, len);

	return *this;
}

asCString &asCString::operator =(const asCString &str)
{
	Assign(str.AddressOf(), str.length);

	return *this;
}

#ifdef AS_CAN_USE_CPP11
asCString &asCString::operator =(asCString &&str)
{
	if( this != &str )
	{
		if( length > 11 && dynamic )
		{
			asDELETEARRAY(dynamic);
		}

		if ( str.length <= 11 )
		{
			length = str.length;

			memcpy(local, str.local, length);
			local[length] = 0;
		}
		else
		{
			dynamic = str.dynamic;
			length = str.length;
		}

		str.dynamic = 0;
		str.length = 0;
	}

	return *this;
}
#endif // c++11

asCString &asCString::operator =(char ch)
{
	Assign(&ch, 1);

	return *this;
}

void asCString::Concatenate(const char *str, size_t len)
{
	asUINT oldLength = length;
	SetLength(length + len);

	memcpy(AddressOf() + oldLength, str, len);
	AddressOf()[length] = 0;
}

asCString &asCString::operator +=(const char *str)
{
	size_t len = strlen(str);
	Concatenate(str, len);

	return *this;
}

asCString &asCString::operator +=(const asCString &str)
{
	Concatenate(str.AddressOf(), str.length);

	return *this;
}

asCString &asCString::operator +=(char ch)
{
	Concatenate(&ch, 1);

	return *this;
}

size_t asCString::GetLength() const
{
	return length;
}

// Returns the length
size_t asCString::Format(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	const size_t startSize = 1024;
	char tmp[startSize];
	int r = asVSNPRINTF(tmp, startSize-1, format, args);

	if( r > 0 && r < int(startSize) )
	{
		Assign(tmp, r);
	}
	else
	{
		// TODO: For some reason this doesn't work properly on Linux. Perhaps the
		//       problem is related to vsnprintf not keeping the state of va_arg.
		//       Perhaps I need to rewrite this in some way to keep the state
		size_t n = startSize*2;
		asCString str; // Use temporary string in case the current buffer is a parameter
		str.Allocate(n, false);

		while( (r = asVSNPRINTF(str.AddressOf(), n, format, args)) < 0 || r >= int(n) )
		{
			n *= 2;
			str.Allocate(n, false);
		}

		Assign(str.AddressOf(), r);
	}

	va_end(args);

	return length;
}

char &asCString::operator [](size_t index) 
{
	asASSERT(index < length);

	return AddressOf()[index];
}

const char &asCString::operator [](size_t index) const
{
	asASSERT(index < length);

	return AddressOf()[index];
}

asCString asCString::SubString(size_t in_start, size_t in_length) const
{
	if( in_start >= GetLength() || in_length == 0 )
		return asCString("");

	if( in_length == (size_t)(-1) ) in_length = GetLength() - in_start;

	asCString tmp;
	tmp.Assign(AddressOf() + in_start, in_length);

	return tmp;
}

int asCString::Compare(const char *str) const
{
	return asCompareStrings(AddressOf(), length, str, strlen(str));
}

int asCString::Compare(const asCString &str) const
{
	return asCompareStrings(AddressOf(), length, str.AddressOf(), str.GetLength());
}

int asCString::Compare(const char *str, size_t len) const
{
	return asCompareStrings(AddressOf(), length, str, len);
}

size_t asCString::RecalculateLength()
{
	SetLength(strlen(AddressOf()));

	return length;
}

int asCString::FindLast(const char *str, int *count) const
{
	// There is no strstr that starts from the end, so 
	// we'll iterate until we find the last occurrance.
	// This shouldn't cause a performance problem because
	// it is not expected that this will be done very often,
	// and then only on quite short strings anyway.

	if( count ) *count = 0;

	const char *last = 0;
	const char *curr = AddressOf()-1;
	while( (curr = strstr(curr+1, str)) != 0 )
	{
		if( count ) (*count)++;
		last = curr;
	}

	if( last )
		return int(last - AddressOf());

	return -1;
}

//-----------------------------------------------------------------------------
// Helper functions

bool operator ==(const asCString &a, const char *b)
{
	return a.Compare(b) == 0;
}

bool operator !=(const asCString &a, const char *b)
{
	return a.Compare(b) != 0;
}

bool operator ==(const asCString &a, const asCString &b)
{
	return a.Compare(b) == 0;
}

bool operator !=(const asCString &a, const asCString &b)
{
	return a.Compare(b) != 0;
}

bool operator ==(const char *a, const asCString &b)
{
	return b.Compare(a) == 0;
}

bool operator !=(const char *a, const asCString &b)
{
	return b.Compare(a) != 0;
}

bool operator <(const asCString &a, const asCString &b)
{
	return a.Compare(b) < 0;
}

asCString operator +(const asCString &a, const asCString &b)
{
	asCString res = a;
	res += b;

	return res;
}

asCString operator +(const char *a, const asCString &b)
{
	asCString res = a;
	res += b;

	return res;
}

asCString operator +(const asCString &a, const char *b)
{
	asCString res = a;
	res += b;

	return res;
}

// wrapper class

asCStringPointer::asCStringPointer()
	: string(0), length(0), cstring(0)
{
}

asCStringPointer::asCStringPointer(const char *str, size_t len)
	: string(str), length(len), cstring(0)
{
}

asCStringPointer::asCStringPointer(asCString *cstr)
	: string(0), length(0), cstring(cstr)
{
}

const char *asCStringPointer::AddressOf() const
{
	return string ? string : cstring->AddressOf();
}

size_t asCStringPointer::GetLength() const
{
	return string ? length : cstring->GetLength();
}

bool asCStringPointer::operator==(const asCStringPointer& other) const
{
	return asCompareStrings(AddressOf(), GetLength(), other.AddressOf(), other.GetLength()) == 0;
}

bool asCStringPointer::operator<(const asCStringPointer& other) const
{
	return asCompareStrings(AddressOf(), GetLength(), other.AddressOf(), other.GetLength()) < 0;
}
