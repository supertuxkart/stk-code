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
#include <unordered_map>
#include <unordered_set>

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef ANDROID
#include <jni.h>
#include "utils/utf8/unchecked.h"
#include "SDL_system.h"
#elif defined(MOBILE_STK)
#include "SDL_locale.h"
#endif

#ifdef __SWITCH__
extern "C" {
  #define u64 uint64_t
  #define u32 uint32_t
  #define s64 int64_t
  #define s32 int32_t
  #include <switch/services/set.h>
  #undef u64
  #undef u32
  #undef s64
  #undef s32
}
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
#include "tinygettext/file_system.hpp"
#include "tinygettext/log.hpp"

std::map<std::string, std::string> Translations::m_localized_name;
std::map<std::string, std::map<std::string, irr::core::stringw> >
    Translations::m_localized_country_codes;
// ============================================================================
std::unordered_map<char32_t,
    std::pair<std::unordered_set<std::u32string>, size_t> > g_thai_dict;
// ============================================================================
constexpr bool isThaiCP(char32_t c)
{
    return c >= 0x0e00 && c <= 0x0e7f;
}   // isThaiCP

// ============================================================================

const bool REMOVE_BOM = false;
/** The list of available languages; this is global so that it is cached (and remains
    even if the translations object is deleted and re-created) */
typedef std::vector<std::string> LanguageList;
static LanguageList g_language_list;

// ============================================================================
// Note : this method is not static because 'g_language_list' is initialized
//        the first time Translations is constructed (despite being a global)
const LanguageList* Translations::getLanguageList() const
{
    return &g_language_list;
}
#endif

// ----------------------------------------------------------------------------
Translations::Translations() //: m_dictionary_manager("UTF-16")
{
#ifndef SERVER_ONLY
    class StkFileSystem : public tinygettext::FileSystem
    {
    public:
    std::vector<std::string> open_directory(const std::string& pathname)
    {
        std::set<std::string> result;
        file_manager->listFiles(result, pathname);
        std::vector<std::string> files;
        for(const std::string& file : result)
            files.push_back(file);
        return files;
    }
    std::unique_ptr<std::istream> open_file(const std::string& filename)
    {
        return std::unique_ptr<std::istream>(new std::ifstream(
          FileUtils::getPortableReadingPath(filename)));
    }
    };
    // Hide log info because it warns about untranslated message
    tinygettext::Log::set_log_info_callback([](const std::string& str) {});
    tinygettext::Log::set_log_warning_callback([](const std::string& str)
        { Log::warn("tinygettext", "%s", str.c_str()); });
    tinygettext::Log::set_log_error_callback([](const std::string& str)
        { Log::error("tinygettext", "%s", str.c_str()); });

    m_dictionary_manager.set_filesystem(std::unique_ptr<tinygettext::FileSystem>(
        new StkFileSystem()));

    m_dictionary_manager.add_directory(
                        file_manager->getAsset(FileManager::TRANSLATION,""));

    if (g_language_list.size() == 0)
    {
        std::set<tinygettext::Language> languages = m_dictionary_manager.get_languages();

        // English is always there but may be not found on file system
        g_language_list.push_back("en");

        for (const tinygettext::Language& language : languages)
        {
            if (language.str() == "en")
                continue;
                
            g_language_list.push_back(language.str());
        }
    }

    if (m_localized_name.empty())
    {
        const std::string file_name = file_manager->getAsset("localized_name.tsv");
        try
        {
            std::ifstream in(FileUtils::getPortableReadingPath(file_name));
            if (!in.is_open())
            {
                Log::error("translation", "error: failure opening: '%s'.",
                    file_name.c_str());
            }
            else
            {
                std::string line;
                while (!StringUtils::safeGetline(in, line).eof())
                {
                    std::vector<std::string> lists = StringUtils::split(line, '\t');
                    if (lists.size() != 2)
                    {
                        Log::error("translation", "Invaild list.");
                        break;
                    }
                    if (lists[0] == "language_code")
                    {
                        continue;
                    }
                    if (lists[1] == "0")
                    {
                        lists[1] =
                            tinygettext::Language::from_name(lists[0]).get_name();
                    }
                    m_localized_name[lists[0]] = lists[1];
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
        const std::string file_name = file_manager->getAsset("country_names.tsv");
        try
        {
            std::ifstream in(FileUtils::getPortableReadingPath(file_name));
            if (!in.is_open())
            {
                Log::error("translation", "error: failure opening: '%s'.",
                    file_name.c_str());
            }
            else
            {
                std::vector<std::string> header;
                std::string line;
                while (!StringUtils::safeGetline(in, line).eof())
                {
                    std::vector<std::string> lists = StringUtils::split(line, '\t');
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

    if (g_thai_dict.empty())
    {
        const std::string file_name = file_manager->getAsset("thaidict.txt");
        try
        {
            std::ifstream in(FileUtils::getPortableReadingPath(file_name));
            if (!in.is_open())
            {
                Log::error("translation", "error: failure opening: '%s'.",
                    file_name.c_str());
            }
            else
            {
                std::string line;
                while (!StringUtils::safeGetline(in, line).eof())
                {
                    const std::u32string& u32line = StringUtils::utf8ToUtf32(line);
                    if (u32line.empty())
                        continue;
                    char32_t thai = u32line[0];
                    if (!isThaiCP(thai))
                        continue;
                    if (g_thai_dict.find(thai) == g_thai_dict.end())
                    {
                        g_thai_dict[thai] =
                            {
                                std::make_pair(
                                    std::unordered_set<std::u32string>{u32line},
                                    u32line.size())
                            };
                        continue;
                    }
                    auto& ret = g_thai_dict.at(thai);
                    ret.first.insert(u32line);
                    if (ret.second < u32line.size())
                        ret.second = u32line.size();
                }
            }
        }
        catch (std::exception& e)
        {
            Log::error("translation", "error: failure extract Thai dictionary.");
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
#ifdef __SWITCH__
            uint64_t languageStr = 0;
            if(!setGetLanguageCode(&languageStr))
            {
                language = (char*)&languageStr;
                language = StringUtils::findAndReplace(language, "-", "_");
            }
#elif defined(ANDROID)
            JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
            jobject native_activity = NULL;
            jclass class_native_activity = NULL;
            jmethodID method_id = NULL;
            jstring text = NULL;
            if (env == NULL)
            {
                Log::error("Translation",
                    "constructor is unable to SDL_AndroidGetJNIEnv.");
                goto end;
            }

            native_activity = (jobject)SDL_AndroidGetActivity();
            if (native_activity == NULL)
            {
                Log::error("Translation",
                    "constructor is unable to SDL_AndroidGetActivity.");
                goto end;
            }

            class_native_activity = env->GetObjectClass(native_activity);
            if (class_native_activity == NULL)
            {
                Log::error("Translation",
                    "constructor is unable to find object class.");
                goto end;
            }

            method_id = env->GetMethodID(class_native_activity,
                "getLocaleString", "()Ljava/lang/String;");
            if (method_id == NULL)
            {
                Log::error("Translation",
                    "constructor is unable to find method id.");
                goto end;
            }

            text =
                (jstring)env->CallObjectMethod(native_activity, method_id);
            if (text == NULL)
            {
                Log::error("Translation",
                    "Failed to CallObjectMethod for constructor.");
                goto end;
            }

end:
            if (text != NULL)
            {
                const uint16_t* utf16_text =
                    (const uint16_t*)env->GetStringChars(text, NULL);
                if (utf16_text != NULL)
                {
                    const size_t str_len = env->GetStringLength(text);
                    utf8::unchecked::utf16to8(
                        utf16_text, utf16_text + str_len,
                        std::back_inserter(language));
                    env->ReleaseStringChars(text, utf16_text);
                }
                env->DeleteLocalRef(text);
            }
            if (class_native_activity != NULL)
                env->DeleteLocalRef(class_native_activity);
            if (native_activity != NULL)
                env->DeleteLocalRef(native_activity);
#elif defined(MOBILE_STK)
            SDL_Locale* locale = SDL_GetPreferredLocales();
            if (locale)
            {
                // First locale only
                for (int l = 0; locale[l].language != NULL; l++)
                {
                    language = locale[l].language;
                    // Convert deprecated language code
                    if (language == "iw")
                        language = "he";
                    else if (language == "in")
                        language = "id";
                    else if (language == "ji")
                        language = "yi";
                    if (locale[l].country != NULL)
                    {
                        language += "-";
                        language += locale[l].country;
                    }
                    // iOS specific
                    if (language.find("zh-Hans") != std::string::npos)
                        language = "zh_CN";
                    else if (language.find("zh-Hant") != std::string::npos)
                        language = "zh_TW";
                    language = StringUtils::findAndReplace(language, "-", "_");
                    break;
                }
                SDL_free(locale);
            }
#elif defined(WIN32)
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
        auto ignore_country = [](const std::string& test_full_form)
        {
            // Use a country to test if the test_full_form is supported by
            // localized name
            auto it = m_localized_country_codes.find("HK");
            if (it != m_localized_country_codes.end())
                return it->second.find(test_full_form) == it->second.end();
            return true;
        };

        Log::verbose("translation", "Env var LANGUAGE = '%s'.",
                     language.c_str());

        // Hong Kong use tranditional chinese, not zh_CN which C > T
        language = StringUtils::findAndReplace(language, "zh_HK", "zh_TW");

        if (language.find(":") != std::string::npos)
        {
            std::vector<std::string> langs = StringUtils::split(language, ':');
            tinygettext::Language l;

            for (unsigned int curr=0; curr<langs.size(); curr++)
            {
                l = tinygettext::Language::from_env(langs[curr]);
                if (l)
                {
                    Log::verbose("translation", "Language '%s'.",
                                 l.get_name().c_str());
                    m_dictionary = &m_dictionary_manager.get_dictionary(l);
                    break;
                }
            }

            m_current_language_name = l.get_name();
            m_current_language_name_code = l.get_language();
            m_current_language_tag = m_current_language_name_code;
            if (!l.get_country().empty() && !ignore_country(
                m_current_language_name_code + "-" + l.get_country()))
            {
                m_current_language_tag += "-";
                m_current_language_tag += l.get_country();
            }
            if (!l)
            {
                m_dictionary = &m_dictionary_manager.get_dictionary();
            }
        }
        else
        {
            const tinygettext::Language& tgtLang = tinygettext::Language::from_env(language);
            if (!tgtLang)
            {
                Log::warn("Translation", "Unsupported language '%s'", language.c_str());
                UserConfigParams::m_language = "system";
                m_current_language_name = "Default language";
                m_current_language_name_code = "en";
                m_current_language_tag = "en";
                m_dictionary = &m_dictionary_manager.get_dictionary();
            }
            else
            {
                m_current_language_name = tgtLang.get_name();
                m_current_language_name_code = tgtLang.get_language();
                m_current_language_tag = m_current_language_name_code;
                if (!tgtLang.get_country().empty() && !ignore_country(
                    m_current_language_name_code + "-" + tgtLang.get_country()))
                {
                    m_current_language_tag += "-";
                    m_current_language_tag += tgtLang.get_country();
                }
                Log::verbose("translation", "Language '%s'.", m_current_language_name.c_str());
                m_dictionary = &m_dictionary_manager.get_dictionary(tgtLang);
            }
        }
    }
    else
    {
        m_current_language_name = "Default language";
        m_current_language_name_code = "en";
        m_current_language_tag = m_current_language_name_code;
        m_dictionary = &m_dictionary_manager.get_dictionary();
    }

#endif
}   // Translations

// ----------------------------------------------------------------------------
Translations::~Translations()
{
}   // ~Translations

// ----------------------------------------------------------------------------
/**
 * \param original Message to translate
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
irr::core::stringw Translations::w_gettext(const wchar_t* original, const char* context)
{
    std::string in = StringUtils::wideToUtf8(original);
    return w_gettext(in.c_str(), context);
}   // w_gettext

// ----------------------------------------------------------------------------
/**
 * \param original Message to translate
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
irr::core::stringw Translations::w_gettext(const char* original, const char* context)
{

#ifdef SERVER_ONLY
    return L"";
#else

    if (original[0] == '\0') return L"";

#if TRANSLATE_VERBOSE
    Log::info("Translations", "Translating %s", original);
#endif

    const std::string& original_t = (context == NULL ?
                                     m_dictionary->translate(original) :
                                     m_dictionary->translate_ctxt(context, original));
    // print
    //for (int n=0;; n+=4)
    const irr::core::stringw wide = StringUtils::utf8ToWide(original_t);
    const wchar_t* out_ptr = wide.c_str();
    if (REMOVE_BOM) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif

    return wide;
#endif

}   // w_gettext

// ----------------------------------------------------------------------------
/**
 * \param original Message to translate
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
std::string Translations::gettext(const char* original, const char* context)
{

#ifdef SERVER_ONLY
    return "";
#else

    if (original[0] == '\0') return "";

#if TRANSLATE_VERBOSE
    Log::info("Translations", "Translating %s", original);
#endif

    const std::string& original_t = (context == NULL ?
                                     m_dictionary->translate(original) :
                                     m_dictionary->translate_ctxt(context, original));

#if TRANSLATE_VERBOSE
    std::cout << "  translation : " << original_t << std::endl;
#endif

    return original_t;
#endif

}   // gettext

// ----------------------------------------------------------------------------
/**
 * \param singular Message to translate in singular form
 * \param plural   Message to translate in plural form (can be the same as the singular form)
 * \param num      Count used to obtain the correct plural form.
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
irr::core::stringw Translations::w_ngettext(const wchar_t* singular, const wchar_t* plural, int num, const char* context)
{
    std::string in = StringUtils::wideToUtf8(singular);
    std::string in2 = StringUtils::wideToUtf8(plural);
    return w_ngettext(in.c_str(), in2.c_str(), num, context);
}   // w_ngettext

// ----------------------------------------------------------------------------
/**
 * \param singular Message to translate in singular form
 * \param plural   Message to translate in plural form (can be the same as the singular form)
 * \param num      Count used to obtain the correct plural form.
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
irr::core::stringw Translations::w_ngettext(const char* singular, const char* plural, int num, const char* context)
{
#ifdef SERVER_ONLY
    return L"";

#else

    const std::string& res = (context == NULL ?
                              m_dictionary->translate_plural(singular, plural, num) :
                              m_dictionary->translate_ctxt_plural(context, singular, plural, num));

    const irr::core::stringw wide = StringUtils::utf8ToWide(res);
    const wchar_t* out_ptr = wide.c_str();
    if (REMOVE_BOM) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif

    return wide;
#endif

}   // w_ngettext

// ----------------------------------------------------------------------------
/**
 * \param singular Message to translate in singular form
 * \param plural   Message to translate in plural form (can be the same as the singular form)
 * \param num      Count used to obtain the correct plural form.
 * \param context  Optional, can be set to differentiate 2 strings that are identical
 *                 in English but could be different in other languages
 */
std::string Translations::ngettext(const char* singular, const char* plural, int num, const char* context)
{
#ifdef SERVER_ONLY
    return "";

#else

    const std::string& res = (context == NULL ?
                              m_dictionary->translate_plural(singular, plural, num) :
                              m_dictionary->translate_ctxt_plural(context, singular, plural, num));

#if TRANSLATE_VERBOSE
    std::cout << "  translation: " << res << std::endl;
#endif

    return res;
#endif

}   // ngettext

// ----------------------------------------------------------------------------
#ifndef SERVER_ONLY
std::set<unsigned int> Translations::getCurrentAllChar()
{
    return m_dictionary->get_all_used_chars();
}   // getCurrentAllChar

// ----------------------------------------------------------------------------
std::string Translations::getCurrentLanguageName()
{
    return m_current_language_name;
    //return m_dictionary_manager.get_language().get_name();
}   // getCurrentLanguageName

// ----------------------------------------------------------------------------
std::string Translations::getCurrentLanguageNameCode()
{
    return m_current_language_name_code;
}   // getCurrentLanguageNameCode

// ----------------------------------------------------------------------------
const std::string Translations::getLocalizedName(const std::string& str) const
{
    std::map<std::string, std::string>::const_iterator n = m_localized_name.find(str);
    if (n == m_localized_name.end())
        return tinygettext::Language::from_name(str).get_name();
    return n->second;
}   // getLocalizedName

// ----------------------------------------------------------------------------
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
}   // getLocalizedCountryName

// ----------------------------------------------------------------------------
/* Insert breakmark to thai sentence according to thai word dictionary, which
 * adds a mark in the begining of a thai vocabulary
 */
void Translations::insertThaiBreakMark(const std::u32string& thai,
                                       std::vector<bool>& breakable)
{
    if (thai.size() < 3)
        return;
    for (size_t i = 0; i < thai.size();)
    {
        char32_t t = thai[i];
        if (i >= thai.size() - 2 || !isThaiCP(t))
        {
            i++;
            continue;
        }
        auto ret = g_thai_dict.find(t);
        if (ret == g_thai_dict.end())
        {
            i++;
            continue;
        }
        size_t checked_word = 1;
        const size_t max_checking_word = ret->second.second;
        for (size_t j = i + 1;; j++)
        {
            if (j - i > max_checking_word || j > thai.size())
                break;
            const std::u32string& ss = thai.substr(i, j - i);
            if (ret->second.first.find(ss) != ret->second.first.end())
            {
                if (ss.size() > checked_word)
                    checked_word = ss.size();
                if (i != 0)
                    breakable[i - 1] = true;
            }
        }
        i += checked_word;
    }
}   // insertThaiBreakMark

#endif
