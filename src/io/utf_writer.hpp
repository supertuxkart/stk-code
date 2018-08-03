//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Marianne Gagnon
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

#ifndef HEADER_UTF_WRITER_HPP
#define HEADER_UTF_WRITER_HPP

#include "utils/string_utils.hpp"

#include <irrString.h>

#include <fstream>

/**
 * \brief utility class used to write wide (UTF-16 or UTF-32, depending of size of wchar_t) XML files
 * \note the inner base class (ofstream) is not public because it will take in any kind of data, and
 *       we only want to accept arrays of wchar_t to make sure we get reasonable files out
 * \ingroup io
 */
class UTFWriter
{
    std::ofstream m_base;

    /** If true, use utf-16/32 (obsolete) */
    bool m_wide;
public:

    UTFWriter(const char* dest, bool wide);
    void close();

    UTFWriter& operator<< (const irr::core::stringw& txt);
    UTFWriter& operator<< (const wchar_t* txt);
    // ------------------------------------------------------------------------
    UTFWriter& operator<< (const char *txt)
    {
        if (m_wide)
        {
            return operator<<(irr::core::stringw(txt));
        }
        else
        {
            m_base.write((char *)txt, strlen(txt));
            return *this;
        }
            
    }   // operator<<(char*)
    // ------------------------------------------------------------------------
    UTFWriter& operator<< (const std::string &txt)
    {
        if (m_wide)
            return operator<<(irr::core::stringw(txt.c_str()));
        else
            return operator<<(txt.c_str());
    }   // operator<<(std::string)
    // ------------------------------------------------------------------------
    UTFWriter& operator<< (const bool &b)
    {
        return operator<<(StringUtils::toString(b));
    }
    // ------------------------------------------------------------------------
    template<typename T>
    UTFWriter& operator<< (const T &t)
    {
        return operator<<(StringUtils::toString<T>(t));
    }   // operator<< (template)
    // ------------------------------------------------------------------------
    bool is_open() { return m_base.is_open(); }
};

#endif
