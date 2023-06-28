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

#include "tracks/drive_graph.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "main_loop.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_lap.hpp"
#include "tracks/check_line.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

// ----------------------------------------------------------------------------
/** Constructor, loads the graph information for a given set of quads
 *  from a graph file.
 *  \param quad_file_name Name of the file of all quads
 *  \param graph_file_name Name of the file describing the actual graph
 */
DriveGraph::DriveGraph(const std::string &quad_file_name,
                       const std::string &graph_file_name,
                       const bool reverse) : m_reverse(reverse)
{
    m_lap_length    = 0;
    m_quad_filename = quad_file_name;
    Graph::setGraph(this);
    load(quad_file_name, graph_file_name);
}   // DriveGraph

// ----------------------------------------------------------------------------
void DriveGraph::addSuccessor(unsigned int from, unsigned int to)
{
    if(m_reverse)
        getNode(to)->addSuccessor(from);
    else
        getNode(from)->addSuccessor(to);

}   // addSuccessor

// ----------------------------------------------------------------------------
/** This function interprets a point specification as an attribute in the
    xml quad file. It understands two different specifications:
    p1="n:p"      : get point p from square n (n, p integers)
    p1="p1,p2,p3" : make a 3d point out of these 3 floating point values
*/
void DriveGraph::getPoint(const XMLNode *xml,
                          const std::string &attribute_name,
                          Vec3* result) const
{
    std::string s;
    xml->get(attribute_name, &s);
    int pos=(int)s.find_first_of(":");
    if(pos>0)   // n:p specification
    {
        std::vector<std::string> l = StringUtils::split(s, ':');
        int n=atoi(l[0].c_str());
        int p=atoi(l[1].c_str());
        *result=(*m_all_nodes[n])[p];
    }
    else
    {
        xml->get(attribute_name, result);
    }

}   // getPoint

// ----------------------------------------------------------------------------
/** Loads a drive graph from a file.
 *  \param filename Name of the quad file to load.
 *  \param filename Name of the graph file to load.
 */
void DriveGraph::load(const std::string &quad_file_name,
                      const std::string &filename)
{
    XMLNode *quad = file_manager->createXMLTree(quad_file_name);
    if (!quad || quad->getName() != "quads")
    {
        Log::error("DriveGraph : Quad xml '%s' not found.", filename.c_str());
        delete quad;
        return;
    }

    float min_height_testing = Graph::MIN_HEIGHT_TESTING;
    float max_height_testing = Graph::MAX_HEIGHT_TESTING;
    // Each quad is part of the graph exactly once now.
    for (unsigned int i = 0; i < quad->getNumNodes(); i++)
    {
        main_loop->renderGUI(3331, i, quad->getNumNodes());

        const XMLNode *xml_node = quad->getNode(i);
        if (!(xml_node->getName() == "quad" || xml_node->getName() == "height-testing"))
        {
            Log::warn("DriveGraph: Unsupported node type '%s' found in '%s' - ignored.",
                xml_node->getName().c_str(), filename.c_str());
            continue;
        }
        if (xml_node->getName() == "height-testing")
        {
            xml_node->get("min", &min_height_testing);
            xml_node->get("max", &max_height_testing);
            continue;
        }

        // Note that it's not easy to do the reading of the parameters here
        // in quad, since the specification in the xml can contain references
        // to previous points. E.g.:
        // <quad p0="40:3" p1="40:2" p2="25.396030 0.770338 64.796539" ...
        Vec3 p0, p1, p2, p3;
        getPoint(xml_node, "p0", &p0);
        getPoint(xml_node, "p1", &p1);
        getPoint(xml_node, "p2", &p2);
        getPoint(xml_node, "p3", &p3);
        bool invisible = false;
        xml_node->get("invisible", &invisible);
        bool ai_ignore = false;
        xml_node->get("ai-ignore", &ai_ignore);

        bool ignored = false;
        std::string direction;
        xml_node->get("direction", &direction);
        if (direction == "forward" && RaceManager::get()->getReverseTrack())
        {
            ignored = true;
            invisible = true;
            ai_ignore = true;
        }
        else if (direction == "reverse" && !RaceManager::get()->getReverseTrack())
        {
            ignored = true;
            invisible = true;
            ai_ignore = true;
        }

        createQuad(p0, p1, p2, p3, (unsigned int)m_all_nodes.size(), invisible, ai_ignore,
                   false/*is_arena*/, ignored);
    }
    for (unsigned i = 0; i < m_all_nodes.size(); i++)
    {
        m_all_nodes[i]->setHeightTesting(min_height_testing,
            max_height_testing);
    }
    delete quad;

    const XMLNode *xml = file_manager->createXMLTree(filename);

    if(!xml)
    {
        // No graph file exist, assume a default loop X -> X+1
        // Set the default loop:
        setDefaultSuccessors();
        computeDirectionData();

        if (m_all_nodes.size() > 0)
        {
            m_lap_length = getNode((int)m_all_nodes.size()-1)->getDistanceFromStart()
                         + getNode((int)m_all_nodes.size()-1)->getDistanceToSuccessor(0);
        }
        else
        {
            Log::error("DriveGraph", "No node in driveline graph.");
            m_lap_length = 10.0f;
        }

        return;
    }

    // The graph file exist, so read it in. The graph file must first contain
    // the node definitions, before the edges can be set.
    for(unsigned int node_index=0; node_index<xml->getNumNodes(); node_index++)
    {
        main_loop->renderGUI(3333, node_index, xml->getNumNodes());

        const XMLNode *xml_node = xml->getNode(node_index);
        // Load the definition of edges between the graph nodes:
        // -----------------------------------------------------
        if (xml_node->getName() == "node-list")
        {
            // Each quad is part of the graph exactly once now.
            unsigned int to = 0;
            xml_node->get("to-quad", &to);
            assert(to + 1 == m_all_nodes.size());
            continue;
        }
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
            Log::error("DriveGraph", "Incorrect specification in '%s': '%s' ignored.",
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
        float l = getNode(i)->getDistanceFromStart()
                + getNode(i)->getDistanceToSuccessor(0);
        if(l > m_lap_length)
            m_lap_length = l;
    }

    loadBoundingBoxNodes();

}   // load

// ----------------------------------------------------------------------------
/** Returns the index of the first graph node (i.e. the graph node which
 *  will trigger a new lap when a kart first enters it). This is always
 *  0 for normal direction (this is guaranteed by the track exporter),
 *  but in reverse mode (where node 0 is actually the end of the track)
 *  this is 0's successor.
 */
unsigned int DriveGraph::getStartNode() const
{
    return m_reverse ? getNode(0)->getSuccessor(0)
                     : 0;
}   // getStartNode

// ----------------------------------------------------------------------------
/** Sets the checkline requirements for all nodes in the graph.
 */
void DriveGraph::computeChecklineRequirements()
{
    computeChecklineRequirements(getNode(0),
                                 Track::getCurrentTrack()->getCheckManager()->getLapLineIndex());
}   // computeChecklineRequirements

// ----------------------------------------------------------------------------
/** Finds which checklines must be visited before driving on this quad
 *  (useful for rescue)
 */
void DriveGraph::computeChecklineRequirements(DriveNode* node,
                                              int latest_checkline)
{
    for (unsigned int n=0; n<node->getNumberOfSuccessors(); n++)
    {
        const int succ_id = node->getSuccessor(n);

        // warp-around
        if (succ_id == 0) break;

        DriveNode* succ = getNode(succ_id);
        int new_latest_checkline =
            Track::getCurrentTrack()->getCheckManager()->getChecklineTriggering(node->getCenter(),
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

        bool doRecursion = true;
        if (new_latest_checkline != -1)
        {
            // If we are about to add a 'new_latest_checkline' that has already been added,
            // we won't add new information and don't need to recurse.
            // This will greatly speed up computeChecklineRequirements for tracks with a higher number
            // of alternative drive lines and will also avoid adding the same value more than once.
            const std::vector<int>& checkline_requirements = succ->getChecklineRequirements();
            for (unsigned int i=0; i<checkline_requirements.size(); i++)
            {
                if (checkline_requirements[i] == new_latest_checkline)
                {
                    doRecursion = false;
                    break;
                }
            }
            if (doRecursion) // we haven't been here, so add it
                succ->setChecklineRequirements(new_latest_checkline);
        }
        if (doRecursion)
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
void DriveGraph::setupPaths()
{
    for(unsigned int i=0; i<getNumNodes(); i++)
    {
        getNode(i)->setupPathsToNode();
    }
}   // setupPaths

// -----------------------------------------------------------------------------
/** This function sets a default successor for all graph nodes that currently
 *  don't have a successor defined. The default successor of node X is X+1.
 */
void DriveGraph::setDefaultSuccessors()
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++) {
        if(getNode(i)->getNumberOfSuccessors()==0) {
            addSuccessor(i,i+1>=m_all_nodes.size() ? 0 : i+1);
            //~ getNode(i)->addSuccessor(i+1>=m_all_nodes.size() ? 0 : i+1);
        }   // if size==0
    }   // for i<m_allNodes.size()
}   // setDefaultSuccessors

// -----------------------------------------------------------------------------
/** Sets all start positions depending on the drive graph. The number of
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
void DriveGraph::setDefaultStartPositions(AlignedArray<btTransform>
                                                       *start_transforms,
                                          unsigned int karts_per_row,
                                          float forwards_distance,
                                          float sidewards_distance,
                                          float upwards_distance) const
{
    // We start just before the start node (which will trigger lap
    // counting when reached). The first predecessor is the one on
    // the main driveline.
    int current_node = getNode(getStartNode())->getPredecessor(0);

    float distance_from_start = 0.75f+forwards_distance;
    float multiplier = 1.0f;
    if (sidewards_distance < 0)
    {
        sidewards_distance = -sidewards_distance;
        multiplier = -1.0f;
    }

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
            while(distance_from_start > getNode(current_node)->getNodeLength())
            {
                distance_from_start -= getNode(current_node)->getNodeLength();
                // Only follow the main driveline, i.e. first predecessor
                current_node = getNode(current_node)->getPredecessor(0);
            }
            const DriveNode* dn = getNode(current_node);
            Vec3 center_line = dn->getLowerCenter() - dn->getUpperCenter();
            center_line.normalize();

            Vec3 horizontal_line = (*dn)[2] - (*dn)[3];
            horizontal_line.normalize();

            Vec3 start = dn->getUpperCenter()
                       + center_line     * distance_from_start
                       + horizontal_line * x_pos * multiplier;
            // Add a certain epsilon to the height in case that the
            // drivelines are beneath the track.
            (*start_transforms)[i].setOrigin(start+Vec3(0,upwards_distance,0));
            (*start_transforms)[i].setRotation(
                btQuaternion(btVector3(0, 1, 0),
                             dn->getAngleToSuccessor(0)));
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
/** Returns the list of successors or a node.
 *  \param node_number The number of the node.
 *  \param succ A vector of ints to which the successors are added.
 *  \param for_ai true if only quads accessible by the AI should be returned.
 */
void DriveGraph::getSuccessors(int node_number,
                               std::vector<unsigned int>& succ,
                               bool for_ai) const
{
    const DriveNode *dn=getNode(node_number);
    for(unsigned int i=0; i<dn->getNumberOfSuccessors(); i++)
    {
        // If getSuccessor is called for the AI, only add
        // quads that are meant for the AI to be used.
        if(!for_ai || !dn->ignoreSuccessorForAI(i))
            succ.push_back(dn->getSuccessor(i));
    }
}   // getSuccessors

// ----------------------------------------------------------------------------
/** Recursively determines the distance the beginning (lower end) of the quads
 *  have from the start of the track.
 *  \param node The node index for which to set the distance from start.
 *  \param new_distance The new distance for the specified graph node.
 */
void DriveGraph::computeDistanceFromStart(unsigned int node, float new_distance)
{
    DriveNode *dn = getNode(node);
    float current_distance = dn->getDistanceFromStart();

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
            updateDistancesForAllSuccessors(dn->getIndex(), delta, 0);
        }
        return;
    }

    // Otherwise this node has no distance defined yet. Set the new
    // distance, and recursively update all following nodes.
    dn->setDistanceFromStart(new_distance);

    for(unsigned int i=0; i<dn->getNumberOfSuccessors(); i++)
    {
        DriveNode *dn_next = getNode(dn->getSuccessor(i));
        // The start node (only node with distance 0) is reached again,
        // recursion can stop now
        if(dn_next->getDistanceFromStart()==0)
            continue;

        computeDistanceFromStart(dn_next->getIndex(),
                                 new_distance + dn->getDistanceToSuccessor(i));
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
void DriveGraph::updateDistancesForAllSuccessors(unsigned int indx, float delta,
                                                 unsigned int recursive_count)
{
    if(recursive_count>getNumNodes())
    {
        Log::error("DriveGraph",
                   "DriveGraph contains a loop (without start node).");
        Log::fatal("DriveGraph",
                   "Fix graph, check for directions of all shortcuts etc.");
    }
    recursive_count++;

    DriveNode* dn = getNode(indx);
    dn->setDistanceFromStart(dn->getDistanceFromStart()+delta);
    for(unsigned int i=0; i<dn->getNumberOfSuccessors(); i++)
    {
        DriveNode* dn_next = getNode(dn->getSuccessor(i));
        // Stop when we reach the start node, i.e. the only node with a
        // distance of 0
        if(dn_next->getDistanceFromStart()==0)
            continue;

        // Only increase the distance from start of a successor node, if
        // this successor has a distance from start that is smaller then
        // the increased amount.
        if(dn->getDistanceFromStart()+dn->getDistanceToSuccessor(i) >
            dn_next->getDistanceFromStart())
        {
            updateDistancesForAllSuccessors(dn->getSuccessor(i), delta,
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
void DriveGraph::computeDirectionData()
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        for(unsigned int succ_index=0;
            succ_index<getNode(i)->getNumberOfSuccessors();
            succ_index++)
        {
            determineDirection(i, succ_index);
        }   // for next < getNumberOfSuccessor

    }   // for i < m_all_nodes.size()
}   // computeDirectionData

//-----------------------------------------------------------------------------
/** Adjust the given angle to be in [-PI, PI].
 */
float DriveGraph::normalizeAngle(float f)
{
    if(f>M_PI)       f -= 2*M_PI;
    else if(f<-M_PI) f += 2*M_PI;
    return f;
}   // normalizeAngle
//-----------------------------------------------------------------------------
/** Determines the direction of the drive graph when driving to the specified
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
void DriveGraph::determineDirection(unsigned int current,
                                    unsigned int succ_index)
{
    // The maximum angle which is still considered to be straight
    const float max_straight_angle=0.1f;

    // Compute the angle from n (=current) to n+1 (=next)
    float angle_current = getAngleToNext(current, succ_index);
    unsigned int next   = getNode(current)->getSuccessor(succ_index);
    float angle_next    = getAngleToNext(next, 0);
    float rel_angle     = normalizeAngle(angle_next-angle_current);
    // Small angles are considered to be straight
    if(fabsf(rel_angle)<max_straight_angle)
        rel_angle = 0;

    next     = getNode(next)->getSuccessor(0);  // next is now n+2

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

        next = getNode(next)->getSuccessor(0);
    }    // while(1)

    DriveNode::DirectionType dir =
        rel_angle==0 ? DriveNode::DIR_STRAIGHT
                     : (rel_angle>0) ? DriveNode::DIR_RIGHT
                                     : DriveNode::DIR_LEFT;
    getNode(current)->setDirectionData(succ_index, dir, next);
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
void DriveGraph::spatialToTrack(Vec3 *dst, const Vec3& xyz,
                              const int sector) const
{
    if(sector == UNKNOWN_SECTOR )
    {
        Log::warn("Drive Graph", "UNKNOWN_SECTOR in spatialToTrack().");
        return;
    }

    getNode(sector)->getDistances(xyz, dst);
}   // spatialToTrack

//-----------------------------------------------------------------------------
float DriveGraph::getDistanceToNext(int n, int j) const
{
    return getNode(n)->getDistanceToSuccessor(j);
}   // getDistanceToNext

//-----------------------------------------------------------------------------
float DriveGraph::getAngleToNext(int n, int j) const
{
    return getNode(n)->getAngleToSuccessor(j);
}   // getAngleToNext

//-----------------------------------------------------------------------------
int DriveGraph::getNumberOfSuccessors(int n) const
{
    return getNode(n)->getNumberOfSuccessors();
}   // getNumberOfSuccessors

//-----------------------------------------------------------------------------
float DriveGraph::getDistanceFromStart(int j) const
{
    return getNode(j)->getDistanceFromStart();
}   // getDistanceFromStart

// -----------------------------------------------------------------------------
void DriveGraph::differentNodeColor(int n, video::SColor* c) const
{
    if (UserConfigParams::m_track_debug)
    {
        if (getNode(n)->is3DQuad())
            *c = video::SColor(255, 0, 255, 0);
        else
            *c = video::SColor(255, 255, 255, 0);
    }

}   // differentNodeColor

// -----------------------------------------------------------------------------
DriveNode* DriveGraph::getNode(unsigned int j) const
{
    assert(j < m_all_nodes.size());
    DriveNode* n = dynamic_cast<DriveNode*>(m_all_nodes[j]);
    assert(n != NULL);
    return n;
}   // getNode

// -----------------------------------------------------------------------------
bool DriveGraph::hasLapLine() const
{
    if (Track::getCurrentTrack()->isCTF() &&
        RaceManager::get()->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        return false;
    return true;
}   // hasLapLine
