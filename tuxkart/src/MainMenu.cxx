//  $Id: MainMenu.cxx,v 1.2 2004/08/05 10:21:21 jamesgregory Exp $
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

#include "MainMenu.h"
#include "tuxkart.h"
#include "WidgetSet.h"

MainMenu::MainMenu()
{
	menu_id = widgetSet -> varray(0);
	widgetSet -> start(menu_id, "start",  GUI_MED, 0, 0);
	widgetSet -> state(menu_id, "state1",  GUI_MED, 0, 0);
	widgetSet -> state(menu_id, "state2",  GUI_MED, 0, 0);
	widgetSet -> state(menu_id, "state3",  GUI_MED, 0, 0);
	widgetSet -> state(menu_id, "state4",  GUI_MED, 0, 0);
	
	widgetSet -> layout(menu_id, 0, 0);
}

MainMenu::~MainMenu()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void MainMenu::update(int dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void MainMenu::click(int button, int x, int y)
{
	(void)button;
	(void)x;
	(void)y;
}

void MainMenu::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void MainMenu::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

