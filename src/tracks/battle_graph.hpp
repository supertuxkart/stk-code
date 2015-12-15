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

#include <vector>
#include <string>
#include <set>

#include <dimension2d.h>
#include "tracks/navmesh.hpp"

namespace irr
{
    namespace scene { class ISceneNode; class IMesh; class IMeshBuffer; }
    namespace video { class ITexture; }
}
using namespace irr;

class FrameBuffer;
class Item;
class ItemManager;
class Navmesh;
class RTT;

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
class BattleGraph
{

private:
    static BattleGraph        *m_battle_graph;

    RTT* m_new_rtt;

    /** The actual graph data structure, it is an adjacency matrix */
    std::vector< std::vector< float > > m_distance_matrix;
    /** The matrix that is used to store computed shortest paths */
    std::vector< std::vector< int > > m_parent_poly;
     /** For debug mode only: the node of the debug mesh. */
    scene::ISceneNode       *m_node;
    /** For debug only: the mesh of the debug mesh. */
    scene::IMesh            *m_mesh;
    /** For debug only: the actual mesh buffer storing the quads. */
    scene::IMeshBuffer      *m_mesh_buffer;

    /** The minimum coordinates of the quad graph. */
    Vec3                     m_min_coord;

    /** Scaling for mini map. */
    float                    m_scaling;

    /** Stores the name of the file containing the NavMesh data */
    std::string              m_navmesh_file;

    std::vector< std::pair<const Item*, int> > m_items_on_graph;

    void buildGraph(NavMesh*);
    void computeFloydWarshall();
    void createMesh(bool enable_transparency=false,
                    const video::SColor *track_color=NULL);

    BattleGraph(const std::string &navmesh_file_name);
    ~BattleGraph(void);

public:
    static const int UNKNOWN_POLY;

    /** Returns the one instance of this object. */
    static BattleGraph *get() { return m_battle_graph; }
    // ----------------------------------------------------------------------
    /** Asserts that no BattleGraph instance exists. Then
    *    creates a BattleGraph instance. */
    static void create(const std::string &navmesh_file_name)
    {
        assert(m_battle_graph==NULL);
        m_battle_graph = new BattleGraph(navmesh_file_name);

    } // create
    // ----------------------------------------------------------------------
    /** Cleans up the BattleGraph instance if it exists */
    static void destroy()
    {
        if(m_battle_graph)
        {
            delete m_battle_graph;
            m_battle_graph = NULL;
        }
    } // destroy

    // ----------------------------------------------------------------------
    /** Returns the number of nodes in the BattleGraph (equal to the number of
    *    polygons in the NavMesh */
    unsigned int      getNumNodes() const { return m_distance_matrix.size(); }

    // ----------------------------------------------------------------------
    /** Returns the NavPoly corresponding to the i-th node of the BattleGraph */
    const NavPoly&    getPolyOfNode(int i) const
                                        { return NavMesh::get()->getNavPoly(i); }

    // ----------------------------------------------------------------------
    /** Returns the next polygon on the shortest path from i to j.
     *    Note: m_parent_poly[j][i] contains the parent of i on path from j to i,
     *    which is the next node on the path from i to j (undirected graph) */
    const int &       getNextShortestPathPoly(int i, int j) const;

    const std::vector < std::pair<const Item*, int> >& getItemList()
                                        { return m_items_on_graph; }

    void              createDebugMesh();
    void              cleanupDebugMesh();
    void              findItemsOnGraphNodes();
    void              makeMiniMap(const core::dimension2du &where,
                                  const std::string &name,
                                  const video::SColor &fill_color,
                                  video::ITexture** oldRttMinimap,
                                  FrameBuffer** newRttMinimap);
    void              mapPoint2MiniMap(const Vec3 &xyz, Vec3 *out) const;
};    //BattleGraph

#endif
