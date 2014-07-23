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

#include "guiengine/scalable_font.hpp"
#include "states_screens/grand_prix_cutscene.hpp"

typedef GUIEngine::ButtonWidget Button;

void GrandPrixCutscene::saveGPButton()
{
    if (race_manager->getGrandPrix().getId() == "random")
    {
        core::stringw text = _("Save Grand Prix");
        Button* save_button = new Button();
        save_button->m_properties[GUIEngine::PROP_ID] = "save gp";
        Button* c = getWidget<Button>("continue");
        save_button->m_x = c->m_x + c->m_w + 20;
        save_button->m_y = c->m_y;
        save_button->m_w = GUIEngine::getFont()->getDimension(text.c_str()).Width + 30;
        save_button->m_h = c->m_h;
        save_button->setText(text);
        save_button->add();
        manualAddWidget(save_button);
    }
}
