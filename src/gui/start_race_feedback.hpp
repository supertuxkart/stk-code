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

/** This class is used to give feedback to the user while loading the track.
 *  It either displays a 'Loading track' or a 'Wait for synchronisation'
 *  message (dependent on the stage of the race manager).
 */
class StartRaceFeedback: public BaseGUI
{
protected:
    /** Flag used to make sure that the text is actually displayed (i.e
     *  update was called once) before loading the track - otherwise the
     *  text is set in the widget, but not on the screen since the screen
     *  wasn't updated.
     */
    bool m_is_first_frame;
public:
    StartRaceFeedback();
    ~StartRaceFeedback();

    void update(float DELTA);
    void select(){};
};

#endif
