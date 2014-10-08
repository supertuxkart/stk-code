//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006,-2013 2007, 2008 Joerg Henrichs
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


// Note: the irrlicht include is only here (and esp. before including
//       translation.hpp, which contradicts our style rule) to avoid the
//        warning message  "  'swprintf' : macro redefinition"
//       This happens if libintl.h is included before irrlicht.h (since
//       both files redefine swprintf).

#include "utils/translation.hpp"

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#if ENABLE_BIDI
#  include <fribidi/fribidi.h>
#endif

#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/utf8.h"


// set to 1 to debug i18n
#define TRANSLATE_VERBOSE 0

using namespace tinygettext;

Translations* translations = NULL;
const bool REMOVE_BOM = false;

#ifdef LINUX // m_debug
#define PACKAGE "supertuxkart"
#endif

/** The list of available languages; this is global so that it is cached (and remains
    even if the translations object is deleted and re-created) */
typedef std::vector<std::string> LanguageList;
static LanguageList g_language_list;

// Note : this method is not static because 'g_language_list' is initialized
//        the first time Translations is constructed (despite being a global)
const LanguageList* Translations::getLanguageList() const
{
    return &g_language_list;
}


char* wide_to_utf8(const wchar_t* input)
{
    static std::vector<char> utf8line;
    utf8line.clear();

    utf8::utf16to8(input, input + wcslen(input), back_inserter(utf8line));
    utf8line.push_back(0);

    return &utf8line[0];
}

wchar_t* utf8_to_wide(const char* input)
{
    static std::vector<wchar_t> utf16line;
    utf16line.clear();

    utf8::utf8to16(input, input + strlen(input), back_inserter(utf16line));
    utf16line.push_back(0);

    return &utf16line[0];
}

// ----------------------------------------------------------------------------
Translations::Translations() //: m_dictionary_manager("UTF-16")
{

    if (g_language_list.size() == 0)
    {
        std::set<std::string> flist;
        file_manager->listFiles(flist,
                                file_manager->getAsset(FileManager::TRANSLATION,""));

        // English is always there but won't be found on file system
        g_language_list.push_back("en");

        std::set<std::string>::iterator it;
        for ( it=flist.begin() ; it != flist.end(); it++ )
        {
            if (StringUtils::hasSuffix(*it, "po"))
            {
                g_language_list.push_back
                    (m_dictionary_manager.convertFilename2Language(*it) );
                // printf("Lang : <%s>\n", (*it).c_str());
            }
        }   // for it in flist
    }   // if (g_language_list.size() == 0)

    // LC_ALL does not work, sscanf will then not always be able
    // to scan for example: s=-1.1,-2.3,-3.3 correctly, which is
    // used in driveline files.
#if defined(WIN32) && !defined(__CYGWIN__)
    // Windows does not have LC_MESSAGES
    setlocale(LC_CTYPE,    "");
#else
    setlocale(LC_MESSAGES, "");
#endif


    /*
    bindtextdomain (PACKAGE, file_manager->getTranslationDir().c_str());

    if (sizeof(wchar_t) == 4)
    {
        if (IS_LITTLE_ENDIAN) bind_textdomain_codeset(PACKAGE, "UTF-32LE");
        else                  bind_textdomain_codeset(PACKAGE, "UTF-32BE");
    }
    else if (sizeof(wchar_t) == 2)
    {
        bind_textdomain_codeset(PACKAGE, "UTF-16LE");
    }
    else
    {
        fprintf(stderr, "Your wchar_t is neither 2 byte-long nor 4. What now??\n");
        exit(1);
    }

    textdomain (PACKAGE);
    */

    m_dictionary_manager.add_directory(
                        file_manager->getAsset(FileManager::TRANSLATION,""));

    /*
    const std::set<Language>& languages = m_dictionary_manager.get_languages();
    Log::info("Translatings", "Number of languages: %d", languages.size());
    for (std::set<Language>::const_iterator i = languages.begin();
                                            i != languages.end(); ++i)
    {
        const Language& language = *i;
        Log::info("Translatings", "Env:       %s", language.str());
        Log::info("Translatings", "Name:      %s", language.get_name());
        Log::info("Translatings", "Language:  %s", language.get_language());
        Log::info("Translatings", "Country:   %s", language.get_country());
        Log::info("Translatings", "Modifier:  %s", language.get_modifier());
    }
    */

    const char *p_language = getenv("LANGUAGE");

    std::string language;

    if(p_language)
    {
        language=p_language;
    }
    else
    {
        const char *p_lang = getenv("LANG");

        if(p_lang)
            language = p_lang;
        else
        {
#ifdef WIN32
            // Thanks to the frogatto developer for this code snippet:
            char c[1024];
            GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME,
                           c, 1024);
            Log::verbose("translation", "GetLocaleInfo langname returns '%s'.",
                         c);
            if(c[0])
            {
                language = c;
                GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME,
                               c, 1024);
                Log::verbose("translation",
                             "GetLocaleInfo tryname returns '%s'.", c);
                if(c[0]) language += std::string("_")+c;
            }   // if c[0]
#endif
        }   // neither LANGUAGE nor LANG defined

    }

    if (language != "")
    {
        Log::verbose("translation", "Env var LANGUAGE = '%s'.",
                     language.c_str());

        if (language.find(":") != std::string::npos)
        {
            std::vector<std::string> langs = StringUtils::split(language, ':');
            Language l;

            for (unsigned int curr=0; curr<langs.size(); curr++)
            {
                l = Language::from_env(langs[curr]);
                if (l)
                {
                    Log::verbose("translation", "Language '%s'.",
                                 l.get_name().c_str());
                    m_dictionary = m_dictionary_manager.get_dictionary(l);
                    break;
                }
            }

            m_current_language_name = l.get_name();

            if (!l)
            {
                m_dictionary = m_dictionary_manager.get_dictionary();
            }
        }
        else
        {
            Log::verbose("translation", "Language '%s'.",
                          Language::from_env(language).get_name().c_str());

            m_current_language_name = Language::from_env(language).get_name() ;

            m_dictionary = m_dictionary_manager.get_dictionary(
                                                Language::from_env(language) );
        }
    }
    else
    {
        m_current_language_name = "Default language";
        m_dictionary = m_dictionary_manager.get_dictionary();
    }

    // This is a silly but working hack I added to determine whether the
    // current language is RTL or not, since gettext doesn't seem to provide
    // this information

    // This one is just for the xgettext parser to pick up
#define ignore(X)

    //I18N: Do NOT literally translate this string!! Please enter Y as the
    //      translation if your language is a RTL (right-to-left) language,
    //      N (or nothing) otherwise
    ignore(_("   Is this a RTL language?"));

    const std::string isRtl =
        m_dictionary.translate("   Is this a RTL language?");

    m_rtl = false;

    for (unsigned int n=0; n < isRtl.size(); n++)
    {
        if (isRtl[n] == 'Y')
        {
            m_rtl = true;
            break;
        }
    }
}   // Translations

// ----------------------------------------------------------------------------

const wchar_t* Translations::fribidize(const wchar_t* in_ptr)
{
#if ENABLE_BIDI
    if(this->isRTLLanguage())
    {
        const int FRIBIDI_BUFFER_SIZE = 512;
        FriBidiChar fribidiInput[FRIBIDI_BUFFER_SIZE];
        int len = 0;
        int n = 0;
        //std::cout << "fribidi input : ";
        for (n = 0; ; n++)
        {
            fribidiInput[n] = in_ptr[n];
            //std::cout << (int)fribidiInput[n] << " ";
            len++;

            if (n == FRIBIDI_BUFFER_SIZE-1) // prevent buffeoverflows
            {
                Log::warn("Translations::fribidize", "translated string too long, truncating");
                fribidiInput[n] = 0;
                break;
            }
            if (fribidiInput[n] == 0) break; // stop on '\0'
        }
        //std::cout << " (len=" << len << ")\n";

        // Assume right to left as start direction.
#if FRIBIDI_MINOR_VERSION==10
        // While the doc for older fribidi versions is somewhat sparse,
        // using the RIGHT-TO-LEFT EMBEDDING character here appears to
        // work correct.
        FriBidiCharType pbase_dir = L'\u202B';
#else
        FriBidiCharType pbase_dir = FRIBIDI_PAR_ON;
#endif

        static FriBidiChar fribidiOutput[FRIBIDI_BUFFER_SIZE];
        for (n = 0; n < 512 ; n++)  { fribidiOutput[n] = 0; }
        fribidi_boolean result = fribidi_log2vis(fribidiInput,
                                                 len-1,
                                                 &pbase_dir,
                                                 fribidiOutput,
              /* gint   *position_L_to_V_list */ NULL,
              /* gint   *position_V_to_L_list */ NULL,
              /* gint8  *embedding_level_list */ NULL
                                                               );

        if (!result)
        {
            Log::error("Translations::fribidize", "Fribidi failed in 'fribidi_log2vis' =(");
            m_converted_string = core::stringw(in_ptr);
            return m_converted_string.c_str();
        }

#ifdef WIN32
        // On windows FriBidiChar is 4 bytes, but wchar_t is 2 bytes.
        // So we simply copy the characters over here (note that this
        // is technically incorrect, all characters we use/support fit
        // in 16 bits, which is what irrlicht supports atm).
        static wchar_t out[FRIBIDI_BUFFER_SIZE];
        for(int i=0; i<len; i++)
            out[i]=fribidiOutput[i];
        out[len]=0;
        return out;
#else
        return (const wchar_t*)fribidiOutput;
#endif //WIND32
    }

#endif // ENABLE_BIDI
    return in_ptr;
}

/**
  * \param original Message to translate
  * \param context  Optional, can be set to differentiate 2 strings that are identical
  *                 in English but could be different in other languages
  */
const wchar_t* Translations::w_gettext(const wchar_t* original, const char* context)
{
    return w_gettext( wide_to_utf8(original), context );
}

/**
 * \param original Message to translate
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
const wchar_t* Translations::w_gettext(const char* original, const char* context)
{
    if (original[0] == '\0') return L"";

#if TRANSLATE_VERBOSE
    Log::info("Translations", "Translating %s", original);
#endif

    const std::string& original_t = (context == NULL ?
                                     m_dictionary.translate(original) :
                                     m_dictionary.translate_ctxt(context, original));

    if (original_t == original)
    {
        m_converted_string = utf8_to_wide(original);

#if TRANSLATE_VERBOSE
        std::wcout << L"  translation : " << m_converted_string << std::endl;
#endif
        return m_converted_string.c_str();
    }

    // print
    //for (int n=0;; n+=4)

    wchar_t* original_tw = utf8_to_wide(original_t.c_str());

    wchar_t* out_ptr = original_tw;
    if (REMOVE_BOM) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif

    return out_ptr;
}



bool Translations::isRTLLanguage() const
{
    return m_rtl;
}

std::string Translations::getCurrentLanguageName()
{
    return m_current_language_name;
    //return m_dictionary_manager.get_language().get_name();
}

