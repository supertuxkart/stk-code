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
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef SERVER_ONLY
#include "tinygettext/tinygettext.hpp"
#endif

#ifdef STK_UTF8_GETTEXT
#define STK_GETTEXT gettext
#define STK_NGETTEXT ngettext
#else
#define STK_GETTEXT w_gettext
#define STK_NGETTEXT w_ngettext
#endif

#  define _(String, ...)        (StringUtils::insertValues(translations->STK_GETTEXT(String), ##__VA_ARGS__))
#undef _C
#undef _P
#  define _C(Ctx, String, ...)  (StringUtils::insertValues(translations->STK_GETTEXT(String, Ctx), ##__VA_ARGS__))
#  define _P(Singular, Plural, Num, ...) (StringUtils::insertValues(translations->STK_NGETTEXT(Singular, Plural, Num), Num, ##__VA_ARGS__))
#  define _CP(Ctx, Singular, Plural, Num, ...) (StringUtils::insertValues(translations->STK_NGETTEXT(Singular, Plural, Num, Ctx), Num, ##__VA_ARGS__))
#  define gettext_noop(String)  (String)
#  define N_(String)            (gettext_noop (String))
// libintl defines its own fprintf, which doesn't work properly
#  if defined(WIN32) && !defined(__CYGWIN__)
#    undef fprintf
#  endif

class Translations
{
private:
#ifndef SERVER_ONLY
    tinygettext::DictionaryManager m_dictionary_manager;
    tinygettext::Dictionary*       m_dictionary;

    static std::map<std::string, std::string> m_localized_name;
    static std::map<std::string, std::map<std::string, irr::core::stringw> > m_localized_country_codes;
    std::string m_current_language_name;
    std::string m_current_language_name_code;
    std::string m_current_language_tag;
#endif

public:
                       Translations();
                      ~Translations();

    irr::core::stringw w_gettext(const wchar_t* original, const char* context=NULL);
    irr::core::stringw w_gettext(const char* original, const char* context=NULL);
    std::string gettext(const char* original, const char* context=NULL);

    irr::core::stringw w_ngettext(const wchar_t* singular, const wchar_t* plural, int num, const char* context=NULL);
    irr::core::stringw w_ngettext(const char* singular, const char* plural, int num, const char* context=NULL);
    std::string ngettext(const char* singular, const char* plural, int num, const char* context=NULL);

#ifndef SERVER_ONLY
    const std::vector<std::string>* getLanguageList() const;

    std::set<unsigned int>   getCurrentAllChar();

    std::string              getCurrentLanguageName();

    std::string              getCurrentLanguageNameCode();

    const std::string        getLocalizedName(const std::string& str) const;

    irr::core::stringw       getLocalizedCountryName(const std::string& country_code) const;

    void                     insertThaiBreakMark(const std::u32string& thai, std::vector<bool>& breakable);
#endif
};   // Translations


extern Translations* translations;
#endif
/* EOF */
