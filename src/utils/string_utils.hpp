//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015  Steve Baker <sjbaker1@airmail.net>,
//  Copyright (C) 2004-2015  Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2015  SuperTuxKart-Team
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

#ifndef HEADER_STRING_UTILS_HPP
#define HEADER_STRING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <irrString.h>
#include "utils/types.hpp"
#include "utils/log.hpp"

namespace StringUtils
{
    int           versionToInt(const std::string &s);

    bool hasSuffix(const std::string& lhs, const std::string &rhs);
    bool startsWith(const std::string& str, const std::string& prefix);

    /** Return the filename part of a path */
    std::string getBasename(const std::string& filename);

    /** Return the path ( i.e. up to the last / )  */
    std::string getPath(const std::string& filename);

    std::string removeExtension(const std::string& filename);
    std::string getExtension(const std::string& filename);

    bool notEmpty(const irr::core::stringw& input);
    std::string timeToString(float time);
    irr::core::stringw loadingDots(float interval = 0.5f, int max_dots = 3);
    irr::core::stringw loadingDots(const wchar_t *s);
    std::string                     toUpperCase(const std::string&);
    std::string                     toLowerCase(const std::string&);
    std::vector<std::string>        split(const std::string& s, char c,
                                          bool keepSplitChar=false);
    std::vector<irr::core::stringw> split(const irr::core::stringw& s,
                                          char c, bool keepSplitChar=false);
    std::vector<uint32_t>           splitToUInt(const std::string& s, char c,
                                                bool keepSplitChar=false);
    std::vector<std::string>        splitPath(const std::string& path);
    std::string replace(const std::string& other, const std::string& from, const std::string& to);

    irr::core::stringw xmlDecode(const std::string& input);

    std::string xmlEncode(const irr::core::stringw &output);

    // ------------------------------------------------------------------------
    template <class T>
    std::string toString (const T& any)
    {
        std::ostringstream oss;
        oss << any ;
        return oss.str();
    }   // toString template

    // ------------------------------------------------------------------------
    /** Specialisiation for bools to return 'true' or 'false'*/
    inline std::string toString(const bool& b)
    {
        return (b ? "true" : "false");
    }    // toString(bool)

    // ------------------------------------------------------------------------
    template <class T>
    irr::core::stringw toWString (const T& any)
    {
        std::ostringstream oss;
        oss << any ;
        return oss.str().c_str();
    }   // toWString

    // ------------------------------------------------------------------------
    /** Convert the contents in string \a rep to type \a T, if conversion
        fails false is returned and the value of \a x is unchanged, if
        true is returned the conversation was successfull. */
    template <class T>
    bool fromString(const std::string& rep, T& x)
    {
        // Don't modify x" if the conversion fails by using a temp
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
    }   // fromString

    // ------------------------------------------------------------------------
    /**
     *  Replaces the first %s or %i/%d in the string with the first value
     *  converted to a string), the 2nd %s or %d with the second value etc.
     *  So this is basically a simplified s(n)printf replacement, but doesn't
     *  do any fancy formatting (and no type checks either - so you can print
     *  a string into a %d field). This is basically a replacement for
     *  sprintf (and similar functions), mostly meant for strings that are
     *  translated (otherwise just use ostringstream) - since e.g. a
     *  translated string like _("Player %s - chose your kart") would
     *  be broken into two strings:
     *  << _("Player ") << name << _(" - chose your kart")
     *  and this is in the best case very confusing for translators (which get
     *  to see two strings instead of one sentence, see xgettext manual
     *  for why this is a bad idea)
     *  In order to accomodate translations even more, you can use formats
     *  %0, %1, %2, etc..., where %0 is replaced by the first argument, %1 by
     *  the second argument, etc... This allows translated strings to not
     *  necessarily insert the words in the same order as in English.
     *  \param s String in which all %s or %d are replaced.
     *  \param all_vals Value(s) to replace all %s or %d with.
     */
    std::string insertValues(const std::string &s,
                             std::vector<std::string>& all_vals);

    // ------------------------------------------------------------------------
    /** Same as above but for wide-strings */
    irr::core::stringw insertValues(const irr::core::stringw &s,
                                    std::vector<irr::core::stringw>& all_vals);

    // ------------------------------------------------------------------------
    /** Intermediate struct to fill a vector using variadic templates */
    struct FillStringVector
    {
        /** FillS takes a vector as the first argument and a variadic list of
         * arguments. The arguments are recursively inserted into the vector
         * which will contain all the arguments converted to strings in the end.
         */
        template<typename T, typename...Args>
        static void FillS(std::vector<std::string> &all_vals, T&& v, Args &&...args)
        {
            std::ostringstream oss;
            oss << v;
            all_vals.push_back(oss.str());
            FillS(all_vals, std::forward<Args>(args)...);
        }

        static void FillS(std::vector<std::string>&) {}

        /** This functions does the same as FillS but for wide strings. */
        template<typename T, typename...Args>
        static void FillW(std::vector<irr::core::stringw> &all_vals, T&& v, Args &&...args)
        {
            all_vals.push_back(irr::core::stringw(std::forward<T>(v)));
            FillW(all_vals, std::forward<Args>(args)...);
        }

        static void FillW(std::vector<irr::core::stringw>&) {}
    };

    template <typename...Args>
    std::string insertValues(const std::string &s, Args ...args)
    {
        std::vector<std::string> all_vals;
        all_vals.reserve(sizeof...(args));
        FillStringVector::FillS(all_vals, std::forward<Args>(args)...);
        return insertValues(s, all_vals);
    }

    template <typename...Args>
    std::string insertValues(const char *s, Args ...args)
    {
        return insertValues(std::string(s), std::forward<Args>(args)...);
    }

    /** Like the other ones above but for wide strings */
    template <typename...Args>
    irr::core::stringw insertValues(const irr::core::stringw &s, Args ...args)
    {
        std::vector<irr::core::stringw> all_vals;
        all_vals.reserve(sizeof...(args));
        FillStringVector::FillW(all_vals, std::forward<Args>(args)...);
        return insertValues(s, all_vals);
    }

    template <typename...Args>
    irr::core::stringw insertValues(const wchar_t *s, Args ...args)
    {
        return insertValues(irr::core::stringw(s), std::forward<Args>(args)...);
    }

    // ------------------------------------------------------------------------
    template<typename T>
    bool parseString(const char* input, T* output)
    {
        std::istringstream conv(input);
        conv >> *output;

        // check reading worked correctly and everything was read
        if (conv.fail() || !conv.eof())
        {
            return false;
        }
        return true;
    }   // parseString

    // ------------------------------------------------------------------------
    template<typename T>
    bool parseString(const std::string& input, T* output)
    {
        return parseString(input.c_str(), output);
    }   // parseString

    // ------------------------------------------------------------------------
    
    irr::core::stringw utf8ToWide(const char* input);
    irr::core::stringw utf8ToWide(const std::string &input);
    std::string wideToUtf8(const wchar_t* input);
    std::string wideToUtf8(const irr::core::stringw& input);

} // namespace StringUtils

#endif
