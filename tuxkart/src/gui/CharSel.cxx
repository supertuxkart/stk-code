//  $Id: CharSel.cxx,v 1.5 2004/08/08 20:27:00 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <set>
#include "Loader.h"
#include "CharSel.h"
#include "tuxkart.h"
#include "WidgetSet.h"

static bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(lhs.length() - rhs.length(), rhs.length(), rhs) == 0;
}

CharSel::CharSel()
{
        std::set<std::string> result;
        loader->listFiles(result, "data/");

        // Findout which characters are available and load them
        for(std::set<std::string>::iterator i = result.begin(); i != result.end(); ++i)
        {
                if (has_suffix(*i, ".tkkf"))
                {
                        characters.push_back(KartProperties("data/" + *i));
                }
        }

        menu_id = widgetSet -> varray(0);

        for(Characters::size_type i = 0; i < characters.size(); ++i)
        {
                widgetSet -> start(menu_id, characters[i].name.c_str(),   GUI_SML, i, 0);
        }
	widgetSet -> space(menu_id);
	widgetSet -> space(menu_id);
	
	widgetSet -> layout(menu_id, 0, -1);
}

CharSel::~CharSel()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void CharSel::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void CharSel::select()
{
	int token = widgetSet -> token (widgetSet -> click());
        
        if (token >= 0 && token < static_cast<int>(characters.size()))
                kart_props = characters[token];
        
        guiStack.push_back(GUIS_TRACKSEL); 
}

void CharSel::keybd(const SDL_keysym& key)
{
	switch ( key.sym )
	{
	case SDLK_LEFT:    
	case SDLK_RIGHT:    
	case SDLK_UP:    
	case SDLK_DOWN:
		widgetSet -> pulse(widgetSet -> cursor(menu_id, key.sym), 1.2f);
		break;
		
	case SDLK_RETURN: select(); break;
	
	case SDLK_ESCAPE:
		guiStack.pop_back();
		
	default: break;
	}
}

void CharSel::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void CharSel::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

