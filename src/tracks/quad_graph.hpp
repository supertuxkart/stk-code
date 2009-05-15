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

class QuadSet;

/** This class stores a graph of quads. */
class QuadGraph {
    /** This class stores a node of the graph, i.e. a list of successor 
     *  edges. */
    class GraphNode 
    {
        /** The list of successors. */
        std::vector<int> m_vertices;
    public:
        /** Adds a successor to a node. */
        void          addSuccessor   (int to)       {m_vertices.push_back(to); }
        /** Returns the i-th successor. */
        unsigned int  operator[]     (int i)  const {return m_vertices[i];     }
        /** Returns the number of successors. */
        unsigned int  getNumberOfSuccessors() const 
                                     { return (unsigned int)m_vertices.size(); }
    };
    // ========================================================================
protected:
    std::vector<GraphNode*>  m_all_nodes;
    void setDefaultSuccessors();
    void load         (const std::string &filename);
    
public:
    QuadGraph         (const std::string &filename, QuadSet *quad_set);
   ~QuadGraph         ();
    void getSuccessors(int quadNumber, std::vector<int>& succ) const;
};   // QuadGraph

#endif
