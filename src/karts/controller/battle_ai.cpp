//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2008-2012 Joerg Henrichs
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

#define AI_DEBUG

#include "karts/controller/battle_ai.hpp"

#ifdef AI_DEBUG
#  include "graphics/irr_driver.hpp"
#endif

#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/controller/ai_properties.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/skidding.hpp"
#include "karts/skidding_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "tracks/nav_poly.hpp"
#include "tracks/navmesh.hpp"

#ifdef AI_DEBUG
#  include "irrlicht.h"
   using namespace irr;
#endif

#if defined(WIN32) && !defined(__CYGWIN__)  && !defined(__MINGW32__)
#  define isnan _isnan
#else
#  include <math.h>
#endif

#include <iostream>

BattleAI::BattleAI(AbstractKart *kart, 
                    StateManager::ActivePlayer *player) 
                       : AIBaseController(kart, player)
{

    reset();
    
    #ifdef AI_DEBUG
    
        video::SColor col_debug(128, 128,0,0);
        m_debug_sphere = irr_driver->addSphere(1.0f, col_debug);
        m_debug_sphere->setVisible(true);
        //m_item_sphere  = irr_driver->addSphere(1.0f);
    #endif

    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES)
    {
        m_world     = dynamic_cast<ThreeStrikesBattle*>(World::getWorld());
        m_track     = m_world->getTrack();
        
    }
    else
    {
        // Those variables are not defined in a battle mode (m_world is
        // a linear world, since it assumes the existance of drivelines)
        m_world           = NULL;
        m_track           = NULL;
    }

    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseController' to the kar.
    Controller::setControllerName("BattleAI");

}

BattleAI::~BattleAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
#endif
}   //  ~BattleAI

void BattleAI::reset()
{
    m_current_node = BattleGraph::UNKNOWN_POLY;
    m_time_since_stuck = 0.0f;
    m_currently_reversing = false;
    AIBaseController::reset();
}

void BattleAI::update(float dt)
{
    //m_controls->m_accel     = 0.5f;
 //   m_controls->m_steer     = 0;
   // updateCurrentNode();
    handleAcceleration(dt);
    handleSteering(dt);
   // handleBraking();
    handleGetUnstuck(dt);
    AIBaseController::update(dt);
}   //BattleAI


void BattleAI::handleGetUnstuck(const float dt)
{
    if(isStuck() == true)
    {
        std::cout<<"GOT STUCK\n";
        m_time_since_stuck = 0.0f;
        m_currently_reversing = true;
        m_controls->reset();
    }
    if(m_currently_reversing == true)
    {
        setSteering(-1.0f*m_target_angle,dt);
        setSteering(-2.0f*m_target_angle,dt);
        setSteering(-2.0f*m_target_angle,dt);
        m_controls->m_accel = -0.35f;
        /*
        if(m_target_angle > 0)
        setSteering(M_PI,dt);
        else setSteering(-M_PI,dt);
        */
        m_time_since_stuck += dt;
        

        if(m_time_since_stuck >= 0.6f)
        {
            m_currently_reversing = false;
            std::cout<<"GOT UNSTUCK\n";
            m_time_since_stuck = 0.0f;
        }
    }
}



// Handles steering. 
void BattleAI::handleSteering(const float dt)
{
    Vec3  target_point;
    const AbstractKart* kart = m_world->getPlayerKart(0);
    PlayerController* pcontroller = (PlayerController*)kart->getController();

    int player_node = pcontroller->getCurrentNode();    
    m_target_node   =   player_node;
    // std::cout<<"PLayer node " << player_node<<" This cpu kart node" << m_current_node<<std::endl;
    if(player_node == BattleGraph::UNKNOWN_POLY || m_current_node == BattleGraph::UNKNOWN_POLY) return;
    if(player_node == m_current_node)
    {
        target_point=kart->getXYZ();  
   //     std::cout<<"Aiming at sire nixt\n";
    }
    else
    {    
    m_next_node = BattleGraph::get()->getNextShortestPathPoly(m_current_node, player_node);
  
   // std::cout<<"Aiming at "<<next_node<<"\n";
    if(m_next_node == -1) return;
    target_point = NavMesh::get()->getCenterOfPoly(m_next_node);
    } 
    m_target_angle = steerToPoint(target_point);
  //  std::cout<<"Target nalge: "<<m_target_angle << "  normalized:"<<normalizeAngle(m_target_angle)<<std::endl;
      setSteering(m_target_angle,dt);
    
#ifdef AI_DEBUG
        m_debug_sphere->setPosition(target_point.toIrrVector());
        Log::debug("skidding_ai","-Outside of road: steer to center point.\n");
#endif
      findPortals(m_current_node, m_target_node, portals);
}


void BattleAI::findPortals(int start, int end, std::vector<std::pair<Vec3,Vec3> >& portals)
{
   int this_node = start;
   int next_node = NULL;
   portals.clear();

   while(next_node != end && this_node != -1 && next_node != -1 && this_node != end)
   {
       next_node = BattleGraph::get()->getNextShortestPathPoly(this_node, end);
       
       const std::vector<int> this_node_verts = 
                    NavMesh::get()->getNavPoly(this_node).getVerticesIndex();
       const std::vector<int> next_node_verts=
                    NavMesh::get()->getNavPoly(next_node).getVerticesIndex();
       
       Vec3 portalLeft, portalRight;
       bool flag = 0;
       for(unsigned int n_i=0; n_i<next_node_verts.size(); n_i++)
       {
           for(unsigned int t_i=0; t_i< this_node_verts.size(); t_i++)
           {
               if(next_node_verts[n_i] == this_node_verts[t_i])
               {
                   if(flag == 0)
                   {
                       portalLeft = NavMesh::get()->getVertex(next_node_verts[n_i]);
                       flag=1;
                   }
                   else
                       portalRight = NavMesh::get()->getVertex(next_node_verts[n_i]);

               }
           }
       }
       portals.push_back(std::make_pair(portalLeft,portalRight));
       //irr_driver->getVideoDriver()->draw3DLine(Vec3(portalLeft+Vec3(0,2,0)).toIrrVector(),Vec3(portalRight+Vec3(0,2,0)).toIrrVector(),video::SColor(255,0,0,255));
       //m_debug_sphere->setPosition(Vec3((portalLeft+portalRight)/2).toIrrVector());
       this_node=next_node;
   }
}


void BattleAI::handleAcceleration( const float dt)
{
    //Do not accelerate until we have delayed the start enough
   /* if( m_start_delay > 0.0f )
    {
        m_start_delay -= dt;
        m_controls->m_accel = 0.0f;
        return;
    }
    */

    if( m_controls->m_brake )
    {
        m_controls->m_accel = 0.0f;
        return;
    }

    if(m_kart->getBlockedByPlungerTime()>0)
    {
        if(m_kart->getSpeed() < m_kart->getCurrentMaxSpeed() / 2)
            m_controls->m_accel = 0.05f;
        else
            m_controls->m_accel = 0.0f;
        return;
    }

    m_controls->m_accel = stk_config->m_ai_acceleration;

}   // handleAcceleration



void BattleAI::handleBraking()
{
    m_controls->m_brake = false;
    

    // A kart will not brake when the speed is already slower than this
    // value. This prevents a kart from going too slow (or even backwards)
    // in tight curves.
    const float MIN_SPEED = 5.0f;

    std::vector<Vec3> points;
    
    if(m_current_node == -1 || m_next_node == -1 || m_target_node == -1) 
        return;

    points.push_back(m_kart->getXYZ());
    points.push_back(NavMesh::get()->getCenterOfPoly(m_next_node));
    points.push_back(NavMesh::get()->getCenterOfPoly(m_target_node));
    
    float current_curve_radius = BattleAI::determineTurnRadius(points);
    std::cout<<"\n Radius: " << current_curve_radius;
    float max_turn_speed =
            m_kart->getKartProperties()
                   ->getSpeedForTurnRadius(current_curve_radius);

        if(m_kart->getSpeed() > max_turn_speed  &&
            m_kart->getSpeed()>MIN_SPEED )//            &&
            //fabsf(m_controls->m_steer) > 0.95f          )
        {
            m_controls->m_brake = true;
            std::cout<<"Braking"<<std::endl;
#ifdef DEBUG
            if(m_ai_debug)
                Log::debug("SkiddingAI",
                       "speed %f too tight curve: radius %f ",
                       m_kart->getSpeed(),
                       m_kart->getIdent().c_str(),
                       current_curve_radius);
#endif
        }
        return;
   
}   // handleBraking

// Fit parabola to 3 points.
// Solve AX=B
float BattleAI::determineTurnRadius( std::vector<Vec3>& points )
{
    // Declaring variables
    float a,b,c;
    irr::core::CMatrix4<float> A;
    irr::core::CMatrix4<float> X;
    irr::core::CMatrix4<float> B;
    
    //Populating matrices
    for(unsigned int i=0; i<3; i++)
    {
        A(i,0)= points[i].x()*points[i].x();
        A(i,1)= points[i].x();
       // std::cout<<"X"<<points[i].x();
        A(i,2)= 1.0f;
        A(i,3)= 0.0f;
    }
    A(3,0)=A(3,1)=A(3,2) = 0.0f;
    A(3,3) = 1.0f;
    
    for(unsigned int i=0; i<3; i++)
    {
        B(i,0)= points[i].z();
        //std::cout<<"Z"<<points[i].z()<<"\n";
        B(i,1)= 0.0f;
        B(i,2)= 0.0f;
        B(i,3)= 0.0f;
    }
    B(3,0)=B(3,1)=B(3,2)=B(3,3) = 0.0f;
    
    //Computing inverse : X = inv(A)*B
    irr::core::CMatrix4<float> invA;
    if(!A.getInverse(invA))
    {
        return -1;    
    }

    X = invA*B;
    a = X(0,0);
    b = X(0,1);
    c = X(0,2);

    float x = points.front().x();
    float z = a*pow(x,2) + b*x + c;
    float zD1 = 2*a*x + b;
    float zD2 = 2*a;

    float radius = pow(abs(1 + pow(zD1,2)),1.5f)/ abs(zD2);

    return radius;

}

/*
float BattleAI::isStuck(const float dt)
{
    // check if kart is stuck
    if(m_kart->getSpeed()<2.0f && !m_kart->getKartAnimation() &&
        !m_world->isStartPhase())
    {
        m_time_since_stuck += dt;
        if(m_time_since_stuck > 2.0f)
        {
            return true;
            m_time_since_stuck=0.0f;
        }   // m_time_since_stuck > 2.0f
    }
    else
    {
        m_time_since_stuck = 0.0f;
        return false;
    }
} 

*/

