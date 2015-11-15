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

#include "items/item_manager.hpp"
#include "items/powerup.hpp"
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

}   // BattleAI

BattleAI::~BattleAI()
{
#ifdef AI_DEBUG
    irr_driver->removeNode(m_debug_sphere);
#endif
}   //  ~BattleAI

//-----------------------------------------------------------------------------
/** Resets the AI when a race is restarted.
 */
void BattleAI::reset()
{
    m_current_node = BattleGraph::UNKNOWN_POLY;
    m_next_node = BattleGraph::UNKNOWN_POLY;
    m_target_node = BattleGraph::UNKNOWN_POLY;
    m_target_point = Vec3(0,0,0);
    m_target_angle = 0.0f;
    m_time_since_stuck = 0.0f;
    m_currently_reversing = false;
    AIBaseController::reset();
}

//-----------------------------------------------------------------------------
/** This is the main entry point for the AI.
 *  It is called once per frame for each AI and determines the behaviour of
 *  the AI, e.g. steering, accelerating/braking, firing.
 */
void BattleAI::update(float dt)
{
    handleAcceleration(dt);
    handleSteering(dt);
	handleItems(dt);
    handleBraking();
    handleGetUnstuck(dt);
    AIBaseController::update(dt);
}   //update


//-----------------------------------------------------------------------------
/** Handles acceleration. It also takes the plunger into account.
 *  \param dt Time step size.
 */
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

//-----------------------------------------------------------------------------
/** This function sets the steering.
 *  NOTE: The Battle AI is in development and currently this function is a
 *  sandbox for testing out the AI. It may actually be doing a lot more than 
 *  just steering to a point, which means this function could be messy. 
 */
void BattleAI::handleSteering(const float dt)
{
    const AbstractKart* kart = m_world->getPlayerKart(0);
    PlayerController* pcontroller = (PlayerController*)kart->getController();
	
	int player_node = pcontroller->getCurrentNode();    
     std::cout<<"PLayer node " << player_node<<" This cpu kart node" << m_current_node<<std::endl;
    
    if(player_node == BattleGraph::UNKNOWN_POLY || m_current_node == BattleGraph::UNKNOWN_POLY) return;
    m_target_node   =   player_node;
	m_target_point = kart->getXYZ();

	//handleItemCollection(&m_target_point, &m_target_node);
	m_debug_sphere->setPosition(m_target_point.toIrrVector());
    if(m_target_node == m_current_node)
    {
        m_target_point=kart->getXYZ();  
    //  std::cout<<"Aiming at sire nixt\n";
    }
    else
    {    
        m_next_node = BattleGraph::get()->getNextShortestPathPoly(m_current_node, m_target_node);
  
        // std::cout<<"Aiming at "<<next_node<<"\n";
        if(m_next_node == -1) return;
        //target_point = NavMesh::get()->getCenterOfPoly(m_next_node);
      
        findPortals(m_current_node, m_target_node);
        stringPull(m_kart->getXYZ(),m_target_point);
        if(m_path_corners.size()>0)  
        {
                m_debug_sphere->setPosition(m_path_corners[0].toIrrVector());
                m_target_point = m_path_corners.front();
        }
        else 
            {
                std::cout<<" ZERO CORNERS \n";
        }
        //        target_point = m_path_corners[0];
    } 

    m_target_angle = steerToPoint(m_target_point);
    // std::cout<<"Target nalge: "<<m_target_angle << "  normalized:"<<normalizeAngle(m_target_angle)<<std::endl;
    setSteering(m_target_angle,dt);
    
#ifdef AI_DEBUG
      //  m_debug_sphere->setPosition(target_point.toIrrVector());
        Log::debug("skidding_ai","-Outside of road: steer to center point.\n");
#endif

}   // handleSteering

//-----------------------------------------------------------------------------
/** This function finds the polyon edges(portals) that the AI will cross before
 *  reaching its destination. We start from the current polygon and call  
 *  BattleGraph::getNextShortestPathPoly() to find the next polygon on the shortest 
 *  path to the destination. Then find the common edge between the current
 *  poly and the next poly, store it and step through the channel.
 *
 *       1----2----3            In this case, the portals are:
 *       |strt|    |            (2,5) (4,5) (10,7) (10,9) (11,12) 
 *       6----5----4
 *            |    |
 *            7----10----11----14
 *            |    |     | end |
 *            8----9-----12----13
 *
 *  \param start The start node(polygon) of the channel.
 *  \param end The end node(polygon) of the channel.
 */
void BattleAI::findPortals(int start, int end)
{
   int this_node = start;

   // We can't use NULL because NULL==0 which is a valid node, so we initialize
   // with a value that is always invalid.
   int next_node = -999; 
      
   m_portals.clear();

   while(next_node != end && this_node != -1 && next_node != -1 && this_node != end)
   {
       next_node = BattleGraph::get()->getNextShortestPathPoly(this_node, end);
       
       std::vector<int> this_node_verts = 
                    NavMesh::get()->getNavPoly(this_node).getVerticesIndex();
       std::vector<int> next_node_verts=
                    NavMesh::get()->getNavPoly(next_node).getVerticesIndex();

       // this_node_verts and next_node_verts hold vertices of polygons in CCW order
       // We reverse next_node_verts so it becomes easy to compare edges in the next step
       std::reverse(next_node_verts.begin(),next_node_verts.end());

       Vec3 portalLeft, portalRight;
       //bool flag = 0;
       for(unsigned int n_i=0; n_i<next_node_verts.size(); n_i++)
       {
           for(unsigned int t_i=0; t_i< this_node_verts.size(); t_i++)
           {
               if((next_node_verts[n_i] == this_node_verts[t_i])&&
                   (next_node_verts[(n_i+1)%next_node_verts.size()]==
                            this_node_verts[(t_i+1)%this_node_verts.size()]))
               {
                     portalLeft = NavMesh::get()->
                         getVertex(this_node_verts[(t_i+1)%this_node_verts.size()]);
                     
                     portalRight = NavMesh::get()->getVertex(this_node_verts[t_i]);
               }
           }
       }
       m_portals.push_back(std::make_pair(portalLeft,portalRight));
       // for debugging:
       //m_debug_sphere->setPosition((portalLeft).toIrrVector());
       this_node=next_node;
   }
}   // findPortals

//-----------------------------------------------------------------------------
/** This function implements the funnel algorithm for finding shortest paths 
 *  through a polygon channel. This means that we should move from corner to 
 *  corner to move on the most straight and shortest path to the destination.
 *  This can be visualized as pulling a string from the end point to the start.
 *  The string will bend at the corners, and this algorithm will find those 
 *  corners using portals from findPortals(). The AI will aim at the first 
 *  corner and the rest can be used for estimating the curve (braking).
 *
 *       1----2----3            In this case, the corners are:
 *       |strt|    |            <5,10,end>
 *       6----5----4
 *            |    |
 *            7----10----11----14
 *            |    |     | end |
 *            8----9-----12----13
 *
 *  \param start_pos The start position (usually the AI's current position).
 *  \param end_pos The end position (m_target_point).
 */
void BattleAI::stringPull(const Vec3& start_pos, const Vec3& end_pos)
{
    Vec3 funnel_apex = start_pos;
    Vec3 funnel_left = m_portals[0].first;
    Vec3 funnel_right = m_portals[0].second;
    unsigned int apex_index=0, fun_left_index=0, fun_right_index=0;
    m_portals.push_back(std::make_pair(end_pos,end_pos));
    m_path_corners.clear();
    const float eps=0.0001f;
    
    for(unsigned int i=0; i<m_portals.size(); i++)
    {
        Vec3 portal_left = m_portals[i].first;
        Vec3 portal_right = m_portals[i].second;


        //Compute for left edge
        if((funnel_left==funnel_apex) || portal_left.sideOfLine2D(funnel_apex,funnel_left)<=-eps)
        {   
            funnel_left = 0.98f*portal_left + 0.02f*portal_right;
            //funnel_left = portal_left;
            fun_left_index = i;
         
                
            if( portal_left.sideOfLine2D(funnel_apex,funnel_right)<-eps)
            {             
                funnel_apex = funnel_right;
                apex_index = fun_right_index;
                m_path_corners.push_back(funnel_apex);
                
                funnel_left = funnel_apex;
                funnel_right = funnel_apex;
                i = apex_index;
                continue;
            }
        }

        //Compute for right edge
        if( (funnel_right==funnel_apex) ||portal_right.sideOfLine2D(funnel_apex,funnel_right)>=eps)
        {

            funnel_right = 0.98f*portal_right + 0.02f*portal_left;
            //funnel_right = portal_right;
            fun_right_index = i;
            if(  portal_right.sideOfLine2D(funnel_apex,funnel_left)>eps)
            {

                funnel_apex = funnel_left;
                apex_index = fun_left_index;
                m_path_corners.push_back(funnel_apex);

                funnel_left = funnel_apex;
                funnel_right = funnel_apex;
                i=apex_index;
                continue;
            }
        }

    }

    //Push end_pos to m_path_corners so if no corners, we aim at target
    m_path_corners.push_back(end_pos);
}   //  stringPull

//-----------------------------------------------------------------------------
/** Calls AIBaseController::isStuck() to determine if the AI is stuck.
 *  If the AI is stuck then it will override the controls and start reverse
 *  the kart while turning.
 */
void BattleAI::handleGetUnstuck(const float dt)
{
    if(isStuck() == true)
    {
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
            m_time_since_stuck = 0.0f;
        }
    }
} // handleGetUnstuck


//-----------------------------------------------------------------------------
/** This function handles braking. It calls determineTurnRadius() to find out
 *  the curve radius. Depending on the turn radius, it finds out the maximum
 *  speed. If the current speed is greater than the max speed and a set minimum 
 *  speed, brakes are applied.
 */
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
    points.push_back(m_path_corners[0]);
    points.push_back((m_path_corners.size()>=2)?m_path_corners[1]:m_path_corners[0]);
    
    float current_curve_radius = BattleAI::determineTurnRadius(points);

	Vec3 d1 = m_kart->getXYZ() - m_target_point; Vec3 d2 = m_kart->getXYZ() - m_path_corners[0];
	if (d1.length2_2d() < d2.length2_2d())
		current_curve_radius = d1.length_2d();

    //std::cout<<"\n Radius: " << current_curve_radius;
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

//-----------------------------------------------------------------------------
/** The turn radius is determined by fitting a parabola to 3 points: current 
 *  location of AI, first corner and the second corner. Once the constants are 
 *  computed, a formula is used to find the radius of curvature at the kart's 
 *  current location.
 *  NOTE: This method does not apply enough braking, should think of something
 *  else.
 */
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
    float dx_by_dz = 2*a*x + b;
    float d2x_by_dz = 2*a;

    float radius = pow(abs(1 + pow(dx_by_dz,2)),1.5f)/ abs(d2x_by_dz);

    return radius;

}

// Alternative implementation of isStuck()
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

void BattleAI::handleItems(const float dt)
{
	m_controls->m_fire = true;
}


void BattleAI::handleItemCollection(Vec3 *aim_point, int* target_node)
{
	if (m_kart->getPowerup()->getType() == PowerupManager::POWERUP_BOWLING) return;
	Vec3 old_aim_point = *aim_point;

	float distance = 5.0f;
	bool found_suitable_item = false;
	const std::vector< std::pair<Item*, int> >& item_list =
		BattleGraph::get()->getItemList();
	int items_count = item_list.size();

	
	for (unsigned int j = 0; j < 50; j++)
		{
			for (unsigned int i = 0; i < items_count; ++i)
			{
				Item* item = item_list[i].first;
				Vec3 d = item->getXYZ() - m_kart->getXYZ();
				if (d.length_2d() <= distance)
				{
					if (item->getType() == Item::ITEM_BONUS_BOX && !item->wasCollected())
					{
						m_item_to_collect = item;
						found_suitable_item = true;
						*aim_point = item->getXYZ();
						*target_node = item_list[i].second;
						break;
					}
				}
			}
			distance = 2.0f * distance;
		}
	

}