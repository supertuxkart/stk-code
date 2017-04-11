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

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <vector>

#if ENABLE_BIDI
#  include <fribidi/fribidi.h>
#endif

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"


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

// ----------------------------------------------------------------------------
/** Frees the memory allocated for the result of toFribidiChar(). */
#ifdef ENABLE_BIDI
void freeFribidiChar(FriBidiChar *str)
{
#ifdef TEST_BIDI
    delete[] str;
#else
    if (sizeof(wchar_t) != sizeof(FriBidiChar))
        delete[] str;
#endif
}
#endif

/** Frees the memory allocated for the result of fromFribidiChar(). */
#ifdef ENABLE_BIDI
void freeFribidiChar(wchar_t *str)
{
    if (sizeof(wchar_t) != sizeof(FriBidiChar))
        delete[] str;
}
#endif

// ----------------------------------------------------------------------------
/** Converts a wstring to a FriBidi-string.
    The caller must take care to free (or not to free) the result after use.
    Freeing should be done with freeFribidiChar().

    On linux, the string doesn't need to be converted because wchar_t is
    already UTF-32. On windows the string is converted from UTF-16 by this
    function. */
#ifdef ENABLE_BIDI
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
    // Prepend a character in each line that forces RTL style
    int lines = 1;
    for (std::size_t i = 0; i <= length; i++)
    {
        if (str[i] == L'\n')
            lines++;
    }
    FriBidiChar *tmp = result;
    length += lines;
    result = new FriBidiChar[length + 1];
    lines = 1;
    result[0] = L'\u202E';
    for (std::size_t i = 1; i <= length; i++)
    {
        result[i] = tmp[i - lines];
        if (str[i - lines] == L'\n')
        {
            lines++;
            i++;
            result[i] = L'\u202E';
        }
    }
    if (sizeof(wchar_t) != sizeof(FriBidiChar))
        delete[] tmp;
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
#endif

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

    const std::string file_name = file_manager->getAsset("localized_name.txt");
    try
    {
        std::unique_ptr<std::istream> in(new std::ifstream(file_name.c_str()));
        if (!in.get())
        {
            Log::error("translation", "error: failure opening: '%s'.",
                file_name.c_str());
        }
        else
        {
            for (std::string line; std::getline(*in, line); )
            {
                std::size_t pos = line.find("=");
                std::string name = line.substr(0, pos);
                std::string localized_name = line.substr(pos + 1);
                if (localized_name == "0")
                {
                    localized_name =
                        tinygettext::Language::from_name(name).get_name();
                }
                m_localized_name[name] = localized_name;
            }
        }
    }
    catch(std::exception& e)
    {
        Log::error("translation", "error: failure extract localized name.");
        Log::error("translation", "%s", e.what());
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
            m_current_language_name_code = l.get_language();

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
                m_current_language_name_code = "en";
                m_dictionary = m_dictionary_manager.get_dictionary();
            }
            else
            {
                m_current_language_name = tgtLang.get_name();
                m_current_language_name_code = tgtLang.get_language();
                Log::verbose("translation", "Language '%s'.", m_current_language_name.c_str());
                m_dictionary = m_dictionary_manager.get_dictionary(tgtLang);
            }
        }
    }
    else
    {
        m_current_language_name = "Default language";
        m_current_language_name_code = "en";
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

Translations::~Translations()
{
}   // ~Translations

// ----------------------------------------------------------------------------

const wchar_t* Translations::fribidize(const wchar_t* in_ptr)
{
    if (isRTLText(in_ptr))
    {
        // Test if this string was already fribidized
        std::map<const irr::core::stringw, const irr::core::stringw>::const_iterator
            found = m_fribidized_strings.find(in_ptr);
        if (found != m_fribidized_strings.cend())
            return found->second.c_str();

        // Use fribidi to fribidize the string
        // Split text into lines
        std::vector<core::stringw> input_lines = StringUtils::split(in_ptr, '\n');
        // Reverse lines for RTL strings, irrlicht will reverse them back
        // This is needed because irrlicht inserts line breaks itself if a text
        // is too long for one line and then reverses the lines again.
        std::reverse(input_lines.begin(), input_lines.end());

        // Fribidize and concat lines
        core::stringw converted_string;
        for (std::vector<core::stringw>::iterator it = input_lines.begin();
             it != input_lines.end(); it++)
        {
            if (it == input_lines.begin())
                converted_string = fribidizeLine(*it);
            else
            {
                converted_string += "\n";
                converted_string += fribidizeLine(*it);
            }
        }

        // Save it in the map
        m_fribidized_strings.insert(std::pair<const irr::core::stringw, const irr::core::stringw>(
            in_ptr, converted_string));
        found = m_fribidized_strings.find(in_ptr);

        return found->second.c_str();
    }
    else
        return in_ptr;
}

bool Translations::isRTLText(const wchar_t *in_ptr)
{
#if ENABLE_BIDI
    std::size_t length = wcslen(in_ptr);
    FriBidiChar *fribidiInput = toFribidiChar(in_ptr);

    FriBidiCharType *types = new FriBidiCharType[length];
    fribidi_get_bidi_types(fribidiInput, (FriBidiStrIndex)length, types);
    freeFribidiChar(fribidiInput);

    // Declare as RTL if one character is RTL
    for (std::size_t i = 0; i < length; i++)
    {
        if (types[i] & FRIBIDI_MASK_RTL)
        {
            delete[] types;
            return true;
        }
    }
    delete[] types;
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
    std::string in = StringUtils::wideToUtf8(original);
    return w_gettext(in.c_str(), context);
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
        static irr::core::stringw converted_string;
        converted_string = StringUtils::utf8ToWide(original);

#if TRANSLATE_VERBOSE
        std::wcout << L"  translation : " << converted_string << std::endl;
#endif
        return converted_string.c_str();
    }

    // print
    //for (int n=0;; n+=4)

    static core::stringw original_tw;
    original_tw = StringUtils::utf8ToWide(original_t);

    const wchar_t* out_ptr = original_tw.c_str();
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
    std::string in = StringUtils::wideToUtf8(singular);
    std::string in2 = StringUtils::wideToUtf8(plural);
    return w_ngettext(in.c_str(), in2.c_str(), num, context);
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

    static core::stringw str_buffer;
    str_buffer = StringUtils::utf8ToWide(res);
    const wchar_t* out_ptr = str_buffer.c_str();
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

std::set<wchar_t> Translations::getCurrentAllChar()
{
    return m_dictionary.get_all_used_chars();
}

std::string Translations::getCurrentLanguageName()
{
    return m_current_language_name;
    //return m_dictionary_manager.get_language().get_name();
}

std::string Translations::getCurrentLanguageNameCode()
{
    return m_current_language_name_code;
}

core::stringw Translations::fribidizeLine(const core::stringw &str)
{
#if ENABLE_BIDI
    FriBidiChar *fribidiInput = toFribidiChar(str.c_str());
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

    // Reverse text line by line
    FriBidiChar *fribidiOutput = new FriBidiChar[length + 1];
    memset(fribidiOutput, 0, (length + 1) * sizeof(FriBidiChar));
    fribidi_boolean result = fribidi_log2vis(fribidiInput,
                                                (FriBidiStrIndex)length,
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
        return core::stringw(str);
    }

    wchar_t *convertedString = fromFribidiChar(fribidiOutput);
    core::stringw converted_string(convertedString);
    freeFribidiChar(convertedString);
    delete[] fribidiOutput;
    return converted_string;

#else
    return core::stringw(str);
#endif // ENABLE_BIDI

}

const std::string& Translations::getLocalizedName(const std::string& str) const
{
    std::map<std::string, std::string>::const_iterator n = m_localized_name.find(str);
    assert (n != m_localized_name.end());
    return n->second;
}
