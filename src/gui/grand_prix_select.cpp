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
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "font.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    //FIXME: finish the tokens

    WTOK_EMPTY0,
    WTOK_DESCRIPTION,
    WTOK_EMPTY1,
    WTOK_QUIT,

    WTOK_FIRSTPRIX
};

GrandPrixSelect::GrandPrixSelect()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->add_wgt(WTOK_TITLE, 40, 7);
    widget_manager->set_wgt_text(WTOK_TITLE,  _("Choose a Grand Prix"));
    widget_manager->break_line();

    std::set<std::string> result;
    loader->listFiles(result, "data");

    widget_manager->set_initial_activation_state(true);
    // Findout which grand prixs are available and load them
    int nId = 0;
    for(std::set<std::string>::iterator i  = result.begin();
            i != result.end()  ; i++)
        {
            if (StringUtils::has_suffix(*i, ".cup"))
            {
                std::string fullPath= "data/" + (std::string)*i;
                CupData cup(fullPath.c_str());
                m_all_cups.push_back(cup);
                widget_manager->add_wgt(WTOK_FIRSTPRIX + nId, 40, 7);
                widget_manager->set_wgt_text(WTOK_FIRSTPRIX + nId, cup.getName());
/*                if(nId==0)
                {
                    widgetSet -> start(m_menu_id, cup.getName(), GUI_SML, nId, 0);
                }
                else
                {
                    widgetSet -> state(m_menu_id, cup.getName(), GUI_SML, nId, 0);
                }*/
                widget_manager->break_line();
                nId++;
            }   // if
        }   // for i

    widget_manager->set_initial_activation_state(false);
    widget_manager->add_wgt(WTOK_EMPTY0, 60, 7);
    widget_manager->hide_wgt_rect(WTOK_EMPTY0);
    widget_manager->hide_wgt_text(WTOK_EMPTY0);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_DESCRIPTION, 80, 7);
    widget_manager->hide_wgt_rect(WTOK_DESCRIPTION);
    widget_manager->set_wgt_text(WTOK_DESCRIPTION, _("No Grand Prix selected"));
    widget_manager->set_wgt_text_size(WTOK_DESCRIPTION, WGT_FNT_SML);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_EMPTY1, 60, 7);
    widget_manager->hide_wgt_rect(WTOK_EMPTY1);
    widget_manager->hide_wgt_text(WTOK_EMPTY1);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_QUIT, 60, 7);
    widget_manager->set_wgt_text(WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size(WTOK_QUIT, WGT_FNT_SML);
    widget_manager->activate_wgt(WTOK_QUIT);

    widget_manager->layout(WGT_AREA_ALL);
    //m_rect = widgetSet->rect(10, 10, user_config->m_width-20, 34, GUI_ALL, 10);*/
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
GrandPrixSelect::~GrandPrixSelect()
{
    widget_manager->delete_wgts();
//    widgetSet -> delete_widget(m_menu_id) ;
//    glDeleteLists(m_rect, 1);
}   // GrandPrixSelect

//-----------------------------------------------------------------------------
void GrandPrixSelect::update(float dt)
{
    BaseGUI::update(dt);
    const int CLICKED_TOKEN = widget_manager->get_selected_wgt();
    if(CLICKED_TOKEN < WTOK_FIRSTPRIX) return;

    const CupData &cup = m_all_cups[CLICKED_TOKEN - WTOK_FIRSTPRIX];
    widget_manager->set_wgt_text(WTOK_DESCRIPTION, cup.getDescription());

    return;
}

//-----------------------------------------------------------------------------
void GrandPrixSelect::select()
{
    const int CLICKED_TOKEN = widget_manager->get_selected_wgt();
    if(CLICKED_TOKEN == WTOK_QUIT)
    {
        menu_manager->popMenu();
        return;
    }
    race_manager->setGrandPrix(m_all_cups[CLICKED_TOKEN-WTOK_FIRSTPRIX]);
    menu_manager->pushMenu(MENUID_DIFFICULTY);
}   // select

/* EOF */
