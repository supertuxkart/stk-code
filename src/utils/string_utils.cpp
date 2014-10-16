//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2013  Steve Baker <sjbaker1@airmail.net>,
//  Copyright (C) 2004-2013  Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2013  SuperTuxKart-Team
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

#include "utils/log.hpp"
#include "utils/time.hpp"

#include "coreutil.h"

#include <math.h>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <exception>
#include <assert.h>

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
            return strcmp(lhs.c_str()+(lhs.length()-rhs.length()),
                          rhs.c_str()                             )==0;
    }   // hasSuffix

    bool startsWith(const std::string& str, const std::string& prefix)
    {
        if (str.length() < prefix.length())
            return false;
        else
            return strncmp(str.c_str(), prefix.c_str(), prefix.size())==0;
    }

    //-------------------------------------------------------------------------
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
    /** Checks if the input string is not empty. ( = has characters different
     *  from a space).
     */
    bool notEmpty(const irr::core::stringw& input)
    {
        const int size = input.size();
        int nonEmptyChars = 0;
        for (int n=0; n<size; n++)
        {
            if (input[n] != L' ')
            {
                nonEmptyChars++;
            }
        }
        return (nonEmptyChars > 0);
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

    //-------------------------------------------------------------------------
    /** Returns a string converted to lower case.
     */
    std::string toLowerCase(const std::string& str)
    {
        std::string name = str;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        return name;
    }   // toLowerCase

    //-------------------------------------------------------------------------
    /** Splits a string into substrings separated by a certain character, and
     *  returns a std::vector of all those substring. E.g.:
     *  split("a b=c d=e",' ')  --> ["a", "b=c", "d=e"]
     *  \param s The string to split.
     *  \param c The character  by which the string is split.
     */
    std::vector<std::string> split(const std::string& s, char c,
                                   bool keepSplitChar)
    {
        std::vector<std::string> result;

        try
        {
            std::string::size_type start=0;
            while(start!=std::string::npos && start<(unsigned int)s.size())
            {
                std::string::size_type i=s.find(c, start);
                if (i!=std::string::npos)
                {
                    if (keepSplitChar)
                    {
                        int from = (int)start-1;
                        if (from < 0) from = 0;

                        result.push_back(std::string(s, from, i-from));
                    }
                    else result.push_back(std::string(s,start, i-start));

                    start=i+1;
                }
                else   // end of string reached
                {
                    if (keepSplitChar)
                        result.push_back(std::string(s,start-1));
                    else
                        result.push_back(std::string(s,start));
                    start = i;
                }
            }
            return result;
        }
        catch (std::exception& e)
        {
            Log::error("StringUtils",
                       "Error in split(std::string) : %s @ line %i : %s.",
                     __FILE__, __LINE__, e.what());
            Log::error("StringUtils", "Splitting '%s'.", s.c_str());

            for (int n=0; n<(int)result.size(); n++)
            {
                Log::error("StringUtils", "Split : %s", result[n].c_str());
            }

            assert(false); // in debug mode, trigger debugger
            exit(1);
        }
    }   // split

    //-------------------------------------------------------------------------
    /** Splits a string into substrings separated by a certain character, and
     *  returns a std::vector of all those substring. E.g.:
     *  split("a b=c d=e",' ')  --> ["a", "b=c", "d=e"]
     *  This is the version for wide strings.
     *  \param s The string to split.
     *  \param c The character  by which the string is split.
     */
    std::vector<irr::core::stringw> split(const irr::core::stringw& s, char c,
                                           bool keepSplitChar)
    {
        try
        {
            std::vector<irr::core::stringw> result;

            irr::s32 start = 0;
            while (start < (irr::s32)s.size())
            {
                irr::s32 i = s.findNext(c, start);
                if (i != -1)
                {
                    if (keepSplitChar)
                    {
                        int from = start-1;
                        if (from < 0) from = 0;
                        result.push_back( s.subString(from, i-from) );
                    }
                    else result.push_back( s.subString(start, i-start) );
                    start = i+1;
                }
                else
                {
                    if (keepSplitChar)
                        result.push_back( s.subString(start - 1,
                                                      s.size()-start + 1) );
                    else
                        result.push_back( s.subString(start, s.size()-start) );

                    return result;
                    //start = i+1;
                }
            }
            return result;
        }
        catch (std::exception& e)
        {
            (void)e;  // avoid warning about unused variable
            Log::fatal("StringUtils",
                       "Fatal error in split(stringw) : %s @ line %i : '%s'.",
                       __FILE__, __LINE__, e.what());
            exit(1);
        }
    }   // split


    std::vector<uint32_t> splitToUInt(const std::string& s, char c, bool keepSplitChar)
    {
        std::vector<std::string> parts = split(s, c, keepSplitChar);
        std::vector<uint32_t> ints;
        for(unsigned int i = 0; i < parts.size(); ++i)
        {
           ints.push_back(atoi(parts[i].c_str()));
        }
        return ints;
    }


    // ------------------------------------------------------------------------
    /** Splits a : separated string (like PATH) into its individual components.
     *  It especially handles Windows-style paths (c:/mydir1:d:/mydir2)
     *  correctly, and removes a trailing "/" which can cause a problem with
     *  windows' stat function.
     *  \param path The string to split.
     */
    std::vector<std::string> splitPath(const std::string& path)
    {
        try
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
                if(i==(int)dirs.size()-1)    // last element
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
        }
        catch (std::exception& e)
        {
            (void)e;  // avoid warning about unused variable
            Log::fatal("StringUtils",
                "Fatal error in splitPath : %s @ line %i: '%s'.",
                        __FILE__, __LINE__, path.c_str());
            exit(1);
        }
    }   // splitPath

    // ------------------------------------------------------------------------
    std::string insertValues(const std::string &s,
                             std::vector<std::string>& all_vals)
    {
        try
        {
            std::vector<std::string> sv = StringUtils::split(s, '%', true);
            std::string new_string="";

            unsigned int insertValID = 0;

            const unsigned int item_count = (int)sv.size();
            for (unsigned int i=0; i<item_count; i++)
            {
                if(sv[i][0] != '%')
                {
                    new_string += sv[i];
                }
                else
                {
                    if(sv[i][1]=='s' || sv[i][1]=='d' || sv[i][1]=='i')
                    {
                        if (insertValID >= all_vals.size())
                        {
                            Log::warn("StringUtils",
                                      "insertValues: "
                                      "Invalid number of arguments in '%s'.",
                                      s.c_str());
                            new_string += "??" + sv[i].substr(2);
                        }
                        else
                        {
                            new_string += all_vals[insertValID]
                                        + sv[i].substr(2);
                        }
                        insertValID++;
                    }
                    else if(sv[i][1]>='0' && sv[i][1]<= '9')
                    {
                        const unsigned int index = sv[i][1] - '0';
                        if (index >= all_vals.size())
                        {
                            Log::warn("StringUtils", "insertValues: "
                                      " Invalid argument index in '%s' "
                                      "for %i.", s.c_str(), index);
                            new_string += "??" + sv[i].substr(2);
                        }
                        else
                        {
                            new_string += all_vals[index] + sv[i].substr(2);
                        }
                    }
                    else
                    {
                        new_string += sv[i];
                    }
                }
            }
            return new_string;
        }
        catch (std::exception& e)
        {
            (void)e;  // avoid warning about unused variable
            Log::fatal("StringUtils",
                       "Fatal error in insertValues(std::string) : %s @ "
                       "line %i: '%s'", __FILE__, __LINE__, s.c_str());
            exit(1);
        }
    }

    // ------------------------------------------------------------------------
    irr::core::stringw insertValues(const irr::core::stringw &s,
                                    std::vector<irr::core::stringw>& all_vals)
    {
        try
        {
            unsigned int insertValID = 0;

            const std::vector<irr::core::stringw> sv =
                                              StringUtils::split(s, '%', true);

            irr::core::stringw new_string="";

            const unsigned int size = (int)sv.size();
            for (unsigned int i=0; i<size; i++)
            {
                if(sv[i][0] != '%')
                {
                    new_string += sv[i];
                }
                else
                {
                    if (sv[i][1]=='s' || sv[i][1]=='d' || sv[i][1]=='i')
                    {
                        if (insertValID >= all_vals.size())
                        {
                            Log::warn("StringUtils", "insertValues: "
                                      "Invalid number of arguments in '%s'\n",
                                      irr::core::stringc(s.c_str()).c_str());
                            new_string += "??";
                            new_string += sv[i].subString(2, sv[i].size()-2);
                        }
                        else
                        {
                            new_string += all_vals[insertValID].c_str();
                            new_string += sv[i].subString(2, sv[i].size()-2);
                        }
                        insertValID++;
                    }
                    else if(irr::core::isdigit(sv[i][1]))
                    {
                        irr::core::stringw rest =
                                            sv[i].subString(2, sv[i].size()-2);
                        int delta = 0;

                        if (sv[i].size() >= 4 && sv[i][2]=='$')
                        {
                            rest = sv[i].subString(4, sv[i].size()-4);
                            delta = -1;
                        }

                        const unsigned int index =
                                 irr::core::stringc(sv[i].c_str()).c_str()[1]
                                 - '0' + delta;
                        if (index >= all_vals.size())
                        {
                            Log::warn("StringUtils", "insertValues: "
                                      "Invalid argument ID in '%s' : %i\n",
                                      irr::core::stringc(s.c_str()).c_str(),
                                      index);
                            new_string += "??";
                            new_string += rest;
                        }
                        else
                        {
                            new_string += all_vals[index] + rest;
                        }
                    }
                    else
                    {
                        new_string+=sv[i];
                    }
                }
            }
            return new_string;
        }
        catch (std::exception& e)
        {
            (void)e;  // avoid warning about unused variable
            Log::fatal("StringUtils",
                       "Fatal error in insertValues(stringw) : %s @ line %i.",
                       __FILE__, __LINE__);
            exit(1);
        }
    }


    // ------------------------------------------------------------------------
    /** Converts a time in seconds into a string of the form mm:ss:hh (minutes,
     *  seconds, 1/100 seconds.
     *  \param time Time in seconds.
     */
    std::string timeToString(float time)
    {
        int int_time   = (int)(time*100.0f+0.5f);

        // Avoid problems if time is negative or way too large (which
        // should only happen if something is broken in a track elsewhere,
        // and an incorrect finishing time is estimated.
        if(int_time<0)
            return std::string("00:00:00");
        else if(int_time >= 10000*60)  // up to 99:59.99
            return std::string("99:59:99");

        int min        = int_time / 6000;
        int sec        = (int_time-min*6000)/100;
        int hundredths = (int_time - min*6000-sec*100);
        // The proper c++ way would be:
        // std::ostringstream s;
        // s<<std::setw(2)<<std::setfill(' ')<<min<<":"
        //     <<std::setw(2)<<std::setfill('0')<<sec<<":"
        //     <<std::setw(2)<<std::setfill(' ')<<hundredths;
        // return s.str();
        // but that appears to be awfully complicated and slow, compared to
        // which admittedly only works for min < 100000 - which is about 68
        // days - good enough.
        char s[12];
        sprintf(s, "%02d:%02d:%02d", min,  sec,  hundredths);
        return std::string(s);
    }   // timeToString

    // ------------------------------------------------------------------------
    /** Shows a increasing number of dots.
      * \param interval A float representing the time it takes to add a new dot
      * \param max_dots The number of dots used. Defaults to 3.
      */
    irr::core::stringw loadingDots(float interval, int max_dots)
    {
        int nr_dots = int(floor(StkTime::getRealTime() / interval))
                    % (max_dots + 1);
        return irr::core::stringw((std::string(nr_dots, '.') +
                                   std::string(max_dots - nr_dots, ' ')).c_str());
    }   // loadingDots

    // ------------------------------------------------------------------------
    /** Returns the string given with loadingDots appended. A simple
     *  convenience function to type less in calls.
     *  \parameter s The string to which the loading dots are appended.
     */
    irr::core::stringw loadingDots(const wchar_t *s)
    {
        return irr::core::stringw(s) + loadingDots();
    }   // loadingDots

    // ------------------------------------------------------------------------
    /** Replaces values in a string.
     * \param other string in which to replace stuff
     * \param from  pattern to remove from the string
     * \param to    pattern to insert instead
     * \return      a string with all occurrences of \c from replaced by
     *              occurrences of \c to
     */
    std::string replace(const std::string& other, const std::string& from,
                        const std::string& to)
    {
        std::string wip = other;


        while (true)
        {
            const int pos = (int) wip.find(from);
            if (pos == -1)
            {
                return wip;
            }
            wip.replace(pos, from.size(), to.c_str(), to.size());
        }

    }

    // ------------------------------------------------------------------------
    /** Converts ASCII text with HTML entities (e.g. &xE9;) to unicode strings
     *  \param input The input string which should be decoded.
     *  \return A irrlicht wide string with unicode characters.
     */
    irr::core::stringw decodeFromHtmlEntities(const std::string& input)
    {
        irr::core::stringw output;
        std::string entity;
        bool isHex = false;

        enum
        {
            NORMAL,
            ENTITY_PREAMBLE,
            ENTITY_BODY
        } state = NORMAL;

        for (unsigned int n=0; n<input.size(); n++)
        {
            switch (state)
            {
                case NORMAL:
                    if (input[n] == '&')
                    {
                        state = ENTITY_PREAMBLE;
                        entity = "";
                        isHex = false;
                    }
                    else
                    {
                        output += wchar_t(input[n]);
                    }
                    break;

                case ENTITY_PREAMBLE:
                    if (input[n] != '#')
                    {
                        output += L"&";
                        output += wchar_t(input[n]);
                        // This is actually an error, but we can't print a
                        // warning here: irrxml replaces &amp; in (e.g.)
                        // attribute values with '&' - so we can have a single
                        // '&' that is correct. On the other hand we have to
                        // convert esp. &# hex codes, since we use this to
                        // encode special characters, but irrlicht does not
                        // decode those.
                        state = NORMAL;
                    }
                    else
                    {
                        state = ENTITY_BODY;
                    }
                    break;

                case ENTITY_BODY:
                    if (input[n] == 'x' && entity.size() == 0)
                    {
                        isHex = true;
                    }
                    else if (input[n] == ';')
                    {
                        int c;

                        const char* format = (isHex ? "%x" : "%i");
                        if (sscanf(entity.c_str(), format, &c) == 1)
                        {
                            output += wchar_t(c);
                        }
                        else
                        {
                            Log::warn("StringUtils", "non-numeric HTML "
                                      "entity not supported in '%s'.",
                                      input.c_str());
                        }
                        state = NORMAL;
                    }
                    else
                    {
                        entity += wchar_t(input[n]);
                    }
                    break;
            }
        }

        return output;
    }   // decodeFromHtmlEntities

    // ------------------------------------------------------------------------
    /** Converts a unicode string to plain ASCII using html-like & codes.
     *  \param s The input string which should be encoded.
     *  \return A std:;string with ASCII characters.
     */
    std::string encodeToHtmlEntities(const irr::core::stringw &s)
    {
        std::ostringstream output;
        for(unsigned int i=0; i<s.size(); i++)
        {
            if(s[i]=='&')
                output<<"&amp;";
            else
            {
                if(s[i]<128)
                {
                    irr::c8 c=(char)(s[i]);
                    output<<c;
                }
                else
                {
                    output <<"&#x" << std::hex <<std::uppercase<< s[i]<<";";
                }
            }
        }
        return output.str();
    }   // encodeToHtmlEntities

    // ------------------------------------------------------------------------
    /** Converts a version string (in the form of 'X.Y.Za-rcU' into an
     *  integer number.
     *  \param s The version string to convert.
     */
    int versionToInt(const std::string &version_string)
    {
        // Special case: SVN
        if(version_string=="SVN" || version_string=="svn")
        {
            // SVN version will be version 99.99.99i-rcJ
            return 1000000*99
                    +  10000*99
                    +    100*99
                    +     10* 9
                    +         9;
        }

        std::string s=version_string;
        // To guarantee that a release gets a higher version number than
        // a release candidate, we assign a 'release_candidate' number
        // of 9 to versions which are not a RC. We assert that any RC
        // is less than 9 to guarantee the ordering.
        int release_candidate=9;
        if(s.length()>4 && sscanf(s.substr(s.length()-4, 4).c_str(), "-rc%d",
                &release_candidate)==1)
        {
            s = s.substr(0, s.length()-4);
            // Otherwise a RC can get a higher version number than
            // the corresponding release! If this should ever get
            // triggered, multiply all scaling factors above and
            // below by 10, to get two digits for RC numbers.
            assert(release_candidate<9);
        }
        int very_minor=0;
        if(s.length()>0 && s[s.size()-1]>='a' && s[s.size()-1]<='z')
        {
            very_minor = s[s.size()-1]-'a'+1;
            s = s.substr(0, s.size()-1);
        }
        std::vector<std::string> l = StringUtils::split(s, '.');
        while(l.size()<3)
            l.push_back("0");
        int version = 1000000*atoi(l[0].c_str())
                    +   10000*atoi(l[1].c_str())
                    +     100*atoi(l[2].c_str())
                    +      10*very_minor
                    +         release_candidate;

        if(version <= 0)
            Log::error("StringUtils", "Invalid version string '%s'.", s.c_str());
        return version;
    }   // versionToInt

} // namespace StringUtils


/* EOF */
