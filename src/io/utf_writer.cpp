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

#include "io/utf_writer.hpp"
#include "utils/file_utils.hpp"

#include <wchar.h>
#include <string>
#include <stdexcept>
using namespace irr;

// ----------------------------------------------------------------------------

UTFWriter::UTFWriter(const char* dest, bool wide)
         : m_base(FileUtils::getPortableWritingPath(dest),
                  std::ios::out | std::ios::binary)
{
    m_wide = wide;

    if (!m_base.is_open())
    {
        throw std::runtime_error("Failed to open file for writing : " +
                                  std::string(dest));
    }

    if (wide)
    {
        // FIXME: make sure to properly handle endianness
        // UTF-16 BOM is 0xFEFF; UTF-32 BOM is 0x0000FEFF. So this works in either case
        wchar_t BOM = 0xFEFF;

        m_base.write((char *) &BOM, sizeof(wchar_t));
    }
}   // UTFWriter

// ----------------------------------------------------------------------------

UTFWriter& UTFWriter::operator<< (const irr::core::stringw& txt)
{
    if (m_wide)
    {
        m_base.write((char *) txt.c_str(), txt.size() * sizeof(wchar_t));
    }
    else
    {
        std::string utf8 = StringUtils::wideToUtf8(txt);
        operator<<(utf8);
    }
    return *this;
}   // operator<< (stringw)

// ----------------------------------------------------------------------------

UTFWriter& UTFWriter::operator<< (const wchar_t*txt)
{
    if (m_wide)
    {
        m_base.write((char *) txt, wcslen(txt) * sizeof(wchar_t));
    }
    else
    {
        std::string utf8 = StringUtils::wideToUtf8(txt);
        operator<<(utf8);
    }
    return *this;
}   // operator<< (wchar_t)

// ----------------------------------------------------------------------------
void UTFWriter::close()
{
    m_base.close();
}   // close

// ----------------------------------------------------------------------------

