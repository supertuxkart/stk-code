// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Eduardo Hernandez Munoz
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
#include "sound.hpp"
#include "grand_prix_ending.hpp"
#include "kart_manager.hpp"
#include "preprocessor.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "start_screen.hpp"
#include "screen_manager.hpp"
#include "config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"

GrandPrixEnd::GrandPrixEnd()
  : kart(0)
{
    // for some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
	context = new ssgContext;
    oldContext->makeCurrent();

	menu_id = widgetSet -> vstack(0);

    int highest = 0;
    //FIXME: We go from the back to the front because the players are in the
    //back and in case of a tie they will win against the AI, *but* if it's
    //player vs. player, the player who goes first would win.
    for(int i = race_manager->getNumKarts() - 1; i > 0; --i)
        if(race_manager->getKartScore(i) > race_manager->getKartScore(highest))
            highest = i;

    const std::string kart_name = race_manager->getKartName(highest);
    const KartProperties* winning_kart = kart_manager->getKart(kart_name);
    
    std::stringstream output;
    //FIXME: when a long string is used the output gets corrupted.
    output << "The winner is " << winning_kart->getName() << "!";
    widgetSet -> label(menu_id, output.str().c_str(), GUI_LRG, GUI_ALL, 0, 0);
    widgetSet -> space(menu_id);
    
    widgetSet -> label(menu_id, "Back to the main menu", GUI_LRG, GUI_ALL, 0, 0);
    
    widgetSet -> layout(menu_id, 0, 1);

    kart = new ssgTransform;
    kart->ref();
    ssgEntity* kartentity = winning_kart->getModel();
    kart->addKid(kartentity);
    preProcessObj ( kart, 0 );

    sound->playSfx(SOUND_WINNER);

	clock = 0;
}

GrandPrixEnd::~GrandPrixEnd()
{
	widgetSet -> delete_widget(menu_id) ;
    ssgDeRefDelete(kart);

    delete context;

    //The next line prevents textures like the background of the main menu from
    //going white after finishing the grandprix
    // FIXME: I think this is not necessary anymore after the
    //        texture bug fix (r733) - but I can't currently test this.
    loader->shared_textures.removeAll();
}

void GrandPrixEnd::update(float dt)
{
	clock += dt * 40.0f;

    glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    BaseGUI::update(dt);

    ssgContext* oldContext = ssgGetCurrentContext();
    context -> makeCurrent();

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

void GrandPrixEnd::select()
{
    startScreen = new StartScreen();
    screen_manager->setScreen(startScreen);
    menu_manager->switchToMainMenu();
}
