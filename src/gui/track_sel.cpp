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

#include "gui/track_sel.hpp"

#include <sstream>

#include "race_manager.hpp"
#include "user_config.hpp"
#include "challenges/unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_IMG0,
    WTOK_IMG1,
    WTOK_AUTHOR,
    WTOK_UP,
    WTOK_DOWN,
    WTOK_EMPTY0  = 1000,
    WTOK_TRACK0  = 2000

};

TrackSel::TrackSel()
{
    const int HEIGHT = 10;

    const float arrows_x = 0.15f;
    
    Widget *prev_widget=NULL, *w;
    w = widget_manager->addTextButtonWgt(WTOK_UP, 20, HEIGHT/2, "^");
    w->setPosition(WGT_DIR_FROM_RIGHT, arrows_x, WGT_DIR_UNDER_WIDGET, 0.12f);
    prev_widget = w;
    for (unsigned int i = 0; i <m_max_entries; i++)
    {
        float offset = (float)(m_max_entries-1)/2.0f+(float)abs((int)(i-(m_max_entries-1)/2))*0.7f+1.0f;
        w = widget_manager->addTextButtonWgt(WTOK_TRACK0+i, 40, HEIGHT, "");
        widget_manager->setWgtTextSize(WTOK_TRACK0+i, WGT_FNT_SML);
        w->setPosition(WGT_DIR_FROM_RIGHT, 0.03f*offset-0.12f, NULL, 
                       WGT_DIR_UNDER_WIDGET, 0.f, prev_widget);
        prev_widget = w;
    }   // for i
    widget_manager->sameWidth(WTOK_TRACK0, WTOK_TRACK0+m_max_entries-1);

    w = widget_manager->addTextButtonWgt(WTOK_DOWN, 20, HEIGHT/2, "v");
    w->setPosition(WGT_DIR_FROM_RIGHT, arrows_x, NULL, WGT_DIR_UNDER_WIDGET, 0, prev_widget);

    w = widget_manager->addImgWgt(WTOK_IMG0, 35, 29, 0);
    w->setPosition(WGT_DIR_FROM_LEFT, 0.1f, WGT_DIR_FROM_TOP, 0.2f);
    prev_widget = w;
    w = widget_manager->addImgWgt(WTOK_IMG1, 35, 35, 0);
    w->setPosition(WGT_DIR_FROM_LEFT, 0.1f, NULL, WGT_DIR_UNDER_WIDGET,0, prev_widget);
    prev_widget = w;
    w = widget_manager->addTextWgt(WTOK_AUTHOR, 60, 9, "" );
    widget_manager->setWgtResizeToText(WTOK_AUTHOR, true);
    widget_manager->hideWgtRect(WTOK_AUTHOR);
    
    // Loop through all tracks to determine the longest description
    for(unsigned int i=0; i<track_manager->getNumberOfTracks(); i++)
    {
        widget_manager->setWgtText(WTOK_AUTHOR, track_manager->getTrack(i)->getDescription());
        w->resizeToText();
    }
    w->setPosition(WGT_DIR_FROM_LEFT, 0.1f, NULL, WGT_DIR_FROM_BOTTOM, 0.0f, prev_widget);

    m_offset        = 0;
    switchGroup();
    m_current_track = -1;
    for(unsigned int i=0; i<m_index_avail_tracks.size(); i++)
    {
        assert(i < m_index_avail_tracks.size());
        //assert( m_index_avail_tracks[i] >= 0);
        // FIXME - someone had a crash because m_index_avail_tracks[i] was set to a negative number, e.g. -2
        // I have no clue what causes this issue, consider this as a temporary fix
        if(m_index_avail_tracks[i] < 0) continue;
        
        if(track_manager->getTrack(m_index_avail_tracks[i])->getIdent()==
            user_config->m_last_track)
        {
            m_offset = i-m_max_entries/2;
            break;
        }
    }
    updateScrollPosition();

    widget_manager->layout(WGT_AREA_TOP);
    // Make sure to select one track. The call to update() here is necessary,
    // since it guarantees that selectedWgt is indeed a track (otherwise the
    // manager might select e.g. arrow up, and then no track is displayed).
    widget_manager->setSelectedWgt(WTOK_TRACK0+(m_max_entries-1)/2);
    displayImages(0);
    //update(0);
}   // TrackSel

//-----------------------------------------------------------------------------
TrackSel::~TrackSel()
{
    widget_manager->reset();
}   // ~TrackSel

//-----------------------------------------------------------------------------
void TrackSel::updateScrollPosition()
{
    unsigned int start = 0, end=m_max_entries;
    if(m_index_avail_tracks.size()<m_max_entries)
    {
        start = (unsigned int)(m_max_entries-m_index_avail_tracks.size()+1)/2;
        end   = start+m_index_avail_tracks.size()-1;
    }

    for(unsigned int i=0; i<(unsigned int)m_max_entries; i++)
    {
        if(i<start || i>end)
        {
            widget_manager->hideWgtRect(WTOK_TRACK0+i);
            widget_manager->hideWgtText(WTOK_TRACK0+i);
            widget_manager->deactivateWgt(WTOK_TRACK0+i);
            continue;
        }
        // Make them visible again (e.g. after a change of groups)
        widget_manager->activateWgt(WTOK_TRACK0+i);
        widget_manager->showWgtRect(WTOK_TRACK0+i);
        widget_manager->showWgtText(WTOK_TRACK0+i);
        
        int i_with_scrolling = i+m_offset;
        if(i_with_scrolling < 0) i_with_scrolling += m_index_avail_tracks.size();
        int indx = m_index_avail_tracks[ i_with_scrolling%m_index_avail_tracks.size() ];
        if(indx>=0)
        {
            const Track *track = track_manager->getTrack(indx);
            widget_manager->setWgtText(WTOK_TRACK0+i, track->getName());
        }
        else
        {
            const std::vector<std::string>& g=track_manager->getAllGroups();
            widget_manager->setWgtText(WTOK_TRACK0+i, g[-indx-1]);
        }
    }   // for i
    m_current_track = -1;  // force new display of tracks
}   // updateScrollPosition

//-----------------------------------------------------------------------------
void TrackSel::switchGroup()
{
    m_index_avail_tracks.clear();

    const std::vector<int> &tracks = 
        RaceManager::isBattleMode( race_manager->getMinorMode() ) ?
                            track_manager->getArenasInGroup(user_config->m_track_group) :
                            track_manager->getTracksInGroup(user_config->m_track_group);

    for(unsigned int i=0; i<tracks.size(); i++)
    {
        if(!unlock_manager->isLocked(track_manager->getTrack(tracks[i])->getIdent()))
        {
            m_index_avail_tracks.push_back(tracks[i]);
        }
    }

    // Now add the groups, indicated by a negative number as kart index
    // ----------------------------------------------------------------
    const std::vector<std::string>& groups=track_manager->getAllGroups();
    const int group_size = (int)groups.size();
    for(int i =0; i<group_size; i++)
    {
        // Only add groups other than the current one
        if(groups[i]!=user_config->m_track_group) m_index_avail_tracks.push_back(-i-1);
    }
    if(m_index_avail_tracks.size()>=m_max_entries) 
    {
        m_offset          = 0;
        widget_manager->showWgtRect(WTOK_DOWN);
        widget_manager->showWgtText(WTOK_DOWN);
        widget_manager->showWgtRect(WTOK_UP);
        widget_manager->showWgtText(WTOK_UP);
    }
    else
    {
        // Less entries than maximum -> set m_offset to a negative number, so
        // that the actual existing entries are displayed 
        m_offset          = - (int)(1+m_max_entries-m_index_avail_tracks.size())/2;
        widget_manager->hideWgtRect(WTOK_DOWN);
        widget_manager->hideWgtText(WTOK_DOWN);
        widget_manager->hideWgtRect(WTOK_UP);
        widget_manager->hideWgtText(WTOK_UP);
    }
}   // switchGroup

//-----------------------------------------------------------------------------
void TrackSel::displayImages(int selected_track)
{
    if( m_current_track == selected_track) return;
    m_current_track = selected_track;
    if(selected_track<0)
    {
        widget_manager->hideWgtTexture(WTOK_IMG0);
        widget_manager->hideWgtTexture(WTOK_IMG1);
        widget_manager->hideWgtRect(WTOK_IMG0);
        widget_manager->hideWgtRect(WTOK_IMG1);
        widget_manager->hideWgtBorder(WTOK_IMG0);
        widget_manager->hideWgtBorder(WTOK_IMG1);
        widget_manager->hideWgtRect(WTOK_AUTHOR);
        widget_manager->hideWgtText(WTOK_AUTHOR);
        return;
    }
    
    // Now we have to display new images
    // ---------------------------------
    widget_manager->showWgtBorder(WTOK_IMG0);
    widget_manager->showWgtBorder(WTOK_IMG1);
    widget_manager->showWgtRect(WTOK_AUTHOR);
    widget_manager->showWgtText(WTOK_AUTHOR);

    widget_manager->hideWgtRect(WTOK_AUTHOR);
    
    const Track* TRACK = track_manager->getTrack(selected_track);
    bool isAvailable = !unlock_manager->isLocked(TRACK->getIdent());

    if( isAvailable )
    {

        const std::string& description = TRACK->getDescription();
        if(description!="")
        {
            widget_manager->setWgtText( WTOK_AUTHOR, TRACK->getDescription() );
            widget_manager->hideWgtRect(WTOK_AUTHOR);
        }
        else
        {
            std::ostringstream designedby;
            designedby<<"Designed by "<<TRACK->getDesigner();
            widget_manager->setWgtText( WTOK_AUTHOR, designedby.str() );
        }
        const std::string& screenshot = TRACK->getScreenshotFile();
        const std::string& topview    = TRACK->getTopviewFile();

        if( !screenshot.empty() && !topview.empty() )
        {
            widget_manager->setWgtColor( WTOK_IMG0, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG0 );
            widget_manager->setWgtTexture( WTOK_IMG0, screenshot.c_str() );
            widget_manager->showWgtTexture( WTOK_IMG0 );
            widget_manager->hideWgtTrack( WTOK_IMG0 );

            widget_manager->setWgtColor( WTOK_IMG1, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG1 );
            widget_manager->setWgtTexture( WTOK_IMG1, topview.c_str() );
            widget_manager->showWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
        else if( topview.empty() )
        {
            widget_manager->setWgtColor( WTOK_IMG0, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG0 );
            widget_manager->setWgtTexture( WTOK_IMG0, screenshot.c_str() );
            widget_manager->showWgtTexture( WTOK_IMG0 );
            widget_manager->hideWgtTrack( WTOK_IMG0 );

            widget_manager->hideWgtRect( WTOK_IMG1 );
            widget_manager->hideWgtTexture( WTOK_IMG1 );
            widget_manager->setWgtTrackNum( WTOK_IMG1, selected_track );
            widget_manager->showWgtTrack( WTOK_IMG1 );
        }
        else if( screenshot.empty() )
        {
            widget_manager->hideWgtRect( WTOK_IMG0 );
            widget_manager->hideWgtTexture( WTOK_IMG0 );
            widget_manager->setWgtTrackNum( WTOK_IMG0, selected_track );
            widget_manager->showWgtTrack( WTOK_IMG0 );

            widget_manager->setWgtColor( WTOK_IMG1, WGT_WHITE);
            widget_manager->showWgtRect( WTOK_IMG1 );
            widget_manager->setWgtTexture( WTOK_IMG1, topview.c_str() );
            widget_manager->showWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
        else //if( screenshot.empty() && topview.empty() )
        {
            widget_manager->hideWgtRect( WTOK_IMG0 );
            widget_manager->hideWgtTexture( WTOK_IMG0 );
            widget_manager->setWgtTrackNum( WTOK_IMG0, selected_track );
            widget_manager->showWgtTrack( WTOK_IMG0 );

            widget_manager->hideWgtRect( WTOK_IMG1 );
            widget_manager->hideWgtTexture( WTOK_IMG1 );
            widget_manager->hideWgtTrack( WTOK_IMG1 );
        }
    }   // isAvailable
}   // displayImages

//-----------------------------------------------------------------------------
void TrackSel::update(float dt)
{
    int indx = widget_manager->getSelectedWgt() - WTOK_TRACK0;
    if(indx<0 || indx >= (int)m_max_entries) 
    {
        widget_manager->update(dt);
        return;
    }

    indx = m_offset + indx;
    // Don't use modulo here, otherwise (one extreme short lists, e.g. 1 track,
    // 1 group, the track is selected when hovering over invisible menu entries
    if(indx< 0                               ) indx += m_index_avail_tracks.size();
    if(indx>=(int)m_index_avail_tracks.size()) indx -= m_index_avail_tracks.size();
    if(indx<0 || indx >= (int)m_index_avail_tracks.size()) 
    {
        widget_manager->update(dt);
        return;
    }
    displayImages(m_index_avail_tracks[indx]);
    widget_manager->update(dt);
}   // update

//-----------------------------------------------------------------------------
void TrackSel::select()
{
    const int CLICKED_TOKEN = widget_manager->getSelectedWgt();
    if(CLICKED_TOKEN==WTOK_UP)
    {
        m_offset--;
        if(m_offset < 0) m_offset = (int)m_index_avail_tracks.size() - 1;
        updateScrollPosition();
        return;
    }
    if(CLICKED_TOKEN==WTOK_DOWN)
    {
        m_offset++;
        if(m_offset >=(int)m_index_avail_tracks.size()) m_offset = 0;
        updateScrollPosition();
        return;
    }
    unsigned int track_number = CLICKED_TOKEN - WTOK_TRACK0;
    if(track_number<0 || track_number >= m_max_entries)
    {
        return;   // not clicked on a track, ignore
    }
    track_number = (track_number+m_offset) % (int)m_index_avail_tracks.size();
    int indx     = m_index_avail_tracks[track_number];
    if(indx<0)   // group selected
    {
        user_config->m_track_group = track_manager->getAllGroups()[-indx-1];
        switchGroup();
        // forces redraw of the model, otherwise (if m_current_kart=0) the new
        // model would not be displayed.
        //m_current_kart = -1;
        updateScrollPosition();
        return;
    }

    const Track* TRACK = track_manager->getTrack(m_index_avail_tracks[track_number]);
    user_config->m_last_track = TRACK->getIdent();
    bool isAvailable = !unlock_manager->isLocked(TRACK->getIdent());

    if( isAvailable )
    {
        race_manager->setTrack(TRACK->getIdent());
        menu_manager->pushMenu(MENUID_RACE_OPTIONS);
    }

    else
    {
        widget_manager->showWgtText( CLICKED_TOKEN );
        widget_manager->setWgtTextColor( CLICKED_TOKEN, WGT_TRANS_GRAY);
        widget_manager->setWgtColor( CLICKED_TOKEN, WGT_TRANS_GRAY);
    }
}   // select

// ----------------------------------------------------------------------------

void TrackSel::handle(GameAction action, int value)
{
    // Forward keypresses to basegui
    if(value) return BaseGUI::handle(action, value);

    if(action==GA_CURSOR_UP)
    {
        m_offset--;
        if(m_offset < 0) m_offset = (int)m_index_avail_tracks.size() - 1;
        updateScrollPosition();
        return;

    }   // if cursor up
    if(action ==GA_CURSOR_DOWN)
    {
        m_offset++;
        if(m_offset >= (int)m_index_avail_tracks.size()) m_offset = 0;
        updateScrollPosition();
        return;
    }   // if cursor down
    BaseGUI::handle(action, value);
}   // handle
/* EOF */
