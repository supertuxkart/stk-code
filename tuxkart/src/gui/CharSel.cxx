//  $Id: CharSel.cxx,v 1.9 2004/08/17 18:55:23 straver Exp $
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
#include <iostream>
#include "Loader.h"
#include "CharSel.h"
#include "tuxkart.h"
#include "KartManager.h"
#include "WidgetSet.h"

CharSel::CharSel()
{
        current_kart = -1;
        switch_to_character(0);

        context = new ssgContext;

        menu_id = widgetSet -> varray(0);

        for(KartManager::Data::size_type i = 0; i < kart_manager.karts.size(); ++i)
        {
          widgetSet -> start(menu_id, kart_manager.karts[i].name.c_str(),   GUI_SML, i, 0);
        }
	widgetSet -> space(menu_id);
	widgetSet -> space(menu_id);
	
	widgetSet -> layout(menu_id, 0, -1);

        clock = 0;
}

CharSel::~CharSel()
{
	widgetSet -> delete_widget(menu_id) ;
}

void CharSel::switch_to_character(int n)
{
        if (current_kart != n && n >= 0 && n < int(kart_manager.karts.size()))
        {
                current_kart = n;
                kart = new ssgTransform;
                kart->ref();
                ssgEntity* kartentity = ssgLoadAC ( kart_manager.karts[n].model_file.c_str(), loader ) ;
                kartentity->ref();
                kart->addKid(kartentity);
        }
}
	
void CharSel::update(float dt)
{
        clock += dt * 40.0f;

        widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;

        switch_to_character(widgetSet->token(widgetSet->click()));

        if (kart)
        {
                glClear(GL_DEPTH_BUFFER_BIT);
                //context -> ref();
                context -> setFOV ( 75.0f, 0.0f ) ;
                context -> setNearFar ( 0.05f, 1000.0f ) ;

                sgCoord cam_pos;
                sgSetCoord(&cam_pos, 0, 0, 0, 0, 0, 0);
                context -> setCamera ( &cam_pos ) ;

		glEnable (GL_DEPTH_TEST);
                context -> makeCurrent () ;
                sgCoord trans;
                sgSetCoord(&trans, 0, 2, -.2, clock, 0, 0);
                kart->setTransform (&trans) ;
                ssgCullAndDraw ( kart ) ;
		glDisable (GL_DEPTH_TEST);
                //delete context;
        }
}

void CharSel::select()
{
	int token = widgetSet -> token (widgetSet -> click());
        
        if (token >= 0 && token < static_cast<int>(kart_manager.karts.size()))
                kart_props = kart_manager.karts[token];
        
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

