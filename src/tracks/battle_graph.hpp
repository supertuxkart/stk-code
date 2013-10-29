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

#ifndef HEADER_BATTLE_GRAPH_HPP
#define HEADER_BATTLE_GRAPH_HPP

#include <vector>
#include <string>
#include <set>

#include "tracks/navmesh.hpp"


class Navmesh;

namespace irr
{
    namespace scene { class ISceneNode; class IMesh; class IMeshBuffer; }
    namespace video { class ITexture; }
}
using namespace irr;

class BattleGraph
{

private:
        
    static BattleGraph        *m_battle_graph;

    std::vector< std::vector< std::pair<int,float> > > m_graph;
    std::vector< std::vector< float > > m_distance_matrix;
    std::vector< std::vector< int > > m_next_poly;
     /** For debug mode only: the node of the debug mesh. */
    scene::ISceneNode       *m_node;
    /** For debug only: the mesh of the debug mesh. */
    scene::IMesh            *m_mesh;
    /** For debug only: the actual mesh buffer storing the quads. */
    scene::IMeshBuffer      *m_mesh_buffer;


    std::string             m_navmesh_file;


    void buildGraph(NavMesh*);
    void computeFloydWarshall();
    void createMesh(bool enable_transparency=false,
                    const video::SColor *track_color=NULL);
    
    BattleGraph(const std::string &navmesh_file_name);
    ~BattleGraph(void);

public:

    static const int UNKNOWN_POLY;

    static BattleGraph *get() { return m_battle_graph; }

    static void create(const std::string &navmesh_file_name)
    {
        assert(m_battle_graph==NULL);
        m_battle_graph = new BattleGraph(navmesh_file_name);

    }

    static void destroy()
    {
        if(m_battle_graph)
        {
            delete m_battle_graph;
            m_battle_graph = NULL;
        }
    }

    unsigned int    getNumNodes() const { return m_distance_matrix.size(); }
    const NavPoly&    getPolyOfNode(int i) const 
                                        { return NavMesh::get()->getNavPoly(i); }

    const int & getNextShortestPathPoly(int i, int j) const 
                                        { return m_next_poly[i][j]; }

    void            createDebugMesh();
    void            cleanupDebugMesh();
};

#endif