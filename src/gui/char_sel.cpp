//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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
#include "kart_properties_manager.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_RACER0,
    WTOK_RACER1,
    WTOK_RACER2,
    WTOK_RACER3,
    WTOK_RACER4,
    WTOK_RACER5,
    WTOK_RACER6,
    WTOK_RACER7,
    WTOK_RACER8,
    WTOK_RACER9,

    WTOK_TITLE,
    WTOK_NAME,

    WTOK_QUIT
};

CharSel::CharSel(int whichPlayer)
        : m_kart(0), m_player_index(whichPlayer)
{
    // For some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    widget_manager->set_initial_activation_state(false);
    widget_manager->add_wgt( WTOK_TITLE, 60, 10);
    widget_manager->show_wgt_rect( WTOK_TITLE );
    char HEADING[MAX_MESSAGE_LENGTH];
    snprintf(HEADING, sizeof(HEADING), _("Player %d, choose a driver"),
             m_player_index + 1);
    widget_manager->set_wgt_text( WTOK_TITLE, HEADING);
    widget_manager->set_wgt_text_size( WTOK_TITLE, WGT_FNT_LRG);
    widget_manager->show_wgt_text( WTOK_TITLE );
    widget_manager->break_line();

    widget_manager->add_wgt( WidgetManager::WGT_NONE, 100, 2);
    widget_manager->break_line();

    //FIXME: this supports only a static number of karts
    for (unsigned int i = 0; i < kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* kp= kart_properties_manager->getKartById(i);
        Material *m = material_manager->getMaterial(kp->getIconFile());
        widget_manager->add_wgt( WTOK_RACER0 + i, 10, 13);
        widget_manager->show_wgt_rect( WTOK_RACER0 + i);
        widget_manager->set_wgt_color( WTOK_RACER0 + i, WGT_GRAY);
        widget_manager->set_wgt_texture( WTOK_RACER0 + i, m->getState()->getTextureHandle());
        widget_manager->show_wgt_texture( WTOK_RACER0 + i );
        widget_manager->activate_wgt( WTOK_RACER0 + i );
    }

    widget_manager->break_line();
    widget_manager->add_wgt( WidgetManager::WGT_NONE, 100, 2);
    widget_manager->break_line();

    //FIXME: the widget should check if the dimensions > 100
    widget_manager->add_wgt( WTOK_NAME, 30, 7);
    widget_manager->show_wgt_rect( WTOK_NAME );
    widget_manager->show_wgt_text( WTOK_NAME );

    //FIXME: widget_manager says that token -1 is already in use
    widget_manager->layout(WGT_AREA_TOP);


    m_current_kart = -1;

    const int LAST_KART = user_config->m_player[m_player_index].getLastKartId();
    if( LAST_KART != -1 )
    {
        widget_manager->set_selected_wgt(WTOK_RACER0 + LAST_KART);
        switchCharacter(LAST_KART);
    }
    else
    {
        switchCharacter(0);
    }


    m_clock = 0;
    //test

}

//-----------------------------------------------------------------------------
CharSel::~CharSel()
{
    widget_manager->reset();
    ssgDeRefDelete(m_kart);

    delete m_context;
}

//-----------------------------------------------------------------------------
void CharSel::switchCharacter(int n)
{
    const KartProperties* kp= kart_properties_manager->getKartById(n);
    if (m_current_kart != n && kp != NULL)
    {
        widget_manager->set_wgt_text( WTOK_NAME, kp->getName().c_str());
        //FIXME: maybe this should be renamed from WGT_SCROLL_* to WGT_POS_*
        widget_manager->set_wgt_x_scroll_pos( WTOK_NAME, WGT_SCROLL_CENTER );

        m_current_kart = n;
        ssgDeRefDelete(m_kart);
        m_kart = new ssgTransform;
        m_kart->ref();
        ssgEntity* kartentity = kp->getModel();

        m_kart->addKid(kartentity);
    }
}

//-----------------------------------------------------------------------------
void CharSel::update(float dt)
{
    m_clock += dt * 40.0f;
    BaseGUI::update(dt);

    switchCharacter(widget_manager->get_selected_wgt() - WTOK_RACER0);

    if (m_kart != NULL)
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

        // Puts the character in the center. Scaling is done by
        // applying a big camera FOV.
        int w = user_config->m_width;
        int h = user_config->m_height;
        glViewport ( 0, 0, w, h);

        m_context -> setFOV ( 65.0f, 65.0f * h/w ) ;
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
    }
}

//----------------------------------------------------------------------------
void CharSel::select()
{
    const int TOKEN = widget_manager->get_selected_wgt() - WTOK_RACER0;
    const KartProperties* KP = kart_properties_manager->getKartById(TOKEN);
    if (KP != NULL)
    {
        race_manager->setPlayerKart(m_player_index, KP->getIdent());
        user_config->m_player[m_player_index].setLastKartId(TOKEN);
    }

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
        race_manager->start();
}
