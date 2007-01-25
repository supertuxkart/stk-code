//  $Id: credits_menu.cpp 694 2006-08-29 07:42:36Z hiker $
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

#include <SDL/SDL.h>
#include "scrolled_text.hpp"
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "config.hpp"

ScrolledText::ScrolledText()
{
    float r       = config->m_width/800.0f;
    m_x_left         = (int)(30.0*r);  m_x_right = config->m_width -m_x_left;
    r             = config->m_height/600.0f;
    m_y_bottom       = (int)(50.0*r);  m_y_top   = config->m_height-(int)(50.0f*r);
    m_y_speed        = 50.0f;
    m_font_size      = 24;
    m_y_pos          = m_y_bottom-m_font_size;
    m_rect          = 0;
    m_menu_id = widgetSet -> varray(0);
    widgetSet->layout(m_menu_id, 0, 0);
}   // ScrolledText

//-----------------------------------------------------------------------------
ScrolledText::~ScrolledText()
{
    glDeleteLists(m_rect, 1);
}   // ~ScrolledText

//-----------------------------------------------------------------------------
void ScrolledText::setText(StringList sl_)
{
    m_string_list=sl_;
    if(m_rect) glDeleteLists(m_rect, 1);
    m_rect = widgetSet->rect(m_x_left, m_y_bottom, m_x_right-m_x_left, m_y_top-m_y_bottom,
                           GUI_ALL, 10);
}   // setText

//-----------------------------------------------------------------------------
void ScrolledText::update(float dt)
{
    BaseGUI::update(dt);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, config->m_width, 0.0, config->m_height, -1.0, +1.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLfloat backgroundColour[4] = { 0.3f, 0.3f, 0.3f, 0.5f };
    glColor4fv(backgroundColour);
    glCallList(m_rect);
    glPopMatrix();
    widgetSet->drawText("Press <ESC> to go back", 24,
                        SCREEN_CENTERED_TEXT, 20, 255, 255, 255);
    glViewport(m_x_left, m_y_bottom, m_x_right-m_x_left, m_y_top-m_y_bottom);

    glScalef(1.0f, config->m_width/(m_y_top-m_y_bottom), 1.0f);

    for(unsigned int i=0; i<m_string_list.size(); i++)
    {

        if((m_y_pos-i*m_font_size < m_y_top + m_y_bottom ) && m_y_pos-i*m_font_size > -m_font_size)
            widgetSet->drawText(m_string_list[i],24,
                                m_x_left,(int)m_y_pos-i*m_font_size,255,255,255);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glViewport(0,0,config->m_width, config->m_height);
    m_y_pos=m_y_pos+dt*m_y_speed;
    if(m_y_speed>0 && m_y_pos>m_string_list.size()*m_font_size+m_y_top-m_y_bottom) m_y_pos=-m_font_size;
    if(m_y_speed<0 && m_y_pos<0) m_y_pos=m_string_list.size()*m_font_size+m_y_top-m_y_bottom;
}   // update

//-----------------------------------------------------------------------------
void ScrolledText::inputKeyboard(int key, int pressed)
{
    switch(key)
    {
    case SDLK_PLUS      :
    case SDLK_UP        : m_y_speed -= 10.0f; break;
    case SDLK_PAGEUP    : m_y_speed -= 50.0f; break;
    case SDLK_PAGEDOWN  : m_y_speed += 50.0f; break;
    case SDLK_MINUS     :
    case SDLK_DOWN      : m_y_speed += 10.0f; break;
    case SDLK_ESCAPE    : menu_manager->popMenu();
    default             : break;
    }   // switch

    if (m_y_speed > 500.0f) m_y_speed = 500.0f;
    if (m_y_speed < -500.0f) m_y_speed = -500.0f;

}   // inputKeyboard

//-----------------------------------------------------------------------------
void ScrolledText::select()
{
    // must be esc, nothing else is available. So just pop this menu
    menu_manager->popMenu();
}   // select

/* EOF */
