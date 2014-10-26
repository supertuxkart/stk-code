//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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

#include "LinearMath/btTransform.h"

#include <IMesh.h>
#include <ICameraSceneNode.h>

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/rtts.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/check_lap.hpp"
#include "tracks/check_line.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/quad_set.hpp"
#include "tracks/track.hpp"
#include "graphics/glwrap.hpp"

const int QuadGraph::UNKNOWN_SECTOR  = -1;
QuadGraph *QuadGraph::m_quad_graph = NULL;

/** Constructor, loads the graph information for a given set of quads
 *  from a graph file.
 *  \param quad_file_name Name of the file of all quads
 *  \param graph_file_name Name of the file describing the actual graph
 */
QuadGraph::QuadGraph(const std::string &quad_file_name,
                     const std::string graph_file_name,
                     const bool reverse) : m_reverse(reverse)
{
    m_node                 = NULL;
    m_mesh                 = NULL;
    m_mesh_buffer          = NULL;
    m_lap_length           = 0;
    QuadSet::create();
    QuadSet::get()->init(quad_file_name);
    m_quad_filename        = quad_file_name;
    m_quad_graph           = this;
    load(graph_file_name);
}   // QuadGraph

// -----------------------------------------------------------------------------
/** Destructor, removes all nodes of the graph. */
QuadGraph::~QuadGraph()
{
    QuadSet::destroy();
    for(unsigned int i=0; i<m_all_nodes.size(); i++) {
        delete m_all_nodes[i];
    }
    if(UserConfigParams::m_track_debug)
        cleanupDebugMesh();
}   // ~QuadGraph

// -----------------------------------------------------------------------------

void QuadGraph::addSuccessor(unsigned int from, unsigned int to) {
    if(m_reverse)
        m_all_nodes[to]->addSuccessor(from);
    else
        m_all_nodes[from]->addSuccessor(to);
}   // addSuccessor

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
        // i.e. each quad is part of the graph exactly once.
        // First create an empty graph node for each quad:
        for(unsigned int i=0; i<QuadSet::get()->getNumberOfQuads(); i++)
            m_all_nodes.push_back(new GraphNode(i, (unsigned int) m_all_nodes.size()));
        // Then set the default loop:
        setDefaultSuccessors();
        computeDirectionData();

        if (m_all_nodes.size() > 0)
        {
            m_lap_length = m_all_nodes[m_all_nodes.size()-1]->getDistanceFromStart()
                         + m_all_nodes[m_all_nodes.size()-1]->getDistanceToSuccessor(0);
        }
        else
        {
            Log::error("Quad Graph", "No node in driveline graph.");
            m_lap_length = 10.0f;
        }

        return;
    }

    // The graph file exist, so read it in. The graph file must first contain
    // the node definitions, before the edges can be set.
    for(unsigned int node_index=0; node_index<xml->getNumNodes(); node_index++)
    {
        const XMLNode *xml_node = xml->getNode(node_index);
        // First graph node definitions:
        // -----------------------------
        if(xml_node->getName()=="node-list")
        {
            // A list of quads is connected to a list of graph nodes:
            unsigned int from, to;
            xml_node->get("from-quad", &from);
            xml_node->get("to-quad", &to);
            for(unsigned int i=from; i<=to; i++)
            {
                m_all_nodes.push_back(new GraphNode(i, (unsigned int) m_all_nodes.size()));
            }
        }
        else if(xml_node->getName()=="node")
        {
            // A single quad is connected to a single graph node.
            unsigned int id;
            xml_node->get("quad", &id);
            m_all_nodes.push_back(new GraphNode(id, (unsigned int) m_all_nodes.size()));
        }

        // Then the definition of edges between the graph nodes:
        // -----------------------------------------------------
        else if(xml_node->getName()=="edge-loop")
        {
            // A closed loop:
            unsigned int from, to;
            xml_node->get("from", &from);
            xml_node->get("to", &to);
            for(unsigned int i=from; i<=to; i++)
            {
                assert(i!=to ? i+1 : from <m_all_nodes.size());
                addSuccessor(i,(i!=to ? i+1 : from));
                //~ m_all_nodes[i]->addSuccessor(i!=to ? i+1 : from);
            }
        }
        else if(xml_node->getName()=="edge-line")
        {
            // A line:
            unsigned int from, to;
            xml_node->get("from", &from);
            xml_node->get("to", &to);
            for(unsigned int i=from; i<to; i++)
            {
                addSuccessor(i,i+1);
                //~ m_all_nodes[i]->addSuccessor(i+1);
            }
        }
        else if(xml_node->getName()=="edge")
        {
            // Adds a single edge to the graph:
            unsigned int from, to;
            xml_node->get("from", &from);
            xml_node->get("to", &to);
            assert(to<m_all_nodes.size());
            addSuccessor(from,to);
            //~ m_all_nodes[from]->addSuccessor(to);
        }   // edge
        else
        {
            Log::error("Quad Graph", "Incorrect specification in '%s': '%s' ignored.",
                    filename.c_str(), xml_node->getName().c_str());
            continue;
        }   // incorrect specification
    }
    delete xml;

    setDefaultSuccessors();
    computeDistanceFromStart(getStartNode(), 0.0f);
    computeDirectionData();

    // Define the track length as the maximum at the end of a quad
    // (i.e. distance_from_start + length till successor 0).
    m_lap_length = -1;
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        float l = m_all_nodes[i]->getDistanceFromStart()
                + m_all_nodes[i]->getDistanceToSuccessor(0);
        if(l > m_lap_length)
            m_lap_length = l;
    }
}   // load

// ----------------------------------------------------------------------------
/** Returns the index of the first graph node (i.e. the graph node which
 *  will trigger a new lap when a kart first enters it). This is always
 *  0 for normal direction (this is guaranteed by the track exporter),
 *  but in reverse mode (where node 0 is actually the end of the track)
 *  this is 0's successor.
 */
unsigned int QuadGraph::getStartNode() const
{
    return m_reverse ? m_all_nodes[0]->getSuccessor(0)
                     : 0;
}   // getStartNode

// ----------------------------------------------------------------------------
/** Sets the checkline requirements for all nodes in the graph.
 */
void QuadGraph::computeChecklineRequirements()
{
    computeChecklineRequirements(m_all_nodes[0],
                                 CheckManager::get()->getLapLineIndex());
}   // computeChecklineRequirements

// ----------------------------------------------------------------------------
/** Finds which checklines must be visited before driving on this quad
 *  (useful for rescue)
 */
void QuadGraph::computeChecklineRequirements(GraphNode* node,
                                             int latest_checkline)
{
    for (unsigned int n=0; n<node->getNumberOfSuccessors(); n++)
    {
        const int succ_id = node->getSuccessor(n);

        // warp-around
        if (succ_id == 0) break;

        GraphNode* succ = m_all_nodes[succ_id];
        int new_latest_checkline =
            CheckManager::get()->getChecklineTriggering(node->getCenter(),
                                                        succ->getCenter() );
        if(new_latest_checkline==-1)
            new_latest_checkline = latest_checkline;

        /*
        printf("Quad %i : checkline %i\n", succ_id, new_latest_checkline);

        printf("Quad %i :\n", succ_id);
        for (std::set<int>::iterator it = these_checklines.begin();it != these_checklines.end(); it++)
        {
            printf("    Depends on checkline %i\n", *it);
        }
        */

        if (new_latest_checkline != -1)
            succ->setChecklineRequirements(new_latest_checkline);

        computeChecklineRequirements(succ, new_latest_checkline);
    }
}   // computeChecklineRequirements

// ----------------------------------------------------------------------------
/** This function defines the "path-to-nodes" for each graph node that has
 *  more than one successor. The path-to-nodes indicates which successor to
 *  use to reach a certain node. This is e.g. used for the rubber ball to
 *  determine which path it is going to use to reach its target (this allows
 *  the ball to hit a target that is on a shortcut). The algorithm for the
 *  path computation favours the use of successor 0, i.e. it will if possible
 *  only use main driveline paths, not a shortcut (even though a shortcut
 *  could result in a faster way to the target) - but since shotcuts can
 *  potentially be hidden they should not be used (unless necessary).
 *  Only graph nodes with more than one successor have this data structure
 *  (since on other graph nodes only one path can be used anyway, this
 *  saves some memory).
 */
void QuadGraph::setupPaths()
{
    for(unsigned int i=0; i<getNumNodes(); i++)
    {
        m_all_nodes[i]->setupPathsToNode();
    }
}   // setupPaths

// -----------------------------------------------------------------------------
/** This function sets a default successor for all graph nodes that currently
 *  don't have a successor defined. The default successor of node X is X+1.
 */
void QuadGraph::setDefaultSuccessors()
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++) {
        if(m_all_nodes[i]->getNumberOfSuccessors()==0) {
            addSuccessor(i,i+1>=m_all_nodes.size() ? 0 : i+1);
            //~ m_all_nodes[i]->addSuccessor(i+1>=m_all_nodes.size() ? 0 : i+1);
        }   // if size==0
    }   // for i<m_allNodes.size()
}   // setDefaultSuccessors

// -----------------------------------------------------------------------------
/** Sets all start positions depending on the quad graph. The number of
 *  entries needed is defined by the size of the start_transform (though all
 *  entries will be overwritten).
 *  E.g. the karts will be placed as:
 *   1           \
 *     2          +--  row with three karts, each kart is 'sidewards_distance'
 *       3       /     to the right of the previous kart, and
 *   4                 'forwards_distance' behind the previous kart.
 *     5               The next row starts again with the kart being
 *       6             'forwards_distance' behind the end of the previous row.
 *  etc.
 *  \param start_transforms A vector sized to the needed number of start
 *               positions. The values will all be overwritten with the
 *               default start positions.
 *  \param karts_per_row How many karts to place in each row.
 *  \param forwards_distance Distance in forward (Z) direction between
 *               each kart.
 *  \param sidewards_distance Distance in sidewards (X) direction between
 *               karts.
 */
void QuadGraph::setDefaultStartPositions(AlignedArray<btTransform>
                                                       *start_transforms,
                                         unsigned int karts_per_row,
                                         float forwards_distance,
                                         float sidewards_distance,
                                         float upwards_distance) const
{
    // We start just before the start node (which will trigger lap
    // counting when reached). The first predecessor is the one on
    // the main driveline.
    int current_node = m_all_nodes[getStartNode()]->getPredecessor(0);

    float distance_from_start = 0.1f+forwards_distance;

    // Maximum distance to left (or right) of centre line
    const float max_x_dist    = 0.5f*(karts_per_row-0.5f)*sidewards_distance;
    // X position relative to the centre line
    float x_pos               = -max_x_dist + sidewards_distance*0.5f;
    unsigned int row_number   = 0;

    for(unsigned int i=0; i<(unsigned int)start_transforms->size(); i++)
    {
        if (current_node == -1)
        {
            (*start_transforms)[i].setOrigin(Vec3(0,0,0));
            (*start_transforms)[i].setRotation(btQuaternion(btVector3(0, 1, 0),
                                                            0));
        }
        else
        {
            // First find on which segment we have to start
            while(distance_from_start > getNode(current_node).getNodeLength())
            {
                distance_from_start -= getNode(current_node).getNodeLength();
                // Only follow the main driveline, i.e. first predecessor
                current_node = getNode(current_node).getPredecessor(0);
            }
            const GraphNode &gn   = getNode(current_node);
            Vec3 center_line = gn.getLowerCenter() - gn.getUpperCenter();
            center_line.normalize();

            Vec3 horizontal_line = gn[2] - gn[3];
            horizontal_line.normalize();

            Vec3 start = gn.getUpperCenter()
                       + center_line     * distance_from_start
                       + horizontal_line * x_pos;
            // Add a certain epsilon to the height in case that the
            // drivelines are beneath the track.
            (*start_transforms)[i].setOrigin(start+Vec3(0,upwards_distance,0));
            (*start_transforms)[i].setRotation(
                btQuaternion(btVector3(0, 1, 0),
                             gn.getAngleToSuccessor(0)));
            if(x_pos >= max_x_dist-sidewards_distance*0.5f)
            {
                x_pos  = -max_x_dist;
                // Every 2nd row will be pushed sideways by half the distance
                // between karts, so that a kart can drive between the karts in
                // the row ahead of it.
                row_number ++;
                if(row_number % 2 == 0)
                    x_pos += sidewards_distance*0.5f;
            }
            else
                x_pos += sidewards_distance;
            distance_from_start += forwards_distance;
        }
    }   // for i<stk_config->m_max_karts
}   // setStartPositions

// -----------------------------------------------------------------------------
/** Creates a mesh for this graph. The mesh is not added to a scene node and
 *  is stored in m_mesh.
 *  \param show_invisble  If true, also create a mesh for parts of the
 *         driveline that are invisible.
 *  \param enable_transparency Enable alpha blending to make the mesh
 *         semi transparent.
 *  \param track_color Colour of the actual quads.
 *  \param lap_color If defined, show the lap counting line in that colour.
 */
void QuadGraph::createMesh(bool show_invisible,
                           bool enable_transparency,
                           const video::SColor *track_color,
                           const video::SColor *lap_color)
{
    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling  = false;
    m.Lighting         = false;
    if(enable_transparency)
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m.setTexture(0, getUnicolorTexture(SColor(255, 255, 255, 255)));
    m.setTexture(1, getUnicolorTexture(SColor(0, 0, 0, 0)));
    m_mesh             = irr_driver->createQuadMesh(&m);
    m_mesh_buffer      = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

    // Count the number of quads to display (some quads might be invisible
    unsigned int  n = 0;
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        if(show_invisible || !m_all_nodes[i]->getQuad().isInvisible())
            n++;
    }

    // Four vertices for each of the n-1 remaining quads
    video::S3DVertex *new_v = new video::S3DVertex[4*n];
    // Each quad consists of 2 triangles with 3 elements, so
    // we need 2*3 indices for each quad.
    irr::u16         *ind   = new irr::u16[6*n];
    video::SColor     c(255, 255, 0, 0);

    if(track_color)
        c = *track_color;

    // Now add all quads
    int i=0;
    for(unsigned int count=0; count<m_all_nodes.size(); count++)
    {
        // Ignore invisible quads
        if(!show_invisible && m_all_nodes[count]->getQuad().isInvisible())
            continue;
        // Swap the colours from red to blue and back
        if(!track_color)
        {
            c.setRed ((i%2) ? 255 : 0);
            c.setBlue((i%2) ? 0 : 255);
        }
        // Transfer the 4 points of the current quad to the list of vertices
        m_all_nodes[count]->getQuad().getVertices(new_v+4*i, c);

        // Set up the indices for the triangles
        // (note, afaik with opengl we could use quads directly, but the code
        // would not be portable to directx anymore).
        ind[6*i  ] = 4*i+2;  // First triangle: vertex 0, 1, 2
        ind[6*i+1] = 4*i+1;
        ind[6*i+2] = 4*i;
        ind[6*i+3] = 4*i+3;  // second triangle: vertex 0, 1, 3
        ind[6*i+4] = 4*i+2;
        ind[6*i+5] = 4*i;
        i++;
    }   // for i=1; i<QuadSet::get()

    m_mesh_buffer->append(new_v, n*4, ind, n*6);

    if(lap_color)
    {
        video::S3DVertex lap_v[4];
        irr::u16         lap_ind[6];
        video::SColor     c(128, 255, 0, 0);
        m_all_nodes[0]->getQuad().getVertices(lap_v, *lap_color);

        // Now scale the length (distance between vertix 0 and 3
        // and between 1 and 2) to be 'length':
        Vec3 bb_min, bb_max;
        QuadSet::get()->getBoundingBox(&bb_min, &bb_max);
        // Length of the lap line about 3% of the 'height'
        // of the track.
        const float length=(bb_max.getZ()-bb_min.getZ())*0.03f;

        core::vector3df dl = lap_v[3].Pos-lap_v[0].Pos;
        float ll2 = dl.getLengthSQ();
        if(ll2<0.001)
            lap_v[3].Pos = lap_v[0].Pos+core::vector3df(0, 0, 1);
        else
            lap_v[3].Pos = lap_v[0].Pos+dl*length/sqrt(ll2);

        core::vector3df dr = lap_v[2].Pos-lap_v[1].Pos;
        float lr2 = dr.getLengthSQ();
        if(lr2<0.001)
            lap_v[2].Pos = lap_v[1].Pos+core::vector3df(0, 0, 1);
        else
            lap_v[2].Pos = lap_v[1].Pos+dr*length/sqrt(lr2);
        lap_ind[0] = 2;
        lap_ind[1] = 1;
        lap_ind[2] = 0;
        lap_ind[3] = 3;
        lap_ind[4] = 2;
        lap_ind[5] = 0;
        // Set it a bit higher to avoid issued with z fighting,
        // i.e. part of the lap line might not be visible.
        for(unsigned int i=0; i<4; i++)
            lap_v[i].Pos.Y += 0.1f;
#ifndef USE_TEXTURED_LINE
        m_mesh_buffer->append(lap_v, 4, lap_ind, 6);
#else
        lap_v[0].TCoords = core::vector2df(0,0);
        lap_v[1].TCoords = core::vector2df(3,0);
        lap_v[2].TCoords = core::vector2df(3,1);
        lap_v[3].TCoords = core::vector2df(0,1);
        m_mesh_buffer->append(lap_v, 4, lap_ind, 6);
        video::SMaterial &m = m_mesh_buffer->getMaterial();
        video::ITexture *t = irr_driver->getTexture("chess.png");
        m.setTexture(0, t);
#endif
    }

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

    m_mesh_buffer->getMaterial().setTexture(0, irr_driver->getTexture("unlit.png"));

    delete[] ind;
    delete[] new_v;
}   // createMesh

// -----------------------------------------------------------------------------

/** Creates the debug mesh to display the quad graph on top of the track
 *  model. */
void QuadGraph::createDebugMesh()
{
    if(m_all_nodes.size()<=0) return;  // no debug output if not graph

    createMesh(/*show_invisible*/true,
               /*enable_transparency*/true);

    // Now colour the quads red/blue/red ...
    video::SColor     c( 128, 255, 0, 0);
    video::S3DVertex *v = (video::S3DVertex*)m_mesh_buffer->getVertices();
    for(unsigned int i=0; i<m_mesh_buffer->getVertexCount(); i++)
    {
        // Swap the colours from red to blue and back
        c.setRed ((i%2) ? 255 : 0);
        c.setBlue((i%2) ? 0 : 255);
        v[i].Color = c;
    }
    m_node = irr_driver->addMesh(m_mesh, "quad_graph_debug");
#ifdef DEBUG
    m_node->setName("track-debug-mesh");
#endif

}   // createDebugMesh

// -----------------------------------------------------------------------------
/** Removes the debug mesh from the scene.
 */
void QuadGraph::cleanupDebugMesh()
{
    irr_driver->removeNode(m_node);
    m_node = NULL;
    // No need to call irr_driber->removeMeshFromCache, since the mesh
    // was manually made and so never added to the mesh cache.
    m_mesh->drop();
    m_mesh = NULL;
}   // cleanupDebugMesh

// -----------------------------------------------------------------------------
/** Returns the list of successors or a node.
 *  \param node_number The number of the node.
 *  \param succ A vector of ints to which the successors are added.
 *  \param for_ai true if only quads accessible by the AI should be returned.
 */
void QuadGraph::getSuccessors(int node_number,
                              std::vector<unsigned int>& succ,
                              bool for_ai) const
{
    const GraphNode *gn=m_all_nodes[node_number];
    for(unsigned int i=0; i<gn->getNumberOfSuccessors(); i++)
    {
        // If getSuccessor is called for the AI, only add
        // quads that are meant for the AI to be used.
        if(!for_ai || !gn->ignoreSuccessorForAI(i))
            succ.push_back(gn->getSuccessor(i));
    }
}   // getSuccessors

// ----------------------------------------------------------------------------
/** Recursively determines the distance the beginning (lower end) of the quads
 *  have from the start of the track.
 *  \param node The node index for which to set the distance from start.
 *  \param new_distance The new distance for the specified graph node.
 */
void QuadGraph::computeDistanceFromStart(unsigned int node, float new_distance)
{
    GraphNode *gn = m_all_nodes[node];
    float current_distance = gn->getDistanceFromStart();

    // If this node already has a distance defined, check if the new distance
    // is longer, and if so adjust all following nodes. Without this the
    // length of the track (as taken by the distance from start of the last
    // node) could be smaller than some of the paths. This can result in
    // incorrect results for the arrival time estimation of the AI karts.
    // See trac #354 for details.
    // Then there is no need to test/adjust any more nodes.
    if(current_distance>=0)
    {
        if(current_distance<new_distance)
        {
            float delta = new_distance - current_distance;
            updateDistancesForAllSuccessors(gn->getQuadIndex(), delta, 0);
        }
        return;
    }

    // Otherwise this node has no distance defined yet. Set the new
    // distance, and recursively update all following nodes.
    gn->setDistanceFromStart(new_distance);

    for(unsigned int i=0; i<gn->getNumberOfSuccessors(); i++)
    {
        GraphNode *gn_next = m_all_nodes[gn->getSuccessor(i)];
        // The start node (only node with distance 0) is reached again,
        // recursion can stop now
        if(gn_next->getDistanceFromStart()==0)
            continue;

        computeDistanceFromStart(gn_next->getQuadIndex(),
                                 new_distance + gn->getDistanceToSuccessor(i));
    }   // for i
}   // computeDistanceFromStart

// ----------------------------------------------------------------------------
/** Increases the distance from start for all nodes that are directly or
 *  indirectly a successor of the given node. This code is used when two
 *  branches merge together, but since the latest 'fork' indicates a longer
 *  distance from start.
 *  \param indx Index of the node for which to increase the distance.
 *  \param delta Amount by which to increase the distance.
 *  \param recursive_count Counts how often this function was called
 *         recursively in order to catch incorrect graphs that contain loops.
 */
void QuadGraph::updateDistancesForAllSuccessors(unsigned int indx, float delta,
                                                 unsigned int recursive_count)
{
    if(recursive_count>getNumNodes())
    {
        Log::error("QuadGraph",
                   "Quad graph contains a loop (without start node).");
        Log::fatal("QuadGraph",
                   "Fix graph, check for directions of all shortcuts etc.");
    }
    recursive_count++;

    GraphNode &g=getNode(indx);
    g.setDistanceFromStart(g.getDistanceFromStart()+delta);
    for(unsigned int i=0; i<g.getNumberOfSuccessors(); i++)
    {
        GraphNode &g_next = getNode(g.getSuccessor(i));
        // Stop when we reach the start node, i.e. the only node with a
        // distance of 0
        if(g_next.getDistanceFromStart()==0)
            continue;

        // Only increase the distance from start of a successor node, if
        // this successor has a distance from start that is smaller then
        // the increased amount.
        if(g.getDistanceFromStart()+g.getDistanceToSuccessor(i) >
            g_next.getDistanceFromStart())
        {
            updateDistancesForAllSuccessors(g.getSuccessor(i), delta,
                                            recursive_count);
        }
    }
}   // updateDistancesForAllSuccessors

//-----------------------------------------------------------------------------
/** Computes the direction (straight, left, right) of all graph nodes and the
 *  lastest graph node that is still turning in the given direction. For
 *  example, if a successor to this graph node is turning left, it will compute
 *  the last graph node that is still turning left. This data is used by the
 *  AI to estimate the turn radius.
 *  At this stage there is one restriction: if a node with more than one
 *  successor is ahead, only successor 0 is used. That might lead to somewhat
 *  incorrect results (i.e. the last successor is determined assuming that
 *  the kart is always using successor 0, while in reality it might follow
 *  a different successor, resulting in a different turn radius. It is not
 *  expected that this makes much difference for the AI (since it will update
 *  its data constantly, i.e. if it takes a different turn, it will be using
 *  the new data).
 */
void QuadGraph::computeDirectionData()
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        for(unsigned int succ_index=0;
            succ_index<m_all_nodes[i]->getNumberOfSuccessors();
            succ_index++)
        {
            determineDirection(i, succ_index);
        }   // for next < getNumberOfSuccessor

    }   // for i < m_all_nodes.size()
}   // computeDirectionData

//-----------------------------------------------------------------------------
/** Adjust the given angle to be in [-PI, PI].
 */
float QuadGraph::normalizeAngle(float f)
{
    if(f>M_PI)       f -= 2*M_PI;
    else if(f<-M_PI) f += 2*M_PI;
    return f;
}   // normalizeAngle
//-----------------------------------------------------------------------------
/** Determines the direction of the quad graph when driving to the specified
 *  successor. It also determines the last graph node that is still following
 *  the given direction. The computed data is saved in the corresponding
 *  graph node.
 *  It compares the lines connecting the center point of node n with n+1 and
 *  the lines connecting n+1 and n+2 (where n is the current node, and +1 etc.
 *  specifying the successor). Then it keeps on testing the line from n+2 to
 *  n+3, n+3 to n+4, ... as long as the turn direction is the same. The last
 *  value that still has the same direction is then set as the last node
 *  with the same direction in the specified graph node.
 *  \param current Index of the graph node with which to start ('n' in the
 *                 description above).
 *  \param succ_index The successor to be followed from the current node.
 *                    If there should be any other branches later, successor
 *                    0 will always be tetsed.
 */
void QuadGraph::determineDirection(unsigned int current,
                                   unsigned int succ_index)
{
    // The maximum angle which is still considered to be straight
    const float max_straight_angle=0.1f;

    // Compute the angle from n (=current) to n+1 (=next)
    float angle_current = getAngleToNext(current, succ_index);
    unsigned int next   = getNode(current).getSuccessor(succ_index);
    float angle_next    = getAngleToNext(next, 0);
    float rel_angle     = normalizeAngle(angle_next-angle_current);
    // Small angles are considered to be straight
    if(fabsf(rel_angle)<max_straight_angle)
        rel_angle = 0;

    next     = getNode(next).getSuccessor(0);  // next is now n+2

    // If the direction is still the same during a lap the last node
    // in the same direction is the previous node;
    int max_step = (int)m_all_nodes.size()-1;

    while(max_step-- != 0)
    {
        // Now compute the angle from n+1 (new current) to n+2 (new next)
        angle_current = angle_next;
        angle_next    = getAngleToNext(next, 0);
        float new_rel_angle = normalizeAngle(angle_next - angle_current);
        if(fabsf(new_rel_angle)<max_straight_angle)
            new_rel_angle = 0;
        // We have reached a different direction if we go from a non-straight
        // section to a straight one, from a straight section to a non-
        // straight section, or from a left to a right turn (or vice versa)
        if( (rel_angle != 0 && new_rel_angle == 0 ) ||
            (rel_angle == 0 && new_rel_angle != 0 ) ||
            (rel_angle * new_rel_angle < 0        )     )
            break;
        rel_angle = new_rel_angle;

        next = getNode(next).getSuccessor(0);
    }    // while(1)

    GraphNode::DirectionType dir =
        rel_angle==0 ? GraphNode::DIR_STRAIGHT
                     : (rel_angle>0) ? GraphNode::DIR_RIGHT
                                     : GraphNode::DIR_LEFT;
    m_all_nodes[current]->setDirectionData(succ_index, dir, next);
}   // determineDirection


//-----------------------------------------------------------------------------
/** This function takes absolute coordinates (coordinates in OpenGL
 *  space) and transforms them into coordinates based on the track. The y-axis
 *  of the returned vector is how much of the track the point has gone
 *  through, the x-axis is on which side of the road it is (relative to a line
 *  connecting the two center points of a quad). The Y axis is not changed.
 *  \param dst Returns the results in the X and Z coordinates.
 *  \param xyz The position of the kart.
 *  \param sector The graph node the position is on.
 */
void QuadGraph::spatialToTrack(Vec3 *dst, const Vec3& xyz,
                              const int sector) const
{
    if(sector == UNKNOWN_SECTOR )
    {
        Log::warn("Quad Graph", "UNKNOWN_SECTOR in spatialToTrack().");
        return;
    }

    getNode(sector).getDistances(xyz, dst);
}   // spatialToTrack

//-----------------------------------------------------------------------------
/** findRoadSector returns in which sector on the road the position
 *  xyz is. If xyz is not on top of the road, it sets UNKNOWN_SECTOR as sector.
 *
 *  \param xyz Position for which the segment should be determined.
 *  \param sector Contains the previous sector (as a shortcut, since usually
 *         the sector is the same as the last one), and on return the result
 *  \param all_sectors If this is not NULL, it is a list of all sectors to
 *         test. This is used by the AI to make sure that it ends up on the
 *         selected way in case of a branch, and also to make sure that it
 *         doesn't skip e.g. a loop (see explanation below for details).
 */
void QuadGraph::findRoadSector(const Vec3& xyz, int *sector,
                                std::vector<int> *all_sectors) const
{
    // Most likely the kart will still be on the sector it was before,
    // so this simple case is tested first.
    if(*sector!=UNKNOWN_SECTOR && getQuadOfNode(*sector).pointInQuad(xyz) )
    {
        return;
    }   // if still on same quad

    // Now we search through all graph nodes, starting with
    // the current one
    int indx       = *sector;
    float min_dist = 999999.9f;

    // If a current sector is given, and max_lookahead is specify, only test
    // the next max_lookahead graph nodes instead of testing the whole graph.
    // This is necessary for the AI: if the track contains a loop, e.g.:
    // -A--+---B---+----F--------
    //     E       C
    //     +---D---+
    // and the track is supposed to be driven: ABCDEBF, the AI might find
    // the node on F, and then keep on going straight ahead instead of
    // using the loop at all.
    unsigned int max_count  = (*sector!=UNKNOWN_SECTOR && all_sectors!=NULL)
                            ? (unsigned int)all_sectors->size()
                            : (unsigned int)m_all_nodes.size();
    *sector = UNKNOWN_SECTOR;
    for(unsigned int i=0; i<max_count; i++)
    {
        if(all_sectors)
            indx = (*all_sectors)[i];
        else
            indx = indx<(int)m_all_nodes.size()-1 ? indx +1 : 0;
        const Quad &q = getQuadOfNode(indx);
        float dist    = xyz.getY() - q.getMinHeight();
        // While negative distances are unlikely, we allow some small negative
        // numbers in case that the kart is partly in the track.
        if(q.pointInQuad(xyz) && dist < min_dist && dist>-1.0f)
        {
            min_dist = dist;
            *sector  = indx;
        }
    }   // for i<m_all_nodes.size()

    return;
}   // findRoadSector

//-----------------------------------------------------------------------------
/** findOutOfRoadSector finds the sector where XYZ is, but as it name
    implies, it is more accurate for the outside of the track than the
    inside, and for STK's needs the accuracy on top of the track is
    unacceptable; but if this was a 2D function, the accuracy for out
    of road sectors would be perfect.

    To find the sector we look for the closest line segment from the
    right and left drivelines, and the number of that segment will be
    the sector.

    The SIDE argument is used to speed up the function only; if we know
    that XYZ is on the left or right side of the track, we know that
    the closest driveline must be the one that matches that condition.
    In reality, the side used in STK is the one from the previous frame,
    but in order to move from one side to another a point would go
    through the middle, that is handled by findRoadSector() which doesn't
    has speed ups based on the side.

    NOTE: This method of finding the sector outside of the road is *not*
    perfect: if two line segments have a similar altitude (but enough to
    let a kart get through) and they are very close on a 2D system,
    if a kart is on the air it could be closer to the top line segment
    even if it is supposed to be on the sector of the lower line segment.
    Probably the best solution would be to construct a quad that reaches
    until the next higher overlapping line segment, and find the closest
    one to XYZ.
 */
int QuadGraph::findOutOfRoadSector(const Vec3& xyz,
                                   const int curr_sector,
                                   std::vector<int> *all_sectors) const
{
    int count = (all_sectors!=NULL) ? (int) all_sectors->size() : getNumNodes();
    int current_sector = 0;
    if(curr_sector != UNKNOWN_SECTOR && !all_sectors)
    {
        // We have to test all nodes here: reason is that on track with
        // shortcuts the n quads of the main drivelines is followed by
        // the quads of the shortcuts. So after quad n-1 (the last one
        // before the lap counting line) quad n will not be 0 (the first
        // quad after the lap counting line), but one of the quads on a
        // shortcut. If we only tested a limited number of quads to
        // improve the performance the crossing of a lap might not be
        // detected (because quad 0 is not tested, only quads on the
        // shortcuts are tested). If this should become a performance
        // bottleneck, we need to set up a graph of 'next' quads for each
        // quad (similar to what the AI does), and only test the quads
        // in this graph.
        const int LIMIT = getNumNodes();
        count           = LIMIT;
        // Start 10 quads before the current quad, so the quads closest
        // to the current position are tested first.
        current_sector  = curr_sector -10;
        if(current_sector<0) current_sector += getNumNodes();
    }

    int   min_sector = UNKNOWN_SECTOR;
    float min_dist_2 = 999999.0f*999999.0f;

    // If a kart is falling and in between (or too far below)
    // a driveline point it might not fulfill
    // the height condition. So we run the test twice: first with height
    // condition, then again without the height condition - just to make sure
    // it always comes back with some kind of quad.
    for(int phase=0; phase<2; phase++)
    {
        for(int j=0; j<count; j++)
        {
            int next_sector;
            if(all_sectors)
                next_sector = (*all_sectors)[j];
            else
                next_sector  = current_sector+1 == (int)getNumNodes()
                ? 0
                : current_sector+1;

            // A first simple test uses the 2d distance to the center of the quad.
            float dist_2 = m_all_nodes[next_sector]->getDistance2FromPoint(xyz);
            if(dist_2<min_dist_2)
            {
                const Quad &q = getQuadOfNode(next_sector);
                float dist    = xyz.getY() - q.getMinHeight();
                // While negative distances are unlikely, we allow some small
                // negative numbers in case that the kart is partly in the
                // track. Only do the height test in phase==0, in phase==1
                // accept any point, independent of height.
                if(phase==1 || (dist < 5.0f && dist>-1.0f) )
                {
                    min_dist_2 = dist_2;
                    min_sector = next_sector;
                }
            }
            current_sector = next_sector;
        }   // for j
        // Leave in phase 0 if any sector was found.
        if(min_sector!=UNKNOWN_SECTOR)
            return min_sector;
    }   // phase

    if(min_sector==UNKNOWN_SECTOR )
    {
        Log::info("Quad Grap", "unknown sector found.");
    }
    return min_sector;
}   // findOutOfRoadSector

//-----------------------------------------------------------------------------
/** Takes a snapshot of the driveline quads so they can be used as minimap.
 */
void QuadGraph::makeMiniMap(const core::dimension2du &dimension,
                            const std::string &name,
                            const video::SColor &fill_color,
                            video::ITexture** oldRttMinimap,
                            FrameBuffer** newRttMinimap)
{
    const SColor oldClearColor = World::getWorld()->getClearColor();
    World::getWorld()->setClearbackBufferColor(SColor(0, 255, 255, 255));
    World::getWorld()->forceFogDisabled(true);
    *oldRttMinimap = NULL;
    *newRttMinimap = NULL;

    RTT* newRttProvider = NULL;
    IrrDriver::RTTProvider* oldRttProvider = NULL;
    if (irr_driver->isGLSL())
    {
        newRttProvider = new RTT(dimension.Width, dimension.Height);
    }
    else
    {
        oldRttProvider = new IrrDriver::RTTProvider(dimension, name, true);
    }

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 255, 255, 255));

    video::SColor red(128, 255, 0, 0);
    createMesh(/*show_invisible part of the track*/ false,
               /*enable_transparency*/ false,
               /*track_color*/    &fill_color,
               /*lap line color*/  &red                       );

    m_node = irr_driver->addMesh(m_mesh, "mini_map");
#ifdef DEBUG
    m_node->setName("minimap-mesh");
#endif

    m_node->setAutomaticCulling(0);
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);

    // Add the camera:
    // ---------------
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
    Vec3 bb_min, bb_max;
    QuadSet::get()->getBoundingBox(&bb_min, &bb_max);
    Vec3 center = (bb_max+bb_min)*0.5f;

    float dx = bb_max.getX()-bb_min.getX();
    float dz = bb_max.getZ()-bb_min.getZ();

    // Set the scaling correctly. Also the center point (which is used
    // as the camera position) needs to be adjusted: the track must
    // be aligned to the left/top of the texture which is used (otherwise
    // mapPoint2MiniMap doesn't work), so adjust the camera position
    // that the track is properly aligned (view from the side):
    //          c        camera
    //         / \       .
    //        /   \     <--- camera angle
    //       /     \     .
    //      {  [-]  }   <--- track flat (viewed from the side)
    // If [-] is the shorter side of the track, then the camera will
    // actually render the area in { } - which is the length of the
    // longer side of the track.
    // To align the [-] side to the left, the camera must be moved
    // the distance betwwen '{' and '[' to the right. This distance
    // is exacly (longer_side - shorter_side) / 2.
    // So, adjust the center point by this amount:
    if(dz > dx)
    {
        center.setX(center.getX() + (dz-dx)*0.5f);
        m_scaling = dimension.Width / dz;
    }
    else
    {
        center.setZ(center.getZ() + (dx-dz)*0.5f);
        m_scaling = dimension.Width / dx;
    }

    float range = (dx>dz) ? dx : dz;

    core::matrix4 projection;
    projection.buildProjectionMatrixOrthoLH(range /* width */,
                                            range /* height */,
                                            -1, bb_max.getY()-bb_min.getY()+1);
    camera->setProjectionMatrix(projection, true);

    irr_driver->suppressSkyBox();
    irr_driver->clearLights();

    // Adjust Y position by +1 for max, -1 for min - this helps in case that
    // the maximum Y coordinate is negative (otherwise the minimap is mirrored)
    // and avoids problems for tracks which have a flat (max Y = min Y) minimap.
    camera->setPosition(core::vector3df(center.getX(), bb_min.getY() + 1.0f, center.getZ()));
    //camera->setPosition(core::vector3df(center.getX() - 5.0f, bb_min.getY() - 1 - 5.0f, center.getZ() - 15.0f));
    camera->setUpVector(core::vector3df(0, 0, 1));
    camera->setTarget(core::vector3df(center.getX(),bb_min.getY()-1,center.getZ()));
    //camera->setAspectRatio(1.0f);
    camera->updateAbsolutePosition();

    video::ITexture* texture = NULL;
    FrameBuffer* frame_buffer = NULL;

    if (irr_driver->isGLSL())
    {
        frame_buffer = newRttProvider->render(camera, GUIEngine::getLatestDt());

        // TODO: leak
        //delete newRttProvider;
    }
    else
    {
        texture = oldRttProvider->renderToTexture();
        delete oldRttProvider;
    }

    cleanupDebugMesh();
    irr_driver->removeCameraSceneNode(camera);
    m_min_coord = bb_min;


    if (texture == NULL && frame_buffer == NULL)
    {
        Log::error("Quad Graph", "[makeMiniMap] WARNING: RTT does not appear to work,"
                        "mini-map will not be available.");
    }

    *oldRttMinimap = texture;
    *newRttMinimap = frame_buffer;
    World::getWorld()->setClearbackBufferColor(oldClearColor);
    World::getWorld()->forceFogDisabled(false);

    irr_driver->getSceneManager()->clear();
    VAOManager::kill();
    irr_driver->clearGlowingNodes();
    irr_driver->clearLights();
    irr_driver->clearForcedBloom();
    irr_driver->clearBackgroundNodes();
}   // makeMiniMap

//-----------------------------------------------------------------------------
    /** Returns the 2d coordinates of a point when drawn on the mini map
     *  texture.
     *  \param xyz Coordinates of the point to map.
     *  \param draw_at The coordinates in pixel on the mini map of the point,
     *         only the first two coordinates will be used.
     */
void QuadGraph::mapPoint2MiniMap(const Vec3 &xyz,Vec3 *draw_at) const
{
    draw_at->setX((xyz.getX()-m_min_coord.getX())*m_scaling);
    draw_at->setY((xyz.getZ()-m_min_coord.getZ())*m_scaling);

}   // mapPoint
