//  $Id$
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
#include <stdexcept>

#include "credits_menu.hpp"
#include "loader.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strdup _strdup
#endif


CreditsMenu::CreditsMenu()
{

    std::string filename;
	StringList credits_text_list;
    try
    {
        filename = loader->getPath("data/CREDITS");
		//FIXME: we should change to c++ - filestreams
        FILE *fd = fopen(filename.c_str(), "r");
        char s[1024];
        char *p;
        while(fgets(s, 1023, fd))
        {
			credits_text_list.push_back(std::string(s));
        }   // while
		fclose(fd);
		fd = NULL;
    }
    catch(std::runtime_error& e)
    {
        printf(_("Couldn't load '%s'\n"),filename.c_str());
        credits_text_list.push_back(_("CREDIT file was not installed properly!!"));
        credits_text_list.push_back(_("Please check 'data/CREDIT'!!"));
    }

    setText(credits_text_list);
}   // CreditsMenu

//-----------------------------------------------------------------------------
CreditsMenu::~CreditsMenu()
{
}   // ~CreditsMenu

/* EOF */
