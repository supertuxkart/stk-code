//  $Id$
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

#include "num_laps.hpp"
#include "race_manager.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_NUMLAPS,

    WTOK_LESS,
    WTOK_MORE,

    WTOK_START,
    WTOK_QUIT
};

NumLaps::NumLaps() : laps(3)
{
    widget_manager->addWgt(WTOK_TITLE, 50, 7);
    widget_manager->showWgtRect(WTOK_TITLE);
    widget_manager->showWgtText(WTOK_TITLE);
    widget_manager->setWgtText(WTOK_TITLE, _("Choose number of laps"));
    widget_manager->breakLine();

    widget_manager->addWgt( WidgetManager::WGT_NONE, 100, 5);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_NUMLAPS, 20, 7);
    widget_manager->showWgtRect(WTOK_NUMLAPS);
    widget_manager->showWgtText(WTOK_NUMLAPS);
    widget_manager->setWgtText(WTOK_NUMLAPS, _("Laps: 3"));
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

    widget_manager->addWgt(WTOK_START, 30, 7);
    widget_manager->showWgtRect(WTOK_START);
    widget_manager->showWgtText(WTOK_START);
    widget_manager->setWgtText(WTOK_START, _("Start race"));
    widget_manager->activateWgt(WTOK_START);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_QUIT, 50, 7);
    widget_manager->showWgtRect(WTOK_QUIT);
    widget_manager->showWgtText(WTOK_QUIT);
    widget_manager->setWgtText(WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->activateWgt(WTOK_QUIT);

    widget_manager->layout(WGT_AREA_ALL);
}

// -----------------------------------------------------------------------------
NumLaps::~NumLaps()
{
    widget_manager->reset();
}   // ~NumLaps

// -----------------------------------------------------------------------------
void NumLaps::select()
{
    const int WGT = widget_manager->getSelectedWgt();
    switch (WGT)
    {
      case WTOK_LESS:
        laps = std::max(1, laps-1);
        snprintf(lap_label, MAX_MESSAGE_LENGTH, "Laps: %d", laps);
	    widget_manager->setWgtText(WTOK_NUMLAPS, lap_label);
        break;
      case WTOK_MORE:
        laps = std::min(10, laps+1);
        snprintf(lap_label, MAX_MESSAGE_LENGTH, "Laps: %d", laps);
        widget_manager->setWgtText(WTOK_NUMLAPS, lap_label);
        break;
      case WTOK_START:
        race_manager->setNumLaps(laps);
        race_manager->start();
        break;
      case WTOK_QUIT:
        menu_manager->popMenu();
	break;
    }
}   // select



