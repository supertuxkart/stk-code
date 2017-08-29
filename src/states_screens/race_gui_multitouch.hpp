//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_RACE_GUI_MULTITOUCH_HPP
#define HEADER_RACE_GUI_MULTITOUCH_HPP

#include <irrString.h>
#include <rect.h>
#include <vector2d.h>
#include <IVideoDriver.h>

using namespace irr;

class AbstractKart;
class MultitouchDevice;
class RaceGUIBase;

class RaceGUIMultitouch
{
private:
    RaceGUIBase* m_race_gui;
    MultitouchDevice* m_device;
    
    bool m_gui_action;
    unsigned int m_minimap_bottom;
    
    video::ITexture* m_directionnal_wheel_tex;
    video::ITexture* m_pause_tex;
    video::ITexture* m_nitro_tex;
    video::ITexture* m_nitro_empty_tex;
    video::ITexture* m_wing_mirror_tex;
    video::ITexture* m_thunderbird_reset_tex;
    video::ITexture* m_drift_tex;
    video::ITexture* m_bg_button_tex;
    video::ITexture* m_bg_button_focus_tex;
    video::ITexture* m_gui_action_tex;

    void initMultitouchSteering();
    void closeMultitouchSteering();

public:
     RaceGUIMultitouch(RaceGUIBase* race_gui);
    ~RaceGUIMultitouch();

    void drawMultitouchSteering(const AbstractKart* kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling);
                                
    unsigned int getMinimapBottom() {return m_minimap_bottom;}
    void setGuiAction(bool enabled = true) {m_gui_action = enabled;}
    void reset();
                                 
};   // RaceGUIMultitouch

#endif
