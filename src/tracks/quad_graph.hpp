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
        /** Returns the distance to the j-th. successor. */
        float         getDistanceToSuccessor(int j) const
                                   { return m_distance_to_next[j];           }
    };

    // ========================================================================
protected:
    /** The actual graph data structure. */
    std::vector<GraphNode*>  m_all_nodes;
    QuadSet                 *m_all_quads;
    void setDefaultSuccessors();
    void load         (const std::string &filename);
    
public:
                 QuadGraph     (const std::string &quad_file_name, 
                                const std::string graph_file_name);
                ~QuadGraph     ();
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

};   // QuadGraph

#endif
