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

#ifndef HEADER_START_RACE_FEEDBACK_H
#define HEADER_START_RACE_FEEDBACK_H

#include "base_gui.hpp"

class StartRaceFeedback: public BaseGUI
{
private:
    char *m_loading_text;        // Used to have the actual text in only
    char *m_synchronising_text;  // one place (easier to change, avoids
                                 // multiple translations in case of typos)
public:
    StartRaceFeedback();
    ~StartRaceFeedback();

    void update(float DELTA);
    void select(){};
};

#endif
