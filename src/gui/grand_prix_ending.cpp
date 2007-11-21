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

#include <SDL/SDL.h>

#include "loader.hpp"
#include "sound_manager.hpp"
#include "grand_prix_ending.hpp"
#include "kart_properties_manager.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "game_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"
#include "translation.hpp"
#include "kart.hpp"
#include "world.hpp"
#include "scene.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_QUIT,

    WTOK_FIRSTKART
};


GrandPrixEnd::GrandPrixEnd()
        : m_kart(0)
{
    // for some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    int highest = 0;
    //FIXME: We go from the back to the front because the players are in the
    //back and in case of a tie they will win against the AI, *but* if it's
    //player vs. player, the player who goes first would win.
    for(int i = race_manager->getNumKarts() - 1; i > 0; --i)
        if(race_manager->getKartScore(i) > race_manager->getKartScore(highest))
            highest = i;

    const std::string KART_NAME = race_manager->getKartName(highest);
    const KartProperties* WINNING_KART = kart_properties_manager->getKart(KART_NAME);

    static char output[MAX_MESSAGE_LENGTH];
    snprintf(output, sizeof(output),
             _("The winner is %s!"),WINNING_KART->getName().c_str());
    widget_manager->add_wgt( WTOK_TITLE, 60, 10);
    widget_manager->show_wgt_rect(WTOK_TITLE);
    widget_manager->show_wgt_text(WTOK_TITLE);
    widget_manager->set_wgt_text(WTOK_TITLE, output);
    widget_manager->set_wgt_text_size(WTOK_TITLE, WGT_FNT_LRG);
    widget_manager->break_line();


    const unsigned int MAX_STR_LEN = 60;
    const unsigned int NUM_KARTS = world->getNumKarts();

    Kart *kart;
	int *scores   = new int[NUM_KARTS];
    int *position = new int[NUM_KARTS];
    for( unsigned int i = 0; i < NUM_KARTS; ++i )
    {
        kart = world->getKart(i);
        position[i] = i;
        scores[i]   = race_manager->getKartScore(i);
    }

    //Bubblesort
    bool sorted;
    do
    {
        sorted = true;
        for( unsigned int i = 0; i < NUM_KARTS - 1; ++i )
        {
            if( scores[i] < scores[i+1] )
            {
                int tmp_score[2];

                tmp_score[0] = position[i];
                tmp_score[1] = scores[i];

                position[i] = position[i+1];
                scores[i] = scores[i+1];

                position[i+1] = tmp_score[0];
                scores[i+1] = tmp_score[1];

                sorted = false;
            }
        }
    } while(!sorted);

    m_score = new char[MAX_STR_LEN*NUM_KARTS];

    for(unsigned int i=0; i < NUM_KARTS; ++i)
    {
        kart = world->getKart(position[i]);
        sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %d",
                i + 1, kart->getName().c_str(), scores[i]);

        widget_manager->add_wgt(WTOK_FIRSTKART + i, 40, 5);
        widget_manager->show_wgt_rect(WTOK_FIRSTKART + i);
        widget_manager->show_wgt_text(WTOK_FIRSTKART + i);
        widget_manager->set_wgt_text(WTOK_FIRSTKART + i,
            (char*)(m_score + MAX_STR_LEN * i));
        widget_manager->set_wgt_text_size(WTOK_FIRSTKART + i, WGT_FNT_SML);
    widget_manager->break_line();
    }
    delete []scores;
    delete []position;

    widget_manager->add_wgt(WTOK_QUIT, 50, 7);
    widget_manager->activate_wgt(WTOK_QUIT);
    widget_manager->show_wgt_rect(WTOK_QUIT);
    widget_manager->show_wgt_text(WTOK_QUIT);
    widget_manager->set_wgt_text(WTOK_QUIT, _("Back to the main menu"));

    widget_manager->layout(WGT_AREA_TOP);

    m_kart = new ssgTransform;
    m_kart->ref();
    ssgEntity* kartentity = WINNING_KART->getModel();
    m_kart->addKid(kartentity);

    sound_manager->playSfx(SOUND_WINNER);

    m_clock = 0;

    //FIXME: this is taken from RaceMode::exit_race,
    //this should be organized better.
    delete world;
    world = 0;
    scene->clear();
    race_manager->m_active_race = false;

}

//-----------------------------------------------------------------------------
GrandPrixEnd::~GrandPrixEnd()
{
    widget_manager->delete_wgts();
    ssgDeRefDelete(m_kart);

    delete m_context;
    delete[] m_score;

    //The next line prevents textures like the background of the main menu from
    //going white after finishing the grandprix
    // FIXME: I think this is not necessary anymore after the
    //        texture bug fix (r733) - but I can't currently test this.
    loader->shared_textures.removeAll();
}

//-----------------------------------------------------------------------------

void GrandPrixEnd::update(float dt)
{
    m_clock += dt * 40.0f;

    glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ssgContext* oldContext = ssgGetCurrentContext();
    m_context -> makeCurrent();

    // FIXME: A bit hackish...
    glViewport ( 0, 0, 800, 320);

    m_context -> setFOV ( 45.0f, 45.0f * 320.0f/800.0f ) ;
    m_context -> setNearFar ( 0.05f, 1000.0f ) ;

    sgCoord cam_pos;
    sgSetCoord(&cam_pos, 0, 0, 0, 0, 0, 0);
    m_context -> setCamera ( &cam_pos ) ;

    glEnable (GL_DEPTH_TEST);
    sgCoord trans;
    sgSetCoord(&trans, 0, 3, -.4f, m_clock, 0, 0);
    m_kart->setTransform (&trans) ;
    //glShadeModel(GL_SMOOTH);
    ssgCullAndDraw ( m_kart ) ;
    glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;

    glDisable (GL_DEPTH_TEST);
    oldContext->makeCurrent();
    BaseGUI::update(dt);

    SDL_GL_SwapBuffers();
}

//-----------------------------------------------------------------------------
void GrandPrixEnd::select()
{
    menu_manager->switchToMainMenu();
}
