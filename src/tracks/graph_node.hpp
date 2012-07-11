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

#include <vector2d.h>
#include <dimension2d.h>
#include <line2d.h>

#include "tracks/quad.hpp"
#include "tracks/quad_set.hpp"
#include "utils/vec3.hpp"

class QuadGraph;

/** 
  * \brief This class stores a node of the graph, i.e. a list of successor 
  *  edges.
  * \ingroup tracks
  */
class GraphNode 
{
public:    
    /** To indiciate in which direction the track is going: 
     *  straight, left, right. The undefined direction is used by the
     *  AI only. */
    enum         DirectionType {DIR_STRAIGHT, DIR_LEFT, DIR_RIGHT,
                                DIR_UNDEFINED};

private:
    /** Index of this node in the set of quads. Several graph nodes can use
     *  the same quad, meaning it is possible to use a quad more than once,
     *  e.g. a figure 8 like track. */
    unsigned int       m_quad_index;

    /** Index of this graph node. */
    unsigned int       m_node_index;

    /** The list of successor graph nodes. */
    std::vector<int>   m_successor_nodes;

    /** The list of predecessors of a node. */
    std::vector<int>   m_predecessor_nodes;

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

     /** A vector from the center of the quad to the right edge. */
     Vec3 m_center_to_right;

     /** Line between lower and upper center, saves computation in 
      *  getDistanceFromLine() later. The line is 2d only since otherwise
      *  taller karts would have a larger distance from the center. It also
      *  saves computation, and it is only needed to determine the distance
      *  from the center of the drivelines anyway. */
     core::line2df  m_line;

     typedef std::vector<int> PathToNodeVector;
     /** This vector is only used if the graph node has more than one
      *  successor. In this case m_path_to_node[X] will contain the index
      *  of the successor to use in order to reach graph node X for this
      *  graph nodes.  */
     PathToNodeVector  m_path_to_node;

     /** The direction for each of the successors. */
     std::vector<DirectionType>  m_direction;
     
     /** Stores for each successor the index of the last graph node that
      *  has the same direction (i.e. if index 0 curves left, this vector
      *  will store the index of the last graph node that is still turning
      *  left. */
     std::vector<unsigned int> m_last_index_same_direction;

    /**
      * Sets of checklines you should have activated when you are driving on
      * this node (there is a possibility of more than one set because of
      * alternate ways)
      */
    std::vector< int > m_checkline_requirements;
    
    void markAllSuccessorsToUse(unsigned int n, 
                                PathToNodeVector *m_path_to_node);

public:
                 GraphNode(unsigned int quad_index, unsigned int node_index);
    void         addSuccessor (unsigned int to);
    void         getDistances(const Vec3 &xyz, Vec3 *result);
    float        getDistance2FromPoint(const Vec3 &xyz);
    void         setupPathsToNode();
    void         setChecklineRequirements(int latest_checkline);
    void         setDirectionData(unsigned int successor, DirectionType dir, 
                                  unsigned int last_node_index);
    // ------------------------------------------------------------------------
    /** Returns the number of successors. */
    unsigned int getNumberOfSuccessors() const 
                             { return (unsigned int)m_successor_nodes.size(); }
    // ------------------------------------------------------------------------
    /** Returns the i-th successor node. */
    unsigned int getSuccessor(unsigned int i)  const 
                               { return m_successor_nodes[i];                 }
    // ------------------------------------------------------------------------
    /** Returns the number of predecessors. */
    unsigned int getNumberOfPredecessors() const
                           { return (unsigned int)m_predecessor_nodes.size(); }
    // ------------------------------------------------------------------------
    /** Returns a predecessor for this node. Note that the first predecessor 
     *  is the most 'natural' one, i.e. the one on the main driveline.
     */
    int getPredecessor(unsigned int i) const {return m_predecessor_nodes[i]; }
    // ------------------------------------------------------------------------
    /** Returns the quad_index in the quad_set of this node. */
    int          getIndex() const { return m_quad_index;                }

    // ------------------------------------------------------------------------
    /** Returns the quad of this graph node. */
    const Quad& getQuad() const {return QuadSet::get()->getQuad(m_quad_index);}
    // ------------------------------------------------------------------------
    /** Returns the i-th. point of a quad. ATM this just returns the vertices
     *  from the quads, but if necessary this method will also consider 
     *  rotated quads. So index 0 will always be lower left point, then 
     *  counterclockwise. */
    const Vec3& operator[](int i) const 
                             {return QuadSet::get()->getQuad(m_quad_index)[i];}
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
    /** Sets the distance from start for this node. */
    void         setDistanceFromStart(float d) {m_distance_from_start = d; }
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
    /** Returns the center point of this graph node. */
    const Vec3  getCenter()      const 
                            {return (m_upper_center + m_lower_center) / 2.0f;}
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
        return QuadSet::get()->getQuad(m_successor_nodes[i]).letAIIgnore();
    };
    // ------------------------------------------------------------------------
    /** Returns which successor node to use in order to be able to reach the
     *  given node n.
     *  \param n Index of the graph node to reach.
     */
    int getSuccessorToReach(unsigned int n)
    {  
        // If we have a path to node vector, use its information, otherwise
        // (i.e. there is only one successor anyway) use this one successor.
        return m_path_to_node.size()>0 ? m_path_to_node[n] : 0;
    }   // getSuccesorToReach
    // ------------------------------------------------------------------------
    /** Returns the checkline requirements of this graph node. */
    const std::vector<int>& getChecklineRequirements() const 
                                           { return m_checkline_requirements; }
    // ------------------------------------------------------------------------
    /** Returns the direction in which the successor n is. */
    void getDirectionData(unsigned int succ, DirectionType *dir,
                          unsigned int *last) const 
    { 
        *dir = m_direction[succ];  *last = m_last_index_same_direction[succ];
    }
    // ------------------------------------------------------------------------
};   // GraphNode

#endif
