//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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
#include "states_screens/dialogs/race_over_dialog.hpp"
#include "utils/translation.hpp"

RaceOverDialog::RaceOverDialog(const float percentWidth, const float percentHeight) : ModalDialog(percentWidth, percentHeight)
{
    const int y1 = m_area.getHeight()/2;
    
    core::rect< s32 > area(0, y1 - 100, m_area.getWidth(), y1 + 100);
    IGUIStaticText* a = GUIEngine::getGUIEnv()->addStaticText( _("Race Results"),
                                                              area, false, false, // border, word warp
                                                              m_irrlicht_window);
    a->setTabStop(false);
}

void RaceOverDialog::onEnterPressedInternal()
{
}

void RaceOverDialog::processEvent(std::string& eventSource)
{
}
