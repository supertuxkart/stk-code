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

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/quad_set.hpp"

/** Constructor, loads the graph information for a given set of quads
 *  from a graph file.
 *  \param quad_file_name Name of the file of all quads
 *  \param graph_file_name Name of the file describing the actual graph
 */
QuadGraph::QuadGraph(const std::string &quad_file_name, 
                     const std::string graph_file_name)
{
    m_all_quads = new QuadSet(quad_file_name);
    // First create all nodes
    for(unsigned int i=0; i<m_all_quads->getSize(); i++) {
        m_all_nodes.push_back(new GraphNode(i));
    }
    load(graph_file_name);
}   // QuadGraph

// -----------------------------------------------------------------------------
/** Destructor, removes all nodes of the graph. */
QuadGraph::~QuadGraph()
{
    delete m_all_quads;
    for(unsigned int i=0; i<m_all_nodes.size(); i++) {
        delete m_all_nodes[i];
    }
}   // ~QuadGraph

// -----------------------------------------------------------------------------
/** Loads a quad graph from a file.
 *  \param filename Name of the file to load.
 */
void QuadGraph::load(const std::string &filename)
{
    const XMLNode *xml = file_manager->createXMLTree(filename);

    if(!xml) 
    {
        // No graph file exist, assume a default loop X -> X+1
        setDefaultSuccessors();
        return;
    }
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        if(node->getName()!="edge")
        {
            fprintf(stderr, "Incorrect specification in '%s': '%s' ignored\n",
                    filename.c_str(), node->getName().c_str());
            continue;
        }
        int from, to;
        node->get("from", &from);
        node->get("to", &to);
        assert(from>=0 && from <(int)m_all_nodes.size());
        m_all_nodes[from]->addSuccessor(to, *m_all_quads);
    }   // from
    delete xml;

    setDefaultSuccessors();
}   // load

// -----------------------------------------------------------------------------
/** This function sets a default successor for all graph nodes that currently
 *  don't have a successor defined. The default successor of node X is X+1.
 */
void QuadGraph::setDefaultSuccessors()
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++) {
        if(m_all_nodes[i]->getNumberOfSuccessors()==0) {
            m_all_nodes[i]->addSuccessor(i+1>=m_all_nodes.size() ? 0 : i+1,
                                         *m_all_quads);
        }   // if size==0
    }   // for i<m_allNodes.size()
}   // setDefaultSuccessors

// -----------------------------------------------------------------------------
/** Returns the list of successors or a node.
 *  \param node_number The number of the node.
 *  \param succ A vector of ints to which the successors are added.
 */
void QuadGraph::getSuccessors(int node_number, std::vector<unsigned int>& succ) const {
    const GraphNode *v=m_all_nodes[node_number];
    for(unsigned int i=0; i<v->getNumberOfSuccessors(); i++) {
        succ.push_back((*v)[i]);
    }
}   // getSuccessors

// ============================================================================
/** Adds a successor to a node. This function will also pre-compute certain
 *  values (like distance from this node to the successor, angle (in world)
 *  between this node and the successor.
 *  \param to The index of the successor.
 */
void QuadGraph::GraphNode::addSuccessor(int to, const QuadSet &quad_set)
{
    m_vertices.push_back(to);
    Vec3 this_xyz = quad_set.getCenterOfQuad(m_index);
    Vec3 next_xyz = quad_set.getCenterOfQuad(to);
    Vec3 diff     = next_xyz - this_xyz;
    m_distance_to_next.push_back(diff.length());
    
    float theta = -atan2(diff.getX(), diff.getY());
    m_angle_to_next.push_back(theta);
}   // addSuccessor
