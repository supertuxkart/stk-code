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
using namespace irr;

#if IRRLICHT_VERSION_MAJOR > 1 || (IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR >= 8)

// ----------------------------------------------------------------------------

XMLWriter::XMLWriter(const char* dest) : m_base(dest, std::ios::out | std::ios::binary)
{
    if (!m_base.is_open())
    {
        throw std::runtime_error("Failed to open file for writing : " + std::string(dest));
    }
    
    // FIXME: make sure to properly handle endianness
    wchar_t BOM = 0xFEFF; // UTF-16 BOM is 0xFEFF; UTF-32 BOM is 0x0000FEFF. So this works in either case
    
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
// ----------------------------------------------------------------------------

#else // Non-unicode version for irrlicht 1.7 and before

XMLWriter::XMLWriter(const char* dest) : m_base(dest, std::ios::out | std::ios::binary)
{
    if (!m_base.is_open())
    {
        throw std::runtime_error("Failed to open file for writing : " + std::string(dest));
    }
}

// ----------------------------------------------------------------------------

XMLWriter& XMLWriter::operator<< (const irr::core::stringw& txt)
{
    core::stringc s( txt.c_str() );
    m_base.write((char *) s.c_str(), s.size());
    return *this;
}

// ----------------------------------------------------------------------------

XMLWriter& XMLWriter::operator<< (const wchar_t*txt)
{
    core::stringc s( txt );
    m_base.write((char *) s.c_str(), s.size());
    return *this;
}

// ----------------------------------------------------------------------------

void XMLWriter::close()
{
    m_base.close();
}

// ----------------------------------------------------------------------------

#endif
