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

#ifndef HEADER_ARENA_GRAPH_HPP
#define HEADER_ARENA_GRAPH_HPP

#include "tracks/graph.hpp"
#include "utils/cpp2011.hpp"

#include <set>

class ArenaNode;
class XMLNode;

/**
 *  \brief A graph made from navmesh
 *  \ingroup tracks
 */
class ArenaGraph : public Graph
{
private:
    /** The actual graph data structure, it is an adjacency matrix. */
    std::vector<std::vector<float>> m_distance_matrix;

    /** The matrix that is used to store computed shortest paths. */
    std::vector<std::vector<int16_t>> m_parent_node;

    /** Used in soccer mode to colorize the goal lines in minimap. */
    std::set<int> m_red_node;

    std::set<int> m_blue_node;

    // ------------------------------------------------------------------------
    void loadGoalNodes(const XMLNode *node);
    // ------------------------------------------------------------------------
    void loadNavmesh(const std::string &navmesh);
    // ------------------------------------------------------------------------
    void buildGraph();
    // ------------------------------------------------------------------------
    void setNearbyNodesOfAllNodes();
    // ------------------------------------------------------------------------
    void computeDijkstra(int n);
    // ------------------------------------------------------------------------
    void computeFloydWarshall();
    // ------------------------------------------------------------------------
    static std::vector<int16_t> getPathFromTo(int from, int to,
                     const std::vector< std::vector< int16_t > >& parent_node);
    // ------------------------------------------------------------------------
    virtual bool hasLapLine() const OVERRIDE                  { return false; }
    // ------------------------------------------------------------------------
    virtual void differentNodeColor(int n, video::SColor* c) const OVERRIDE;

public:
    static ArenaGraph* get()     { return dynamic_cast<ArenaGraph*>(m_graph); }
    // ------------------------------------------------------------------------
    static void unitTesting();
    // ------------------------------------------------------------------------
    ArenaGraph(const std::string &navmesh, const XMLNode *node = NULL);
    // ------------------------------------------------------------------------
    virtual ~ArenaGraph() {}
    // ------------------------------------------------------------------------
    ArenaNode* getNode(unsigned int i) const;
    // ------------------------------------------------------------------------
    /** Returns the next node on the shortest path from i to j.
     *  Note: m_parent_node[j][i] contains the parent of i on path from j to i,
     *  which is the next node on the path from i to j (undirected graph)
     */
    int getNextNode(int i, int j) const
    {
        if (i == Graph::UNKNOWN_SECTOR || j == Graph::UNKNOWN_SECTOR)
            return Graph::UNKNOWN_SECTOR;
        return (int)(m_parent_node[j][i]);
    }
    // ------------------------------------------------------------------------
    /** Returns the distance between any two nodes */
    float getDistance(int from, int to) const
    {
        if (from == Graph::UNKNOWN_SECTOR || to == Graph::UNKNOWN_SECTOR)
            return 99999.0f;
        return m_distance_matrix[from][to];
    }

};   // ArenaGraph

#endif
