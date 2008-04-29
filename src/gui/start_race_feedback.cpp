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

#include <SDL/SDL.h>

#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "translation.hpp"

#include "start_race_feedback.hpp"

enum WidgetTokens
{
    WTOK_MSG
};

StartRaceFeedback::StartRaceFeedback()
{
    //Add some feedback so people know they are going to start the race
    widget_manager->reset();
    widget_manager->addTextWgt( WTOK_MSG, 60, 7, _("Loading race...") );
    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
StartRaceFeedback::~StartRaceFeedback()
{
    widget_manager->reset();
}


//-----------------------------------------------------------------------------
void StartRaceFeedback::update(float DELTA)
{
    widget_manager->update(0.0f);

    //I consider that in this case, a static variable is cleaner than a
    //member variable of this class. -Coz
    static bool updated = false;
    if( updated == true ) race_manager->startNew();
    else updated = true;
}

