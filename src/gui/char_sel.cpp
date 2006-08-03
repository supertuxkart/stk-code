//  $Id: CharSel.cxx,v 1.7 2005/07/27 08:08:53 joh Exp $
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

#include <sstream>
#include <string>

#include "loader.hpp"
#include "char_sel.hpp"
#include "kart_manager.hpp"
#include "preprocessor.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "start_screen.hpp"
#include "config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"

CharSel::CharSel(int whichPlayer)
  : kart(0), playerIndex(whichPlayer)
{
    // for some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
	context = new ssgContext;
    oldContext->makeCurrent();

	menu_id = widgetSet -> vstack(0);

    std::ostringstream tmp;
    //FIXME: when a long string is used, like the previous one which was
    //"Player #, choose a character" the output gets corrupted.
    tmp << "Player " << playerIndex + 1 << ", choose a driver";
	widgetSet -> label(menu_id, tmp.str().c_str(), GUI_LRG, GUI_ALL, 0, 0);
	widgetSet -> space(menu_id);

	int ha = widgetSet -> harray(menu_id);
	widgetSet -> filler(ha);
	int va = widgetSet -> varray(ha);

	int icon_size = 64;
	/* plib keeps track of textures which are already loaded.
	   Since the icons are deleted, the texture 'cache' needs
	   to be cleared. This problem actually appears only if
	   in the next menu (track) ESC is pressed and control
	   returns to this menu: all icons are simply white then.
	   Clearing the texture cache can either be done with:
	   loader->shared_textures.removeAll(); or
	   loader->endLoad(); which calls removeAll() */
    loader->shared_textures.removeAll();   // remove cached textures

	int row1 = widgetSet -> harray(va);
	
	for (unsigned int i = 0; NULL != kart_manager->getKartById(i); i++)
	{
      	const KartProperties* kp= kart_manager->getKartById(i);
		int c = widgetSet->image(row1, kp->getIconFile(), icon_size, icon_size);
		widgetSet->activate_widget(c, i, 0);

		if (NULL == kart_manager->getKartById(i + 1)) // last in the list
			widgetSet -> set_active(c);
	}
	widgetSet -> filler(ha);
    kart_name_label = widgetSet -> label(menu_id, "No driver choosed", GUI_MED, GUI_ALL, 0, 0);
	widgetSet -> layout(menu_id, 0, 1);

	current_kart = -1;
	switch_to_character(3);

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
	const KartProperties* kp= kart_manager->getKartById(n);
	if (current_kart != n && kp != NULL)
	{
        widgetSet -> set_label(kart_name_label, kp->getName());

		current_kart = n;
        ssgDeRefDelete(kart);
		kart = new ssgTransform;
		kart->ref();
		ssgEntity* kartentity = kp->getModel();

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

	if (kart != NULL)
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
		glViewport ( 0, 0, config->width, config->height ) ;

		glDisable (GL_DEPTH_TEST);
        oldContext->makeCurrent();
	}
}

void CharSel::select()
{
	int token = widgetSet -> token (widgetSet -> click());
	const KartProperties* kp= kart_manager->getKartById(token);
	if (kp != NULL)
          race_manager->setPlayerKart(playerIndex, kp->getIdent());

	if (race_manager->getNumPlayers() > 1)
	{
		if (menu_manager->isCurrentMenu(MENUID_CHARSEL_P1))
		{
			menu_manager->pushMenu(MENUID_CHARSEL_P2);
			return;
		}
    }

	if (race_manager->getNumPlayers() > 2)
	{
		if (menu_manager->isCurrentMenu(MENUID_CHARSEL_P2))
		{
			menu_manager->pushMenu(MENUID_CHARSEL_P3);
			return;
		}
    }

	if (race_manager->getNumPlayers() > 3)
	{
		if (menu_manager->isCurrentMenu(MENUID_CHARSEL_P3))
		{
			menu_manager->pushMenu(MENUID_CHARSEL_P4);
			return;
		}
	}

    if (race_manager->getRaceMode() != RaceSetup::RM_GRAND_PRIX)
        menu_manager->pushMenu(MENUID_TRACKSEL);
    else
        startScreen->switchToGame();
}
