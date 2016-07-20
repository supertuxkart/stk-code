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

#include "tracks/battle_graph.hpp"

#include <IMesh.h>
#include <ICameraSceneNode.h>
#include <IMeshSceneNode.h>

#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "items/item_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/navmesh.hpp"
#include "utils/log.hpp"

#include <queue>

const int BattleGraph::UNKNOWN_POLY  = -1;
BattleGraph * BattleGraph::m_battle_graph = NULL;

/** Constructor, Creates a navmesh, builds a graph from the navmesh and
*    then runs shortest path algorithm to find and store paths to be used
*    by the AI. */
BattleGraph::BattleGraph(const std::string &navmesh_file_name,
                         const XMLNode& node)
{
    m_items_on_graph.clear();

    NavMesh::create(navmesh_file_name);
    m_navmesh_file = navmesh_file_name;
    buildGraph(NavMesh::get());
    double s = StkTime::getRealTime();
    for(unsigned int i=0; i < NavMesh::get()->getNumberOfPolys(); i++)
        computeDijkstra(i);
    double e = StkTime::getRealTime();
    Log::error("dijkstra:","Time %lf", e-s);
    for(unsigned int i=0; i<m_distance_matrix[2].size(); i++)
        Log::error("dijkstra", "%d %f ", i, m_distance_matrix[2][i]);
    buildGraph(NavMesh::get());
    s = StkTime::getRealTime();
    computeFloydWarshall();
    e = StkTime::getRealTime();
    Log::error("floyd warshall", "Time %lf", e-s);
    for(unsigned int i=0; i<m_distance_matrix[2].size(); i++)
        Log::error("floyd", "%d %f ", i, m_distance_matrix[2][i]);

    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
        loadGoalNodes(node);

} // BattleGraph

// -----------------------------------------------------------------------------
/** Destructor, destroys NavMesh and the debug mesh if it exists */
BattleGraph::~BattleGraph(void)
{
    NavMesh::destroy();

    if(UserConfigParams::m_track_debug)
        cleanupDebugMesh();
    GraphStructure::destroyRTT();
} // ~BattleGraph

// -----------------------------------------------------------------------------
/** Builds a graph from an existing NavMesh. The graph is stored as an adjacency
*    matrix. */
void BattleGraph::buildGraph(NavMesh* navmesh)
{
    unsigned int n_polys = navmesh->getNumberOfPolys();

    m_distance_matrix = std::vector< std::vector<float> > (n_polys, std::vector<float>(n_polys, 9999.9f));
    for(unsigned int i=0; i<n_polys; i++)
    {
        NavPoly currentPoly = navmesh->getNavPoly(i);
        const std::vector<int> &adjacents = navmesh->getAdjacentPolys(i);
        for(unsigned int j=0; j<adjacents.size(); j++)
        {
            Vec3 diff = navmesh->getCenterOfPoly(adjacents[j]) - currentPoly.getCenter();
            float distance = diff.length();
            m_distance_matrix[i][adjacents[j]] = distance;
            //m_distance_matrix[adjacents[j]][i] = distance;
        }
        m_distance_matrix[i][i] = 0.0f;
    }

}    // buildGraph

// -----------------------------------------------------------------------------
#include <iostream>
#include <queue>
#include <vector>
#include <climits>

void BattleGraph::computeDijkstra(int source)
{
    // Stores the distance (float) to 'source' from a specified node (int)
    typedef std::pair<int, float> IndDistPair;

    IndDistPair begin(source, 0.0f);
    class Shortest
    {
    public:
        bool operator()(const IndDistPair &p1, const IndDistPair &p2)
        {
            return p1.second > p2.second;
        }
    };
    std::priority_queue<IndDistPair, std::vector<IndDistPair>, Shortest> queue;
    queue.push(begin);
    const unsigned int n=getNumNodes();
    std::vector<bool> visited;
    visited.resize(n, false);
    NavMesh *navmesh = NavMesh::get();
    while(!queue.empty())
    {
        // Get element with shortest path
        IndDistPair current = queue.top();
        queue.pop();
        int cur_index = current.first;
        if(visited[cur_index]) continue;
        visited[cur_index] = true;
        const NavPoly &current_poly = navmesh->getNavPoly(cur_index);
        const std::vector<int> &adjacents = current_poly.getAdjacents();
        for(unsigned int j=0; j<adjacents.size(); j++)
        {
            int adjacent = adjacents[j];
            // Distance already computed, can be ignored
            if(visited[adjacent]) continue;

            float new_dist = current.second + m_distance_matrix[cur_index][adjacent];
            if(new_dist < m_distance_matrix[source][adjacent])
                m_distance_matrix[source][adjacent] = new_dist;
            IndDistPair pair(adjacent, new_dist);
            queue.push(pair);
        }
    }
    return;

    using namespace std;
#define INF INT_MAX //Infinity
    const int sz=10001; //Maximum possible number of vertices. Preallocating space for DataStructures accordingly
    vector<pair<int,int> > a[sz]; //Adjacency list
    int dis[sz]; //Stores shortest distance
    bool vis[sz]={0}; //Determines whether the node has been visited or not

    for (int i = 0; i < sz; i++) //Set initial distances to Infinity
        dis[i] = INF;

    //Custom Comparator for Determining priority for priority queue (shortest edge comes first)
    class prioritize {
    public: bool operator ()(pair<int, int>&p1, pair<int, int>&p2)
    {
        return p1.second > p2.second;
    }
    };

    //Priority queue to store vertex,weight pairs
    priority_queue<pair<int, int>, vector<pair<int, int> >, prioritize> pq;
    pq.push(make_pair(source, dis[source] = 0)); //Pushing the source with distance from itself as 0
    while (!pq.empty())
    {
        pair<int, int> curr = pq.top(); //Current vertex. The shortest distance for this has been found
        pq.pop();
        int cv = curr.first, cw = curr.second; //'cw' the final shortest distance for this vertex
        if (vis[cv]) //If the vertex is already visited, no point in exploring adjacent vertices
            continue;
        vis[cv] = true;
        for (int i = 0; i < a[cv].size(); i++) //Iterating through all adjacent vertices
            if (!vis[a[cv][i].first] && a[cv][i].second + cw < dis[a[cv][i].first]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
                pq.push(make_pair(a[cv][i].first, (dis[a[cv][i].first] = a[cv][i].second + cw))); //Set the new distance and add to priority queue
    }

}   // computeDijkstra

// -----------------------------------------------------------------------------
/** computeFloydWarshall() computes the shortest distance between any two nodes.
 *  At the end of the computation, m_distance_matrix[i][j] stores the shortest path
 *  distance from i to j and m_parent_poly[i][j] stores the last vertex visited on the
 *  shortest path from i to j before visiting j. Suppose the shortest path from i to j is
 *  i->......->k->j  then m_parent_poly[i][j] = k
 */
void BattleGraph::computeFloydWarshall()
{
    unsigned int n = getNumNodes();

    // initialize m_parent_poly with unknown_poly so that if no path is found b/w i and j
    // then m_parent_poly[i][j] = -1 (UNKNOWN_POLY)
    // AI must check this
    m_parent_poly = std::vector< std::vector<int> > (n, std::vector<int>(n,BattleGraph::UNKNOWN_POLY));
    for(unsigned int i=0; i<n; i++)
    {
        for(unsigned int j=0; j<n; j++)
        {
            if(i == j || m_distance_matrix[i][j]>=9899.9f) m_parent_poly[i][j]=-1;
            else    m_parent_poly[i][j] = i;
        }
    }

    for(unsigned int k=0; k<n; k++)
    {
        for(unsigned int i=0; i<n; i++)
        {
            for(unsigned int j=0; j<n; j++)
            {
                if( (m_distance_matrix[i][k] + m_distance_matrix[k][j]) < m_distance_matrix[i][j])
                {
                    m_distance_matrix[i][j] = m_distance_matrix[i][k] + m_distance_matrix[k][j];
                    m_parent_poly[i][j] = m_parent_poly[k][j];
                }
            }
        }
    }

}    // computeFloydWarshall

// -----------------------------------------------------------------------------
/** Maps items on battle graph */
void BattleGraph::findItemsOnGraphNodes()
{
    const ItemManager* item_manager = ItemManager::get();
    unsigned int item_count = item_manager->getNumberOfItems();

    for (unsigned int i = 0; i < item_count; ++i)
    {
        const Item* item = item_manager->getItem(i);
        Vec3 xyz = item->getXYZ();
        int polygon = BattleGraph::UNKNOWN_POLY;

        for (unsigned int j = 0; j < this->getNumNodes(); ++j)
        {
            if (NavMesh::get()->getNavPoly(j).pointInPoly(xyz, false))
                polygon = j;
        }

        if (polygon != BattleGraph::UNKNOWN_POLY)
        {
            m_items_on_graph.push_back(std::make_pair(item, polygon));
            Log::debug("BattleGraph","item number %d is on polygon %d", i, polygon);
        }
        else
            Log::debug("BattleGraph","Can't map item number %d with a suitable polygon", i);
    }
}    // findItemsOnGraphNodes

// -----------------------------------------------------------------------------
int BattleGraph::pointToNode(const int cur_node,
                             const Vec3& cur_point,
                             bool ignore_vertical) const
{
    int final_node = BattleGraph::UNKNOWN_POLY;

    if (cur_node == BattleGraph::UNKNOWN_POLY)
    {
        // Try all nodes in the battle graph
        bool found = false;
        unsigned int node = 0;
        while (!found && node < this->getNumNodes())
        {
            const NavPoly& p_all = this->getPolyOfNode(node);
            if (p_all.pointInPoly(cur_point, ignore_vertical))
            {
                final_node = node;
                found = true;
            }
            node++;
        }
    }
    else
    {
        // Check if the point is still on the same node
        const NavPoly& p_cur = this->getPolyOfNode(cur_node);
        if (p_cur.pointInPoly(cur_point, ignore_vertical)) return cur_node;

        // If not then check all adjacent polys
        const std::vector<int>& adjacents = NavMesh::get()
            ->getAdjacentPolys(cur_node);

        bool found = false;
        unsigned int num = 0;
        while (!found && num < adjacents.size())
        {
            const NavPoly& p_temp = this->getPolyOfNode(adjacents[num]);
            if (p_temp.pointInPoly(cur_point, ignore_vertical))
            {
                final_node = adjacents[num];
                found = true;
            }
            num++;
        }

        // Current node is still unkown
        if (final_node == BattleGraph::UNKNOWN_POLY)
        {
            // Calculated distance from saved node to current position,
            // if it's close enough than use the saved node anyway, it
            // may happen when the kart stays on the edge of obstacles
            const NavPoly& p = this->getPolyOfNode(cur_node);
            const float dist = (p.getCenter() - cur_point).length_2d();

            if (dist < 3.0f)
                final_node = cur_node;
        }
    }
    return final_node;
}    // pointToNode

// -----------------------------------------------------------------------------
const int BattleGraph::getNextShortestPathPoly(int i, int j) const
{
    if (i == BattleGraph::UNKNOWN_POLY || j == BattleGraph::UNKNOWN_POLY)
        return BattleGraph::UNKNOWN_POLY;
    return m_parent_poly[j][i];
}    // getNextShortestPathPoly

// -----------------------------------------------------------------------------
const bool BattleGraph::differentNodeColor(int n, NodeColor* c) const
{
    std::set<int>::iterator it;
    it = m_red_node.find(n);
    if (it != m_red_node.end())
    {
        *c = COLOR_RED;
        return true;
    }

    it = m_blue_node.find(n);
    if (it != m_blue_node.end())
    {
        *c = COLOR_BLUE;
        return true;
    }
    return false;
}    // differentNodeColor

// -----------------------------------------------------------------------------
void BattleGraph::loadGoalNodes(const XMLNode& node)
{
    m_red_node.clear();
    m_blue_node.clear();

    const XMLNode *check_node = node.getNode("checks");
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

            int first = pointToNode(/*cur_node*/-1, p1, true);
            int last = pointToNode(/*cur_node*/-1, p2, true);

            first_goal ? m_blue_node.insert(first) : m_red_node.insert(first);
            first_goal ? m_blue_node.insert(last) : m_red_node.insert(last);
            while (first != last)
            {
                // Find all the nodes which connect the two points of
                // goal, notice: only work if it's a straight line
                first = getNextShortestPathPoly(first, last);
                first_goal ? m_blue_node.insert(first) :
                    m_red_node.insert(first);
            }
        }
    }
}    // loadGoalNodes
