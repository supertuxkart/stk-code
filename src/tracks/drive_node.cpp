//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
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
//  Foundation, Inc., 59 Temple Place - Suite 330, B

#include "tracks/drive_node.hpp"

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/drive_graph.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------
DriveNode::DriveNode(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                     const Vec3 &p3, const Vec3 &normal,
                     unsigned int node_index, bool invisible,
                     bool ai_ignore, bool ignored)
          :Quad(p0, p1, p2, p3, normal, node_index, invisible, ignored)
{
    m_ai_ignore           = ai_ignore;
    m_distance_from_start = -1.0f;

    // The following values should depend on the actual orientation
    // of the quad. ATM we always assume that indices 0,1 are the lower end,
    // and 2,3 are the upper end (or the reverse if reverse mode is selected).
    // The width is the average width at the beginning and at the end.
    m_right_unit_vector = ( m_p[0]-m_p[1]
                           +m_p[3]-m_p[2]) * 0.5f;
    m_right_unit_vector.normalize();

    m_width = (  (m_p[1]-m_p[0]).length()
               + (m_p[3]-m_p[2]).length() ) * 0.5f;

    if(DriveGraph::get()->isReverse())
    {
        m_lower_center = (m_p[2]+m_p[3]) * 0.5f;
        m_upper_center = (m_p[0]+m_p[1]) * 0.5f;
        m_right_unit_vector *= -1.0f;
    }
    else
    {
        m_lower_center = (m_p[0]+m_p[1]) * 0.5f;
        m_upper_center = (m_p[2]+m_p[3]) * 0.5f;
    }

}   // DriveNode

// ----------------------------------------------------------------------------
/** Adds a successor to a node. This function will also pre-compute certain
 *  values (like distance from this node to the successor, angle (in world)
 *  between this node and the successor.
 *  \param to The index of the drive node of the successor.
 */
void DriveNode::addSuccessor(unsigned int to)
{
    m_successor_nodes.push_back(to);
    // to is the drive node
    DriveNode* dn_to = DriveGraph::get()->getNode(to);

    // Note that the first predecessor is (because of the way the drive graph
    // is exported) the most 'natural' one, i.e. the one on the main
    // driveline.
    dn_to->m_predecessor_nodes.push_back(m_index);

    Vec3 d = m_lower_center - dn_to->m_lower_center;
    m_distance_to_next.push_back(d.length());

    Vec3 loc_pos = btTransform(shortestArcQuat(Vec3(0, 1, 0), getNormal()),
        getCenter()).inverse()(dn_to->getCenter());
    m_angle_to_next.push_back(atan2(loc_pos.x(), loc_pos.z()));

}   // addSuccessor

// ----------------------------------------------------------------------------
/** If this node has more than one successor, it will set up a vector that
 *  contains the direction to use when a certain drive node X should be
 *  reached.
 */
void DriveNode::setupPathsToNode()
{
    if(m_successor_nodes.size()<2) return;

    const unsigned int num_nodes = DriveGraph::get()->getNumNodes();
    m_path_to_node.resize(num_nodes);

    // Initialise each drive node with -1, indicating that
    // it hasn't been reached yet.
    for(unsigned int i=0; i<num_nodes; i++)
        m_path_to_node[i] = -1;

    // Indicate that this node can be reached from this node by following
    // successor 0 - just a dummy value that might only be used during the
    // recursion below.
    m_path_to_node[m_index] = 0;

    // A simple depth first search is used to determine which successor to
    // use to reach a certain drive node. Using Dijkstra's algorithm  would
    // give the shortest way to reach a certain node, but the shortest way
    // might involve some shortcuts which are hidden, and should therefore
    // not be used.
    for(unsigned int i=0; i<getNumberOfSuccessors(); i++)
    {
        DriveNode* dn = DriveGraph::get()->getNode(getSuccessor(i));
        dn->markAllSuccessorsToUse(i, &m_path_to_node);
    }
#ifdef DEBUG
    for(unsigned int i = 0; i < m_path_to_node.size(); ++i)
    {
        if(m_path_to_node[i] == -1)
            Log::warn("DriveNode", "No path to node %d found on drive node %d.",
                   i, m_index);
    }
#endif
}   // setupPathsToNode

// ----------------------------------------------------------------------------
/** This function marks that the successor n should be used to reach this
 *  node. It then recursively (depth first) does the same for all its
 *  successors. Depth-first
 *  \param n The successor which should be used in m_path_node to reach
 *         this node.
 *  \param path_to_node The path-to-node data structure of the node for
 *         which the paths are currently determined.
 */
void DriveNode::markAllSuccessorsToUse(unsigned int n,
                                       PathToNodeVector *path_to_node)
{
    // End recursion if the path to this node has already been found.
    if( (*path_to_node)[m_index] >-1) return;

    (*path_to_node)[m_index] = n;
    for(unsigned int i=0; i<getNumberOfSuccessors(); i++)
    {
        DriveNode* dn = DriveGraph::get()->getNode(getSuccessor(i));
        dn->markAllSuccessorsToUse(n, path_to_node);
    }
}   // markAllSuccesorsToUse

// ----------------------------------------------------------------------------
void DriveNode::setDirectionData(unsigned int successor, DirectionType dir,
                                 unsigned int last_node_index)
{
    if(m_direction.size()<successor+1)
    {
        m_direction.resize(successor+1);
        m_last_index_same_direction.resize(successor+1);
    }
    m_direction[successor]                 = dir;
    m_last_index_same_direction[successor] = last_node_index;
}   // setDirectionData

// ----------------------------------------------------------------------------
void DriveNode::setChecklineRequirements(int latest_checkline)
{
    m_checkline_requirements.push_back(latest_checkline);
}   // setChecklineRequirements

// ----------------------------------------------------------------------------
/** Returns true if the index-successor of this node is one that the AI
 *  is allowed to use.
 *  \param index Index of the successor.
 */
bool DriveNode::ignoreSuccessorForAI(unsigned int i) const
{
    return DriveGraph::get()->getNode(m_successor_nodes[i])->letAIIgnore();
}   // ignoreSuccessorForAI
