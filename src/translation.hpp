//  $Id: transation.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef TRANSLATION_H
#define TRANSLATION_H

#ifdef HAS_GETTEXT
#  include <libintl.h>
#  define _(String) gettext(String)
#  define gettext_noop(String) String
#  define N_(String) gettext_noop (String)
// libintl defines its own fprintf, which doesn't work for me :(
#  if defined(WIN32) && !defined(__CYGWIN__)
#    undef fprintf
#  endif
#else
#  define _(String) (String)
#  define gettext_noop(String) String
#  define N_(String) String
#endif

// This length is used for all translated error messages.
#define MAX_ERROR_MESSAGE_LENGTH 160

// This length is used for all normal messages
#define MAX_MESSAGE_LENGTH        80
class Translations
{
public:
    Translations();
};

extern Translations* translations;
#endif
/* EOF */
