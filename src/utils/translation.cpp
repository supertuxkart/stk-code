//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006, 2007, 2008 Joerg Henrichs
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

#include "irrlicht.h"

#include "io/file_manager.hpp"
#include "utils/constants.hpp"

//#include "tinygettext/iconv.hpp"
#include "utils/utf8.h"

#if ENABLE_BIDI
#include <fribidi/fribidi.h>
#endif

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
std::vector<std::string> g_language_list;

// Note : this method is not static because 'g_language_list' is initialized
//        the first time Translations is constructed (despite being a global)
const std::vector<std::string>* Translations::getLanguageList() const
{
    return &g_language_list;
}


wchar_t* utf8_to_wide(const char* input)
{
    static std::vector<wchar_t> utf16line;
    utf16line.clear();
    
    utf8::utf8to16(input, input + strlen(input), back_inserter(utf16line));
    utf16line.push_back(0);
    
    return &utf16line[0];
    
    /*
    static tinygettext_iconv_t cd = 0;
    
    if (cd == 0) cd = tinygettext_iconv_open("UTF-16", "UTF-8");
    if (cd == reinterpret_cast<tinygettext_iconv_t>(-1))
    {
        fprintf(stderr, "[utf8_to_wide] ERROR: failed to init libiconv\n");
        return L"?";
    }
    
    size_t inbytesleft  = strlen(input);
    size_t outbytesleft = 4*inbytesleft; // Worst case scenario: ASCII -> UTF-32?
    
    const unsigned int BUFF_SIZE = 512*4;
    
    if (outbytesleft > BUFF_SIZE)
    {
        fprintf(stderr, "[utf8_to_wide] ERROR: stirng too long : '%s'\n", input);
    }
    
    static char temp_buffer[BUFF_SIZE];
    
    // Try to convert the text.
    size_t ret = tinygettext_iconv(cd, &input, &inbytesleft, (char**)&temp_buffer, &outbytesleft);
    if (ret == static_cast<size_t>(-1))
    {
        if (errno == EILSEQ || errno == EINVAL)
        { // invalid multibyte sequence
            tinygettext_iconv(cd, NULL, NULL, NULL, NULL); // reset state
            
            // FIXME: Could try to skip the invalid byte and continue
            fprintf(stderr, "[Translation] ERROR: invalid multibyte sequence in '%s'\n", input);
        }
        else if (errno == E2BIG)
        { // output buffer to small
            fprintf(stderr, "[Translation] ERROR: E2BIG: This should never be reached\n");
        }
        else if (errno == EBADF)
        {
            fprintf(stderr, "[Translation] ERROR: EBADF: This should never be reached\n");
        }
        else
        {
            fprintf(stderr, "[Translation] ERROR: <unknown>: This should never be reached\n");
        }
        return L"?";
    }
    else
    {
        if (sizeof(wchar_t) == 2)
        {
            return (wchar_t*)temp_buffer;
        }
        else if (sizeof(wchar_t) == 4)
        {
            static wchar_t out_buffer[512];

            // FIXME: endianness?
            int i = 0;
            for (char* ptr = temp_buffer; ; ptr += 2)
            {
                out_buffer[i] = (*ptr << 8) | *(ptr + 1);
                
                if (*ptr == 0 && *(ptr + 1) == 0) break;
                
                i++;
            }
            
            return out_buffer;
        }
        else
        {
            fprintf(stderr, "Unknown wchar_t size : %lui\n", sizeof(wchar_t));
            return L"?";
        }
    }
     */
}

// ----------------------------------------------------------------------------
Translations::Translations() //: m_dictionary_manager("UTF-16")
{
#ifdef ENABLE_NLS

    if (g_language_list.size() == 0)
    {
        std::set<std::string> flist;
        file_manager->listFiles(flist,
                                file_manager->getTranslationDir(),
                                true);
        
        // English is always there but won't be found on file system
        g_language_list.push_back("en");
        
        std::set<std::string>::iterator it;
        for ( it=flist.begin() ; it != flist.end(); it++ )
        {
            if (file_manager->fileExists(file_manager->getTranslationDir() + "/" + (*it).c_str() + "/LC_MESSAGES/supertuxkart.mo"))
            {
                g_language_list.push_back( *it );
                // printf("Lang : <%s>\n", (*it).c_str());
            }
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
    
    m_dictionary_manager.add_directory( file_manager->getTranslationDir().c_str() );
    
    /*
    const std::set<Language>& languages = m_dictionary_manager.get_languages();
    std::cout << "Number of languages: " << languages.size() << std::endl;
    for (std::set<Language>::const_iterator i = languages.begin(); i != languages.end(); ++i)
    {
        const Language& language = *i;
        std::cout << "Env:       " << language.str()           << std::endl
                  << "Name:      " << language.get_name()      << std::endl
                  << "Language:  " << language.get_language()  << std::endl
                  << "Country:   " << language.get_country()   << std::endl
                  << "Modifier:  " << language.get_modifier()  << std::endl
                  << std::endl;
    }
    */

    const char* lang = getenv("LANG");
    const char* language = getenv("LANGUAGE");

    if (language != NULL && strlen(language) > 0)
    {
        printf("Env var LANGUAGE = '%s', which corresponds to %s\n", language, Language::from_env(language).get_name().c_str());
        m_dictionary = m_dictionary_manager.get_dictionary(Language::from_env(language));
    }
    else if (lang != NULL && strlen(lang) > 0)
    {
        printf("Env var LANG = '%s'\n", lang);
        m_dictionary = m_dictionary_manager.get_dictionary(Language::from_env(lang));
    }
    else
    {
        m_dictionary = m_dictionary_manager.get_dictionary();
    }
    
    // This is a silly but working hack I added to determine whether the current language is RTL or
    // not, since gettext doesn't seem to provide this information

    //std::string test = m_dictionary.translate("Loading");
    //printf("'%s'\n", test.c_str());
    
    // This one is just for the xgettext parser to pick up
#define ignore(X)

    ignore(_("   Is this a RTL language?"));

    //I18N: Do NOT literally translate this string!! Please enter Y as the translation if your language is a RTL (right-to-left) language, N (or nothing) otherwise
    const std::string isRtl = m_dictionary.translate("   Is this a RTL language?");
    
    m_rtl = false;
    
    for (unsigned int n=0; n < isRtl.size(); n++)
    {
        if (isRtl[n] == 'Y')
        {
            m_rtl = true;
            break;
        }
    }
#endif

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
                std::cerr << "WARNING : translated string too long, truncating!\n";
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
                                                 /* gint        *position_L_to_V_list */ NULL,
                                                 /* gint        *position_V_to_L_list */ NULL,
                                                 /* gint8       *embedding_level_list */ NULL
                                                 );
        
        if (!result)
        {
            std::cerr << "Fribidi failed in 'fribidi_log2vis' =(\n";
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

const wchar_t* Translations::w_gettext(const char* original)
{
    if (original[0] == '\0') return L"";

#if TRANSLATE_VERBOSE
    #if ENABLE_NLS
    std::cout << "Translating " << original << "\n";
    #else
    std::cout << "NOT Translating " << original << "\n";
    #endif
#endif

#if ENABLE_NLS
    const std::string& original_t = m_dictionary.translate(original);
#else
    m_converted_string = core::stringw(original);
    return m_converted_string.c_str();
#endif

    /*
    std::cout << "--> original_t==original ? " << (original_t==original) << std::endl;
    int zeros = 0;
    for (int n=0;; n+=1)
    {
        std::cout << original_t[n] << " (" << (unsigned)(original_t[n]) << ")\n";
        if (original_t[n] == 0)
        {
            zeros++;
            if (zeros >= 4) break;
        }
        else
        {
            zeros = 0;
        }
    }*/

    if (original_t == original)
    {
        m_converted_string = core::stringw(original);

#if TRANSLATE_VERBOSE
        std::wcout << L"  translation : " << m_converted_string.c_str() << std::endl;
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

