//  $Id: transation.cpp 839 2006-10-24 00:01:56Z hiker $
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

#include "irrlicht.h"

#include "io/file_manager.hpp"
#include "utils/constants.hpp"

#if ENABLE_BIDI
#include <fribidi/fribidi.h>
#endif

// set to 1 to debug i18n
#define TRANSLATE_VERBOSE 0

Translations* translations=NULL;
bool remove_bom = false;

#ifdef LINUX // m_debug
#define PACKAGE "supertuxkart"
#endif

// ----------------------------------------------------------------------------
Translations::Translations()
{
#ifdef ENABLE_NLS

    // LC_ALL does not work, sscanf will then not always be able
    // to scan for example: s=-1.1,-2.3,-3.3 correctly, which is
    // used in driveline files.
#if defined(WIN32) && !defined(__CYGWIN__)
    // Windows does not have LC_MESSAGES
    setlocale(LC_CTYPE,    "");
#else
    setlocale(LC_MESSAGES, "");
#endif


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

    // This is a silly but working hack I added to determine whether the current language is RTL or
    // not, since gettext doesn't seem to provide this information

    // This one is just for the xgettext parser to pick up
#define ignore(X)

    ignore(_("   Is this a RTL language?"));

    //I18N: Do NOT literally translate this string!! Please enter Y as the translation if your language is a RTL (right-to-left) language, N (or nothing) otherwise
    const char* isRtl = gettext("   Is this a RTL language?");
    const wchar_t* isRtlW = reinterpret_cast<const wchar_t*>(isRtl);
    
    m_rtl = false;
    
    for (int n=0; isRtlW[n] != 0; n++)
    {
        if (isRtlW[n] == 'Y')
        {
            m_rtl = true;
            break;
        }
    }
    
    //m_rtl = (isRtl[0] == 'Y');

#endif

}   // Translations
// ----------------------------------------------------------------------------
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
    const char* original_t = gettext(original);
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

    if(original_t==original)
    {
        m_converted_string = core::stringw(original);

#if TRANSLATE_VERBOSE
        std::wcout << L"  translation : " << m_converted_string.c_str() << std::endl;
#endif
        return m_converted_string.c_str();
    }

    // print
    //for (int n=0;; n+=4)

    wchar_t* out_ptr = (wchar_t*)original_t;
    if (remove_bom) out_ptr++;

#if TRANSLATE_VERBOSE
    std::wcout << L"  translation : " << out_ptr << std::endl;
#endif


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
            fribidiInput[n] = out_ptr[n];
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

        FriBidiCharType pbase_dir = FRIBIDI_TYPE_ON; // Not sure what's that for, but it seems to work...

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
            m_converted_string = core::stringw(original);
            return m_converted_string.c_str();
        }

        //std::cout << "fribidi output : ";
        //for (FriBidiChar* c=fribidiOutput; *c != 0; c++)
        //{
        //    std::cout << (int)fribidiOutput[n] << " ";
        //}
        //std::cout << "\n";


        return (const wchar_t*)fribidiOutput;
    }

#endif
    return out_ptr;

}

bool Translations::isRTLLanguage() const
{
    return m_rtl;
}

