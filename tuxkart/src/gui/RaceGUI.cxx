//  $Id: RaceGUI.cxx,v 1.6 2004/08/08 16:35:26 jamesgregory Exp $
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

#include "RaceGUI.h"
#include "status.h"
#include "tuxkart.h"
#include "WidgetSet.h"
#include "Driver.h"

RaceGUI::RaceGUI()
{
	if ((fps_id = widgetSet -> count(0, 1000, GUI_SML, GUI_SE)))
		widgetSet -> layout(fps_id, -1, 1);
}

RaceGUI::~RaceGUI()
{
	widgetSet -> delete_widget(fps_id) ;
}
	
void RaceGUI::update(float dt)
{
	widgetSet -> timer(fps_id, dt) ;
		
	if ( getShowFPS() )
		drawFPS ();
}

void RaceGUI::keybd(const SDL_keysym& key)
{
	static int isWireframe = FALSE ;
	
	if (key.mod & KMOD_CTRL)
	{
      	((PlayerKartDriver*)kart[0])->incomingKeystroke ( key ) ;
      	return;
	}
    
	switch ( key.sym )
	{
	case SDLK_F12: fpsToggle() ; return;
	
	case SDLK_w : 
		if ( isWireframe )
			glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
		else
      		glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
      	isWireframe = ! isWireframe ;
		return ;
	
	#ifdef DEBUG
	case SDLK_z : stToggle () ; return ;
	#endif
	
	case SDLK_ESCAPE:
		widgetSet -> tgl_paused();
		guiStack.push_back(GUIS_RACEMENU);
		break;
		
	default: break;
	}
}

void RaceGUI::drawFPS ()
{
  static int fpsCounter;
  static int fpsSave = 0;
  static int fpsTimer = SDL_GetTicks();
  
  int now = SDL_GetTicks();

  if (now - fpsTimer > 1000)
    {
      fpsSave = fpsCounter;
      fpsCounter = 0;
      fpsTimer = now;
	
	widgetSet -> set_count(fps_id, fpsSave);
    }
  else
    ++fpsCounter;
    
  widgetSet -> paint(fps_id) ;
}


