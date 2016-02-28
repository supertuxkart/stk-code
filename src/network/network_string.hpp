//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

/*! \file network_string.hpp
 *  \brief Defines functions to easily manipulate 8-bit network destinated strings.
 */

#ifndef NETWORK_STRING_HPP
#define NETWORK_STRING_HPP

#include "network/protocol.hpp"
#include "utils/types.hpp"
#include "utils/vec3.hpp"

#include "LinearMath/btQuaternion.h"

#include "irrString.h"

#include <assert.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <vector>

typedef unsigned char uchar;

/** \class BareNetworkString
 *  \brief Describes a chain of 8-bit unsigned integers.
 *  This class allows you to easily create and parse 8-bit strings, has 
 *  functions to add and read other data types (e.g. int, strings). It does
 *  not enforce any structure on the sequence (NetworkString uses this as
 *  a base class, and enforces a protocol type in the first byte, and a
 *  4-byte authentication token in bytes 2-5)
 */

class BareNetworkString
{
protected:
    /** The actual buffer. */
    std::vector<uint8_t> m_buffer;

    /** To avoid copying the buffer when bytes are deleted (which only
    *  happens at the front), use an offset index. All positions given
    *  by the user will be relative to this index. Note that the type
    *  should be left as signed, otherwise certain arithmetic (e.g.
    *  1-m_current_offset in checkToken() will be done unsigned).
    */
    int m_current_offset;

    // ------------------------------------------------------------------------
    /** Returns a part of the network string as a std::string. This is an
    *  internal function only, the user should call decodeString(W) instead.
    *  \param pos First position to be in the string.
    *  \param len Number of bytes to copy.
    */
    std::string getString(int pos, int len) const 
    {
        return std::string(m_buffer.begin() + (m_current_offset+pos      ),
            m_buffer.begin() + (m_current_offset+pos + len) ); 
    }   // getString
    // ------------------------------------------------------------------------
    /** Adds a std::string. Internal use only. */
    BareNetworkString& addString(const std::string& value)
    {
        for (unsigned int i = 0; i < value.size(); i++)
            m_buffer.push_back((uint8_t)(value[i]));
        return *this;
    }   // addString

    // ------------------------------------------------------------------------
    /** Template to get n bytes from a buffer into a single data type. */
    template<typename T, size_t n>
    T get(int pos) const
    {
        int a = n;
        T result = 0;
        int offset = m_current_offset + pos + n -1;
        while (a--)
        {
            result <<= 8; // offset one byte
                          // add the data to result
            result += m_buffer[offset - a];
        }
        return result;
    }   // get(int pos)
    // ------------------------------------------------------------------------
    /** Another function for n == 1 to surpress warnings in clang. */
    template<typename T>
    T get(int pos) const
    {
        return m_buffer[pos];
    }   // get

public:

    /** Constructor, sets the protocol type of this message. */
    BareNetworkString(int capacity=16)
    {
        m_buffer.reserve(capacity);
        m_current_offset = 0;
    }   // BareNetworkString

    // ------------------------------------------------------------------------
    BareNetworkString(const std::string &s)
    {
        m_current_offset = 0;
        encodeString(s);
    }   // BareNetworkString
    // ------------------------------------------------------------------------
    /** Initialises the string with a sequence of characters. */
    BareNetworkString(const char *data, int len)
    {
        m_current_offset = 0;
        m_buffer.resize(len);
        memcpy(m_buffer.data(), data, len);
    }   // BareNetworkString

    // ------------------------------------------------------------------------
    BareNetworkString& encodeString(const std::string &value);
    BareNetworkString& encodeString(const irr::core::stringw &value);
    int decodeString(int n, std::string *out) const;
    int decodeStringW(int n, irr::core::stringw *out) const;
    std::string getLogMessage() const;
    // ------------------------------------------------------------------------
    /** Returns a byte pointer to the content of the network string. */
    char* getData() { return (char*)(m_buffer.data()); };

    // ------------------------------------------------------------------------
    /** Returns a byte pointer to the content of the network string. */
    const char* getData() const { return (char*)(m_buffer.data()); };

    // ------------------------------------------------------------------------
    /** Returns the remaining length of the network string. */
    unsigned int size() const { return (int)m_buffer.size()-m_current_offset; }

    // ------------------------------------------------------------------------
    // All functions related to adding data to a network string
    /** Add 8 bit unsigned int. */
    BareNetworkString& addUInt8(const uint8_t value)
    {
        m_buffer.push_back(value);
        return *this;
    }   // addUInt8

    // ------------------------------------------------------------------------
    /** Adds a single character to the string. */
    BareNetworkString& addChar(const char value)
    {
        m_buffer.push_back((uint8_t)(value));
        return *this;
    }   // addChar
    // ------------------------------------------------------------------------
    /** Adds 16 bit unsigned int. */
    BareNetworkString& addUInt16(const uint16_t value)
    {
        m_buffer.push_back((value >> 8) & 0xff);
        m_buffer.push_back(value & 0xff);
        return *this;
    }   // addUInt16

    // ------------------------------------------------------------------------
    /** Adds unsigned 32 bit integer. */
    BareNetworkString& addUInt32(const uint32_t& value)
    {
        m_buffer.push_back((value >> 24) & 0xff);
        m_buffer.push_back((value >> 16) & 0xff);
        m_buffer.push_back((value >>  8) & 0xff);
        m_buffer.push_back( value        & 0xff);
        return *this;
    }   // addUInt32

    // ------------------------------------------------------------------------
    /** Adds a 4 byte floating point value. */
    BareNetworkString& addFloat(const float value)
    {
        uint32_t *p = (uint32_t*)&value;
        return addUInt32(*p);
    }   // addFloat

    // ------------------------------------------------------------------------
    /** Adds the content of another network string. */
    BareNetworkString& operator+=(BareNetworkString const& value)
    {
        m_buffer.insert(m_buffer.end(), value.m_buffer.begin(), 
            value.m_buffer.end()   );
        return *this;
    }   // operator+=

    // ------------------------------------------------------------------------
    /** Adds the xyz components of a Vec3 to the string. */
    BareNetworkString& add(const Vec3 &xyz)
    {
        return addFloat(xyz.getX()).addFloat(xyz.getY()).addFloat(xyz.getZ());
    }   // add

    // ------------------------------------------------------------------------
    /** Adds the four components of a quaternion. */
    BareNetworkString& add(const btQuaternion &quat)
    {
        return addFloat(quat.getX()).addFloat(quat.getY())
              .addFloat(quat.getZ()).addFloat(quat.getW());
    }   // add

    // Functions related to getting data from a network string
    // ------------------------------------------------------------------------
    /** Ignore the next num_bytes from the message. */
    void removeFront(unsigned int num_bytes)
    {
        assert(m_current_offset + num_bytes <= m_buffer.size());
        m_current_offset += num_bytes;
    }   // removeFront

    // ------------------------------------------------------------------------
    /** Gets the byte at the specified position (taking current offset into
    *  account). */
    uint8_t operator[](const int pos) const
    {
        return m_buffer[m_current_offset + pos];
    }   // operator[]

    // ------------------------------------------------------------------------
    /** Returns a unsigned 32 bit integer. */
    inline uint32_t getUInt32(int pos = 0) const { return get<uint32_t, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 16 bit integer. */
    inline uint16_t getUInt16(int pos=0) const { return get<uint16_t, 2>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 8-bit integer. */
    inline uint8_t getUInt8(int pos = 0)  const
    {
        return m_buffer[m_current_offset + pos];
    }   // getUInt8

    // ------------------------------------------------------------------------
    /** Gets a 4 byte floating point value. */
    float getFloat(int pos=0) const
    {
        uint32_t u = getUInt32(pos);
        return *(float*)&u;
    }   // getFloat

    // ------------------------------------------------------------------------
    /** Gets a Vec3. */
    Vec3 getVec3(int pos=0) const
    {
        return Vec3(getFloat(pos), getFloat(pos+4), getFloat(pos+8));
    }   // getVec3

    // ------------------------------------------------------------------------
    /** Gets a Vec3. */
    btQuaternion getQuat(int pos=0) const
    {
        return btQuaternion(getFloat(pos),   getFloat(pos+ 4),
            getFloat(pos+8), getFloat(pos+12) );
    }   // getQuat
    // ------------------------------------------------------------------------

};   // class BareNetworkString


// ============================================================================

/** A new implementation of NetworkString, which has a fixed format:
 *  Byte 0: The type of the message, which is actually a bit field:
 *          bit 7:    if set, the message needs to be handled synchronously,
 *                    otherwise it can be handled by the separate protocol
 *                    manager thread.
 *          bits 6-0: The protocol ID, which identifies the receiving protocol
 *                    for this message.
 *  Byte 1-4: A token to authenticate the sender.
 * 
 *  Otherwise this class offers template functions to add arbitrary variables,
 *  and retrieve them again. It kept the functionality of 'removing' bytes
 *  (e.g. the ProtocolManager would remove the first byte - protocol type -
 *  so that the protocols would not see this byte). But this is implemented
 *  now by using a base pointer (and not by moving the buffer content).
 */
class NetworkString : public BareNetworkString
{
public:
    static void unitTesting();
        
    /** Constructor for a message to be sent. It sets the 
     *  protocol type of this message. */
    NetworkString(ProtocolType type,  int capacity=16)
        : BareNetworkString(capacity)
    {
        m_buffer.push_back(type);
        addUInt32(0);   // add dummy token for now
    }   // NetworkString

    // ------------------------------------------------------------------------
    /** Constructor for a received message. It automatically ignored the first
     *  5 bytes which contain the type and token. Those will be access using
     *  special functions. */
    NetworkString(const uint8_t *data, int len) 
        : BareNetworkString((char*)data, len)
    {
        m_current_offset = 5;   // ignore type and token
    }   // NetworkString

    // ------------------------------------------------------------------------
    /** Returns the protocol type of this message. */
    ProtocolType getProtocolType() const
    {
        assert(!m_buffer.empty());
        return (ProtocolType)(m_buffer[0] & ~PROTOCOL_SYNCHRONOUS);
    }   // getProtocolType

    // ------------------------------------------------------------------------
    /** Sets if this message is to be sent synchronous or asynchronous. */
    void setSynchronous(bool b)
    {
        if(b)
            m_buffer[0] |= PROTOCOL_SYNCHRONOUS;
        else
            m_buffer[0] &= ~PROTOCOL_SYNCHRONOUS;
    }   // setSynchronous
    // ------------------------------------------------------------------------
    /** Returns if this message is synchronous or not. */
    bool isSynchronous() const
    {
        return (m_buffer[0] & PROTOCOL_SYNCHRONOUS) == PROTOCOL_SYNCHRONOUS;
    }   // isSynchronous
    // ------------------------------------------------------------------------
    /** Sets a token for a message. Note that the token in an already
    *  assembled message might be updated (e.g. if the same message is sent
    *  from the server to a set of clients). */
    void setToken(uint32_t token)
    {
        // Make sure there is enough space for the token:
        if(m_buffer.size()<5)
            m_buffer.resize(5);

        m_buffer[1] = (token >> 24) & 0xff;
        m_buffer[2] = (token >> 16) & 0xff;
        m_buffer[3] = (token >>  8) & 0xff;
        m_buffer[4] =  token        & 0xff;
    }   // setToken

    // ------------------------------------------------------------------------
    /** Returns if the security token of this message is the same as the
     *  specified token. */
    uint32_t getToken() const 
    {
        // Since m_current_offset might be set, we need to make sure
        // to really access bytes 1-4
        return getUInt32(1-m_current_offset);
    }   // getToken

};   // class NetworkString


#endif // NETWORK_STRING_HPP
