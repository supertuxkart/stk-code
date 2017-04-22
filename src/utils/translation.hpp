//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef TRANSLATION_HPP
#define TRANSLATION_HPP

#include <irrString.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "utils/string_utils.hpp"

#include "tinygettext/tinygettext.hpp"

#  define _(String, ...)        (translations->fribidize(StringUtils::insertValues(translations->w_gettext(String), ##__VA_ARGS__)))
#undef _C
#undef _P
#  define _C(Ctx, String, ...)  (translations->fribidize(StringUtils::insertValues(translations->w_gettext(String, Ctx), ##__VA_ARGS__)))
#  define _P(Singular, Plural, Num, ...) (translations->fribidize(StringUtils::insertValues(translations->w_ngettext(Singular, Plural, Num), Num, ##__VA_ARGS__)))
#  define _CP(Ctx, Singular, Plural, Num, ...) (translations->fribidize(StringUtils::insertValues(translations->w_ngettext(Singular, Plural, Num, Ctx), Num, ##__VA_ARGS__)))
#  define _LTR(String, ...)     (StringUtils::insertValues(translations->w_gettext(String), ##__VA_ARGS__))
#  define gettext_noop(String)  (String)
#  define N_(String)            (gettext_noop (String))
// libintl defines its own fprintf, which doesn't work properly
#  if defined(WIN32) && !defined(__CYGWIN__)
#    undef fprintf
#  endif

class Translations
{
private:
    tinygettext::DictionaryManager m_dictionary_manager;
    tinygettext::Dictionary        m_dictionary;

    /** A map that saves all fribidized strings: Original string, fribidized string */
    std::map<const irr::core::stringw, const irr::core::stringw> m_fribidized_strings;
    bool m_rtl;

    std::map<std::string, std::string> m_localized_name;

    std::string m_current_language_name;
    std::string m_current_language_name_code;

public:
                       Translations();
                      ~Translations();

    const wchar_t     *w_gettext(const wchar_t* original, const char* context=NULL);
    const wchar_t     *w_gettext(const char* original, const char* context=NULL);

    const wchar_t     *w_ngettext(const wchar_t* singular, const wchar_t* plural, int num, const char* context=NULL);
    const wchar_t     *w_ngettext(const char* singular, const char* plural, int num, const char* context=NULL);

    bool               isRTLLanguage() const;
    const wchar_t*     fribidize(const wchar_t* in_ptr);
    const wchar_t*     fribidize(const irr::core::stringw &str) { return fribidize(str.c_str()); }

    bool               isRTLText(const wchar_t* in_ptr);
    bool               isRTLText(const irr::core::stringw &str) { return isRTLText(str.c_str()); }

    const std::vector<std::string>* getLanguageList() const;

    std::set<wchar_t>        getCurrentAllChar();

    std::string              getCurrentLanguageName();

    std::string              getCurrentLanguageNameCode();

    const std::string&       getLocalizedName(const std::string& str) const;

private:
    irr::core::stringw fribidizeLine(const irr::core::stringw &str);
};   // Translations


extern Translations* translations;
#endif
/* EOF */
