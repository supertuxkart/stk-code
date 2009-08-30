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
    bool hasSuffix(const std::string& lhs, const std::string rhs)
    {
        if (lhs.length() < rhs.length())
            return false;
        else
            // While this is basically correct, it fails with older
            // g++ versions (at least 2.95.3), which have a wrong template. To
            // avoid this issue, a more C-traditional way is used.
            return strcmp(lhs.c_str()+(lhs.length()-rhs.length()), rhs.c_str())==0;
    }   // hasSuffix

    //--------------------------i-----------------------------------------------
    /** Returns the path of a filename, i.e. everything till the last '/'.
     */
    std::string getPath(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i]=='/' || filename[i]=='\\')
            {
                return filename.substr(0,i);
            }
        }
        return "";
    }   // getPath

    //-------------------------------------------------------------------------
    /** Returns the basename of a filename, i.e. everything after the last "/".
     */
    std::string getBasename(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i]=='/' || filename[i]=='\\')
            {
                return filename.substr(i+1);
            }
        }
        return filename;
    }   // getBasename

    //-------------------------------------------------------------------------
    /** Removes the extension, i.e. everything after the last ".".
     */
    std::string removeExtension(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i] == '.')
            {
                return filename.substr(0, i);
            }
        }
        return filename;
    }   // removeExtension

    //-------------------------------------------------------------------------
    /** Returns the extension, i.e. everything after the last "."
     */
    std::string getExtension(const std::string& filename)
    {
        for(int i = int(filename.size()) - 1; i >= 0; --i)
        {
            if (filename[i] == '.')
            {
                return filename.substr(i+1);
            }
        }
        return filename;
    }   // getExtension

    //-------------------------------------------------------------------------
    /** Returns a string converted to upper case.
     */
    std::string toUpperCase(const std::string& str)
    {
        std::string name = str;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        return name;
    }   // toUpperCase

    //-----------------------------------------------------------------------------
    /** Splits a string into substrings separated by a certain character, and 
     *  returns a std::vector of all those substring. E.g.:
     *  split("a b=c d=e",' ')  --> ["a", "b=c", "d=e"]
     *  \param s The string to split.
     *  \param c The character  by which the string is split.
     */
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

    /** Same as above, but for wide strings */
    std::vector<irr::core::stringw> split(const irr::core::stringw& s, char c)
    {
        std::vector<irr::core::stringw> result;
        
        irr::s32 start = 0;
        while (start < (irr::s32)s.size())
        {
            irr::s32 i = s.findNext(c, start);
            if (i != -1)
            {
                result.push_back( s.subString(start, i-start) );
                start = i+1;
            } 
            else
            {
                result.push_back( s.subString(start, s.size()-start) );
                return result;
                //start = i+1;
            }
        }
        return result;
    }   // split
    
    
    // ------------------------------------------------------------------------
    /** Splits a : separated string (like PATH) into its individual components.
     *  It especially handles Windows-style paths (c:/mydir1:d:/mydir2)
     *  correctly, and removes a trailing "/" which can cause a problem with
     *  windows' stat function.
     *  \param path The string to split.
     */
    std::vector<std::string> splitPath(const std::string& path)
    {
        std::vector<std::string> dirs=StringUtils::split(path,':');
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            // Remove '/' at the end of paths, since this can cause
            // problems with windows when using stat()
            while(dirs[i].size()>=1 && dirs[i][dirs[i].size()-1]=='/')
            {
                dirs[i]=dirs[i].substr(0, dirs[i].size()-1);
            }
            // remove empty entries
            if(dirs[i].size()==0)
            {
                dirs.erase(dirs.begin()+i);
                continue;
            }
        }   // for i
#ifdef WIN32
        // Handle filenames like d:/dir, which becomes ["d","/dir"]
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            if(dirs[i].size()>1) continue;
            if(i==dirs.size()-1)    // last element
            {
                dirs[i]+=":";      // turn "c" back into "c:"
            }
            else
            {
                dirs[i]+=":"+dirs[i+1]; // restore "d:/dir" back
                dirs.erase(dirs.begin()+i+1);
            }
        }   // for i
#endif
        return dirs;
    }   // splitPath

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
