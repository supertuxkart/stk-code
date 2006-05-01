//  $Id: TrackSel.cxx,v 1.4 2005/08/31 17:25:27 joh Exp $
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

#include "TrackSel.h"
#include "WidgetSet.h"
#include "RaceManager.h"
#include "TrackManager.h"
#include "Track.h"

TrackSel::TrackSel()
{
  menu_id = widgetSet -> vstack(0);

  widgetSet -> label(menu_id, "Choose a Track", GUI_LRG, GUI_TOP, 0, 0);
  widgetSet -> space(menu_id);

  int ha = widgetSet -> harray(menu_id);

  int col1 = widgetSet -> varray(ha);
  int col2 = widgetSet -> varray(ha);

  for (size_t i = 0; i != track_manager->getTrackCount()/2; ++i)
    widgetSet -> state(col1, track_manager->getTrack(i)->getName().c_str(), GUI_SML, i, 0);

  for (size_t i = track_manager->getTrackCount()/2; 
      i != track_manager->getTrackCount(); ++i)
  {
    int tmp = widgetSet -> state(col2, track_manager->getTrack(i)->getName().c_str(), GUI_SML, i, 0);
    if (i == track_manager->getTrackCount()/2)
      widgetSet -> set_active(tmp);
  }

  widgetSet -> layout(menu_id, 0, 1);
}

TrackSel::~TrackSel()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void TrackSel::update(float dt)
{
  widgetSet -> timer(menu_id, dt) ;
  widgetSet -> paint(menu_id) ;

  {
  glClear(GL_DEPTH_BUFFER_BIT);
  if( widgetSet -> token (widgetSet -> click()) != MENU_RETURN )
    {

    // draw a track preview of the currently highlighted track menu entry
    const Track* track =
          track_manager->getTrack(widgetSet->token (widgetSet -> click()));

    // preview's map color
    glColor3f ( 1, 1, 1 ) ;

    // glOrtho was feed with 0.0, getScreenWidth(), 0.0, getScreenHeight();
    // NOTE: it's weird that these values do not fit at all with the glOrtho()
    track->drawScaled2D(0.0, -0.7, 1.0, 0.3, true);  // (x, y, w, h, stretch)
    }
  }
}

void TrackSel::select() {
  race_manager->setTrack(track_manager->
			 getTrack(widgetSet->token(widgetSet->click()))->getIdent());

  guiStack.push_back(GUIS_NUMLAPS);
}

/* EOF */
