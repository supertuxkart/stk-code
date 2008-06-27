//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <sstream>
#include <string>

#include "char_sel.hpp"
#include "kart_properties_manager.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "kart_properties.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_QUIT,
    WTOK_EMPTY_UP,
    WTOK_UP,
    WTOK_EMPTY_DOWN,
    WTOK_DOWN,
    WTOK_EMPTY0 = 10,
    WTOK_NAME0  = 20,
    WTOK_RACER0 = 30
};

CharSel::CharSel(int whichPlayer)
        : m_kart(0), m_player_index(whichPlayer)
{
    // For some strange reasons plib calls makeCurrent() in ssgContext
    // constructor, so we have to save the old one here and restore it
    ssgContext* oldContext = ssgGetCurrentContext();
    m_context = new ssgContext;
    oldContext->makeCurrent();

    // If m_player_index is 0 then this is a single player game or the first
    // player of a multiplayer game so we need to ensure that all karts are available.
    // If m_player_index is less than the number of elements in selected_karts then
    // the user is moving back through the menus and the last value in the vector
    // needs to be made available again.
    if (m_player_index == 0)
        kart_properties_manager->m_selected_karts.clear();

    if (m_player_index < (int)kart_properties_manager->m_selected_karts.size())
        kart_properties_manager->m_selected_karts.pop_back();

    char heading[MAX_MESSAGE_LENGTH];
    snprintf(heading, sizeof(heading), _("Player %d, choose a driver"),
             m_player_index + 1);
    widget_manager->addTitleWgt( WTOK_TITLE, 100, 10, heading );
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 2);
    widget_manager->breakLine();

    m_num_entries = 0;
    for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
    {
        if(kartAvailable(i))
        {
            m_indexAvailKart.push_back(i);
            m_num_entries++;
        }
    }
    if(m_num_entries>7) m_num_entries = 7;

    const int HEIGHT = 10;
    widget_manager->addEmptyWgt(WTOK_EMPTY_UP, computeIndent(0), HEIGHT/2);
    widget_manager->addTextButtonWgt(WTOK_UP, 20, HEIGHT/2, "^");
    widget_manager->breakLine();


    int indx;
    const KartProperties* kp;

    for (unsigned int i = 0; i < m_num_entries; i++)
    {
        indx = m_indexAvailKart[i];
        kp = kart_properties_manager->getKartById(indx);
        widget_manager->addEmptyWgt( WTOK_EMPTY0+i, computeIndent(i), HEIGHT );
        widget_manager->addImgButtonWgt(WTOK_RACER0 + i, 8, HEIGHT,
            kp->getIconFile() );
        widget_manager->addTextButtonWgt(WTOK_NAME0+i, 30, HEIGHT, kp->getName());
        widget_manager->setWgtTextSize(WTOK_NAME0+i, WGT_FNT_SML);
        widget_manager->breakLine();
    }

    widget_manager->addEmptyWgt(WTOK_EMPTY_DOWN, computeIndent(m_num_entries), HEIGHT/2);
    widget_manager->addTextButtonWgt(WTOK_DOWN, 20, HEIGHT/2, "v");
    widget_manager->breakLine();

    widget_manager->breakLine();
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 2);
    widget_manager->breakLine();

    widget_manager->layout(WGT_AREA_RGT);

    m_current_kart = -1;

    const int LAST_KART = user_config->m_player[m_player_index].getLastKartId();
    if( LAST_KART != -1 && kartAvailable(LAST_KART))// is LAST_KART not in vector of selected karts
    {
        m_offset = LAST_KART - m_num_entries/2;
        widget_manager->setSelectedWgt(WTOK_RACER0 + LAST_KART);
        switchCharacter(LAST_KART);
    }
    else
    {
        m_offset = 0;
        switchCharacter(0);
    }
    updateScrollPosition();

    m_clock  = 0;
    //test

}   // CharSel

//-----------------------------------------------------------------------------
CharSel::~CharSel()
{
    widget_manager->reset();
    ssgDeRefDelete(m_kart);

    delete m_context;
}   // ~CharSel

//-----------------------------------------------------------------------------
void CharSel::updateScrollPosition()
{
    for(unsigned int i=0; i<m_num_entries; i++)
    {
        int indx = (i+m_offset) % m_indexAvailKart.size();
        indx     = m_indexAvailKart[indx];

        const KartProperties* kp= kart_properties_manager->getKartById(indx);

        if(unlock_manager->isLocked(kp->getIdent())) continue;
        widget_manager->setWgtText(WTOK_NAME0 + i, kp->getName());
        widget_manager->setWgtTexture(WTOK_RACER0 + i, kp->getIconFile() );
    }   // for i
}   // updateScrollPosition<

//-----------------------------------------------------------------------------
void CharSel::switchCharacter(int n)
{
    const KartProperties* kp= kart_properties_manager->getKartById(m_indexAvailKart[n]);
    if (m_current_kart != n && kp != NULL)
    {
        m_current_kart = n;
        ssgDeRefDelete(m_kart);
        m_kart = new ssgTransform;
        m_kart->ref();
        ssgEntity* kartentity = kp->getModel();

        m_kart->addKid(kartentity);
    }
}   // switchCharacter

//-----------------------------------------------------------------------------
void CharSel::update(float dt)
{
    m_clock += dt * 40.0f;

    if( widget_manager->selectionChanged() )
    {
        int token = widget_manager->getSelectedWgt() - WTOK_RACER0;
        if(token<0 || token>(int)m_num_entries)
            token = widget_manager->getSelectedWgt() - WTOK_NAME0;
        switchCharacter((token+m_offset)%kart_properties_manager->getNumberOfKarts());
    }

    if (m_kart != NULL)
    {
        ssgContext* oldContext = ssgGetCurrentContext();
        m_context -> makeCurrent();

        glClear(GL_DEPTH_BUFFER_BIT);

        // Puts the character in the center. Scaling is done by
        // applying a big camera FOV.
        int w = user_config->m_width;
        int h = user_config->m_height;
        glViewport ( 0, h*1/4, (int)(0.7f*w), (int)(0.7f*h));

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

    widget_manager->update(dt);
}   // update

//----------------------------------------------------------------------------
void CharSel::select()
{
    int wgt = widget_manager->getSelectedWgt();
    if(wgt==WTOK_UP)
    {
        m_offset--;
        if(m_offset < 0) m_offset = m_indexAvailKart.size() - 1;
        updateScrollPosition();
        return;
    }
    if(wgt==WTOK_DOWN)
    {
        m_offset++;
        if( m_offset >= (int)m_indexAvailKart.size() ) m_offset=0;
        updateScrollPosition();
        return;
    }
    int token = widget_manager->getSelectedWgt() - WTOK_RACER0;
    if(token<0 || token>(int)m_num_entries)
    {
        token = widget_manager->getSelectedWgt() - WTOK_NAME0;
    }
    token = (token+m_offset) % m_indexAvailKart.size();
    int kart_id = m_indexAvailKart[token];
    const KartProperties* KP = kart_properties_manager->getKartById(kart_id);
    if (KP != NULL)
    {
        race_manager->setPlayerKart(m_player_index, KP->getIdent());
        user_config->m_player[m_player_index].setLastKartId(kart_id);
        // Add selected kart (token) to selected karts vector so it cannot be
        // selected again
        kart_properties_manager->m_selected_karts.push_back(kart_id);
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

    if (race_manager->getRaceMode() == RaceManager::RM_GRAND_PRIX)
        menu_manager->pushMenu(MENUID_GRANDPRIXSELECT);
    else
        menu_manager->pushMenu(MENUID_TRACKSEL);
}   // select

//----------------------------------------------------------------------------
// Function checks the vector of previously selected karts and returns true if
// kart i is in the vector and false if it is not.

bool CharSel::kartAvailable(int kart)
{
	if (!kart_properties_manager->m_selected_karts.empty())
        {
            std::vector<int>::iterator it;
            for (it = kart_properties_manager->m_selected_karts.begin();
                it < kart_properties_manager->m_selected_karts.end(); it++)
            {
                if ( kart == *it)
                return false;
            }
        }
    return true;
}   // kartAvailable
