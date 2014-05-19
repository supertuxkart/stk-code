//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 konstin
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

#include "guiengine/engine.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/dialogs/random_gp_dialog.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>

using namespace irr::gui;
using namespace irr::video;
using namespace irr::core;
using namespace GUIEngine;

randomGPInfoDialog::randomGPInfoDialog()
{
    // Defaults - loading selection from last time frrom a file would be better
    m_number_of_tracks = 2;
    m_track_group = "nextgen";
    m_use_reverse = true;

    doInit();
    m_curr_time = 0.0f;

    int y1 = m_area.getHeight()/7;

    m_gp_ident = "random";
    m_gp = new GrandPrixData(m_number_of_tracks, m_track_group, m_use_reverse);

    // ---- GP Name
    core::rect< s32 > area_top(0, 0, m_area.getWidth(), y1);
    IGUIStaticText* title = GUIEngine::getGUIEnv()->addStaticText(translations->fribidize("Random Grand Prix"),
                                                               area_top, false, true, // border, word wrap
                                                               m_irrlicht_window);
    title->setTabStop(false);
    title->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);

    InitAfterDrawingTheHeader(m_area.getHeight()/7 + 50, m_area.getHeight()*6/7, "random");
}
