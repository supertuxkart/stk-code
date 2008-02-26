//  $Id: num_laps.cpp 1369 2007-12-25 03:23:32Z cosmosninja $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006, 2007 SuperTuxKart-Team
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

#include "race_manager.hpp"
#include "num_karts.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_NUMLAPS,

    WTOK_LESS,
    WTOK_MORE,

    WTOK_CONTINUE,
    WTOK_QUIT
};

NumKarts::NumKarts()
{
    widget_manager->addWgt(WTOK_TITLE, 50, 7);
    widget_manager->showWgtRect(WTOK_TITLE);
    widget_manager->showWgtText(WTOK_TITLE);
    widget_manager->setWgtText(WTOK_TITLE, _("Choose number of karts"));
    widget_manager->breakLine();

    widget_manager->addWgt( WidgetManager::WGT_NONE, 100, 5);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_NUMLAPS, 20, 7);
    widget_manager->showWgtRect(WTOK_NUMLAPS);
    widget_manager->showWgtText(WTOK_NUMLAPS);
    m_num_karts = race_manager->getNumKarts();
    snprintf(m_kart_label, MAX_MESSAGE_LENGTH, _("Karts: %d"), m_num_karts);
	widget_manager->setWgtText(WTOK_NUMLAPS, m_kart_label);
    widget_manager->breakLine();

    widget_manager->addWgt( WidgetManager::WGT_NONE, 100, 5);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_LESS, 20, 7);
    widget_manager->showWgtRect(WTOK_LESS);
    widget_manager->showWgtText(WTOK_LESS);
    widget_manager->setWgtText(WTOK_LESS, _("Less"));
    widget_manager->activateWgt(WTOK_LESS);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_MORE, 20, 7);
    widget_manager->showWgtRect(WTOK_MORE);
    widget_manager->showWgtText(WTOK_MORE);
    widget_manager->setWgtText(WTOK_MORE, _("More"));
    widget_manager->activateWgt(WTOK_MORE);
    widget_manager->breakLine();

    widget_manager->addWgt( WidgetManager::WGT_NONE, 100, 5);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_CONTINUE, 30, 7);
    widget_manager->showWgtRect(WTOK_CONTINUE);
    widget_manager->showWgtText(WTOK_CONTINUE);
    if (race_manager->getRaceMode() == RaceSetup::RM_GRAND_PRIX)
        widget_manager->setWgtText(WTOK_CONTINUE, _("Start race"));
    else
        widget_manager->setWgtText(WTOK_CONTINUE, _("Continue"));

    widget_manager->activateWgt(WTOK_CONTINUE);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_QUIT, 50, 7);
    widget_manager->showWgtRect(WTOK_QUIT);
    widget_manager->showWgtText(WTOK_QUIT);
    widget_manager->setWgtText(WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->activateWgt(WTOK_QUIT);

    widget_manager->layout(WGT_AREA_ALL);
}

// -----------------------------------------------------------------------------
NumKarts::~NumKarts()
{
    widget_manager->reset();
}   // ~NumKarts

// -----------------------------------------------------------------------------
void NumKarts::select()
{
    const int WGT = widget_manager->getSelectedWgt();
    switch (WGT)
    {
      case WTOK_LESS:
        m_num_karts = std::max(race_manager->getNumPlayers(), m_num_karts-1);
        snprintf(m_kart_label, MAX_MESSAGE_LENGTH, "Karts: %d", m_num_karts);
	             widget_manager->setWgtText(WTOK_NUMLAPS, m_kart_label);
        break;
      case WTOK_MORE:
        m_num_karts = std::min(stk_config->m_max_karts, m_num_karts+1);
        snprintf(m_kart_label, MAX_MESSAGE_LENGTH, "Karts: %d", m_num_karts);
        widget_manager->setWgtText(WTOK_NUMLAPS, m_kart_label);
        break;
      case WTOK_CONTINUE:
        race_manager->setNumKarts(m_num_karts);
        if (race_manager->getRaceMode() == RaceSetup::RM_GRAND_PRIX)
            race_manager->start();
        else
            menu_manager->pushMenu(MENUID_NUMLAPS);
        break;
      case WTOK_QUIT:
        menu_manager->popMenu();
	break;
    }

}   // select



