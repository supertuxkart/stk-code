//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs
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

#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_structure.hpp"
#include "tracks/track.hpp"
#include "tracks/track_sector.hpp"
#include "tracks/quad_graph.hpp"

// ----------------------------------------------------------------------------
/** Initialises the object, and sets the current graph node to be undefined.
 */
TrackSector::TrackSector()
{
    reset();
}   // TrackSector

// ----------------------------------------------------------------------------
void TrackSector::reset()
{
    m_current_graph_node       = QuadGraph::UNKNOWN_SECTOR;
    m_last_valid_graph_node    = QuadGraph::UNKNOWN_SECTOR;
    m_on_road                  = false;
    m_last_triggered_checkline = -1;
}   // reset

// ----------------------------------------------------------------------------
/** Updates the current graph node index, and the track coordinates for
 *  the specified point.
 *  \param xyz The new coordinates to search the graph node for.
 */
void TrackSector::update(const Vec3 &xyz, Kart* kart, Track* track)
{
    int prev_sector = m_current_graph_node;

    QuadGraph::get()->findRoadSector(xyz, &m_current_graph_node);
    m_on_road = m_current_graph_node != QuadGraph::UNKNOWN_SECTOR;

    int kart_id = -1;
    World* world = World::getWorld();
    for (unsigned int i=0; i<world->getNumKarts(); i++)
    {
        if (world->getKart(i) == kart)
        {
            kart_id = i;
            break;
        }
    }
    
    assert(kart_id != -1);
    
    // If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
    // the road, so we have to use search for the closest graph node.
    if(m_current_graph_node == QuadGraph::UNKNOWN_SECTOR)
    {
        m_current_graph_node = 
            QuadGraph::get()->findOutOfRoadSector(xyz,
                                                  prev_sector);
    }
    else
    {
        // keep the current quad as the latest valid one IF the player has one
        // of the required checklines
        const std::vector<int>& checkline_requirements =
            QuadGraph::get()->getNode(m_current_graph_node).getChecklineRequirements();
        
        if (checkline_requirements.size() == 0)
        {
            //printf("    Check %i last visited; curr requirement = NONE\n", m_last_triggered_checkline);
            m_last_valid_graph_node = m_current_graph_node;
        }
        else
        {
            bool has_prerequisite = false;
            
            for (unsigned int i=0; i<checkline_requirements.size(); i++)
            {
                //printf("    Check %i last visited; requirement[%i] = %i\n", m_last_triggered_checkline,
                //       i, checkline_requirements[i]);
                
                if (m_last_triggered_checkline == checkline_requirements[i])
                {
                    has_prerequisite = true;
                    //if (m_last_valid_graph_node != m_current_graph_node)
                    //    printf("[2] m_last_valid_graph_node : %i\n", m_last_valid_graph_node);
                    
                    m_last_valid_graph_node = m_current_graph_node;
                    break;
                }
            }
            
            
            // TODO: show a message when we detect a user cheated.
            // The code below almost works but fails for 2 reasons :
            // 1) if a user goes to an earlier part of the track, the message
            //    was still shown even if they're not actual;ly cheating
            // 2) on the quad where the checkline is the message can appear
            //    and disappear quickly as the per-quad resolution is
            //    insufficient
            //if (!has_prerequisite)
            //{
            //    World* w = World::getWorld();
            //    if (dynamic_cast<LinearWorld*>(w) != NULL &&
            //        dynamic_cast<LinearWorld*>(w)->getKartLap(kart_id) > -1)
            //    {
            //        RaceGUIBase* race_gui = w->getRaceGUI();
            //        race_gui->addMessage(_("CHEATER!"), kart, -1.0f /* time */,
            //                             video::SColor(255,255,255,255), true /* important */,
            //                             true /* big font */);
            //   }
            //}
        }
    }
    
    // Now determine the 'track' coords, i.e. ow far from the start of the 
    // track, and how far to the left or right of the center driveline.
    QuadGraph::get()->spatialToTrack(&m_current_track_coords, xyz, 
                                     m_current_graph_node);
}   // update

// ----------------------------------------------------------------------------
void TrackSector::rescue()
{
    if (m_last_valid_graph_node != QuadGraph::UNKNOWN_SECTOR)
        m_current_graph_node = m_last_valid_graph_node;

    // Using the predecessor has the additional advantage (besides punishing
    // the player a bit more) that it makes it less likely to fall in a 
    // rescue loop since the kart moves back on each attempt. 
    m_current_graph_node = QuadGraph::get()->getNode(m_current_graph_node)
                                               .getPredecessor();
    m_last_valid_graph_node = QuadGraph::get()->getNode(m_current_graph_node)
                                               .getPredecessor();
}    // rescue
