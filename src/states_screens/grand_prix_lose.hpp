//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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
#include "karts/kart_model.hpp"

#include <vector>
#include <string>

namespace irr { namespace scene { class ISceneNode; class ICameraSceneNode; class ILightSceneNode; class IMeshSceneNode; } }
class KartProperties;

/**
  * \brief Screen shown at the end of a Grand Prix
  * \ingroup states_screens
  */
class GrandPrixLose : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<GrandPrixLose>
{
    friend class GUIEngine::ScreenSingleton<GrandPrixLose>;
    
    GrandPrixLose();
    
    /** sky angle, 0-360 */
    float m_sky_angle;
    
    /** Global evolution of time */
    float m_global_time;
    
    irr::scene::IMeshSceneNode* m_garage;

    irr::scene::IAnimatedMeshSceneNode* m_garage_door;

    irr::scene::ISceneNode* m_kart_node[4];
    
    irr::scene::ISceneNode* m_sky;
    irr::scene::ICameraSceneNode* m_camera;

    irr::scene::ILightSceneNode* m_light;
    
    /** A copy of the kart model for each kart used. */
    std::vector<KartModel*> m_all_kart_models;

    int m_phase;
    
    float m_kart_x, m_kart_y, m_kart_z;
    
    float m_camera_x, m_camera_y, m_camera_z;
    float m_camera_target_x, m_camera_target_z;

    MusicInformation* m_music;
    
    //irr::core::recti m_viewport[4];
    
public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;
    
    /** \brief implement optional callback from parent class GUIEngine::Screen */
    void onUpdate(float dt, irr::video::IVideoDriver*) OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void init() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void tearDown() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                       const int playerID) OVERRIDE;
    
    /** \brief set which karts lost this GP */
    void setKarts(std::vector<std::string> ident);

    virtual MusicInformation* getMusic() const OVERRIDE { return m_music; }

};

#endif

