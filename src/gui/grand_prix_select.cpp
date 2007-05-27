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
#include "loader.hpp"
#include "string_utils.hpp"
#include "grand_prix_select.hpp"
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "font.hpp"
#include "translation.hpp"

GrandPrixSelect::GrandPrixSelect()
{
    m_menu_id = widgetSet -> varray(0);

    widgetSet -> label(m_menu_id, _("Choose a Grand Prix"), GUI_LRG, GUI_ALL, 0, 0);
    widgetSet -> space(m_menu_id);

    std::set<std::string> result;
    loader->listFiles(result, "data");

    // Findout which grand prixs are available and load them
    int nId = 0;
    for(std::set<std::string>::iterator i  = result.begin();
            i != result.end()  ; i++)
        {
            if (StringUtils::has_suffix(*i, ".cup"))
            {
                std::string fullPath= "data/" + (std::string)*i;
                CupData *cup = new CupData(fullPath.c_str());
                m_all_cups.push_back(cup);
                if(nId==0)
                    widgetSet -> start(m_menu_id, cup->getName().c_str(), GUI_SML, nId, 0);
                else
                    widgetSet -> state(m_menu_id, cup->getName().c_str(), GUI_SML, nId, 0);
                nId++;
            }   // if
        }   // for i
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id,_("Press <ESC> to go back"), GUI_SML, -1);
    widgetSet -> layout(m_menu_id, 0, 0);
    m_rect = widgetSet->rect(10, 10, user_config->m_width-20, 34, GUI_ALL, 10);
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
GrandPrixSelect::~GrandPrixSelect()
{
    widgetSet -> delete_widget(m_menu_id) ;
    glDeleteLists(m_rect, 1);
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
void GrandPrixSelect::update(float dt)
{
    BaseGUI::update(dt);
    const int CLICKED_TOKEN = widgetSet->get_token(widgetSet->click());
    if(CLICKED_TOKEN == -1) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, user_config->m_width, 0.0, user_config->m_height, -1.0, +1.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    CupData *cup = m_all_cups[CLICKED_TOKEN];
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLfloat BACKGROUND_COLOUR[4] = { 0.3f, 0.3f, 0.3f, 0.5f };
    glColor4fv(BACKGROUND_COLOUR);
    glCallList(m_rect);
    glPopMatrix();
    font_gui->Print(cup->getDescription(), GUI_MED, 
                    Font::ALIGN_CENTER, -1, Font::ALIGN_BOTTOM, 10);
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    return;
}

//-----------------------------------------------------------------------------
void GrandPrixSelect::select()
{
    const int CLICKED_TOKEN = widgetSet->get_token(widgetSet->click());
    if(CLICKED_TOKEN == -1)
    {
        menu_manager->popMenu();
        return;
    }
    race_manager->setGrandPrix(m_all_cups[CLICKED_TOKEN]);
    menu_manager->pushMenu(MENUID_DIFFICULTY);
}   // select

/* EOF */
