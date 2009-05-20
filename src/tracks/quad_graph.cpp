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

#include "user_config.hpp"
#include "graphics/irr_driver.hpp"
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
    m_node        = NULL;
    m_mesh        = NULL;
    m_mesh_buffer = NULL;
    m_all_quads = new QuadSet(quad_file_name);
    // First create all nodes
    for(unsigned int i=0; i<m_all_quads->getNumberOfQuads(); i++) {
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
/** Creates the debug mesh to display the quad graph on top of the track 
 *  model. */
void QuadGraph::createDebugMesh()
{
    if(m_all_nodes.size()<=0) return;  // no debug output if not graph

    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling = false;
    m.Lighting        = false;
    m_mesh            = irr_driver->createQuadMesh(&m);
    m_mesh_buffer     = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

    video::S3DVertex* v=(video::S3DVertex*)m_mesh_buffer->getVertices();
    // The mesh buffer already contains one quad, so set the coordinates
    // of the first quad in there.
    video::SColor c(255, 255, 0, 0);
    m_all_quads->getQuad(0).setVertices(v, c);
    
    unsigned int      n     = m_all_quads->getNumberOfQuads();
    // Four vertices for each of the n-1 remaining quads
    video::S3DVertex *new_v = new video::S3DVertex[(n-1)*4];
    // Each quad consists of 2 triangles with 3 elements, so 
    // we need 2*3 indices for each quad.
    irr::u16         *ind   = new irr::u16[(n-1)*6];

    // Now add all other quads
    for(unsigned int i=1; i<n; i++)
    {
        // Swap the colours from red to blue and back
        c.setRed (i%2 ? 255 : 0); 
        c.setBlue(i%2 ? 0 : 255);
        // Transfer the 4 points of the current quad to the list of vertices
        m_all_quads->getQuad(i).setVertices(new_v+(4*i-4), c);

        // Set up the indices for the triangles
        // (note, afaik with opengl we could use quads directly, but the code 
        // would not be portable to directx anymore).
        ind[6*i-6] = 4*i-4;  // First triangle: vertex 0, 1, 2
        ind[6*i-5] = 4*i-3;
        ind[6*i-4] = 4*i-2;
        ind[6*i-3] = 4*i-4;  // second triangle: vertex 0, 1, 3
        ind[6*i-2] = 4*i-2;
        ind[6*i-1] = 4*i-1;
    }   // for i=1; i<m_all_quads
    
    m_mesh_buffer->append(new_v, (n-1)*4, ind, (n-1)*6);
    m_node           = irr_driver->addMesh(m_mesh);

}   // createDebugMesh

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
