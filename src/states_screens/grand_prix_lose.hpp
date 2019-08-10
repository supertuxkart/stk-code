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

#ifndef HEADER_GRAND_PRIX_LOSE_HPP
#define HEADER_GRAND_PRIX_LOSE_HPP

#include "guiengine/screen.hpp"
#include "states_screens/grand_prix_cutscene.hpp"

#include <string>
#include <utility>
#include <vector>

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; class IMeshSceneNode; } }
class KartModel;
class KartProperties;
class TrackObject;

/**
  * \brief Screen shown at the end of a Grand Prix
  * \ingroup states_screens
  */
class GrandPrixLose :
    public GrandPrixCutscene,
    public GUIEngine::ScreenSingleton<GrandPrixLose>
{
    friend class GUIEngine::ScreenSingleton<GrandPrixLose>;

    GrandPrixLose(): GrandPrixCutscene("grand_prix_lose.stkgui") {};

    /** Global evolution of time */
    float m_global_time;

    TrackObject* m_kart_node[4];

    /** A copy of the kart model for each kart used. */
    std::vector<KartModel*> m_all_kart_models;

    int m_phase;

    float m_kart_x, m_kart_y, m_kart_z;

public:
    // implement callbacks from parent class GUIEngine::Screen
    void init() OVERRIDE;
    void loadedFromFile() OVERRIDE;
    void onCutsceneEnd() OVERRIDE;
    void onUpdate(float dt) OVERRIDE;
    /** \brief set which karts lost this GP */
    void setKarts(std::vector<std::pair<std::string, float> > ident);
    MusicInformation* getInGameMenuMusic() const OVERRIDE;
};

#endif

