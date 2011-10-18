//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#include "tracks/quad_graph.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"


// A static variable that gives a single graph node easy access to 
// all quads and avoids unnecessary parameters in many calls.
QuadSet *GraphNode::m_all_quads=NULL;

// This static variable gives a node access to the graph, and therefore
// to the quad to which a graph node index belongs.
QuadGraph *GraphNode::m_all_nodes=NULL;

// ----------------------------------------------------------------------------
/** Constructor. Saves the quad index which belongs to this graph node.
 *  \param index Index of the quad to use for this node (in m_all_quads).
 */
GraphNode::GraphNode(unsigned int quad_index, unsigned int node_index) 
{ 
    assert(quad_index<m_all_quads->getNumberOfQuads());
    m_quad_index          = quad_index;
    m_node_index          = node_index;
    m_predecessor         = -1;
    m_distance_from_start = 0;

    const Quad &quad      = m_all_quads->getQuad(m_quad_index);
    // FIXME: the following values should depend on the actual orientation 
    // of the quad. ATM we always assume that indices 0,1 are the lower end,
    // and 2,3 are the upper end.
    // The width is the average width at the beginning and at the end.
    m_width = (  (quad[1]-quad[0]).length() 
               + (quad[3]-quad[2]).length() ) * 0.5f;
    m_lower_center = (quad[0]+quad[1]) * 0.5f;
    m_upper_center = (quad[2]+quad[3]) * 0.5f;
    m_line     = core::line2df(m_lower_center.getX(), m_lower_center.getZ(),
                               m_upper_center.getX(), m_upper_center.getZ() );
    // Only this 2d point is needed later
    m_lower_center_2d = core::vector2df(m_lower_center.getX(), 
                                        m_lower_center.getZ() );

    // The vector from the center of the quad to the middle of the right
    // side of the quad
    m_center_to_right = (quad[1]+quad[2])*0.5f - quad.getCenter();
}   // GraphNode

// ----------------------------------------------------------------------------
/** Adds a successor to a node. This function will also pre-compute certain
 *  values (like distance from this node to the successor, angle (in world)
 *  between this node and the successor.
 *  \param to The index of the graph node of the successor.
 */
void GraphNode::addSuccessor(unsigned int to)
{
    m_successor_node.push_back(to);
    // m_quad_index is the quad index, so we use m_all_quads
    const Quad &this_quad = m_all_quads->getQuad(m_quad_index);
    // to is the graph node, so we have to use m_all_nodes to get the right quad
    GraphNode &gn = m_all_nodes->getNode(to);
    const Quad &next_quad = m_all_nodes->getQuadOfNode(to);

    // Keep the first predecessor, which is usually the most 'natural' one.
    if(gn.m_predecessor==-1)
        gn.m_predecessor = m_node_index;
    core::vector2df d2    = m_lower_center_2d
                          - m_all_nodes->getNode(to).m_lower_center_2d;

    Vec3 diff     = next_quad.getCenter() - this_quad.getCenter();
    m_distance_to_next.push_back(d2.getLength());
    
    float theta = atan2(diff.getX(), diff.getZ());
    m_angle_to_next.push_back(theta);

    // The length of this quad is the average of the left and right side
    float distance_to_next = (   this_quad[2].distance(this_quad[1])
                               + this_quad[3].distance(this_quad[0]) ) *0.5f;
    // The distance from start for the successor node 
    if(to!=0)
    {
        float distance_for_next = m_distance_from_start+distance_to_next;
        // If the successor node does not have a distance from start defined,
        // update its distance. Otherwise if the node already has a distance,
        // it is potentially necessary to update its distance from start:
        // without this the length of the track (as taken by the distance
        // from start of the last node) could be smaller than some of the 
        // paths. This can result in incorrect results for the arrival time
        // estimation of the AI karts. See trac #354 for details.
        if(m_all_nodes->getNode(to).m_distance_from_start==0)
        {
            m_all_nodes->getNode(to).m_distance_from_start = distance_for_next;
        }
        else if(m_all_nodes->getNode(to).m_distance_from_start
                   <  distance_for_next)
        {
            float delta = distance_for_next
                        - m_all_nodes->getNode(to).getDistanceFromStart();
            m_all_nodes->updateDistancesForAllSuccessors(to, delta);
        }
    }
}   // addSuccessor

// ----------------------------------------------------------------------------
/** If this node has more than one successor, it will set up a vector that
 *  contains the direction to use when a certain graph node X should be 
 *  reached.
 */
void GraphNode::setupPathsToNode()
{
    if(m_successor_node.size()<2) return;

    const unsigned int num_nodes = QuadGraph::get()->getNumNodes();
    m_path_to_node.resize(num_nodes);

    // Initialise each graph node with -1, indicating that 
    // it hasn't been reached yet.
    for(unsigned int i=0; i<num_nodes; i++)
        m_path_to_node[i] = -1;

    // Indicate that this node can be reached from this node by following
    // successor 0 - just a dummy value that might only be used during the
    // recursion below.
    m_path_to_node[m_node_index] = 0;

    // A simple depth first search is used to determine which successor to 
    // use to reach a certain graph node. Using Dijkstra's algorithm  would
    // give the shortest way to reach a certain node, but the shortest way
    // might involve some shortcuts which are hidden, and should therefore 
    // not be used.
    for(unsigned int i=0; i<getNumberOfSuccessors(); i++)
    {
        GraphNode &gn = QuadGraph::get()->getNode(getSuccessor(i));
        gn.markAllSuccessorsToUse(i, &m_path_to_node);
    }
#ifdef DEBUG
    for(unsigned int i=0; i<m_path_to_node.size(); i++)
    {
        if(m_path_to_node[i]==-1)
            printf("[WARNING] No path to node %d found on graph node %d.\n",
                   i, m_node_index);
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
void GraphNode::markAllSuccessorsToUse(unsigned int n, 
                                       PathToNodeVector *path_to_node)
{
    // End recursion if the path to this node has already been found.
    if( (*path_to_node)[m_node_index] >-1) return;

    (*path_to_node)[m_node_index] = n;
    for(unsigned int i=0; i<getNumberOfSuccessors(); i++)
    {
        GraphNode &gn = QuadGraph::get()->getNode(getSuccessor(i));
        gn.markAllSuccessorsToUse(n, path_to_node);
    }
}   // markAllSuccesorsToUse

// ----------------------------------------------------------------------------
/** Returns the distance a point has from this quad in forward and sidewards
 *  direction, i.e. how far forwards the point is from the beginning of the 
 *  quad, and how far to the side from the line connecting the center points
 *  is it. All these computations are done in 2D only.
 *  \param xyz The coordinates of the point.
 *  \param result The X coordinate contains the sidewards distance, the
 *                Z coordinate the forward distance.
 */
void GraphNode::getDistances(const Vec3 &xyz, Vec3 *result)
{
    core::vector2df xyz2d(xyz.getX(), xyz.getZ());
    core::vector2df closest = m_line.getClosestPoint(xyz2d);
    if(m_line.getPointOrientation(xyz2d)>0)
        result->setX( (closest-xyz2d).getLength());   // to the right
    else
        result->setX(-(closest-xyz2d).getLength());   // to the left
    result->setZ( m_distance_from_start + (closest-m_lower_center_2d).getLength());
}   // getDistances

// ----------------------------------------------------------------------------
/** Returns the square of the distance between the given point and any point
 *  on the 'centre' line, i.e. the finite line from the middle point of the
 *  lower end of the quad node to the middle point of the upper end of the
 *  quad which belongs to this graph node. The value is computed in 2d only!
 *  \param xyz The point for which the distance to the line is computed.
 */
float GraphNode::getDistance2FromPoint(const Vec3 &xyz)
{
    core::vector2df xyz2d(xyz.getX(), xyz.getZ());
    core::vector2df closest = m_line.getClosestPoint(xyz2d);
    return (closest-xyz2d).getLengthSQ();
}   // getDistance2FromPoint

// ----------------------------------------------------------------------------

void GraphNode::setChecklineRequirements(int latest_checkline)
{
    m_checkline_requirements.push_back(latest_checkline);
}
