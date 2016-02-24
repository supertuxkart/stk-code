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

#include <algorithm>   // for std::min
#include <ostream>

// ============================================================================
/** Unit testing function.
 */
void NewNetworkString::unitTesting()
{
    NewNetworkString s(PROTOCOL_LOBBY_ROOM);
    assert(s.getProtocolType() == PROTOCOL_LOBBY_ROOM);
    assert(s.getProtocolType() != PROTOCOL_KART_UPDATE);
    assert(!s.isSynchronous());
    s.setSynchronous(true);
    assert(s.isSynchronous());
    s.setSynchronous(false);
    assert(!s.isSynchronous());

    uint32_t token = 0x12345678;
    // Check token setting and reading
    s.setToken(token);
    assert(s.getToken()==token);
    assert(s.getToken()!=0x87654321);

    // Append some values from the message
    s.addUInt16(12345);
    s.addFloat(1.2345f);

    // Ignore message type and token
    s.removeFront(5);

    assert(s.getUInt16(0) == 12345);
    float f = s.getFloat(2);
    assert(f==1.2345f);

    // Check modifying a token in an already assembled message
    uint32_t new_token = 0x87654321;
    s.setToken(new_token);
    assert(s.getToken()!=token);
    assert(s.getToken()==new_token);

}   // unitTesting

// ============================================================================

// ----------------------------------------------------------------------------
/** Adds one byte for the length of the string, and then (up to 255 of)
 *  the characters of the given string. */
BareNetworkString& BareNetworkString::encodeString(const std::string &value)
{
    int len = value.size();
    if(len<=255)
        return this->addUInt8(len).addString(value);
    else
        return addUInt8(255).addString(value.substr(0, 255));
}   // encodeString

// ----------------------------------------------------------------------------
 /** Adds one byte for the length of the string, and then (up to 255 of)
 *  the characters of the given string. */
BareNetworkString& BareNetworkString::encodeString(const irr::core::stringw &value)
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
int BareNetworkString::decodeString(int pos, std::string *out) const
{
    uint8_t len = get<uint8_t>(pos);
    *out = getString(pos+1, len);
    return len+1;
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
int BareNetworkString::decodeStringW(int pos, irr::core::stringw *out) const
{
    std::string s;
    int len = decodeString(pos, &s);
    *out = StringUtils::utf8ToWide(s);
    return len;
}   // decodeString 

// ----------------------------------------------------------------------------
/** Returns a string representing this message suitable to be printed
 *  to stdout or via the Log mechanism. Format
 *   0000 : 1234 5678 9abc  ...    ASCII-
 */
std::string BareNetworkString::getLogMessage() const
{
    std::ostringstream oss;
    for(unsigned int line=0; line<16; line+=16)
    {
        oss << line << " : ";
        unsigned int upper_limit = std::min(line+16, size());
        for(unsigned int i=line; i<upper_limit; i++)
        {
            oss << getUInt8(i);
            if(i%2==1) oss << " ";
        }   // for i
        // Add ascii representation
        for(unsigned int i=line; i<upper_limit; i++)
        {
            oss << getUInt8(i);
        }   // for i
        oss << "\n";
    }   // for line

    return oss.str();
}   // getLogMessage

