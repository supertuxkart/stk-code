//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006,-2015 2007, 2008 Joerg Henrichs
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

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/utf8.h"


// set to 1 to debug i18n
#define TRANSLATE_VERBOSE 0
// Define TEST_BIDI to force right-to-left style for all languages
//#define TEST_BIDI

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
/** Frees the memory allocated for the result of toFribidiChar(). */
void freeFribidiChar(FriBidiChar *str)
{
#ifdef TEST_BIDI
    delete[] str;
#else
    if (sizeof(wchar_t) != sizeof(FriBidiChar))
        delete[] str;
#endif
}

/** Frees the memory allocated for the result of fromFribidiChar(). */
void freeFribidiChar(wchar_t *str)
{
    if (sizeof(wchar_t) != sizeof(FriBidiChar))
        delete[] str;
}

// ----------------------------------------------------------------------------
/** Converts a wstring to a FriBidi-string.
    The caller must take care to free (or not to free) the result after use.
    Freeing should be done with freeFribidiChar().

    On linux, the string doesn't need to be converted because wchar_t is
    already UTF-32. On windows the string is converted from UTF-16 by this
    function. */
FriBidiChar* toFribidiChar(const wchar_t* str)
{
    std::size_t length = wcslen(str);
    FriBidiChar *result;
    if (sizeof(wchar_t) == sizeof(FriBidiChar))
        result = (FriBidiChar*) str;
    else
    {
        // On windows FriBidiChar is 4 bytes, but wchar_t is 2 bytes.
        // So we simply copy the characters over here (note that this
        // is technically incorrect, all characters we use/support fit
        // in 16 bits, which is what irrlicht supports atm).
        result = new FriBidiChar[length + 1];
        for (std::size_t i = 0; i <= length; i++)
            result[i] = str[i];
    }

#ifdef TEST_BIDI
    // Prepend a character that forces RTL style
    FriBidiChar *tmp = result;
    result = new FriBidiChar[++length + 1];
    std::memcpy(result + 1, tmp, length * sizeof(FriBidiChar));
    result[0] = L'\u202E';
    freeFribidiChar(tmp);
#endif

    return result;
}

wchar_t* fromFribidiChar(const FriBidiChar* str)
{
    wchar_t *result;
    if (sizeof(wchar_t) == sizeof(FriBidiChar))
        result = (wchar_t*) str;
    else
    {
        std::size_t length = 0;
        while (str[length])
            length++;

        // Copy back to wchar_t array
        result = new wchar_t[length + 1];
        for (std::size_t i = 0; i <= length; i++)
            result[i] = str[i];
    }
    return result;
}

// ----------------------------------------------------------------------------
Translations::Translations() //: m_dictionary_manager("UTF-16")
{
    m_dictionary_manager.add_directory(
                        file_manager->getAsset(FileManager::TRANSLATION,""));
                        
    if (g_language_list.size() == 0)
    {
        std::set<Language> languages = m_dictionary_manager.get_languages();      
        
        // English is always there but won't be found on file system
        g_language_list.push_back("en");
    
        std::set<Language>::iterator it;
        for (it = languages.begin(); it != languages.end(); it++)
        {
            g_language_list.push_back((*it).str());
        }
    }

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
            const Language& tgtLang = Language::from_env(language);
            if (!tgtLang)
            {
                Log::warn("Translation", "Unsupported langage '%s'", language.c_str());
                UserConfigParams::m_language = "system";
                m_current_language_name = "Default language";
                m_dictionary = m_dictionary_manager.get_dictionary();
            }
            else
            {
                m_current_language_name = tgtLang.get_name();
                Log::verbose("translation", "Language '%s'.", m_current_language_name.c_str());
                m_dictionary = m_dictionary_manager.get_dictionary(tgtLang);
            }
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
#ifdef TEST_BIDI
    m_rtl = true;
#endif
}   // Translations

// ----------------------------------------------------------------------------

const wchar_t* Translations::fribidize(const wchar_t* in_ptr)
{
#if ENABLE_BIDI
    if(this->isRTLLanguage())
    {
        FriBidiChar *fribidiInput = toFribidiChar(in_ptr);
        std::size_t length = 0;
        while (fribidiInput[length])
            length++;

        // Assume right to left as start direction.
#if FRIBIDI_MINOR_VERSION==10
        // While the doc for older fribidi versions is somewhat sparse,
        // using the RIGHT-TO-LEFT EMBEDDING character here appears to
        // work correct.
        FriBidiCharType pbase_dir = L'\u202B';
#else
        FriBidiCharType pbase_dir = FRIBIDI_PAR_ON;
#endif

        FriBidiChar *fribidiOutput = new FriBidiChar[length + 1];
        memset(fribidiOutput, 0, (length + 1) * sizeof(FriBidiChar));
        fribidi_boolean result = fribidi_log2vis(fribidiInput,
                                                 length,
                                                 &pbase_dir,
                                                 fribidiOutput,
              /* gint   *position_L_to_V_list */ NULL,
              /* gint   *position_V_to_L_list */ NULL,
              /* gint8  *embedding_level_list */ NULL
                                                               );

        freeFribidiChar(fribidiInput);

        if (!result)
        {
            delete[] fribidiOutput;
            Log::error("Translations::fribidize", "Fribidi failed in 'fribidi_log2vis' =(");
            m_converted_string = core::stringw(in_ptr);
            return m_converted_string.c_str();
        }

        wchar_t *convertedString = fromFribidiChar(fribidiOutput);
        m_converted_string = core::stringw(convertedString);
        freeFribidiChar(convertedString);
        delete[] fribidiOutput;
        return m_converted_string.c_str();
    }

#endif // ENABLE_BIDI
    return in_ptr;
}

bool Translations::isRTLText(const wchar_t *in_ptr)
{
#if ENABLE_BIDI
    if (this->isRTLLanguage())
    {
        std::size_t length = wcslen(in_ptr);
        FriBidiChar *fribidiInput = toFribidiChar(in_ptr);

        FriBidiCharType *types = new FriBidiCharType[length];
        fribidi_get_bidi_types(fribidiInput, length, types);
        freeFribidiChar(fribidiInput);

        // Declare as RTL if one character is RTL
        for (std::size_t i = 0; i < length; i++)
        {
            if (types[i] == FRIBIDI_TYPE_RTL ||
                types[i] == FRIBIDI_TYPE_RLO)
            {
                delete[] types;
                return true;
            }
        }
        delete[] types;
    }
    return false;
#else
    return false;
#endif
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

/**
 * \param singular Message to translate in singular form
 * \param plural   Message to translate in plural form (can be the same as the singular form)
 * \param num      Count used to obtain the correct plural form.
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
const wchar_t* Translations::w_ngettext(const wchar_t* singular, const wchar_t* plural, int num, const char* context)
{
    return w_ngettext( wide_to_utf8(singular), wide_to_utf8(plural), num, context);
}

/**
 * \param singular Message to translate in singular form
 * \param plural   Message to translate in plural form (can be the same as the singular form)
 * \param num      Count used to obtain the correct plural form.
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
const wchar_t* Translations::w_ngettext(const char* singular, const char* plural, int num, const char* context)
{
    const std::string& res = (context == NULL ?
                              m_dictionary.translate_plural(singular, plural, num) :
                              m_dictionary.translate_ctxt_plural(context, singular, plural, num));

    wchar_t* out_ptr = utf8_to_wide(res.c_str());
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

