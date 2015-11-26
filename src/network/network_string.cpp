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
#include "network/network_string.hpp"

#include "utils/string_utils.hpp"

NetworkString operator+(NetworkString const& a, NetworkString const& b)
{
    NetworkString ns(a);
    ns += b;
    return ns;
}   // operator+

// ----------------------------------------------------------------------------
/** Adds one byte for the length of the string, and then (up to 255 of)
*  the characters of the given string. */
NetworkString& NetworkString::encodeString(const std::string &value)
{
    int len = value.size();
    if(len<=255)
        return addUInt8(len).addString(value);
    else
        return addUInt8(255).addString(value.substr(0, 255));
}   // encodeString

// ----------------------------------------------------------------------------
/** Adds one byte for the length of the string, and then (up to 255 of)
 *  the characters of the given string. */
NetworkString& NetworkString::encodeString(const irr::core::stringw &value)
{
    std::string v = StringUtils::wideToUtf8(value);
    return encodeString(v);
}   // encodeString

// ----------------------------------------------------------------------------
/** Returns a string at the given position. The first byte indicates the
 *  length, followed by the actual string (not 0 terminated).
 *  \param[in] pos Buffer position where the encoded string starts.
 *  \param[out] out The decoded string.
 *  \return number of bytes read = 1+length of string
 */
int NetworkString::decodeString(int pos, std::string *out) const
{
    uint8_t len = getUInt8(pos);
    *out = getString(pos+1, len);
    return len;
}    // decodeString

// ----------------------------------------------------------------------------
/** Returns an irrlicht wide string from the utf8 encoded string at the 
 *  given position.
 *  \param[in] pos Buffer position where the encoded string starts.
 *  \param[out] out The decoded string.
 *  \return number of bytes read. If there are no special characters in the
 *          string that will be 1+length of string, but multi-byte encoded
 *          characters can mean that the length of the returned string is
 *          less than the number of bytes read.
 */
int NetworkString::decodeStringW(int pos, irr::core::stringw *out) const
{
    std::string s;
    int len = decodeString(pos, &s);
    *out = StringUtils::utf8ToWide(s);
    return len;
}   // decodeString 
