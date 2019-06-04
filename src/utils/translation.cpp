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
#include <fstream>
#include <iostream>
#include <thread>

#if ENABLE_BIDI
#  include <fribidi/fribidi.h>
#endif

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

#ifdef ANDROID
#include "main_android.hpp"
#endif


// set to 1 to debug i18n
#define TRANSLATE_VERBOSE 0
// Define TEST_BIDI to force right-to-left style for all languages
//#define TEST_BIDI

Translations* translations = NULL;

#ifdef LINUX // m_debug
#define PACKAGE "supertuxkart"
#endif

#ifndef SERVER_ONLY
std::map<std::string, std::string> Translations::m_localized_name;
std::map<std::string, std::map<std::string, irr::core::stringw> >
    Translations::m_localized_country_codes;

const bool REMOVE_BOM = false;
using namespace tinygettext;
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
#endif

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
#ifndef SERVER_ONLY
    m_dictionary_manager.add_directory(
                        file_manager->getAsset(FileManager::TRANSLATION,""));

    if (g_language_list.size() == 0)
    {
        std::set<Language> languages = m_dictionary_manager.get_languages();      

        // English is always there but may be not found on file system
        g_language_list.push_back("en");

        for (const Language& language : languages)
        {
            if (language.str() == "en")
                continue;
                
            g_language_list.push_back(language.str());
        }
    }

    if (m_localized_name.empty())
    {
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
                for (std::string line; std::getline(*in, line, ';'); )
                {
                    line = StringUtils::removeWhitespaces(line);

                    if (line.empty())
                        continue;

                    std::size_t pos = line.find("=");

                    if (pos == std::string::npos)
                        continue;

                    std::string name = line.substr(0, pos);
                    std::string localized_name = line.substr(pos + 1);

                    if (name.empty() || localized_name.empty())
                        continue;

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
    }

    if (m_localized_country_codes.empty())
    {
        const std::string file_name = file_manager->getAsset("country_names.csv");
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
                std::vector<std::string> header;
                std::string line;
                while (!StringUtils::safeGetline(*in, line).eof())
                {
                    std::vector<std::string> lists = StringUtils::split(line, ';');
                    if (lists.size() < 2)
                    {
                        Log::error("translation", "Invaild list.");
                        break;
                    }
                    if (lists[0] == "country_code")
                    {
                        header = lists;
                        continue;
                    }
                    if (lists.size() != header.size())
                    {
                        Log::error("translation", "Different column size.");
                        break;
                    }
                    if (m_localized_country_codes.find(lists[0]) ==
                        m_localized_country_codes.end())
                    {
                        m_localized_country_codes[lists[0]] =
                        std::map<std::string, irr::core::stringw>();
                    }
                    for (unsigned i = 1; i < lists.size(); i++)
                    {
                        auto& ret = m_localized_country_codes.at(lists[0]);
                        ret[header[i]] = StringUtils::utf8ToWide(lists[i]);
                    }
                }
            }
        }
        catch (std::exception& e)
        {
            Log::error("translation", "error: failure extract localized country name.");
            Log::error("translation", "%s", e.what());
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
            
#elif defined(ANDROID)
            if (global_android_app)
            {
                char p_language[3] = {};
                AConfiguration_getLanguage(global_android_app->config, 
                                           p_language);
                std::string s_language(p_language);
                if (!s_language.empty())
                {
                    language += s_language;

                    char p_country[3] = {};
                    AConfiguration_getCountry(global_android_app->config, 
                                              p_country);
                    std::string s_country(p_country);
                    if (!s_country.empty())
                    {
                        language += "_";
                        language += s_country;
                    }
                }
            }
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
            m_current_language_tag = m_current_language_name_code;
            if (!l.get_country().empty())
            {
                m_current_language_tag += "-";
                m_current_language_tag += l.get_country();
            }
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
                Log::warn("Translation", "Unsupported language '%s'", language.c_str());
                UserConfigParams::m_language = "system";
                m_current_language_name = "Default language";
                m_current_language_name_code = "en";
                m_current_language_tag = "en";
                m_dictionary = m_dictionary_manager.get_dictionary();
            }
            else
            {
                m_current_language_name = tgtLang.get_name();
                m_current_language_name_code = tgtLang.get_language();
                m_current_language_tag = m_current_language_name_code;
                if (!tgtLang.get_country().empty())
                {
                    m_current_language_tag += "-";
                    m_current_language_tag += tgtLang.get_country();
                }
                Log::verbose("translation", "Language '%s'.", m_current_language_name.c_str());
                m_dictionary = m_dictionary_manager.get_dictionary(tgtLang);
            }
        }
    }
    else
    {
        m_current_language_name = "Default language";
        m_current_language_name_code = "en";
        m_current_language_tag = m_current_language_name_code;
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

#endif
}   // Translations

// ----------------------------------------------------------------------------

Translations::~Translations()
{
}   // ~Translations

// ----------------------------------------------------------------------------

const wchar_t* Translations::fribidize(const wchar_t* in_ptr)
{
#ifdef SERVER_ONLY
    return in_ptr;
#else
    if (isRTLText(in_ptr))
    {
        std::lock_guard<std::mutex> lock(m_fribidized_mutex);
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
#endif
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

#ifdef SERVER_ONLY
    static irr::core::stringw dummy_for_server;
    dummy_for_server = StringUtils::utf8ToWide(original);
    return dummy_for_server.c_str();
#else

    if (original[0] == '\0') return L"";

#if TRANSLATE_VERBOSE
    Log::info("Translations", "Translating %s", original);
#endif

    const std::string& original_t = (context == NULL ?
                                     m_dictionary.translate(original) :
                                     m_dictionary.translate_ctxt(context, original));
    // print
    //for (int n=0;; n+=4)
    std::lock_guard<std::mutex> lock(m_gettext_mutex);

    static std::map<std::thread::id, core::stringw> original_tw;
    original_tw[std::this_thread::get_id()] = StringUtils::utf8ToWide(original_t);

    const wchar_t* out_ptr = original_tw.at(std::this_thread::get_id()).c_str();
    if (REMOVE_BOM) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif

    return out_ptr;
#endif
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
#ifdef SERVER_ONLY
    static core::stringw str_buffer;
    str_buffer = StringUtils::utf8ToWide(singular);
    return str_buffer.c_str();

#else

    const std::string& res = (context == NULL ?
                              m_dictionary.translate_plural(singular, plural, num) :
                              m_dictionary.translate_ctxt_plural(context, singular, plural, num));

    std::lock_guard<std::mutex> lock(m_ngettext_mutex);

    static std::map<std::thread::id, core::stringw> str_buffer;
    str_buffer[std::this_thread::get_id()] = StringUtils::utf8ToWide(res);

    const wchar_t* out_ptr = str_buffer.at(std::this_thread::get_id()).c_str();
    if (REMOVE_BOM) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif

    return out_ptr;
#endif

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

#ifndef SERVER_ONLY
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

const std::string& Translations::getLocalizedName(const std::string& str) const
{
    std::map<std::string, std::string>::const_iterator n = m_localized_name.find(str);
    assert (n != m_localized_name.end());
    return n->second;
}

/* Convert 2-letter country code to localized readable name.
 */
irr::core::stringw Translations::getLocalizedCountryName(const std::string& country_code) const
{
    auto it = m_localized_country_codes.find(country_code);
    // If unknown 2 letter country just return the same
    if (it == m_localized_country_codes.end())
        return StringUtils::utf8ToWide(country_code);
    auto name_itr = it->second.find(m_current_language_tag);
    if (name_itr != it->second.end())
        return name_itr->second;
    // If there should be invalid language tag, use en (which always exists)
    name_itr = it->second.find("en");
    if (name_itr != it->second.end())
        return name_itr->second;
    // Fallback
    return StringUtils::utf8ToWide(country_code);
}

#endif
