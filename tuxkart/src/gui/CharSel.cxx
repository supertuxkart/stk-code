//  $Id: CharSel.cxx,v 1.26 2004/09/05 20:09:59 matzebraun Exp $
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
#include "sdldrv.h"
#include "Loader.h"
#include "CharSel.h"
#include "tuxkart.h"
#include "Loader.h"
#include "material.h"
#include "KartManager.h"
#include "preprocessor.h"
#include "WidgetSet.h"
#include "RaceManager.h"
#include "StartScreen.h"

CharSel::CharSel(int whichPlayer)
  : kart(0), playerIndex(whichPlayer)
{
	current_kart = -1;
	switch_to_character(3);

        // for some strange reasons plib calls makeCurrent() in ssgContext
        // constructor, so we have to save the old one here and restore it
        ssgContext* oldContext = ssgGetCurrentContext();
	context = new ssgContext;
        oldContext->makeCurrent();

	menu_id = widgetSet -> vstack(0);
	char output[60];
	sprintf(output, "Player %d, choose your character", playerIndex + 1);
	widgetSet -> label(menu_id, output, GUI_LRG, GUI_ALL, 0, 0);
	widgetSet -> space(menu_id);

	int ha = widgetSet -> harray(menu_id);
	widgetSet -> filler(ha);
	int va = widgetSet -> varray(ha);

	int icon_size = 64;

	int row1 = widgetSet -> harray(va);
	for(KartManager::KartPropertiesVector::size_type i = 0;
            i < kart_manager.karts.size(); ++i)
	{
		//widgetSet ->state(row2, kart_manager.karts[i].name.c_str(), GUI_MED, i, 0);
		int c = widgetSet -> image(row1,
                    loader->getPath(
                      "images/" + kart_manager.karts[i]->icon_file).c_str(),
                    icon_size, icon_size);
		widgetSet -> activate_widget(c, i, 0);
	}

	if (0)
	{
		int row2 = widgetSet -> harray(va);
		for(KartManager::KartPropertiesVector::size_type i = 0;
                    i < kart_manager.karts.size()/2; ++i)
		{
			//widgetSet ->state(row1, kart_manager.karts[i].name.c_str(), GUI_MED, i, 0);
			// FIXME: images needs to be 'clickable'
			int c = widgetSet -> image(row1,
                            loader->getPath(
                              "images/" + kart_manager.karts[i]->icon_file).c_str(),
                            icon_size, icon_size);
			widgetSet -> activate_widget(c, i, 0);
		}
		for(KartManager::KartPropertiesVector::size_type i =
                    kart_manager.karts.size()/2; i < kart_manager.karts.size(); ++i)
		{
			//widgetSet ->state(row2, kart_manager.karts[i].name.c_str(), GUI_MED, i, 0);
			int c = widgetSet -> image(row2,
                            loader->getPath("images/" + kart_manager.karts[i]->icon_file).c_str(), icon_size, icon_size);
			widgetSet -> activate_widget(c, i, 0);
		}
	}


	widgetSet -> filler(ha);

	widgetSet -> layout(menu_id, 0, 1);

	clock = 0;
	//test

}

CharSel::~CharSel()
{
	widgetSet -> delete_widget(menu_id) ;
        ssgDeRefDelete(kart);

        delete context;
}

void CharSel::switch_to_character(int n)
{
	if (current_kart != n && n >= 0 && n < int(kart_manager.karts.size()))
	{
		current_kart = n;
                ssgDeRefDelete(kart);
		kart = new ssgTransform;
		kart->ref();
		ssgEntity* kartentity = kart_manager.karts[n]->getModel();

		kart->addKid(kartentity);

		preProcessObj ( kart, 0 );
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
                ssgContext* oldContext = ssgGetCurrentContext();
                context -> makeCurrent();                                   
            
		glClear(GL_DEPTH_BUFFER_BIT);
		// FIXME: A bit hackish...
		glViewport ( 0, 0, 800, 320);

		context -> setFOV ( 45.0f, 45.0f * 320.0f/800.0f ) ;
		context -> setNearFar ( 0.05f, 1000.0f ) ;

		sgCoord cam_pos;
		sgSetCoord(&cam_pos, 0, 0, 0, 0, 0, 0);
		context -> setCamera ( &cam_pos ) ;

		glEnable (GL_DEPTH_TEST);
		sgCoord trans;
		sgSetCoord(&trans, 0, 3, -.4, clock, 0, 0);
		kart->setTransform (&trans) ;
		//glShadeModel(GL_SMOOTH);
		ssgCullAndDraw ( kart ) ;

		glViewport ( 0, 0, getScreenWidth(), getScreenHeight() ) ;

		glDisable (GL_DEPTH_TEST);
                oldContext->makeCurrent();
	}
}

void CharSel::select()
{
	int token = widgetSet -> token (widgetSet -> click());

	if (token >= 0 && token < static_cast<int>(kart_manager.karts.size()))
          RaceManager::instance()->setPlayerKart(playerIndex,
              kart_manager.getKartById(token)->ident);

	if (RaceManager::instance()->getNumPlayers() > 1)
	{
		if (guiStack.back() == GUIS_CHARSEL)
		{
			guiStack.push_back(GUIS_CHARSELP2); 
			return;
		}
			
		if (RaceManager::instance()->getNumPlayers() > 2)
		{
			if (guiStack.back() == GUIS_CHARSELP2)
			{
				guiStack.push_back(GUIS_CHARSELP3); 
				return;
			}

			if (RaceManager::instance()->getNumPlayers() > 3)
			{
				if (guiStack.back() == GUIS_CHARSELP3)
				{
					guiStack.push_back(GUIS_CHARSELP4); 
					return;
				}
			}	
		}	
	}
	
        if (RaceManager::instance()->getRaceMode() != RaceSetup::RM_GRAND_PRIX)
          guiStack.push_back(GUIS_TRACKSEL); 
        else
          startScreen->switchToGame();
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

