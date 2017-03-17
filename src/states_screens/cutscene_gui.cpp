//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Marianne Gagnon
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
#include "guiengine/scalable_font.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/irr_driver.hpp"
#include "states_screens/cutscene_gui.hpp"

// -----------------------------------------------------------------------------

CutsceneGUI::CutsceneGUI()
{
    m_fade_level = 0.0f;
}

// -----------------------------------------------------------------------------

CutsceneGUI::~CutsceneGUI()
{

}

// -----------------------------------------------------------------------------

void CutsceneGUI::renderGlobal(float dt)
{
#ifndef SERVER_ONLY
    core::dimension2d<u32> screen_size = irr_driver->getActualScreenSize();

    if (m_fade_level > 0.0f)
    {
        GL32_draw2DRectangle(
                                video::SColor((int)(m_fade_level*255), 0,0,0),
                                core::rect<s32>(0, 0,
                                                screen_size.Width,
                                                screen_size.Height));
    }

    if (m_subtitle.size() > 0)
    {
        core::rect<s32> r(0, screen_size.Height - GUIEngine::getFontHeight()*2,
                          screen_size.Width, screen_size.Height);

        if (GUIEngine::getFont()->getDimension(m_subtitle.c_str()).Width > screen_size.Width)
        {
            GUIEngine::getSmallFont()->draw(m_subtitle, r,
                                            video::SColor(255,255,255,255), true, true, NULL);
        }
        else
        {
            GUIEngine::getFont()->draw(m_subtitle, r,
                                       video::SColor(255,255,255,255), true, true, NULL);
        }
    }
#endif
}

