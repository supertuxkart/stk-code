//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010      Joerg Henrichs
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

#ifndef HEADER_AI_BASE_CONTROLLER_HPP
#define HEADER_AI_BASE_CONTROLLER_HPP

#include "karts/controller/controller.hpp"
#include "states_screens/state_manager.hpp"


class AIProperties;
class LinearWorld;
class ThreeStrikesBattle;
class QuadGraph;
class BattleGraph;
class Track;
class Vec3;

class AIBaseController : public Controller
{




protected:
     /** Length of the kart, storing it here saves many function calls. */
    float m_kart_length;

    /** Cache width of kart. */
    float m_kart_width;

    
    /** Keep a pointer to the track to reduce calls */
    Track       *m_track;

    
    
    /** A pointer to the AI properties for this kart. */
    const AIProperties *m_ai_properties;



    

    virtual void setSteering   (float angle, float dt);
    void    setControllerName(const std::string &name);
    float   steerToPoint(const Vec3 &point);
    float    normalizeAngle(float angle);
    virtual bool doSkid(float steer_fraction);
    static bool m_ai_debug;

public:
    
    AIBaseController(AbstractKart *kart,
                              StateManager::ActivePlayer *player=NULL);
    virtual ~AIBaseController() {};

    virtual bool  disableSlipstreamBonus() const;

};
#endif