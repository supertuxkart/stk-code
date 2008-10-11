// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Eduardo Hernandez Munoz
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "grand_prix_ending.hpp"

#include <sstream>
#include <string>

#include <SDL/SDL.h>

#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "loader.hpp"
#include "kart_properties_manager.hpp"
#include "unlock_manager.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"
#include "translation.hpp"
#include "kart.hpp"
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

// ----------------------------------------------------------------------------
GrandPrixEnd::GrandPrixEnd()
        : m_kart(0)
{
    m_winner_sound = sfx_manager->newSFX(SFXManager::SOUND_WINNER);
    // for some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    const unsigned int MAX_STR_LEN = 60;
    const unsigned int NUM_KARTS = race_manager->getNumKarts();

    int *scores   = new int[NUM_KARTS];
    int *position = new int[NUM_KARTS];
    double *race_time = new double[NUM_KARTS];
    // Ignore the first kart if it's a follow-the-leader race.
    int start=(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER) ? 1 : 0;
    for( unsigned int kart_id = start; kart_id < NUM_KARTS; ++kart_id )
    {
        position[kart_id]  = kart_id;
        scores[kart_id]    = race_manager->getKartScore(kart_id);
        race_time[kart_id] = race_manager->getOverallTime(kart_id);
    }

    //Bubblesort
    bool sorted;
    do
    {
        sorted = true;
        for( unsigned int i = start; i < NUM_KARTS - 1; ++i )
        {
            if( scores[i] < scores[i+1] || (scores[i] == scores[i+1] 
              && race_time[i] > race_time[i+1]))
            {
                int tmp_score[2];
                double tmp_time;

                tmp_score[0] = position[i];
                tmp_score[1] = scores[i];
                tmp_time = race_time[i];

                position[i] = position[i+1];
                scores[i] = scores[i+1];
                race_time[i] = race_time[i+1];

                position[i+1] = tmp_score[0];
                scores[i+1] = tmp_score[1];
                race_time[i+1] = tmp_time;

                sorted = false;
            }
        }
    } while(!sorted);
    
    static char output[MAX_MESSAGE_LENGTH];
    snprintf(output, sizeof(output),
        _("The winner is %s!"),race_manager->getKartName(position[start]).c_str());
    widget_manager->addWgt( WTOK_TITLE, 60, 10);
    widget_manager->showWgtRect(WTOK_TITLE);
    widget_manager->showWgtText(WTOK_TITLE);
    widget_manager->setWgtText(WTOK_TITLE, output);
    widget_manager->setWgtTextSize(WTOK_TITLE, WGT_FNT_LRG);
    widget_manager->breakLine();

    m_score = new char[MAX_STR_LEN*NUM_KARTS];

    for(unsigned int i=start; i < NUM_KARTS; ++i)
    {
        char sTime[20];sTime[0]=0;
        if(race_manager->getWorld()->getClockMode()==CHRONO)
            TimeToString(race_time[i], sTime);
        sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %d %s",
            i + 1-start, race_manager->getKartName(position[i]).c_str(), scores[i], sTime );

        widget_manager->addWgt(WTOK_FIRSTKART + i, 40, 5);
        widget_manager->showWgtRect(WTOK_FIRSTKART + i);
        widget_manager->showWgtText(WTOK_FIRSTKART + i);
        widget_manager->setWgtText(WTOK_FIRSTKART + i,
            (char*)(m_score + MAX_STR_LEN * i));
        widget_manager->setWgtTextSize(WTOK_FIRSTKART + i, WGT_FNT_SML);
        widget_manager->breakLine();
    }
    const std::string KART_NAME = race_manager->getKartName(position[start]);
    const KartProperties* WINNING_KART = kart_properties_manager->getKart(KART_NAME);
    delete []scores;
    delete []position;
    delete []race_time;

    widget_manager->addWgt(WTOK_QUIT, 50, 7);
    widget_manager->activateWgt(WTOK_QUIT);
    widget_manager->showWgtRect(WTOK_QUIT);
    widget_manager->showWgtText(WTOK_QUIT);
    if(unlock_manager->getUnlockedFeatures().size()>0)
    {
        widget_manager->setWgtText(WTOK_QUIT, _("Continue"));
    }
    else
    {
        widget_manager->setWgtText(WTOK_QUIT, _("Back to the main menu"));
    }

    widget_manager->layout(WGT_AREA_TOP);

    m_kart = new ssgTransform;
    m_kart->ref();
    ssgEntity* kartentity = WINNING_KART->getModel();
    m_kart->addKid(kartentity);

    m_winner_sound->play();

    m_clock = 0;

    //FIXME: this is taken from RaceMode::exit_race,
    //this should be organized better.
    scene->clear();
    RaceManager::setWorld(NULL);
    race_manager->m_active_race = false;

}

//-----------------------------------------------------------------------------
GrandPrixEnd::~GrandPrixEnd()
{
    sfx_manager->deleteSFX(m_winner_sound);
    widget_manager->reset();
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
}

//-----------------------------------------------------------------------------
void GrandPrixEnd::select()
{
    // If a new feature was unlocked, display the new feature first
    // before returning to the main menu
    if(unlock_manager->getUnlockedFeatures().size()>0)
    {
        // This removes this menu from the stack, and adds the main menu. 
        // Then we push the new feature menu on top, so that it will be
        // displayed next, and on return the main menu is shown.
        menu_manager->switchToMainMenu();
        menu_manager->pushMenu(MENUID_UNLOCKED_FEATURE);
        return;
    }

    menu_manager->switchToMainMenu();
}
