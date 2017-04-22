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

        // --------------------------------------------------------------------
        /** Adds a key-value pair to the json string. */
        template <typename C>
        void add(const std::string &key, const C&value)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+key+"\":"+StringUtils::toString(value);
        }   // add
        // --------------------------------------------------------------------
        /** Specialisation for adding string values. String values in
         *  are enclosed in "". */
        void add(const std::string &key, const std::string &value)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+key+"\":\""+StringUtils::toString(value)+"\"";
        }   // add
        // --------------------------------------------------------------------
        /** Specialisation for adding character pointers. String values in
         *  are enclosed in "". */
        void add(const std::string &key, const char *s)
        {
            if(m_data.size()>1)   // more than '{'
                m_data += ",";
            m_data += "\""+key+"\":\""+StringUtils::toString(s)+"\"";
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
