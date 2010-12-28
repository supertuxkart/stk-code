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

#include "io/xml_writer.hpp"
#include <wchar.h>
#include <string>
#include <stdexcept>

// ----------------------------------------------------------------------------

XMLWriter::XMLWriter(const char* dest) : m_base(dest, std::ios::out | std::ios::binary)
{
    if (!m_base.is_open())
    {
        throw std::runtime_error("Failed to open file for writing : " + std::string(dest));
    }
    
    // FIXME: make sure the BOM makes sense on platforms where sizeof(wchar_t) is 32 bits
    // FIXME: make sure to properly handle endianness
    wchar_t BOM = 0xFEFF;
    
    m_base.write((char *) &BOM, sizeof(wchar_t));
}

// ----------------------------------------------------------------------------

XMLWriter& XMLWriter::operator<< (const irr::core::stringw& txt)
{
    m_base.write((char *) txt.c_str(), txt.size() * sizeof(wchar_t));
    return *this;
}

// ----------------------------------------------------------------------------

XMLWriter& XMLWriter::operator<< (const wchar_t*txt)
{
    m_base.write((char *) txt, wcslen(txt) * sizeof(wchar_t));
    return *this;
}

// ----------------------------------------------------------------------------

void XMLWriter::close()
{
    m_base.close();
}

// ----------------------------------------------------------------------------

