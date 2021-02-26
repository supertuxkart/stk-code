//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart Team
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

#include "tracks/arena_graph.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"
#include "tracks/arena_node.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"

#include <algorithm>
#include <queue>

// -----------------------------------------------------------------------------
ArenaGraph::ArenaGraph(const std::string &navmesh, const XMLNode *node)
          : Graph()
{
    loadNavmesh(navmesh);
    buildGraph();
    // Compute shortest distance from all nodes
    for (unsigned int i = 0; i < getNumNodes(); i++)
        computeDijkstra(i);

    setNearbyNodesOfAllNodes();
    if (node && RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
        loadGoalNodes(node);

    loadBoundingBoxNodes();

}   // ArenaGraph

// -----------------------------------------------------------------------------
ArenaNode* ArenaGraph::getNode(unsigned int i) const
{
    assert(i < m_all_nodes.size());
    ArenaNode* n = dynamic_cast<ArenaNode*>(m_all_nodes[i]);
    assert(n != NULL);
    return n;
}   // getNode

// -----------------------------------------------------------------------------
void ArenaGraph::differentNodeColor(int n, video::SColor* c) const
{
    std::set<int>::iterator it;
    it = m_red_node.find(n);
    if (it != m_red_node.end())
    {
        *c = video::SColor(255, 255, 0, 0);
        return;
    }

    it = m_blue_node.find(n);
    if (it != m_blue_node.end())
    {
        *c = video::SColor(255, 0, 0, 255);
        return;
    }

    if (UserConfigParams::m_track_debug)
    {
        if (m_all_nodes[n]->is3DQuad())
            *c = video::SColor(255, 0, 255, 0);
        else
            *c = video::SColor(255, 255, 255, 0);
    }

}   // differentNodeColor

// -----------------------------------------------------------------------------
void ArenaGraph::loadNavmesh(const std::string &navmesh)
{
    XMLNode *xml = file_manager->createXMLTree(navmesh);
    if (xml->getName() != "navmesh")
    {
        Log::error("ArenaGraph", "NavMesh is invalid.");
        delete xml;
        return;
    }

    std::vector<Vec3> all_vertices;
    for (unsigned int i = 0; i < xml->getNumNodes(); i++)
    {
        const XMLNode *xml_node = xml->getNode(i);
        if (xml_node->getName() == "vertices")
        {
            for (unsigned int i = 0; i < xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if (!(xml_node_node->getName() == "vertex"))
                {
                    Log::error("ArenaGraph", "Unsupported type '%s' found"
                        "in '%s' - ignored.",
                        xml_node_node->getName().c_str(), navmesh.c_str());
                    continue;
                }

                // Reading vertices
                float x, y, z;
                xml_node_node->get("x", &x);
                xml_node_node->get("y", &y);
                xml_node_node->get("z", &z);
                Vec3 p(x, y, z);
                all_vertices.push_back(p);
            }
        }

        if (xml_node->getName() == "faces")
        {
            for (unsigned int i = 0; i < xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if (xml_node_node->getName() != "face")
                {
                    Log::error("ArenaGraph", "Unsupported type '%s'"
                        " found in '%s' - ignored.",
                        xml_node_node->getName().c_str(), navmesh.c_str());
                    continue;
                }

                // Reading quads
                std::vector<int> quad_index;
                std::vector<int> adjacent_quad_index;
                xml_node_node->get("indices", &quad_index);
                xml_node_node->get("adjacents", &adjacent_quad_index);
                if (quad_index.size() != 4)
                {
                    Log::error("ArenaGraph", "A Node in navmesh is not made"
                        " of quad, will only use the first 4 vertices");
                }

                createQuad(all_vertices[quad_index[0]],
                    all_vertices[quad_index[1]], all_vertices[quad_index[2]],
                    all_vertices[quad_index[3]], (int)m_all_nodes.size(),
                    false/*invisible*/, false/*ai_ignore*/, true/*is_arena*/,
                    false/*ignore*/);

                ArenaNode* cur_node = getNode((int)m_all_nodes.size() - 1);
                cur_node->setAdjacentNodes(adjacent_quad_index);
            }
        }
    }
    const XMLNode* ht = xml->getNode("height-testing");
    if (ht)
    {
        float min = Graph::MIN_HEIGHT_TESTING;
        float max = Graph::MAX_HEIGHT_TESTING;
        ht->get("min", &min);
        ht->get("max", &max);
        for (unsigned i = 0; i < m_all_nodes.size(); i++)
        {
            m_all_nodes[i]->setHeightTesting(min, max);
        }
    }
    delete xml;

}   // loadNavmesh

// ----------------------------------------------------------------------------
void ArenaGraph::buildGraph()
{
    const unsigned int n_nodes = getNumNodes();

    m_distance_matrix = std::vector<std::vector<float>>
        (n_nodes, std::vector<float>(n_nodes, 9999.9f));
    for (unsigned int i = 0; i < n_nodes; i++)
    {
        ArenaNode* cur_node = getNode(i);
        for (const int& adjacent : cur_node->getAdjacentNodes())
        {
            Vec3 diff = getNode(adjacent)->getCenter() - cur_node->getCenter();
            float distance = diff.length();
            m_distance_matrix[i][adjacent] = distance;
        }
        m_distance_matrix[i][i] = 0.0f;
    }

    // Allocate and initialise the previous node data structure:
    m_parent_node = std::vector<std::vector<int16_t>>
        (n_nodes, std::vector<int16_t>(n_nodes, Graph::UNKNOWN_SECTOR));
    for (unsigned int i = 0; i < n_nodes; i++)
    {
        for (unsigned int j = 0; j < n_nodes; j++)
        {
            if (i == j || m_distance_matrix[i][j] >= 9899.9f)
                m_parent_node[i][j] = -1;
            else
                m_parent_node[i][j] = i;
        }   // for j
    }   // for i

}   // buildGraph

// ----------------------------------------------------------------------------
/** Dijkstra shortest path computation. It computes the shortest distance from
 *  the specified node 'source' to all other nodes. At the end of the
 *  computation, m_distance_matrix[i][j] stores the shortest path distance from
 *  source to j and m_parent_node[source][j] stores the last vertex visited on
 *  the shortest path from i to j before visiting j. Suppose the shortest path
 *  from i to j is i->......->k->j  then m_parent_node[i][j] = k
 */
void ArenaGraph::computeDijkstra(int source)
{
    // Stores the distance (float) to 'source' from a specified node (int)
    typedef std::pair<int, float> IndDistPair;

    class Shortest
    {
    public:
        bool operator()(const IndDistPair &p1, const IndDistPair &p2)
        {
            return p1.second > p2.second;
        }
    };

    std::priority_queue<IndDistPair, std::vector<IndDistPair>, Shortest> queue;
    IndDistPair begin(source, 0.0f);
    queue.push(begin);
    const unsigned int n = getNumNodes();
    std::vector<bool> visited;
    visited.resize(n, false);
    while (!queue.empty())
    {
        // Get element with shortest path
        IndDistPair current = queue.top();
        queue.pop();
        int cur_index = current.first;
        if (visited[cur_index]) continue;
        visited[cur_index] = true;

        for (const int& adjacent : getNode(cur_index)->getAdjacentNodes())
        {
            // Distance already computed, can be ignored
            if (visited[adjacent]) continue;

            float new_dist =
                current.second + m_distance_matrix[cur_index][adjacent];
            if (new_dist < m_distance_matrix[source][adjacent])
            {
                m_distance_matrix[source][adjacent] = new_dist;
                m_parent_node[source][adjacent] = cur_index;
            }
            IndDistPair pair(adjacent, new_dist);
            queue.push(pair);
        }
    }
}   // computeDijkstra

// ----------------------------------------------------------------------------
/** THIS FUNCTION IS ONLY USED FOR UNIT-TESTING, to verify that the new
 *  Dijkstra algorithm gives the same results.
 *  computeFloydWarshall() computes the shortest distance between any two
 *  nodes. At the end of the computation, m_distance_matrix[i][j] stores the
 *  shortest path distance from i to j and m_parent_node[i][j] stores the last
 *  vertex visited on the shortest path from i to j before visiting j. Suppose
 *  the shortest path from i to j is i->......->k->j  then
 *  m_parent_node[i][j] = k
 */
void ArenaGraph::computeFloydWarshall()
{
    unsigned int n = getNumNodes();

    for (unsigned int k = 0; k < n; k++)
    {
        for (unsigned int i = 0; i < n; i++)
        {
            for (unsigned int j = 0; j < n; j++)
            {
                if ((m_distance_matrix[i][k] + m_distance_matrix[k][j]) <
                    m_distance_matrix[i][j])
                {
                    m_distance_matrix[i][j] =
                        m_distance_matrix[i][k] + m_distance_matrix[k][j];
                    m_parent_node[i][j] = m_parent_node[k][j];
                }
            }
        }
    }

}   // computeFloydWarshall

// -----------------------------------------------------------------------------
void ArenaGraph::loadGoalNodes(const XMLNode *node)
{
    const XMLNode *check_node = node->getNode("checks");
    for (unsigned int i = 0; i < check_node->getNumNodes(); i++)
    {
        const XMLNode *goal = check_node->getNode(i);
        if (goal->getName() =="goal")
        {
            Vec3 p1, p2;
            bool first_goal = false;
            goal->get("first_goal", &first_goal);
            goal->get("p1", &p1);
            goal->get("p2", &p2);

            int first = Graph::UNKNOWN_SECTOR;
            findRoadSector(p1, &first, NULL, true);
            int last = Graph::UNKNOWN_SECTOR;
            findRoadSector(p2, &last, NULL, true);

            // Avoid possible infinite loop
            if (first == Graph::UNKNOWN_SECTOR ||
                last == Graph::UNKNOWN_SECTOR)
                continue;

            first_goal ? m_blue_node.insert(first) : m_red_node.insert(first);
            first_goal ? m_blue_node.insert(last) : m_red_node.insert(last);
            while (first != last)
            {
                // Find all the nodes which connect the two points of
                // goal, notice: only work if it's a straight line
                first = getNextNode(first, last);
                // Happens when broken navmesh with separated nodes
                if (first == Graph::UNKNOWN_SECTOR)
                    break;
                first_goal ? m_blue_node.insert(first) :
                    m_red_node.insert(first);
            }
        }
    }
}   // loadGoalNodes

// ----------------------------------------------------------------------------
void ArenaGraph::setNearbyNodesOfAllNodes()
{
    // Only save the nearby 8 nodes
    const unsigned int try_count = 8;
    for (unsigned int i = 0; i < getNumNodes(); i++)
    {
        // Get the distance to all nodes at i
        ArenaNode* cur_node = getNode(i);
        std::vector<int> nearby_nodes;
        std::vector<float> dist = m_distance_matrix[i];

        // Skip the same node
        dist[i] = 999999.0f;
        for (unsigned int j = 0; j < try_count; j++)
        {
            std::vector<float>::iterator it =
                std::min_element(dist.begin(), dist.end());
            const int pos = int(it - dist.begin());
            nearby_nodes.push_back(pos);
            dist[pos] = 999999.0f;
        }
        cur_node->setNearbyNodes(nearby_nodes);
    }

}   // setNearbyNodesOfAllNodes

// ----------------------------------------------------------------------------
/** Determines the full path from 'from' to 'to' and returns it in a
 *  std::vector (in reverse order). Used only for unit testing.
 */
std::vector<int16_t> ArenaGraph::getPathFromTo(int from, int to,
                      const std::vector< std::vector< int16_t > >& parent_node)
{
    std::vector<int16_t> path;
    path.push_back(to);
    while(from!=to)
    {
        to = parent_node[from][to];
        path.push_back(to);
    }
    return path;
}   // getPathFromTo

// ============================================================================
/** Unit testing for arena graph distance and parent node computation.
 *  Instead of using hand-tuned test cases we use the tested, verified and
 *  easier to understand Floyd-Warshall algorithm to compute the distances,
 *  and check if the (significanty faster) Dijkstra algorithm gives the same
 *  results. For now we use the cave mesh as test case.
 */
void ArenaGraph::unitTesting()
{
    Track *track = track_manager->getTrack("cave");
    std::string navmesh_file_name=track->getTrackFile("navmesh.xml");

    double s = StkTime::getRealTime();
    ArenaGraph* ag = new ArenaGraph(navmesh_file_name);
    double e = StkTime::getRealTime();
    Log::error("Time", "Dijkstra       %lf", e-s);

    // Save the Dijkstra results
    std::vector< std::vector< float > > distance_matrix = ag->m_distance_matrix;
    std::vector< std::vector< int16_t > > parent_node = ag->m_parent_node;
    ag->buildGraph();

    // Now compute results with Floyd-Warshall
    s = StkTime::getRealTime();
    ag->computeFloydWarshall();
    e = StkTime::getRealTime();
    Log::error("Time", "Floyd-Warshall %lf", e-s);

    int error_count = 0;
    for(unsigned int i=0; i<ag->m_distance_matrix.size(); i++)
    {
        for(unsigned int j=0; j<ag->m_distance_matrix[i].size(); j++)
        {
            if(ag->m_distance_matrix[i][j] - distance_matrix[i][j] > 0.001f)
            {
                Log::error("ArenaGraph",
                           "Incorrect distance %d, %d: Dijkstra: %f F.W.: %f",
                           i, j, distance_matrix[i][j], ag->m_distance_matrix[i][j]);
                error_count++;
            }    // if distance is too different

            // Unortunately it happens frequently that there are different
            // shortest path with the same length. And Dijkstra might find
            // a different path then Floyd-Warshall. So the test for parent
            // polygon often results in false positives, so it is disabled,
            // but I leave the code in place in case it is useful for some
            // debugging in the feature
#undef TEST_PARENT_POLY_EVEN_THOUGH_MANY_FALSE_POSITIVES
#ifdef TEST_PARENT_POLY_EVEN_THOUGH_MANY_FALSE_POSITIVES
            if(ag->m_parent_node[i][j] != parent_node[i][j])
            {
                error_count++;
                std::vector<int16_t> dijkstra_path = getPathFromTo(i, j, parent_node);
                std::vector<int16_t> floyd_path = getPathFromTo(i, j, ag->m_parent_node);
                if(dijkstra_path.size()!=floyd_path.size())
                {
                    Log::error("ArenaGraph",
                               "Incorrect path length %d, %d: Dijkstra: %d F.W.: %d",
                               i, j, parent_node[i][j], ag->m_parent_node[i][j]);
                    continue;
                }
                Log::error("ArenaGraph", "Path problems from %d to %d:",
                           i, j);
                for (unsigned k = 0; k < dijkstra_path.size(); k++)
                {
                    if(dijkstra_path[k]!=floyd_path[k])
                        Log::error("ArenaGraph", "%d/%d dijkstra: %d floyd %d",
                            k, dijkstra_path.size(), dijkstra_path[k],
                            floyd_path[k]);
                }    // for k<dijkstra_path.size()

            }   // if dijkstra parent_node != floyd parent node
#endif
        }   // for j
    }   // for i

    delete ag;

}   // unitTesting
