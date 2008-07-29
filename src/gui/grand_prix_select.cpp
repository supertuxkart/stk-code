//  $Id: track_sel.cpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include <set>
#include "grand_prix_select.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "track_manager.hpp"
#include "material_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"
#include "grand_prix_manager.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    //FIXME: finish the tokens

    WTOK_DESCRIPTION,
    WTOK_TRACKS,
    WTOK_IMG,
    WTOK_QUIT,

    WTOK_FIRSTPRIX
};

GrandPrixSelect::GrandPrixSelect() : m_curr_track_img(0), m_clock(0.0f)
{
    widget_manager->switchOrder();
    widget_manager->addEmptyWgt(WTOK_TITLE, 60, 7);

    int nId=0;
    m_gp_index.clear();
    for(unsigned int i=0; i<grand_prix_manager->getNumberOfGrandPrix(); i++)
    {
        const CupData *cup = grand_prix_manager->getCup(i);
        if(unlock_manager->isLocked(cup->getName())) continue;
        widget_manager->addTextButtonWgt(WTOK_FIRSTPRIX + nId, 60, 7, cup->getName() );
        m_gp_index.push_back(i);
        nId++;
    }   // for i

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );

    widget_manager->addTextWgt( WTOK_DESCRIPTION, 80, 7, _("No Grand Prix selected") );
    widget_manager->setWgtTextSize(WTOK_DESCRIPTION, WGT_FNT_SML);
    widget_manager->hideWgtRect(WTOK_DESCRIPTION);
    widget_manager->breakLine();
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_TRACKS, 60, 40, _("No Grand Prix selected"));
    widget_manager->enableWgtScroll( WTOK_TRACKS );
    widget_manager->setWgtYScrollSpeed( WTOK_TRACKS, -60 );

    widget_manager->addImgWgt( WTOK_IMG, 40, 40, 0 );
    widget_manager->setWgtColor( WTOK_IMG, WGT_BLACK );
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt(WTOK_QUIT, 40, 7, _("Press <ESC> to go back") );
    widget_manager->setWgtTextSize(WTOK_QUIT, WGT_FNT_SML);

    widget_manager->layout(WGT_AREA_ALL);
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
GrandPrixSelect::~GrandPrixSelect()
{
    widget_manager->reset();
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
void GrandPrixSelect::update(float dt)
{
    const int SELECTED_TOKEN = widget_manager->getSelectedWgt();

    if( widget_manager->selectionChanged() &&
        !( SELECTED_TOKEN < WTOK_FIRSTPRIX ))
    {
        const int CUP_NUM    = m_gp_index[SELECTED_TOKEN - WTOK_FIRSTPRIX];
        const CupData *cup   = grand_prix_manager->getCup(CUP_NUM);
        const int NUM_TRACKS = cup->getTrackCount();

        widget_manager->setWgtText(WTOK_DESCRIPTION, cup->getDescription());


        std::string track_list;
        m_cup_tracks = cup->getTracks();

        for( int i = 0; i < NUM_TRACKS; ++i )
        {
            track_list.append( track_manager->getTrack( m_cup_tracks[i] )->getName() );
            track_list.push_back('\n');
        }
        widget_manager->setWgtText( WTOK_TRACKS, track_list );


        std::string img_filename;
        Material *mat;

        m_track_imgs.clear();

        for( int i = 0; i < NUM_TRACKS; ++i )
        {
            img_filename = track_manager->getTrack( m_cup_tracks[i] )->getTopviewFile();
            if( img_filename.empty() )
            {
                img_filename = track_manager->getTrack( m_cup_tracks[i] )->getScreenshotFile();
                if( img_filename.empty() ) continue;
            }

            mat = material_manager->getMaterial( img_filename, true );

            m_track_imgs.push_back(mat->getState()->getTextureHandle());
        }

        if( !m_track_imgs.empty() )
        {
            m_clock = 0.0f;
            m_curr_track_img = 0;

            widget_manager->showWgtTexture( WTOK_IMG );
            widget_manager->setWgtTexture( WTOK_IMG,
                                           m_track_imgs[ m_curr_track_img ] );
            widget_manager->setWgtColor( WTOK_IMG, WGT_WHITE );
        }
        else
        {
            widget_manager->hideWgtTexture( WTOK_IMG );
            widget_manager->setWgtColor( WTOK_IMG, WGT_BLACK );
        }
    }

    if( !m_track_imgs.empty() )
    {
        m_clock += dt;

        if( m_clock > 1.0f )
        {
            m_clock = 0.0f;

            ++m_curr_track_img;
            if( m_curr_track_img >= m_track_imgs.size() ) m_curr_track_img = 0;

            widget_manager->setWgtTexture(WTOK_IMG,
                                          m_track_imgs[ m_curr_track_img ] );
        }
    }

    widget_manager->update(dt);

    return;
}

//-----------------------------------------------------------------------------
//FIXME:Should select() be renamed for 'click()' or 'enter()' or something?
void GrandPrixSelect::select()
{
    const int CLICKED_TOKEN = widget_manager->getSelectedWgt();
    if(CLICKED_TOKEN == WTOK_QUIT)
    {
        menu_manager->popMenu();
        return;
    }
    const CupData *cup=grand_prix_manager->getCup(m_gp_index[CLICKED_TOKEN-WTOK_FIRSTPRIX]);
    race_manager->setGrandPrix(*cup);
    menu_manager->pushMenu(MENUID_RACE_OPTIONS);
}   // select

/* EOF */
