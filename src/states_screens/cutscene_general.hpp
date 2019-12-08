//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#ifndef CUTSCENE_GENERAL_HPP
#define CUTSCENE_GENERAL_HPP

#include "guiengine/screen.hpp"

/**
  * \brief Screen shown when a feature has been unlocked
  * \ingroup states_screens
 */
class CutSceneGeneral : public GUIEngine::CutsceneScreen,
                                public GUIEngine::ScreenSingleton<CutSceneGeneral>
{
    friend class GUIEngine::ScreenSingleton<CutSceneGeneral>;

    CutSceneGeneral();

public:

    virtual void onCutsceneEnd() OVERRIDE {};

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    void onUpdate(float dt) OVERRIDE {};

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    void tearDown() OVERRIDE;

    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;

    /** override from base class to handle escape press */
    virtual bool onEscapePressed() OVERRIDE;
};

#endif
