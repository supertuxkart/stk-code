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

#ifndef HEADER_RACE_GUI_HPP
#define HEADER_RACE_GUI_HPP

#include <string>
#include <vector>

#include <irrString.h>
using namespace irr;

#include "states_screens/race_gui_base.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class InputMap;
class Material;
class RaceSetup;

/**
  * \brief Handles the overlay for cutscenes
  * \ingroup states_screens
  */
class CutsceneGUI : public RaceGUIBase
{
private:

    float m_fade_level;
    core::stringw m_subtitle;

public:

     CutsceneGUI();
    ~CutsceneGUI();

    void setFadeLevel(float level) { m_fade_level = level; }
    void setSubtitle(const core::stringw& subtitle) { m_subtitle = subtitle; }

    virtual void renderGlobal(float dt) OVERRIDE;
    virtual void renderPlayerView(const Camera *camera, float dt) OVERRIDE {}

    virtual const core::dimension2du getMiniMapSize() const OVERRIDE
    {
        return core::dimension2du(1,1);
    }
};   // RaceGUI

#endif
