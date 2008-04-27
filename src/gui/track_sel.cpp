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

#include "track_sel.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "menu_manager.hpp"
#include "user_config.hpp"
#include "material.hpp"
#include "material_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_IMG0,
    WTOK_IMG1,

    WTOK_AUTHOR,

    WTOK_TRACK0
};

TrackSel::TrackSel()
{
    widget_manager->addTitleWgt( WTOK_TITLE, 40, 7, _("Choose a track") );
    widget_manager->breakLine();

    widget_manager->addWgt( WidgetManager::WGT_NONE, 100, 1);
    widget_manager->breakLine();

    for (unsigned int i = 0; i != track_manager->getTrackCount(); ++i)
    {
        // snowtuxpeak must be unlocked
        const Track *track = track_manager->getTrack(i);
        bool isAvailable = !unlock_manager->isLocked(track->getIdent());
        widget_manager->addTextButtonWgt( WTOK_TRACK0 + i, 40, 6, track->getName());
        widget_manager->setWgtTextSize( WTOK_TRACK0 + i, WGT_FNT_SML );
        if(!isAvailable)
        {
            widget_manager->setWgtText( WTOK_TRACK0 + i, "???");
            widget_manager->deactivateWgt(WTOK_TRACK0 + i);
        }
        if( i%2 != 0 ) widget_manager->breakLine();
        else if (i + 1 == track_manager->getTrackCount() )
        {
            widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 40, 6 );
            widget_manager->breakLine();
        }
    }

    widget_manager->addEmptyWgt(WTOK_IMG0, 35, 35);

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 5, 35 );

    widget_manager->addEmptyWgt(WTOK_IMG1, 35, 35);
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_AUTHOR, 80, 9, _("No track selected") );

    widget_manager->layout(WGT_AREA_TOP);
}

//-----------------------------------------------------------------------------
TrackSel::~TrackSel()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void TrackSel::update(float dt)
{
    const int SELECTED_TRACK = widget_manager->getSelectedWgt() - WTOK_TRACK0;
    if( widget_manager->selectionChanged() &&
        SELECTED_TRACK >= 0 &&
        SELECTED_TRACK < (int)track_manager->getTrackCount() )
    {
        const Track* TRACK = track_manager->getTrack( SELECTED_TRACK );
        const bool FULL_PATH=true;

        widget_manager->setWgtText( WTOK_AUTHOR, TRACK->getDescription() );

        const std::string& screenshot = TRACK->getScreenshotFile();
        const std::string& topview    = TRACK->getTopviewFile();

        if( !screenshot.empty() && !topview.empty() )
        {
            const Material *m =material_manager->getMaterial(screenshot, FULL_PATH);
            widget_manager->setWgtColor( WTOK_IMG0, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG0 );
            widget_manager->setWgtTexture( WTOK_IMG0, m->getState()->getTextureHandle() );
            widget_manager->showWgtTexture( WTOK_IMG0 );
            widget_manager->hideWgtTrack( WTOK_IMG0 );

            m = material_manager->getMaterial(topview, FULL_PATH);
            widget_manager->setWgtColor( WTOK_IMG1, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG1 );
            widget_manager->setWgtTexture( WTOK_IMG1, m->getState()->getTextureHandle() );
            widget_manager->showWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
        else if( topview.empty() )
        {
            const Material *m = material_manager->getMaterial(screenshot, FULL_PATH);
            widget_manager->setWgtColor( WTOK_IMG0, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG0 );
            widget_manager->setWgtTexture( WTOK_IMG0, m->getState()->getTextureHandle() );
            widget_manager->showWgtTexture( WTOK_IMG0 );
            widget_manager->hideWgtTrack( WTOK_IMG0 );

            widget_manager->hideWgtRect( WTOK_IMG1 );
            widget_manager->hideWgtTexture( WTOK_IMG1 );
            widget_manager->setWgtTrackNum( WTOK_IMG1, SELECTED_TRACK );
            widget_manager->showWgtTrack( WTOK_IMG1 );
        }
        else if( screenshot.empty() )
        {
            widget_manager->hideWgtRect( WTOK_IMG0 );
            widget_manager->hideWgtTexture( WTOK_IMG0 );
            widget_manager->setWgtTrackNum( WTOK_IMG0, SELECTED_TRACK );
            widget_manager->showWgtTrack( WTOK_IMG0 );

            Material *m = material_manager->getMaterial(topview, FULL_PATH);
            widget_manager->setWgtColor( WTOK_IMG1, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG1 );
            widget_manager->setWgtTexture( WTOK_IMG1, m->getState()->getTextureHandle() );
            widget_manager->showWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
        else //if( screenshot.empty() && topview.empty() )
        {
            widget_manager->hideWgtRect( WTOK_IMG0 );
            widget_manager->hideWgtTexture( WTOK_IMG0 );
            widget_manager->setWgtTrackNum( WTOK_IMG0, SELECTED_TRACK );
            widget_manager->showWgtTrack( WTOK_IMG0 );

            widget_manager->hideWgtRect( WTOK_IMG1 );
            widget_manager->hideWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
    }

    widget_manager->update(dt);
}

//-----------------------------------------------------------------------------
void TrackSel::select()
{
    const int CLICKED_TOKEN = widget_manager->getSelectedWgt();
    unsigned int track_number = CLICKED_TOKEN - WTOK_TRACK0;
    if(track_number<0 || track_number >= track_manager->getTrackCount())
    {
        return;   // not clicked on a track, ignore
    }
    const Track* TRACK = track_manager->getTrack(CLICKED_TOKEN - WTOK_TRACK0);
    race_manager->setTrack(TRACK->getIdent());
    menu_manager->pushMenu(MENUID_RACE_OPTIONS);
}   // select

/* EOF */
