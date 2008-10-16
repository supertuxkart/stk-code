//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Eduardo Hernandez Munoz
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

#ifndef HEADER_RACERESULTSGUI_H
#define HEADER_RACERESULTSGUI_H

#include <vector>

#include "base_gui.hpp"
#include "widget.hpp"

/** GUI that shows the RaceResults, times and such */
class RaceResultsGUI : public BaseGUI
{
private:
    /** Widgets. WTOK_NONE is used for detecting in update when 
     *  a selection was made (see m_selected_token).            */
    enum WidgetTokens
    {
        WTOK_NONE,
        WTOK_CONTINUE,
        WTOK_RESTART_RACE,
        WTOK_SETUP_NEW_RACE,
        WTOK_RESULTS,
        WTOK_FIRST_RESULT,
        WTOK_FIRST_IMAGE     = 1000,
        WTOK_HIGHSCORES      = 2000,
        WTOK_FIRST_HIGHSCORE = 2001,
    };
private:
    std::vector<int> m_order;
    bool             m_first_time;
    /** The widget selected by the user, so that the right action can be done
     *  once clients and server are synchronised. */
    WidgetTokens     m_selected_widget;
    Widget          *displayRaceResults();
    Widget          *displayKartList(Widget *w_prev, int *order, float horizontal);
public:
                     RaceResultsGUI();
                    ~RaceResultsGUI();
    void             handle(GameAction, int);
    void             select();
    virtual void     update(float dt);
    void             setSelectedWidget(int);
};

#endif

/* EOF */
