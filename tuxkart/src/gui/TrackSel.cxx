//  $Id: TrackSel.cxx,v 1.1 2004/08/05 22:53:56 jamesgregory Exp $
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

#include "TrackSel.h"
#include "tuxkart.h"
#include "WidgetSet.h"

TrackSel::TrackSel()
{
	menu_id = widgetSet -> varray(0);
	widgetSet -> start(menu_id, "Track 1",  GUI_SML, 0, 0);
	widgetSet -> state(menu_id, "Track 2",  GUI_SML, 1, 0);
	widgetSet -> state(menu_id, "Track 3",  GUI_SML, 2, 0);
	widgetSet -> space(menu_id);
	widgetSet -> space(menu_id);
	
	widgetSet -> layout(menu_id, 0, -1);
}

TrackSel::~TrackSel()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void TrackSel::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void TrackSel::select()
{
	int token = widgetSet -> token ( widgetSet -> click() );
	switch ( token )
	{
	//FIXME
	default: switchToGame (3, 0, 0, token, 1) ; break ;
	}
}

void TrackSel::cursor(SDLKey key)
{
	widgetSet -> pulse(widgetSet -> cursor(menu_id, key), 1.2f);
}

void TrackSel::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void TrackSel::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

