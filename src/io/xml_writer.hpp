//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Marianne Gagnon
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

#ifndef HEADER_XML_WRITER_HPP
#define HEADER_XML_WRITER_HPP

#include <fstream>
#include <irrlicht.h>

/**
 * \brief utility class used to write wide (UTF-16 or UTF-32, depending of size of wchar_t) XML files
 * \note the inner base class (ofstream) is not public because it will take in any kind of data, and
 *       we only want to accept arrays of wchar_t to make sure we get reasonable files out
 * \ingroup io
 */
class XMLWriter
{
    std::ofstream m_base;
public:
    
    XMLWriter(const char* dest);
    
    XMLWriter& operator<< (const irr::core::stringw& txt);
    XMLWriter& operator<< (const wchar_t* txt);
    
    template<typename T>
    XMLWriter& operator<< (const T t)
    {
        irr::core::stringw tmp;
        tmp += t;
        (*this) << tmp;
        return *this;
    }
    
    void close();
};

#endif
