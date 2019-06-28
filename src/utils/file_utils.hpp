//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#ifndef HEADER_FILE_UTILS_HPP
#define HEADER_FILE_UTILS_HPP

#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include "irrString.h"

namespace FileUtils
{
    namespace Private
    {
        std::string getShortPath(const std::string& u8_path);
        std::string getShortPathW(const irr::core::stringw& w_path);
        std::string getShortPathWriting(const std::string& u8_path);
    }
    // ------------------------------------------------------------------------
    FILE* fopenU8Path(const std::string& u8_path, const char* mode);
    // ------------------------------------------------------------------------
    int statU8Path(const std::string& u8_path, struct stat *buf);
    // ------------------------------------------------------------------------
    int renameU8Path(const std::string& u8_path_old,
                     const std::string& u8_path_new);
    // ------------------------------------------------------------------------
    /* Return a path which can be opened for writing in all systems, as long as
     * u8_path is unicode encoded. */
    inline std::string getPortableWritingPath(const std::string& u8_path)
    {
#if defined(WIN32)
        return Private::getShortPathWriting(u8_path);
#else
        return u8_path;
#endif
    }
    // ------------------------------------------------------------------------
    /* Return a path which can be opened in all systems, as long as u8_path
     * is unicode encoded. */
    inline std::string getPortableReadingPath(const std::string& u8_path)
    {
#if defined(WIN32)
        return Private::getShortPath(u8_path);
#else
        return u8_path;
#endif
    }
} // namespace FileUtils

#endif
