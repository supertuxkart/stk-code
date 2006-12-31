//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>,
//                     Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_STRING_UTILS_H
#define HEADER_STRING_UTILS_H

#include <string>
#include <vector>
#include <sstream>

namespace StringUtils
{

    bool has_suffix(const std::string& lhs, const std::string rhs);

    /** Return the filename part of a path */
    std::string basename(const std::string& filename);

    std::string without_extension(const std::string& filename);
    std::string extension(const std::string& filename);

    template <class T>
    std::string to_string (const T& any)
    {
        std::ostringstream oss;
        oss << any ;
        return oss.str();
    }

    /** Convert the contents in string \a rep to type \a T, if conversion
        fails false is returned and the value of \a x is unchanged, if
        true is returned the conversation was successfull. */
    template <class T>
    bool from_string(const std::string& rep, T& x)
    {
        // this is necessary so that if "x" is not modified if the conversion fails
        T temp;
        std::istringstream iss(rep);
        iss >> temp;

        if (iss.fail())
        {
            return false;
        }
        else
        {
            x = temp;
            return true;
        }
    }

    std::string upcase (const std::string&);
    std::string downcase (const std::string&);
    std::vector<std::string> split(const std::string& s, char c);

} // namespace StringUtils

#endif

/* EOF */
