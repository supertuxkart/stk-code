//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs
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

#include "tracks/track_sector.hpp"

#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_structure.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"

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
    m_current_graph_node         = Graph::UNKNOWN_SECTOR;
    m_last_valid_graph_node      = Graph::UNKNOWN_SECTOR;
    m_estimated_valid_graph_node = Graph::UNKNOWN_SECTOR;
    m_on_road                    = false;
    m_last_triggered_checkline   = -1;
}   // reset

// ----------------------------------------------------------------------------
/** Updates the current graph node index, and the track coordinates for
 *  the specified point.
 *  \param xyz The new coordinates to search the graph node for.
 */
void TrackSector::update(const Vec3 &xyz, bool ignore_vertical)
{
    int prev_sector = m_current_graph_node;
    const ArenaGraph* ag = ArenaGraph::get();
    std::vector<int>* test_nodes = NULL;

    if (ag && prev_sector != Graph::UNKNOWN_SECTOR)
    {
        // For ArenaGraph, only test nodes around current node
        test_nodes = ag->getNode(prev_sector)->getNearbyNodes();
    }

    // Don't only test nodes around if it was not on road
    Graph::get()->findRoadSector(xyz, &m_current_graph_node,
        m_on_road ? test_nodes : NULL, ignore_vertical);
    m_on_road = m_current_graph_node != Graph::UNKNOWN_SECTOR;

    // If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
    // the road, so we have to use search for the closest graph node.
    if (m_current_graph_node == Graph::UNKNOWN_SECTOR)
    {
        m_current_graph_node = Graph::get()->findOutOfRoadSector(xyz,
            prev_sector, test_nodes, ignore_vertical);
    }

    // Keep the last valid graph node for arena mode
    if (ag)
    {
        if (prev_sector != Graph::UNKNOWN_SECTOR)
            m_last_valid_graph_node = prev_sector;
        return;
    }

    // keep the current quad as the latest valid one IF the player has one
    // of the required checklines AND is on road
    // The on-road condition isn't required for the estimated valid node
    // used for distances.
    const DriveNode* dn = DriveGraph::get()->getNode(m_current_graph_node);
    const std::vector<int>& checkline_requirements = dn->getChecklineRequirements();

    if (checkline_requirements.size() == 0)
    {
        m_estimated_valid_graph_node = m_current_graph_node;
        if (m_on_road)
            m_last_valid_graph_node = m_current_graph_node;
    }
    else
    {
        for (unsigned int i=0; i<checkline_requirements.size(); i++)
        {
            // If a checkline is validated while off-road and rescue is then
            // used ; checking for > is required to have the rescue position
            // correctly updating until the checkline is crossed again.
            // This requires an ordering of checklines such that
            // if checkline N is validated, all checklines for n<N are too.
            if (m_last_triggered_checkline >= checkline_requirements[i])
            {
                //has_prerequisite = true;
                m_estimated_valid_graph_node = m_current_graph_node;
                if (m_on_road)
                    m_last_valid_graph_node = m_current_graph_node;
                break;
            }
        }

        // TODO: show a message when we detect a user missed a checkline.

    }

    // Now determine the 'track' coords, i.e. ow far from the start of the
    // track, and how far to the left or right of the center driveline.
    DriveGraph::get()->spatialToTrack(&m_current_track_coords, xyz,
        m_current_graph_node);

    if (m_last_valid_graph_node != Graph::UNKNOWN_SECTOR)
    {
        DriveGraph::get()->spatialToTrack(&m_latest_valid_track_coords, xyz,
            m_last_valid_graph_node);
    }

    if (m_estimated_valid_graph_node != Graph::UNKNOWN_SECTOR)
    {
        DriveGraph::get()->spatialToTrack(&m_estimated_valid_track_coords, xyz,
            m_estimated_valid_graph_node);
    }
}   // update

// ----------------------------------------------------------------------------
/** Sets current and last valid graph node to the rescue location.
 */
void TrackSector::rescue()
{
    if (m_last_valid_graph_node != Graph::UNKNOWN_SECTOR)
        m_current_graph_node = m_last_valid_graph_node;

    // Using the predecessor has the additional advantage (besides punishing
    // the player a bit more) that it makes it less likely to fall in a
    // rescue loop since the kart moves back on each attempt. At this stage
    // STK does not keep track of where the kart is coming from, so always
    // use the first predecessor, which is the one on the main driveline.
    m_current_graph_node = DriveGraph::get()->getNode(m_current_graph_node)
                                            ->getPredecessor(0);
    m_last_valid_graph_node = DriveGraph::get()->getNode(m_current_graph_node)
                                               ->getPredecessor(0);
    m_estimated_valid_graph_node = m_current_graph_node;
}    // rescue

// ----------------------------------------------------------------------------
/** Returns the relative distance of the corresponding kart from the center,
 *  i.e. a value between -1 and 1 inclusive.
 *  \return THe relative distance between -1.0f and +1.0f;
 */
float TrackSector::getRelativeDistanceToCenter() const
{
    float w = DriveGraph::get()->getNode(m_current_graph_node)->getPathWidth();
    // w * 0.5 is the distance from center of the quad to the left or right
    // This way we get a value between -1 and 1.
    float ratio = getDistanceToCenter()/(w*0.5f);
    if(ratio>1.0f)
        ratio=1.0f;
    else if(ratio<-1.0f)
        ratio=-1.0f;
    return ratio;
}   // getRelativeDistanceToCenter

// ----------------------------------------------------------------------------
/** Only basket ball is used for rewind for TrackSector so save the minimum.
 */
void TrackSector::saveState(BareNetworkString* buffer) const
{
    buffer->addUInt16((int16_t)m_current_graph_node);
    buffer->addFloat(m_current_track_coords.getZ());
}   // saveState

// ----------------------------------------------------------------------------
void TrackSector::rewindTo(BareNetworkString* buffer)
{
    int16_t node = buffer->getUInt16();
    m_current_graph_node = node;
    m_current_track_coords.setZ(buffer->getFloat());
}   // rewindTo

// ----------------------------------------------------------------------------
/** Save completely for spectating in linear race
 */
void TrackSector::saveCompleteState(BareNetworkString* bns)
{
    bns->addUInt32(m_current_graph_node);
    bns->addUInt32(m_estimated_valid_graph_node);
    bns->addUInt32(m_last_valid_graph_node);
    bns->add(m_current_track_coords);
    bns->add(m_estimated_valid_track_coords);
    bns->add(m_latest_valid_track_coords);
    bns->addUInt8(m_on_road ? 1 : 0);
    bns->addUInt32(m_last_triggered_checkline);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TrackSector::restoreCompleteState(const BareNetworkString& b)
{
    const int max_node = Graph::get()->getNumNodes();
    m_current_graph_node = b.getUInt32();
    if (m_current_graph_node >= max_node)
    {
        Log::warn("TrackSector", "Server has different graph node list.");
        // 0 so that if any function is called before update track sector
        // again it will have at least a valid node
        m_current_graph_node = 0;
    }
    m_estimated_valid_graph_node = b.getUInt32();
    if (m_estimated_valid_graph_node >= max_node)
    {
        Log::warn("TrackSector", "Server has different graph node list.");
        m_estimated_valid_graph_node = 0;
    }
    m_last_valid_graph_node = b.getUInt32();
    if (m_last_valid_graph_node >= max_node)
    {
        Log::warn("TrackSector", "Server has different graph node list.");
        m_last_valid_graph_node = 0;
    }
    m_current_track_coords = b.getVec3();
    m_estimated_valid_track_coords = b.getVec3();
    m_latest_valid_track_coords = b.getVec3();
    m_on_road = b.getUInt8() == 1;
    m_last_triggered_checkline = b.getUInt32();
}   // restoreCompleteState
