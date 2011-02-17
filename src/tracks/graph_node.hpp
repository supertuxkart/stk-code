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

#ifndef HEADER_GRAPH_NODE_HPP
#define HEADER_GRAPH_NODE_HPP

#include <vector>

#include "irrlicht.h"

#include "tracks/quad.hpp"
#include "tracks/quad_set.hpp"
#include "utils/vec3.hpp"

class QuadGraph;

/** 
  * \brief This class stores a node of the graph, i.e. a list of successor edges.
  * \ingroup tracks
  */
class GraphNode 
{
    /** Index of this node in the set of quads. Several graph nodes can use
     *  the same quad, meaning it is possible to use a quad more than once,
     *  e.g. a figure 8 like track. */
    unsigned int       m_index;
    /** The list of successors. */
    std::vector<int>   m_vertices;
    /** The distance to each of the successors. */
    std::vector<float> m_distance_to_next;
    /** The angle of the line from this node to each neighbour. */
    std::vector<float> m_angle_to_next;

    /** Distance from the start to the beginning of this quad. */
    float m_distance_from_start;

    /** Width of the track, which is the average of the width at the 
     *  beginning and at the end. FIXME: for now the width is independent
     *  of the orientation (e.g. a quad used more than once might once
     *  be used from top to bottom, one from left to right, so it should
     *  have a different width then). */
     float m_width;

     /** The center point of the lower two points (e.g. points 0 and 1).
      *  This saves some computations in getDistances later. Only the
      *  start point is needed, and only in 2d. */
     core::vector2df m_lower_center_2d;

     /** Lower center point of the graph node. */
     Vec3 m_lower_center;

     /** Upper center point of the graph node. */
     Vec3 m_upper_center;

     /** Line between lower and upper center, saves computation in 
      *  getDistanceFromLine() later. The line is 2d only since otherwise
      *  taller karts would have a larger distance from the center. It also
      *  saves computation, and it is only needed to determine the distance
      *  from the center of the drivelines anyway. */
     core::line2df  m_line;
public:
    /** Keep a shared pointer so that some asserts and tests can be 
    *  done without adding additional parameters. */
    static QuadSet   *m_all_quads;
    /** Keep a shared pointer to the graph structure so that each node
    *  has access to the actual quad to which a node points. */
    static QuadGraph *m_all_nodes;

                 GraphNode(unsigned int index);
    void         addSuccessor (unsigned int to);
    void         getDistances(const Vec3 &xyz, Vec3 *result);
    float        getDistance2FromPoint(const Vec3 &xyz);

    /** Returns the i-th successor. */
    unsigned int getSuccessor(unsigned int i)  const 
                               { return m_vertices[i];                   }
    // ------------------------------------------------------------------------
    /** Returns the number of successors. */
    unsigned int getNumberOfSuccessors() const 
                               { return (unsigned int)m_vertices.size(); }
    // ------------------------------------------------------------------------
    /** Returns the index in the quad_set of this node. */
    int          getIndex() const { return m_index;                     }

    // ------------------------------------------------------------------------
    /** Returns the quad of this graph node. */
    const Quad& getQuad() const {return m_all_quads->getQuad(m_index);}
    // ------------------------------------------------------------------------
    /** Returns the i-th. point of a quad. ATM this just returns the vertices
     *  from the quads, but if necessary this method will also consider 
     *  rotated quads. So index 0 will always be lower left point, then 
     *  counterclockwise. */
    const Vec3& operator[](int i) const 
                                    {return m_all_quads->getQuad(m_index)[i];}
    // ------------------------------------------------------------------------
    /** Returns the distance to the j-th. successor. */
    float        getDistanceToSuccessor(unsigned int j) const
                               { return m_distance_to_next[j];           }

    // ------------------------------------------------------------------------
    /** Returns the angle from this node to the j-th. successor. */
    float        getAngleToSuccessor(unsigned int j) const
                               { return m_angle_to_next[j];              }
    // ------------------------------------------------------------------------
    /** Returns the distance from start. */
    float        getDistanceFromStart() const  
                               { return m_distance_from_start;           }
    // ------------------------------------------------------------------------
    /** Returns the width of the part for this quad. */
    float        getPathWidth() const  { return m_width;                 }
    // ------------------------------------------------------------------------
    /** Returns the center point of the lower edge of this graph node. */
    const Vec3& getLowerCenter() const {return m_lower_center;}
    // ------------------------------------------------------------------------
    /** Returns the center point of the upper edge of this graph node. */
    const Vec3& getUpperCenter() const {return m_upper_center;}
    // ------------------------------------------------------------------------
    /** Returns the length of the quad of this node. */
    float       getNodeLength() const 
                {return (m_lower_center-m_upper_center).length();}
    // ------------------------------------------------------------------------
    /** Returns true if the index-successor of this node is one that the AI
     *  is allowed to use.
     *  \param index Index of the successor. */
    bool        ignoreSuccessorForAI(unsigned int i) const
    {
        return m_all_quads->getQuad(m_vertices[i]).letAIIgnore();
    };
};   // GraphNode

#endif
