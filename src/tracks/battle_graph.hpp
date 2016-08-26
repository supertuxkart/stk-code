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

#ifndef HEADER_BATTLE_GRAPH_HPP
#define HEADER_BATTLE_GRAPH_HPP

#include <string>
#include <set>
#include <vector>

#include "tracks/graph_structure.hpp"
#include "tracks/navmesh.hpp"

class Item;
class ItemManager;
class XMLNode;

/**
* \ingroup tracks
*
*  \brief This class stores a graph constructed from the navigatoin mesh.
*    It uses a 'simplified singleton' design pattern: it has a static create
*    function to create exactly one instance, a destroy function, and a get
*    function (that does not have the side effect  of the 'normal singleton'
*    design pattern to create an instance).
\ingroup tracks
*/
class BattleGraph : public GraphStructure
{

private:
    static BattleGraph        *m_battle_graph;

    /** The actual graph data structure, it is an adjacency matrix */
    std::vector< std::vector< float > > m_distance_matrix;
    /** The matrix that is used to store computed shortest paths */
    std::vector< std::vector< int > > m_parent_poly;

    std::vector< std::vector< int > > m_nearby_quads;

    /** Stores the name of the file containing the NavMesh data */
    std::string              m_navmesh_file;

    std::vector< std::pair<const Item*, int> > m_items_on_graph;

    std::set<int> m_red_node;
    std::set<int> m_blue_node;

    void buildGraph(NavMesh*);
    void computeFloydWarshall();
    void loadGoalNodes(const XMLNode *node);
    void sortNearbyQuad();

    BattleGraph(const std::string &navmesh_file_name, const XMLNode *node=NULL);
    ~BattleGraph(void);

    // ------------------------------------------------------------------------
    virtual void set3DVerticesOfGraph(int i, video::S3DVertex *v,
                                      const video::SColor &color) const;
    // ------------------------------------------------------------------------
    virtual void getGraphBoundingBox(Vec3 *min, Vec3 *max) const
                                { NavMesh::get()->getBoundingBox(min, max); }
    // ------------------------------------------------------------------------
    virtual const bool isNodeInvisible(int n) const;
    // ------------------------------------------------------------------------
    virtual const bool hasLapLine() const
                                                            { return false; }
    // ------------------------------------------------------------------------
    virtual const bool differentNodeColor(int n, NodeColor* c) const;
    void computeDijkstra(int n);
    static std::vector<int> getPathFromTo(int from, int to, 
                          const std::vector< std::vector< int > > parent_poly);

public:
    static const int UNKNOWN_POLY;

    void              findItemsOnGraphNodes();
    // ----------------------------------------------------------------------
    int               pointToNode(const int cur_node,
                                  const Vec3& cur_point,
                                  bool ignore_vertical) const;
    // ------------------------------------------------------------------------
    static void unitTesting();
    // ------------------------------------------------------------------------
    /** Returns the one instance of this object. */
    static BattleGraph *get() { return m_battle_graph; }
    // ------------------------------------------------------------------------
    /** Asserts that no BattleGraph instance exists. Then
    *    creates a BattleGraph instance. */
    static void create(const std::string &navmesh_file_name,
                       const XMLNode *node)
    {
        assert(m_battle_graph==NULL);
        m_battle_graph = new BattleGraph(navmesh_file_name, node);
    } // create
    // ------------------------------------------------------------------------
    /** Cleans up the BattleGraph instance if it exists */
    static void destroy()
    {
        if(m_battle_graph)
        {
            delete m_battle_graph;
            m_battle_graph = NULL;
        }
    } // destroy
    // ------------------------------------------------------------------------
    /** Returns the number of nodes in the BattleGraph (equal to the number
     *  of quads in the NavMesh
     */
    virtual const unsigned int getNumNodes() const
                               { return NavMesh::get()->getNumberOfQuads(); }
    // ------------------------------------------------------------------------
    /** Returns the distance between any two nodes */
    float getDistance(int from, int to) const
    {
        if (from == BattleGraph::UNKNOWN_POLY ||
            to == BattleGraph::UNKNOWN_POLY)
            return 0.0f;
        return m_distance_matrix[from][to];
    }
    // ------------------------------------------------------------------------
    /** Returns the next polygon on the shortest path from i to j.
     *  Note: m_parent_poly[j][i] contains the parent of i on path from j to i,
     *  which is the next node on the path from i to j (undirected graph)
     */
    int getNextShortestPathPoly(int i, int j) const
    {
        if (i == BattleGraph::UNKNOWN_POLY || j == BattleGraph::UNKNOWN_POLY)
            return BattleGraph::UNKNOWN_POLY;
        return m_parent_poly[j][i];
    }
    // ------------------------------------------------------------------------
    std::vector<std::pair<const Item*, int>>& getItemList()
                                                 { return m_items_on_graph; }
    // ------------------------------------------------------------------------
    void              insertItems(Item* item, int polygon)
               { m_items_on_graph.push_back(std::make_pair(item, polygon)); }
    // ------------------------------------------------------------------------
    /** Returns the quad that belongs to a node. */
    const Quad& getQuadOfNode(unsigned int n) const
                                       { return NavMesh::get()->getQuad(n); }
    // ------------------------------------------------------------------------
    /** Returns true if the quad lies near the edge, which means it doesn't
     *  have 4 adjacent quads.
     */
    bool isNearEdge(unsigned int n) const
                  { return NavMesh::get()->getAdjacentQuads(n).size() != 4; }
    // ------------------------------------------------------------------------
};    //BattleGraph

#endif
