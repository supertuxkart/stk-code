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
#include "user_config.hpp"
#include "unlock_manager.hpp"
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
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI );

    widget_manager->insertColumn();
    widget_manager->addWgt(WTOK_TITLE, 40, 7);
    widget_manager->setWgtText(WTOK_TITLE,  _("Choose a Grand Prix"));

    std::set<std::string> result;
    file_manager->listFiles(result, "data");

    widget_manager->setInitialActivationState(true);
    // Findout which grand prixs are available and load them
    int nId = 0;
    for(std::set<std::string>::iterator i  = result.begin();
            i != result.end()  ; i++)
        {
            if (StringUtils::has_suffix(*i, ".cup"))
            {
                CupData cup(*i);
                if(unlock_manager->isLocked(cup.getName())) continue;
                m_all_cups.push_back(cup);
                widget_manager->addWgt(WTOK_FIRSTPRIX + nId, 40, 7);
                widget_manager->setWgtText(WTOK_FIRSTPRIX + nId, cup.getName());
                nId++;
            }   // if
        }   // for i

    widget_manager->setInitialActivationState(false);
    widget_manager->addWgt(WTOK_EMPTY0, 60, 7);
    widget_manager->hideWgtRect(WTOK_EMPTY0);
    widget_manager->hideWgtText(WTOK_EMPTY0);

    widget_manager->addWgt(WTOK_DESCRIPTION, 80, 7);
    widget_manager->setWgtText(WTOK_DESCRIPTION, _("No Grand Prix selected"));
    widget_manager->setWgtTextSize(WTOK_DESCRIPTION, WGT_FNT_SML);

    widget_manager->addWgt(WTOK_EMPTY1, 60, 7);
    widget_manager->hideWgtRect(WTOK_EMPTY1);
    widget_manager->hideWgtText(WTOK_EMPTY1);

    widget_manager->addWgt(WTOK_QUIT, 60, 7);
    widget_manager->setWgtText(WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize(WTOK_QUIT, WGT_FNT_SML);
    widget_manager->activateWgt(WTOK_QUIT);

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
    BaseGUI::update(dt);
    const int CLICKED_TOKEN = widget_manager->getSelectedWgt();
    if(CLICKED_TOKEN < WTOK_FIRSTPRIX) return;

    const CupData &cup = m_all_cups[CLICKED_TOKEN - WTOK_FIRSTPRIX];
    widget_manager->setWgtText(WTOK_DESCRIPTION, cup.getDescription());

    return;
}

//-----------------------------------------------------------------------------
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
