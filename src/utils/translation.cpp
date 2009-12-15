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

#include <locale.h>

#include "io/file_manager.hpp"

Translations* translations=NULL;

Translations::Translations() { 
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
    //bind_textdomain_codeset(PACKAGE, "UTF-8");
    bind_textdomain_codeset(PACKAGE, "iso-8859-1");
    textdomain (PACKAGE);
#endif
        
}   // Translations

const int BUFFER_SIZE = 512;
wchar_t out_buffer[BUFFER_SIZE];

wchar_t* w_gettext(const char* original)
{
#if ENABLE_NLS
    const char* original_t = gettext(original);
#else
    const char* original_t = original;
#endif
    
    int index = 0;
    for (const char* c=original_t; *c != 0; c++)
    {
        out_buffer[index] = (wchar_t)(unsigned char)*c;
        index++;
    }
    out_buffer[index] = 0;
                                    
    //mbstowcs(out_buffer, original_t, BUFFER_SIZE);

    return out_buffer;
}
