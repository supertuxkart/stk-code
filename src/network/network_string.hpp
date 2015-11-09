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

#include "utils/types.hpp"

#include <string>
#include <vector>
#include <stdarg.h>
#include <assert.h>

typedef unsigned char uchar;

/** \class NetworkString
 *  \brief Describes a chain of 8-bit unsigned integers.
 *  This class allows you to easily create and parse 8-bit strings.
 */
class NetworkString
{
private:
    union FloatAsInt 
    {
        float f;
        uint8_t i[4];
    }; // float as integer

    // ------------------------------------------------------------------------
    union {
        double d;
        uint8_t i[8];
    } d_as_i; // double as integer

    // ------------------------------------------------------------------------

    std::vector<uint8_t> m_string;

public:
    /** Dummy constructor. */
    NetworkString() { }

    // ------------------------------------------------------------------------
    /** Constructor to store one byte. */
    NetworkString(int len)
    {
        m_string.reserve(len);
    }   // NetworkString(int)

    // ------------------------------------------------------------------------
    /** Copy constructor. */
    NetworkString(NetworkString const& copy) { m_string = copy.m_string; }

    // ------------------------------------------------------------------------
    /** Copy the data from a string. */
    NetworkString(const std::string & value)
    {
        m_string = std::vector<uint8_t>(value.begin(), value.end());
    }   // NetworkString

    // ------------------------------------------------------------------------
    NetworkString(const char *p, int len)
    {
        m_string.resize(len);
        memcpy(m_string.data(), p, len);
    }   // NetworkString(char*, int)
    // ------------------------------------------------------------------------
    NetworkString& add(const std::string &s)
    {
        return addUInt8(uint8_t(s.size())).as(s);
    }   // add

    // ------------------------------------------------------------------------
    NetworkString& removeFront(int size)
    {
        m_string.erase(m_string.begin(), m_string.begin() + size);
        return *this;
    }   // removeFront

    // ------------------------------------------------------------------------
    NetworkString& remove(int pos, int size)
    {
        m_string.erase(m_string.begin() + pos, m_string.begin() + pos + size);
        return *this;
    }   // remove

    // ------------------------------------------------------------------------
    uint8_t operator[](const int& pos) const
    {
        return getUInt8(pos);
    }   // operator[]

    // ------------------------------------------------------------------------
    /** Add 8 bit unsigned int. */
    NetworkString& addUInt8(const uint8_t& value)
    {
        m_string.push_back(value);
        return *this;
    }   // addUInt8

    // ------------------------------------------------------------------------
    /** Adds 8 bit integer. */
    inline NetworkString& ai8(const uint8_t& value) { return addUInt8(value); }

    // ------------------------------------------------------------------------
    /** Adds 16 bit unsigned int. */
    NetworkString& addUInt16(const uint16_t& value)
    {
        m_string.push_back((value >> 8) & 0xff);
        m_string.push_back(value & 0xff);
        return *this;
    }   // addUInt16

    // ------------------------------------------------------------------------
    /** Adds 16 bit integer. */
    inline NetworkString& ai16(const uint16_t& value)
    {
        return addUInt16(value); 
    }   // ai16

    // ------------------------------------------------------------------------
    /** Adds unsigned 32 bit integer. */
    NetworkString& addUInt32(const uint32_t& value)
    {
        m_string.push_back((value >> 24) & 0xff);
        m_string.push_back((value >> 16) & 0xff);
        m_string.push_back((value >> 8) & 0xff);
        m_string.push_back(value & 0xff);
        return *this;
    }   // addUInt32

    // ------------------------------------------------------------------------
    /** Adds 32 bit integer. */
    inline NetworkString& ai32(const uint32_t& value)
    {
        return addUInt32(value); 
    }   // ai32

    // ------------------------------------------------------------------------
    NetworkString& addInt(const int& value)
    {
        m_string.push_back((value >> 24) & 0xff);
        m_string.push_back((value >> 16) & 0xff);
        m_string.push_back((value >> 8) & 0xff);
        m_string.push_back(value & 0xff);
        return *this;
    }   // addInt

    // ------------------------------------------------------------------------
    inline NetworkString& ai(const int& value) { return addInt(value); }
    // ------------------------------------------------------------------------
    /** Adds a 4 byte floating point value. */
    NetworkString& addFloat(const float& value) //!< BEWARE OF PRECISION
    {
        assert(sizeof(float) == 4);
        FloatAsInt f_as_i;
        f_as_i.f = value;
        m_string.push_back(f_as_i.i[0]);
        m_string.push_back(f_as_i.i[1]);
        m_string.push_back(f_as_i.i[2]);
        m_string.push_back(f_as_i.i[3]);
        return *this;
    }
    // ------------------------------------------------------------------------
    /** Adds a 4 byte floating point value. */
    inline NetworkString& af(const float& value) { return addFloat(value); }

    // ------------------------------------------------------------------------
    /** Adds a 8 byte floating point value. */
    NetworkString& addDouble(const double& value) //!< BEWARE OF PRECISION
    {
        assert(sizeof(double) == 8);
        d_as_i.d = value;
        m_string.push_back(d_as_i.i[0]);
        m_string.push_back(d_as_i.i[1]);
        m_string.push_back(d_as_i.i[2]);
        m_string.push_back(d_as_i.i[3]);
        m_string.push_back(d_as_i.i[4]);
        m_string.push_back(d_as_i.i[5]);
        m_string.push_back(d_as_i.i[6]);
        m_string.push_back(d_as_i.i[7]);
        return *this;
    }   // addDouble

    // ------------------------------------------------------------------------
    /** Adds n 8 byte floating point value. */
    inline NetworkString& ad(const double& value) { return addDouble(value); }

    // ------------------------------------------------------------------------
    /** Adds a single character to the string. */
    NetworkString& addChar(const char& value)
    {
        m_string.push_back((uint8_t)(value));
        return *this;
    }   // addChar

    // ------------------------------------------------------------------------
    /** Adds a single character. */
    inline NetworkString& ac(const char& value) { return addChar(value); }

    // ------------------------------------------------------------------------
    /** Adds a string. */
    NetworkString& addString(const std::string& value)
    {
        for (unsigned int i = 0; i < value.size(); i++)
            m_string.push_back((uint8_t)(value[i]));
        return *this;
    }

    // ------------------------------------------------------------------------
    /** Adds a string. */
    inline NetworkString& as(const std::string& value)
    {
        return addString(value);
    }   // as

    // ------------------------------------------------------------------------
    /** Adds the content of another network string. */
    NetworkString& operator+=(NetworkString const& value)
    {
        m_string.insert(m_string.end(), value.m_string.begin(), 
                                        value.m_string.end()   );
        return *this;
    }   // operator+=

    // ------------------------------------------------------------------------
    /** Returns the content of the network string as a std::string. */
    const std::string std_string() const
    {
        std::string str(m_string.begin(), m_string.end());
        return str;
    }   // std_string

    // ------------------------------------------------------------------------
    /** Returns the current length of the network string. */
    int size() const { return (int)m_string.size(); }

    // ------------------------------------------------------------------------
    /** Returns a byte pointer to the content of the network string. */
    uint8_t* getBytes() { return &m_string[0]; };

    // ------------------------------------------------------------------------
    /** Returns a byte pointer to the content of the network string. */
    const uint8_t* getBytes() const { return &m_string[0]; };

    // ------------------------------------------------------------------------
    template<typename T, size_t n>
    T get(int pos) const
    {
        int a = n;
        T result = 0;
        while (a--)
        {
            result <<= 8; // offset one byte
            // add the data to result
            result += ((uint8_t)(m_string[pos + n - 1 - a]) & 0xff);
        }
        return result;
    }   // get(int pos)

    // ------------------------------------------------------------------------
    // Another function for n == 1 to surpress warnings in clang
    template<typename T>
    T get(int pos) const
    {
        return m_string[pos];
    }   // get

    // ------------------------------------------------------------------------
    /** Returns a standard integer. */
    inline int getInt(int pos = 0) const { return get<int, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns a standard unsigned integer. */
    inline uint32_t getUInt(int pos = 0) const { return get<uint32_t, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns a unsigned 32 bit integer. */
    inline uint32_t getUInt32(int pos = 0) const { return get<uint32_t, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 16 bit integer. */
    inline uint16_t getUInt16(int pos=0) const { return get<uint16_t, 2>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 8-bit integer. */
    inline uint8_t getUInt8(int pos = 0)  const { return get<uint8_t>(pos); }
    // ------------------------------------------------------------------------
    /** Returns a character. */
    inline char getChar(int pos = 0) const { return get<char>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned character. */
    inline unsigned char getUChar(int pos = 0) const
    {
        return get<unsigned char>(pos); 
    }   // getUChar
    // ------------------------------------------------------------------------
    /** Returns a part of the network string as a std::string.
     *  \param pos First position to be in the string.
     *  \param len Number of bytes to copy.
     */
    std::string getString(int pos, int len) const 
    {
        return std::string(m_string.begin() + pos,
                           m_string.begin() + pos + len); 
    }   // getString

    // ------------------------------------------------------------------------
    /** Returns a standard integer. */
    inline int gi(int pos = 0) const { return get<int, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Retrusn an unsigned standard integer. */
    inline uint32_t gui(int pos = 0) const { return get<uint32_t, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 32-bit integer. */
    inline uint32_t gui32(int pos = 0) const { return get<uint32_t, 4>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 16-bit integer. */
    inline uint16_t gui16(int pos = 0) const { return get<uint16_t, 2>(pos); }
    // ------------------------------------------------------------------------
    /** Returns an unsigned 8-bit integer. */
    inline uint8_t gui8(int pos = 0) const { return get<uint8_t>(pos); }
    // ------------------------------------------------------------------------
    /** Return a character. */
    inline char gc(int pos = 0) const { return get<char>(pos); }
    // ------------------------------------------------------------------------
    /** Return an unsigned character. */
    inline unsigned char guc(int pos = 0) const
    {
        return get<unsigned char>(pos); 
    }   // guc

    // ------------------------------------------------------------------------
    /** Returns a 4-byte floating point value. */
    float getFloat(int pos = 0) const //!< BEWARE OF PRECISION
    {
        FloatAsInt f_as_i;
        for (int i = 0; i < 4; i++)
            f_as_i.i[i] = m_string[pos + i];
        return f_as_i.f;
    }  // getFloat

    // ------------------------------------------------------------------------
    //! Functions to get while removing
    template<typename T, size_t n>
    T getAndRemove(int pos)
    {
        int a = n;
        T result = 0;
        while (a--)
        {
            result <<= 8; // offset one byte
            result += ((uint8_t)(m_string[pos + n - 1 - a]) & 0xff); // add the data
        }
        remove(pos, n);
        return result;
    }   // getAndRemove

    // ------------------------------------------------------------------------
    // Another function for n == 1 to surpress warnings in clang
    template<typename T>
    T getAndRemove(int pos)
    {
        T result = m_string[pos];
        remove(pos, 1);
        return result;
    }   // getAndRemove

    // ------------------------------------------------------------------------
    inline int getAndRemoveInt(int pos = 0)
    {
        return getAndRemove<int, 4>(pos);
    }   // getAndRemoveInt

    // ------------------------------------------------------------------------
    inline uint32_t getAndRemoveUInt(int pos = 0)
    {
        return getAndRemove<uint32_t, 4>(pos);
    }   // getAndRemoveUInt

    // ------------------------------------------------------------------------
    inline uint32_t getAndRemoveUInt32(int pos = 0)
    {
        return getAndRemove<uint32_t, 4>(pos); 
    }   // getAndRemoveUInt32
    
    // ------------------------------------------------------------------------
    inline uint16_t getAndRemoveUInt16(int pos = 0)
    {
        return getAndRemove<uint16_t, 2>(pos); 
    }   // getAndRemoveUInt16

    // ------------------------------------------------------------------------
    inline uint8_t getAndRemoveUInt8(int pos = 0)
    {
        return getAndRemove<uint8_t>(pos); 
    }   // getAndRemoveUInt8
    // ------------------------------------------------------------------------
    inline char getAndRemoveChar(int pos = 0)
    {
        return getAndRemove<char>(pos); 
    }   // getAndRemoveChar

    // ------------------------------------------------------------------------
    inline unsigned char getAndRemoveUChar(int pos = 0)
    {
        return getAndRemove<unsigned char>(pos); 
    }   // getAndRemoveUChar

    // ------------------------------------------------------------------------
    double getAndRemoveDouble(int pos = 0) //!< BEWARE OF PRECISION
    {
        for (int i = 0; i < 8; i++)
            d_as_i.i[i] = m_string[pos + i];
        return d_as_i.d;
        remove(pos, 8);
    }   // getAndRemoveDouble

    // ------------------------------------------------------------------------
    /** Get and remove a 4 byte floating point value. */
    float getAndRemoveFloat(int pos = 0) //!< BEWARE OF PRECISION
    {
        FloatAsInt f_as_i;
        for (int i = 0; i < 4; i++)
            f_as_i.i[i] = m_string[pos + i];
        return f_as_i.f;
        remove(pos, 4);
    }   // getAndRemoveFloat

    // ------------------------------------------------------------------------
    /** Removes a 8 bit unsigned int. */
    inline NetworkString& gui8(uint8_t* dst)
    {
        *dst = getAndRemoveUInt8(0);
        return *this; 
    }   // gui8

    // ------------------------------------------------------------------------
    /** Returns a 16 bit integer. */
    inline NetworkString& gui16(uint16_t* dst)
    {
        *dst = getAndRemoveUInt16(0);
        return *this; 
    }   // gui16

    // ------------------------------------------------------------------------
    /** Returns a 32 bit integer. */
    inline NetworkString& gui32(uint32_t* dst)
    {
        *dst = getAndRemoveUInt32(0);
        return *this; 
    }   // gui32

    // ------------------------------------------------------------------------
    /** Returns a 32 bit integer. */
    inline NetworkString& gui(uint32_t* dst)
    {
        *dst = getAndRemoveUInt32(0);
        return *this;
    }   // gui
    
    // ------------------------------------------------------------------------
    /** Returns 4 byte integer. */
    inline NetworkString& gi(int* dst)
    {
        *dst = getAndRemoveInt(0);
        return *this;
    }    // gi

    // ------------------------------------------------------------------------
    /** Returns a single character. */
    inline NetworkString& gc(char* dst)
    {
        *dst = getAndRemoveChar(0);
        return *this; 
    }   // gc

    // ------------------------------------------------------------------------
    /** Returns an unsigned character. */
    inline NetworkString& guc(uchar* dst)
    {
        *dst = getAndRemoveUChar(0);
        return *this; 
    }   // guc

};   // class NetworkString

NetworkString operator+(NetworkString const& a, NetworkString const& b);

#endif // NETWORK_STRING_HPP
