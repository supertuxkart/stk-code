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

#ifndef HEADER_GRAND_PRIX_WIN_HPP
#define HEADER_GRAND_PRIX_WIN_HPP

#include "audio/sfx_base.hpp"
#include "guiengine/screen.hpp"
#include "karts/kart_model.hpp"
#include "states_screens/grand_prix_cutscene.hpp"
#include <utility>

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; class IMeshSceneNode; } }
namespace GUIEngine { class LabelWidget; }
class KartProperties;
class TrackObject;

/**
  * \brief Screen shown at the end of a Grand Prix
  * \ingroup states_screens
  */
class GrandPrixWin :
    public GrandPrixCutscene,
    public GUIEngine::ScreenSingleton<GrandPrixWin>
{
    friend class GUIEngine::ScreenSingleton<GrandPrixWin>;

    GrandPrixWin();

    virtual ~GrandPrixWin() {};

    /** Global evolution of time */
    double m_global_time;

    TrackObject* m_podium_steps[3];

    TrackObject* m_kart_node[3];

    /** A copy of the kart model for each kart used. */
    std::vector<KartModel*> m_all_kart_models;

    GUIEngine::LabelWidget* m_unlocked_label;

    int m_phase;

    /** Used to pick the happy/sad animations of karts */
    int m_num_gp_karts;

    float m_kart_x[3], m_kart_y[3], m_kart_z[3];
    float m_kart_rotation[3];

    float m_podium_x[3], m_podium_y[3], m_podium_z[3];

    /** Used to display a different message if a player is 1st */
    bool m_player_won;

public:
    // implement callbacks from parent class GUIEngine::Screen
    void init() OVERRIDE;
    void loadedFromFile() OVERRIDE {};
    void onCutsceneEnd() OVERRIDE;
    void onUpdate(float dt) OVERRIDE;
    MusicInformation* getInGameMenuMusic() const OVERRIDE;

    /** \pre must be called after pushing the screen, but before onUpdate had the chance to be invoked */
    void setNumGPKarts(int num_gp_karts);
    void setKarts(const std::pair<std::string, float> karts[3]);
    void setPlayerWon(bool some_player_won) { m_player_won = some_player_won; }
};

#endif

