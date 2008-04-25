//  $Id: track_sel.cpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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
#include "file_manager.hpp"
#include "string_utils.hpp"
#include "grand_prix_select.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "track_manager.hpp"
#include "material_manager.hpp"
#include "user_config.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

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
    widget_manager->insertColumn();
    widget_manager->addTitleWgt(WTOK_TITLE, 60, 7, _("Choose a Grand Prix") );

    // Findout which grand prixs are available and load them
    std::set<std::string> result;
    file_manager->listFiles(result, "data");
    int nId = 0;
    for(std::set<std::string>::iterator i  = result.begin();
            i != result.end()  ; i++)
        {
            if (StringUtils::has_suffix(*i, ".cup"))
            {
                CupData cup(*i);
                if(unlock_manager->isLocked(cup.getName())) continue;
                m_all_cups.push_back(cup);
                widget_manager->addTextButtonWgt(WTOK_FIRSTPRIX + nId, 40, 7, cup.getName() );
                nId++;
            }   // if
        }   // for i

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );

    widget_manager->addTextWgt( WTOK_DESCRIPTION, 80, 7, _("No Grand Prix selected") );
    widget_manager->setWgtTextSize(WTOK_DESCRIPTION, WGT_FNT_SML);
    widget_manager->breakLine();
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_TRACKS, 40, 40, _("No Grand Prix selected"));
    widget_manager->enableWgtScroll( WTOK_TRACKS );
    widget_manager->setWgtYScrollSpeed( WTOK_TRACKS, -60 );

    widget_manager->addEmptyWgt( WTOK_IMG, 40, 40 );
    widget_manager->showWgtRect( WTOK_IMG );
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
        const int CUP_NUM = SELECTED_TOKEN - WTOK_FIRSTPRIX;
        const int NUM_TRACKS = m_all_cups[CUP_NUM].getTrackCount();

        const CupData &cup = m_all_cups[CUP_NUM];
        widget_manager->setWgtText(WTOK_DESCRIPTION, cup.getDescription());


        std::string track_list;
        m_cup_tracks = m_all_cups[CUP_NUM].getTracks();

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

        if( !( m_track_imgs.empty() ))
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

    if( !( m_track_imgs.empty() ))
    {
        m_clock += dt;

        if( m_clock > 1.0f )
        {
            m_clock = 0.0f;

            ++m_curr_track_img;
            if( m_curr_track_img >= m_track_imgs.size() ) m_curr_track_img = 0;

            widget_manager->setWgtTexture( WTOK_IMG,
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
    race_manager->setGrandPrix(m_all_cups[CLICKED_TOKEN-WTOK_FIRSTPRIX]);
    menu_manager->pushMenu(MENUID_DIFFICULTY);
}   // select

/* EOF */
