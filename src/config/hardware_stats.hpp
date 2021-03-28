//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifndef HEADER_HARDWARE_STATS_HPP
#define HEADER_HARDWARE_STATS_HPP

/**
 * \defgroup config
 */

#include "utils/no_copy.hpp"
#include "utils/string_utils.hpp"

namespace HardwareStats
{
    /** A class to manage json data. */
    class Json : public NoCopy
    {
    private:
        /** The accumulated json data. */
        std::string m_data;
    public:
        /** Constructor. */
        Json()
        {
            m_data.reserve(1024);
            m_data ="{";
        }   // Constructor

        const std::string sanitize(std::string value)
        {
            // A string is a sequence of Unicode code points wrapped with quotation marks (U+0022). All code points may
            // be placed within the quotation marks except for the code points that must be escaped: quotation mark
            // (U+0022), reverse solidus (U+005C), and the control characters U+0000 to U+001F. There are two-character
            // escape sequence representations of some characters.
            char temp[7] = {0};
            for (size_t i = 0; i < value.size(); i++)
            {
                uint8_t codepoint = value[i];
                if (codepoint <= 0x1f)
                {
                    sprintf(temp, "\\u%04x", codepoint);
                    std::string suffix = value.substr(i + 1);
                    value = value.substr(0, i);
                    value.append(temp);
                    value.append(suffix);
                    i += 5; // \u0000 = 6 chars, but we're replacing one so 5
                }
                else if (codepoint == '"' || codepoint == '\\')
                {
                    std::string suffix = value.substr(i + 1);
                    value = value.substr(0, i);
                    value.push_back('\\');
                    value.push_back((char)codepoint);
                    value.append(suffix);
                    // Skip the added solidus
                    i++;
                }
            }
            return value;
        }   // sanitize
        // --------------------------------------------------------------------
        /** Adds a key-value pair to the json string. */
        template <typename C>
        void add(const std::string &key, const C&value)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+sanitize(key)+"\":"+StringUtils::toString(value);
        }   // add
        // --------------------------------------------------------------------
        /** Specialisation for adding string values. String values in
         *  are enclosed in "". */
        void add(const std::string &key, const std::string &value)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+sanitize(key)+"\":\""+sanitize(value)+"\"";
        }   // add
        // --------------------------------------------------------------------
        /** Specialisation for adding character pointers. String values in
         *  are enclosed in "". */
        void add(const std::string &key, const char *s)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+sanitize(key)+"\":\""+sanitize(s)+"\"";
        }   // add
        // --------------------------------------------------------------------
        void finish()
        {
            m_data += "}";
        }
        // --------------------------------------------------------------------
        /** Returns the json data as one string. */
        std::string toString() { return m_data; }
    };   // class Json

    // ========================================================================
    void reportHardwareStats();
    const std::string& getOSVersion();
    int getNumProcessors();
};   // HardwareStats

#endif
