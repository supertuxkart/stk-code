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

#ifndef HEADER_QUAD_GRAPH_HPP
#define HEADER_QUAD_GRAPH_HPP

#include <vector>
#include <string>

#include "tracks/quad_set.hpp"

/** This class stores a graph of quads. */
class QuadGraph {
    /** This class stores a node of the graph, i.e. a list of successor 
     *  edges. */
    class GraphNode 
    {
        /** Index of this node in m_all_quads. */
        int                m_index;
        /** The list of successors. */
        std::vector<int>   m_vertices;
        /** The distance to each of the successors. */
        std::vector<float> m_distance_to_next;
        /** The angle of the line from this node to each neighbour. */
        std::vector<float> m_angle_to_next;
    public:
        /** Constructor, stores the index. 
         *  \param index Index of this node in m_all_quads. */
                      GraphNode(int index) { m_index = index; }
        void          addSuccessor (int to, const QuadSet &quad_set);
        /** Returns the i-th successor. */
        unsigned int  operator[]   (int i)  const {return m_vertices[i];     }
        /** Returns the number of successors. */
        unsigned int  getNumberOfSuccessors() const 
                                   { return (unsigned int)m_vertices.size(); }
        /** Returns the index in the quad_set of this node. */
        int           getIndex() const { return m_index;                     }
        /** Returns the distance to the j-th. successor. */
        float         getDistanceToSuccessor(int j) const
                                   { return m_distance_to_next[j];           }
        /** Returns the angle from this node to the j-th. successor. */
        float         getAngleToSuccessor(int j) const
                                   { return m_angle_to_next[j];              }
    };   // GraphNode

    // ========================================================================
protected:
    /** The actual graph data structure. */
    std::vector<GraphNode*>  m_all_nodes;
    /** The set of all quads. */
    QuadSet                 *m_all_quads;
    /** For debug mode only: the node of the debug mesh. */
    scene::ISceneNode       *m_node;
    /** For debug only: the mesh of the debug mesh. */
    scene::IMesh            *m_mesh;
    /** For debug only: the actual mesh buffer storing the quads. */
    scene::IMeshBuffer      *m_mesh_buffer;

    void setDefaultSuccessors();
    void load         (const std::string &filename);
public:
                 QuadGraph     (const std::string &quad_file_name, 
                                const std::string graph_file_name);
                ~QuadGraph     ();
    void         createDebugMesh();
    void         getSuccessors(int quadNumber, 
                               std::vector<unsigned int>& succ) const;

    /** Returns the number of nodes in the graph. */
    unsigned int   getNumNodes() const { return m_all_nodes.size();         } 
    /** Returns the quad set for this graph. */
    const QuadSet* getQuads()    const { return m_all_quads;                }
    /** Returns the center of a quad. 
     *  \param n Index of the quad.    */
    const Vec3&    getCenterOfQuad(int n) const 
                                  { return m_all_quads->getCenterOfQuad(n); }
    /** Return the distance to the j-th successor of node n. */
    float          getDistanceToNext(int n, int j) const
                         { return m_all_nodes[n]->getDistanceToSuccessor(j);}
    /** Returns the angle of the line between node n and its j-th. 
     *  successor. */
    float          getAngleToNext(int n, int j) const
                         { return m_all_nodes[n]->getAngleToSuccessor(j);   }
    /** Returns the number of successors of a node n. */
    int            getNumberOfSuccessors(int n) const 
                         { return m_all_nodes[n]->getNumberOfSuccessors();  }
};   // QuadGraph

#endif
