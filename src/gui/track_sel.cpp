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
#include "font.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_TRACK0,
    WTOK_TRACK1,
    WTOK_TRACK2,
    WTOK_TRACK3,
    WTOK_TRACK4,
    WTOK_TRACK5,
    WTOK_TRACK6,
    WTOK_TRACK7,
    WTOK_TRACK8,
    WTOK_TRACK9,
    WTOK_TRACK10,
    WTOK_TRACK11,
    WTOK_TRACK12,
    WTOK_TRACK13,

    WTOK_IMG0,
    WTOK_IMG1,

    WTOK_AUTHOR
};

TrackSel::TrackSel()
{
    widget_manager->add_wgt( WTOK_TITLE, 40, 7);
    widget_manager->show_wgt_rect( WTOK_TITLE );
    widget_manager->set_wgt_text( WTOK_TITLE, _("Choose a track"));
    widget_manager->show_wgt_text( WTOK_TITLE );
    widget_manager->break_line();

    widget_manager->add_wgt( WidgetManager::WGT_NONE, 100, 2);
    widget_manager->break_line();

    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_activation_state(true);
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_SML );
    for (size_t i = 0; i != track_manager->getTrackCount(); i += 2)
    {
        widget_manager->add_wgt( WTOK_TRACK0 + i, 40, 7);
        widget_manager->set_wgt_text( WTOK_TRACK0 + i, track_manager->getTrack(i)->getName());
        widget_manager->add_wgt( WTOK_TRACK0 + i + 1, 40, 7);
        widget_manager->set_wgt_text( WTOK_TRACK0 + i + 1, track_manager->getTrack(i+1)->getName());
        widget_manager->break_line();
    }

//FIXME: Right now, the image and the author's name is not controlled by the widget manager.
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
    BaseGUI::update(dt);

    glClear(GL_DEPTH_BUFFER_BIT);

    // draw a track preview of the currently highlighted track menu entry
    const int CLICKED_TOKEN = widget_manager->get_selected_wgt();
    const Track* TRACK = track_manager->getTrack(CLICKED_TOKEN - WTOK_TRACK0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, user_config->m_width, 0.0, user_config->m_height, -1.0, +1.0);
    const std::string& screenshot = TRACK->getScreenshotFile();
    const std::string& topview    = TRACK->getTopviewFile();
    if(screenshot.size()==0 && topview.size()==0)
    {
        glDisable ( GL_TEXTURE_2D ) ;
        TRACK->drawScaled2D(0.0, 50, user_config->m_width, user_config->m_height/3); // (x, y, w, h)
        glEnable ( GL_TEXTURE_2D ) ;
    }
    else
    {                   // either topview or screenshot specified
        int xLeft   = user_config->m_width/2;
        int yBottom = 50;
        int w       = user_config->m_width/3;
        int h       = user_config->m_height/3;
        if(topview.size()==0)
        {  // no topview, but there is a screenshot!
            glDisable ( GL_TEXTURE_2D ) ;
            TRACK->drawScaled2D(xLeft, yBottom, w, h);
            glEnable ( GL_TEXTURE_2D ) ;
        }
        else
        {                 // topview is defined
            Material *m=material_manager->getMaterial(topview);
            m->apply();
            glBegin(GL_QUADS) ;
            glColor4f(1, 1, 1, 1 );
            glTexCoord2f(0, 0); glVertex2i( xLeft,   yBottom   );
            glTexCoord2f(1, 0); glVertex2i( xLeft+w, yBottom  );
            glTexCoord2f(1, 1); glVertex2i( xLeft+w, yBottom+h);
            glTexCoord2f(0, 1); glVertex2i( xLeft,   yBottom+h);
            glEnd () ;

        }   // topview is defined
        Material *m=material_manager->getMaterial(screenshot);
        xLeft = xLeft - w - 10;
        m->apply();
        glBegin(GL_QUADS) ;
        glColor4f(1, 1, 1, 1 );
        glTexCoord2f(0, 0); glVertex2i( xLeft,   yBottom   );
        glTexCoord2f(1, 0); glVertex2i( xLeft+w, yBottom  );
        glTexCoord2f(1, 1); glVertex2i( xLeft+w, yBottom+h);
        glTexCoord2f(0, 1); glVertex2i( xLeft,   yBottom+h);
        glEnd () ;
    }   // either topview or screenshot specified
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLfloat backgroundColour[4] = { 0.3f, 0.3f, 0.3f, 0.5f };
    glColor4fv(backgroundColour);
//    glCallList(m_rect);
    glPopMatrix();
    font_gui->Print(TRACK->getDescription(), WGT_FNT_MED,
                    Font::CENTER_OF_SCREEN, 10);
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}

//-----------------------------------------------------------------------------
void TrackSel::select()
{
    const int CLICKED_TOKEN = widget_manager->get_selected_wgt();
    const Track* TRACK = track_manager->getTrack(CLICKED_TOKEN - WTOK_TRACK0);
    race_manager->setTrack(TRACK->getIdent());

    menu_manager->pushMenu(MENUID_NUMLAPS);
}

/* EOF */
