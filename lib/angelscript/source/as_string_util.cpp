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

#include <string.h>     // some compilers declare memcpy() here
#include <math.h>       // pow()

#if !defined(AS_NO_MEMORY_H)
#include <memory.h>
#endif

#include "as_string.h"
#include "as_string_util.h"

BEGIN_AS_NAMESPACE

int asCompareStrings(const char *str1, size_t len1, const char *str2, size_t len2)
{
	if( len1 == 0 ) 
	{
		if( str2 == 0 || len2 == 0 ) return 0; // Equal

		return 1; // The other string is larger than this
	}

	if( str2 == 0 )
	{
		if( len1 == 0 ) 
			return 0; // Equal

		return -1; // The other string is smaller than this
	}

	if( len2 < len1 )
	{
		int result = memcmp(str1, str2, len2);
		if( result == 0 ) return -1; // The other string is smaller than this

		return result;
	}

	int result = memcmp(str1, str2, len1);
	if( result == 0 && len1 < len2 ) return 1; // The other string is larger than this

	return result;
}

double asStringScanDouble(const char *string, size_t *numScanned)
{
	// I decided to do my own implementation of strtod() because this function
	// doesn't seem to be present on all systems. iOS 5 for example doesn't appear 
	// to include the function in the standard lib.
	
	// Another reason is that the standard implementation of strtod() is dependent
	// on the locale on some systems, i.e. it may use comma instead of dot for 
	// the decimal indicator. This can be avoided by forcing the locale to "C" with
	// setlocale(), but this is another thing that is highly platform dependent.

	double value = 0;
	double fraction = 0.1;
	int exponent = 0;
	bool negativeExponent = false;
	int c = 0;

	// The tokenizer separates the sign from the number in   
	// two tokens so we'll never have a sign to parse here

	// Parse the integer value
	for( ;; )
	{
		if( string[c] >= '0' && string[c] <= '9' )
			value = value*10 + double(string[c] - '0');
		else 
			break;

		c++;
	}

	if( string[c] == '.' )
	{
		c++;

		// Parse the fraction
		for( ;; )
		{
			if( string[c] >= '0' && string[c] <= '9' )
				value += fraction * double(string[c] - '0');
			else
				break;

			c++;
			fraction *= 0.1;
		}
	}

	if( string[c] == 'e' || string[c] == 'E' )
	{
		c++;

		// Parse the sign of the exponent
		if( string[c] == '-' )
		{
			negativeExponent = true;
			c++;
		}
		else if( string[c] == '+' )
			c++;

		// Parse the exponent value
		for( ;; )
		{
			if( string[c] >= '0' && string[c] <= '9' )
				exponent = exponent*10 + int(string[c] - '0');
			else
				break;

			c++;
		}
	}

	if( exponent )
	{
		if( negativeExponent )
			exponent = -exponent;
		value *= pow(10.0, exponent);
	}

	if( numScanned )
		*numScanned = c;

	return value;
}

// Converts a character to the decimal number based on the radix
// Returns -1 if the character is not valid for the radix
static int asCharToNbr(char ch, int radix)
{
	if( ch >= '0' && ch <= '9' ) return ((ch -= '0') < radix ? ch : -1);
	if( ch >= 'A' && ch <= 'Z' ) return ((ch -= 'A'-10) < radix ? ch : -1);
	if( ch >= 'a' && ch <= 'z' ) return ((ch -= 'a'-10) < radix ? ch : -1);
	return -1;
}

// If base is 0 the string should be prefixed by 0x, 0d, 0o, or 0b to allow the function to automatically determine the radix
asQWORD asStringScanUInt64(const char *string, int base, size_t *numScanned, bool *overflow)
{
	asASSERT(base == 10 || base == 16 || base == 0);

	if (overflow)
		*overflow = false;

	const char *end = string;

	static const asQWORD QWORD_MAX = (~asQWORD(0));

	asQWORD res = 0;
	if( base == 10 )
	{
		while( *end >= '0' && *end <= '9' )
		{
			if( overflow && ((res > QWORD_MAX / 10) || ((asUINT(*end - '0') > (QWORD_MAX - (QWORD_MAX / 10) * 10)) && res == QWORD_MAX / 10)) )
				*overflow = true;
			res *= 10;
			res += *end++ - '0';
		}
	}
	else
	{
		if( base == 0 && string[0] == '0')
		{
			// Determine the radix from the prefix
			switch( string[1] )
			{
			case 'b': case 'B': base = 2; break;
			case 'o': case 'O': base = 8; break;
			case 'd': case 'D': base = 10; break;
			case 'x': case 'X': base = 16; break;
			}
			end += 2;
		}

		asASSERT( base );

		if( base )
		{
			for (int nbr; (nbr = asCharToNbr(*end, base)) >= 0; end++)
			{
				if (overflow && ((res > QWORD_MAX / base) || ((asUINT(nbr) > (QWORD_MAX - (QWORD_MAX / base) * base)) && res == QWORD_MAX / base)) )
					*overflow = true;

				res = res * base + nbr;
			}
		}
	}

	if( numScanned )
		*numScanned = end - string;

	return res;
}

//
// The function will encode the unicode code point into the outEncodedBuffer, and then
// return the length of the encoded value. If the input value is not a valid unicode code 
// point, then the function will return -1.
//
// This function is taken from the AngelCode ToolBox.
//
int asStringEncodeUTF8(unsigned int value, char *outEncodedBuffer)
{
	unsigned char *buf = (unsigned char*)outEncodedBuffer;

	int length = -1;

	if( value <= 0x7F )
	{
		buf[0] = static_cast<unsigned char>(value);
		return 1;
	}
	else if( value >= 0x80 && value <= 0x7FF )
	{
		// Encode it with 2 characters
		buf[0] = static_cast<unsigned char>(0xC0 + (value >> 6));
		length = 2;
	}
	else if( (value >= 0x800 && value <= 0xD7FF) || (value >= 0xE000 && value <= 0xFFFF) )
	{
		// Note: Values 0xD800 to 0xDFFF are not valid unicode characters
		buf[0] = static_cast<unsigned char>(0xE0 + (value >> 12));
		length = 3;
	}
	else if( value >= 0x10000 && value <= 0x10FFFF )
	{
		buf[0] = static_cast<unsigned char>(0xF0 + (value >> 18));
		length = 4;
	}

	int n = length-1;
	for( ; n > 0; n-- )
	{
		buf[n] = static_cast<unsigned char>(0x80 + (value & 0x3F));
		value >>= 6;
	}

	return length;
}

//
// The function will decode an UTF8 character and return the unicode code point.
// outLength will receive the number of bytes that were decoded.
//
// This function is taken from the AngelCode ToolBox.
//
int asStringDecodeUTF8(const char *encodedBuffer, unsigned int *outLength)
{
	const unsigned char *buf = (const unsigned char*)encodedBuffer;
	
	int value = 0;
	int length = -1;
	unsigned char byte = buf[0];
	if( (byte & 0x80) == 0 )
	{
		// This is the only byte
		if( outLength ) *outLength = 1;
		return byte;
	}
	else if( (byte & 0xE0) == 0xC0 )
	{
		// There is one more byte
		value = int(byte & 0x1F);
		length = 2;

		// The value at this moment must not be less than 2, because 
		// that should have been encoded with one byte only.
		if( value < 2 )
			length = -1;
	}
	else if( (byte & 0xF0) == 0xE0 )
	{
		// There are two more bytes
		value = int(byte & 0x0F);
		length = 3;
	}
	else if( (byte & 0xF8) == 0xF0 )
	{
		// There are three more bytes
		value = int(byte & 0x07);
		length = 4;
	}

	int n = 1;
	for( ; n < length; n++ )
	{
		byte = buf[n];
		if( (byte & 0xC0) == 0x80 )
			value = (value << 6) + int(byte & 0x3F);
		else 
			break;
	}

	if( n == length )
	{
		if( outLength ) *outLength = (unsigned)length;
		return value;
	}

	// The byte sequence isn't a valid UTF-8 byte sequence.
	return -1;
}

//
// The function will encode the unicode code point into the outEncodedBuffer, and then
// return the length of the encoded value. If the input value is not a valid unicode code 
// point, then the function will return -1.
//
// This function is taken from the AngelCode ToolBox.
//
int asStringEncodeUTF16(unsigned int value, char *outEncodedBuffer)
{
	if( value < 0x10000 )
	{
#ifndef AS_BIG_ENDIAN
		outEncodedBuffer[0] = (value & 0xFF);
		outEncodedBuffer[1] = ((value >> 8) & 0xFF);
#else
		outEncodedBuffer[1] = (value & 0xFF);
		outEncodedBuffer[0] = ((value >> 8) & 0xFF);
#endif
		return 2;
	}
	else
	{
		value -= 0x10000;
		int surrogate1 = ((value >> 10) & 0x3FF) + 0xD800;
		int surrogate2 = (value & 0x3FF) + 0xDC00;

#ifndef AS_BIG_ENDIAN
		outEncodedBuffer[0] = (surrogate1 & 0xFF);
		outEncodedBuffer[1] = ((surrogate1 >> 8) & 0xFF);
		outEncodedBuffer[2] = (surrogate2 & 0xFF);
		outEncodedBuffer[3] = ((surrogate2 >> 8) & 0xFF);
#else
		outEncodedBuffer[1] = (surrogate1 & 0xFF);
		outEncodedBuffer[0] = ((surrogate1 >> 8) & 0xFF);
		outEncodedBuffer[3] = (surrogate2 & 0xFF);
		outEncodedBuffer[2] = ((surrogate2 >> 8) & 0xFF);
#endif

		return 4;
	}
}


END_AS_NAMESPACE
