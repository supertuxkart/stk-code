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
 *  \param filename Name of the file to load.
 *  \param quad_set The set of quads to which the graph applies.
 */
QuadGraph::QuadGraph(const std::string &filename, QuadSet* quad_set)
{
    // First create all nodes
    for(unsigned int i=0; i<quad_set->getSize(); i++) {
        m_all_nodes.push_back(new GraphNode);
    }
    load(filename);
}   // QuadGraph

// -----------------------------------------------------------------------------
/** Destructor, removes all nodes of the graph. */
QuadGraph::~QuadGraph()
{
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
            fprintf(stderr, "Incorrect specification in '%s': '%' ignored\n",
                    filename.c_str(), node->getName().c_str());
            continue;
        }
        int from, to;
        node->get("from", &from);
        node->get("to", &to);
        assert(from>=0 && from <(int)m_all_nodes.size());
        m_all_nodes[from]->addSuccessor(to);
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
            m_all_nodes[i]->addSuccessor( i+1>=m_all_nodes.size() ? 0 : i+1);
        }   // if size==0
    }   // for i<m_allNodes.size()
}   // setDefaultSuccessors

// -----------------------------------------------------------------------------
/** Returns the list of successors or a node.
 *  \param node_number The number of the node.
 *  \param succ A vector of ints to which the successors are added.
 */
void QuadGraph::getSuccessors(int node_number, std::vector<int>& succ) const {
    const GraphNode *v=m_all_nodes[node_number];
    for(unsigned int i=0; i<v->getNumberOfSuccessors(); i++) {
        succ.push_back((*v)[i]);
    }
}   // getSuccessors

