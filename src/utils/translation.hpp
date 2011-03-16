//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "irrlicht.h"
#include <vector>
#include <string>
#include "utils/string_utils.hpp"

#if ENABLE_NLS
#  include "tinygettext/tinygettext.hpp"

#  define _(String, ...)    (translations->fribidize(StringUtils::insertValues(translations->w_gettext(String), ##__VA_ARGS__)))
#  define _LTR(String, ...) (StringUtils::insertValues(translations->w_gettext(String), ##__VA_ARGS__))
#  define gettext_noop(String)  (String)
#  define N_(String)            (gettext_noop (String))
// libintl defines its own fprintf, which doesn't work properly
#  if defined(WIN32) && !defined(__CYGWIN__)
#    undef fprintf
#  endif
#else   // No NLS
#  define _(String)            (translations->w_gettext(String))
#  define _LTR(String)         (translations->w_gettext(String))
#  define gettext_noop(String) (String)
#  define N_(String)           (String)
#endif

class Translations
{
private:
    tinygettext::DictionaryManager m_dictionary_manager;
    tinygettext::Dictionary        m_dictionary;
    
    irr::core::stringw m_converted_string;
    bool m_rtl;
    
public:
                       Translations();
    const wchar_t     *w_gettext(const char* original);
    bool               isRTLLanguage() const;
    const wchar_t*     fribidize(const wchar_t* in_ptr);
    const wchar_t*     fribidize(const irr::core::stringw &str) { return fribidize(str.c_str()); }

    const std::vector<std::string>* getLanguageList() const;
};   // Translations


extern Translations* translations;
#endif
/* EOF */
