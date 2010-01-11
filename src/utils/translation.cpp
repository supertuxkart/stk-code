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
#include "irrlicht.h"

#include "utils/translation.hpp"
#include "io/file_manager.hpp"

#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iconv.h>
#include <stdlib.h>
#include <iostream>

enum CONVERT_TO
{
    UTF_16 = 0,
    UTF_32 = 1
};
const char* CODE_NAMES[] = 
{
    "UTF-16",
    "UTF-32"
};

/*
 The code below is based on Solaris' man pages code. No license was specified, but I assume that
 they wrote man pages for people to use them...
 */

/*
 * For state-dependent encodings, changes the state of the conversion
 * descriptor to initial shift state.  Also, outputs the byte sequence
 * to change the state to initial state.
 * This code is assuming the iconv call for initializing the state
 * won't fail due to lack of space in the output buffer.
 */
#if defined(__APPLE__) && _LIBICONV_VERSION < 0x010B
    #define INIT_SHIFT_STATE(cd, fptr, ileft, tptr, oleft)  \
    {                                                       \
        fptr = NULL;                                        \
        ileft = 0;                                          \
        tptr = to;                                          \
        oleft = BUFSIZ;                                     \
        iconv(cd, &fptr, &ileft, &tptr, &oleft);            \
    }
#else
    #define INIT_SHIFT_STATE(cd, fptr, ileft, tptr, oleft)  \
    {                                                       \
        fptr = NULL;                                        \
        ileft = 0;                                          \
        tptr = to;                                          \
        oleft = BUFSIZ;                                     \
        iconv(cd, const_cast<char**>(&fptr), &ileft, &tptr, &oleft); \
    }
#endif

bool convertToEncoding(const char* from, char* to, const int BUF_SIZE, const char* from_code, CONVERT_TO to_code)
{
    iconv_t cd;
    char    *tptr;
    const char  *fptr;
    size_t  ileft, oleft, ret;
    
    cd = iconv_open(CODE_NAMES[to_code], from_code);
    if (cd == (iconv_t)-1)
    {
        // failed
        fprintf(stderr, "iconv_open(%s, %s) failed\n", CODE_NAMES[to_code], from_code);
        return false;
    }
    
    ileft = strlen(from);
    
    
    fptr = from;
    for (;;)
    {
        tptr = to;
        oleft = BUFSIZ;
        
#if defined(__APPLE__) && _LIBICONV_VERSION < 0x010B 
        ret = iconv(cd, &fptr, &ileft, &tptr, &oleft);
#else
        ret = iconv(cd, const_cast<char**>(&fptr), &ileft, &tptr, &oleft);
#endif
        
        if (ret != (size_t)-1)
        {
            // iconv succeeded
            
            // Manually add ending '\0'
            const int curr_offset = BUFSIZ - oleft;
            
            to[curr_offset] = '\0';
            
            if (curr_offset+1 >= BUFSIZ)
            {
                fprintf(stderr, "iconv failed : error while adding '\\0' character (Output buffer not big enough) \n");
                return false;
            }
            to[curr_offset+1] = '\0';
            
            // UTF-32 needs 2 more bytes at 0
            if (to_code == UTF_32)
            {
                to[curr_offset+2] = '\0';
                
                if (curr_offset+3 >= BUFSIZ)
                {
                    fprintf(stderr, "iconv failed : error while adding '\\0' character (Output buffer not big enough) \n");
                    return false;
                }
                to[curr_offset+3] = '\0';
            }
            break;
        }
        
        // iconv failed
        if (errno == EINVAL)
        {
            fprintf(stderr, "iconv failed : EINVAL error (Incomplete character or shift sequence) \n");
            break;
        }
        else if (errno == E2BIG)
        {
            // Lack of space in output buffer
            fprintf(stderr, "iconv failed : E2BIG error (Output buffer not big enough) \n");
            
            std::cout << "So far, I have :\n";
            for (int i=0; i<BUF_SIZE; i++)
            {
                std::cout << "    " << to[i] << " (" <<  (unsigned int)to[i] << ")\n";
            }
            
            //Outputs converted characters
            //fwrite(to, 1, BUFSIZ - oleft, stdout);
            
            // Tries to convert remaining characters in
            // input buffer with emptied output buffer
            return false;
        }
        else if (errno == EILSEQ)
        {
            
            fprintf(stderr, "iconv failed : EILSEQ error (Illegal character or shift sequence) \n");
            
            std::cout << "Original :\n";
            for (int i=0; ; i++)
            {
                std::cout << "    " << from[i] << " (" << std::hex << (unsigned int)from[i] << ")\n";
                if (from[i] == 0) break;
            }
            
            // Outputs converted characters
            //fwrite(to, 1, BUFSIZ - oleft, stdout);
            
            // Initializes the conversion descriptor and
            // outputs the sequence to change the state to
            // initial state.
            INIT_SHIFT_STATE(cd, fptr, ileft, tptr, oleft);
            iconv_close(cd);
            
            return false;
        }
        else if (errno == EBADF)
        {
            fprintf(stderr, "iconv failed : EBADF error (invalid conversion description) \n");
            return false;
        }
        else
        {
            fprintf(stderr, "iconv failed : other error\n");
            return false;
        }
    }
    
    /*
     * Initializes the conversion descriptor and outputs
     * the sequence to change the state to initial state.
     */
    INIT_SHIFT_STATE(cd, fptr, ileft, tptr, oleft);
    
    iconv_close(cd);
    return true;
}

// TODO: use a type that works on all platforms
typedef unsigned int uint32;

/**
 * \param input_text   Some UTF-8 text
 * \return             The appropriate representation of this text in the platform's native wchar_t type, or
 *                     NULL if conversion failed (in this case a message will be printed to the console)
 */
wchar_t* utf8_to_wchar_t(const char* input_text)
{
    const int buffer_size = 2048;                       // let's arbitrarly support 2048 chars
    const int byte_size = buffer_size*sizeof(wchar_t);  // we need more space if the system uses UTF-32
    
    static char output_buffer[byte_size];
    static wchar_t wide_form[buffer_size*2];            // Allow for some surrogate pairs
    
    
    if (sizeof(wchar_t) == 2)
    {
        // UTF-16
        if (!convertToEncoding(input_text, output_buffer, byte_size, "UTF-8", UTF_16)) return NULL;
        
        return ((wchar_t*)output_buffer)+1; // Skip initial Unicode header
        
        
        for (int n=0; true; n+=2)
        {
            wide_form[n/2] = (uint32(output_buffer[n  ]) << 8) |
            uint32(output_buffer[n+1]     );
            
            //std::cout << "char : <" << to[n] << ", " << to[n+1] << ">; wide char = " << std::hex << (uint32)wide_form[n/2] << "\n";
            
            if (wide_form[n/2] == 0) break;
        }
        
        return &wide_form[1];
        
    }
    else if (sizeof(wchar_t) == 4)
    {
        // UTF-32
        if (!convertToEncoding(input_text, output_buffer, byte_size, "UTF-8", UTF_32)) return NULL;
        
        //return ((wchar_t*)output_buffer)+1; // Skip initial Unicode header
        
        
        for (int n=0; true; n+=4)
        {
            wide_form[n/4] = (uint32(output_buffer[ n ]) << 24) |
            (uint32(output_buffer[n+1]) << 16) |
            (uint32(output_buffer[n+2]) << 8 ) |
            uint32(output_buffer[n+3])      ;
            
            //std::cout << "char : <" << to[n] << ", " << to[n+1] << ", " << to[n+2] << ", " << to[n+3]
            //          << ">; wide char = " << std::hex << (uint32)wide_form[n/4] << "\n";
            
            if (wide_form[n/4] == 0) break;
        }
        return &wide_form[1];
    }
    else
    {
        std::cout << "ERROR : Unknown wchar_t size!\n";
        return NULL;
    }
}

/*
int main()
{
    const char* from = "Some text in plain ASCII. z";
    
    
    wchar_t* out_ptr = utf8_to_wchar_t(from);
    
    if (out_ptr == NULL)
    {
        std::cout << "ERROR!\n";
        return 1;
    }
    else
    {
        std::wcout << L"Converted text : <" << out_ptr << L">\n";
    }
    
    return 0;
}
*/


Translations* translations=NULL;

Translations::Translations()
{ 
#ifdef HAVE_GETTEXT
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
    if (sizeof(wchar_t) == 32) bind_textdomain_codeset(PACKAGE, "UTF-32");
    else if (sizeof(wchar_t) == 16) bind_textdomain_codeset(PACKAGE, "UTF-16");
    else assert(false);
    
    //bind_textdomain_codeset(PACKAGE, "iso-8859-1");
    textdomain (PACKAGE);
#endif
        
}   // Translations

//const int BUFFER_SIZE = 512;
//wchar_t out_buffer[BUFFER_SIZE];

wchar_t* w_gettext(const char* original)
{
    if (original[0] == '\0') return L"";
    
    std::cout << "Translating " << original << "\n";
    
#if ENABLE_NLS
    const char* original_t = gettext(original);
#else
    const char* original_t = original;
#endif

    /*
    // print
    for (int n=0;; n+=4)
    {
        std::cout << original_t[n] << " (" << (unsigned int)(original_t[n]) << "), "
                  << original_t[n+1] << " (" << (unsigned int)(original_t[n+1]) << "), "
                  << original_t[n+2] << " (" << (unsigned int)(original_t[n+2]) << "), "
                  << original_t[n+3] << " (" << (unsigned int)(original_t[n+3]) << ")\n";
        
        if (original_t[n] | original_t[n+1] | original_t[n+2] | original_t[n+3] == 0) break;
    }
    */
    
    //wchar_t* out_ptr = utf8_to_wchar_t(original_t);
    wchar_t* out_ptr = (wchar_t*)original_t;
    
    /*
    if (out_ptr == NULL)
    {
        std::cerr << "  ERROR in w_gettext! could not be converted to wchar_t.\n";
    }
    */
    std::wcout << L"  translation : " << out_ptr << std::endl;
    //std::wcout << L"  translation : " << irr::core::stringc(out_ptr).c_str() << std::endl;
    //fprintf(stdout, "translation is '%s'.\n",  irr::core::stringc(out_ptr).c_str());
    
    return out_ptr;

    
    /*
    int index = 0;
    for (const char* c=original_t; *c != 0; c++)
    {
        out_buffer[index] = (wchar_t)(unsigned char)*c;
        index++;
    }
    out_buffer[index] = 0;
                                    
    //mbstowcs(out_buffer, original_t, BUFFER_SIZE);

    return out_buffer;
     */
}
