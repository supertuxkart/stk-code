//  $Id: ConfigSound.cxx,v 1.2 2005/05/27 10:25:52 joh Exp $
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

#include "config_sound.hpp"
#include "widget_set.hpp"
#include "config.hpp"
#include "menu_manager.hpp"

enum WidgetTokens {
  WTOK_MUSIC_TOGGLE,
  WTOK_SFX_TOGGLE,
};

ConfigSound::ConfigSound()
{
	menu_id = widgetSet -> vstack(0);
	widgetSet -> label(menu_id, "Sound Settings", GUI_LRG, GUI_ALL, 0, 0);

	int va = widgetSet -> varray(menu_id);
	music_menu_id = widgetSet -> start(va, "Turn on music",  GUI_MED, WTOK_MUSIC_TOGGLE, 0);
	sfx_menu_id = widgetSet -> start(va, "Turn on sound effects",  GUI_MED, WTOK_SFX_TOGGLE, 0);

	widgetSet -> layout(menu_id, 0, 0);

    if(config->music) widgetSet->set_label(music_menu_id, "Turn off music");
    if(config->sfx) widgetSet->set_label(sfx_menu_id, "Turn off sound effects");
}

ConfigSound::~ConfigSound()
{
	widgetSet -> delete_widget(menu_id) ;
}

void ConfigSound::update(float dt)
{
	widgetSet -> timer(menu_id, dt) ;
    // This menu can be triggered from the game, when it is paused
    // so we have to check it and draw it as in pause
    if(widgetSet -> get_paused()) widgetSet -> blank() ;

	widgetSet -> paint(menu_id) ;
}

void ConfigSound::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
    case WTOK_MUSIC_TOGGLE:
        if(config->music)
        {
          config->music = false;
          widgetSet->set_label(music_menu_id, "Turn on music");
        }
        else
        {
          config->music = true;
          widgetSet->set_label(music_menu_id, "Turn off music");
        }
        break;
    case WTOK_SFX_TOGGLE:
        if(config->sfx)
        {
          config->sfx = false;
          widgetSet->set_label(sfx_menu_id, "Turn on sound effects");
        }
        else
        {
          config->sfx = true;
          widgetSet->set_label(sfx_menu_id, "Turn off sound effects");
        }
        break;
	default: break;
	}
}



