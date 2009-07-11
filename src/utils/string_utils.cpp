//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>,
//                     Ingo Ruhnke <grumbel@gmx.de>
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

#include "utils/string_utils.hpp"

#include "math.h"
#include <algorithm>
#include <cstring>
#include <stdio.h>

namespace StringUtils
{
    bool has_suffix(const std::string& lhs, const std::string rhs)
    {
        if (lhs.length() < rhs.length())
            return false;
        else
            // While this is basically correct, it fails with older
            // g++ versions (at least 2.95.3), which have a wrong template. To
            // avoid this issue, a more C-traditional way is used.
            return strcmp(lhs.c_str()+(lhs.length()-rhs.length()), rhs.c_str())==0;
    }   // has_suffix

//--------------------------i---------------------------------------------------
    std::string path(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i]=='/' || filename[i]=='\\')
            {
                return filename.substr(0,i);
            }
        }
        return "";
    }   // path

//-----------------------------------------------------------------------------
    std::string basename(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i]=='/' || filename[i]=='\\')
            {
                return filename.substr(i+1);
            }
        }
        return filename;
    }   // basename

//-----------------------------------------------------------------------------
    std::string without_extension(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i] == '.')
            {
                return filename.substr(0, i);
            }
        }
        return filename;
    }   // without_extension

//-----------------------------------------------------------------------------
    std::string extension(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i] == '.')
            {
                return filename.substr(i+1);
            }
        }
        return filename;
    }   // extension

//-----------------------------------------------------------------------------
    std::string upcase (const std::string& str)
    {
        std::string name = str;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        return name;
    }   // upcase

//-----------------------------------------------------------------------------
    std::string downcase (const std::string& str)
    {
        std::string name = str;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        return name;
    }   // downcase

//-----------------------------------------------------------------------------
// Splits a string into substrings separated by a certain character, and 
// returns a std::vector of all those substring. E.g.:
//  split("a b=c d=e",' ')  --> ["a", "b=c", "d=e"]
    std::vector<std::string> split(const std::string& s, char c)
    {
        std::vector<std::string> result;
    
        std::string::size_type start=0;
        while(start!=std::string::npos)
        {
            std::string::size_type i=s.find(c, start);
            if(i!=std::string::npos)
            {
                result.push_back(std::string(s,start, i-start));
                start=i+1;
            } 
            else
            {
                result.push_back(std::string(s,start));
                start = i;
            }
        }
        return result;
    }   // split

    // ------------------------------------------------------------------------
    /** Converts a time in seconds into a string of the form mm:ss:hh (minutes,
     *  seconds, 1/100 seconds.
     *  \param time Time in seconds.
     */
    std::string timeToString(float time)
    {
        int min     = (int) floor ( time / 60.0 ) ;
        int sec     = (int) floor ( time - (double) ( 60 * min ) ) ;
        int tenths  = (int) floor ( 10.0f * (time - (double)(sec + 60* min)));
        char s[9];
        sprintf ( s, "%d:%02d:%d", min,  sec,  tenths ) ;
        return std::string(s);
    }   // timeToString

} // namespace StringUtils

/* EOF */
