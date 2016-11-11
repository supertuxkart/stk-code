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

#ifndef HEADER_DRIVE_NODE_HPP
#define HEADER_DRIVE_NODE_HPP

#include <vector>

#include "tracks/quad.hpp"

/**
  * \brief This class stores a node of the drive graph, i.e. a list of
  *  successor edges, it can either be 2d or 3d.
  * \ingroup tracks
  */
class DriveNode : public Quad
{
public:
    /** To indiciate in which direction the track is going:
     *  straight, left, right. The undefined direction is used by the
     *  AI only. */
    enum         DirectionType {DIR_STRAIGHT, DIR_LEFT, DIR_RIGHT,
                                DIR_UNDEFINED};
protected:
    /** Lower center point of the drive node. */
    Vec3 m_lower_center;

    /** Upper center point of the drive node. */
    Vec3 m_upper_center;

    /** Distance from the start to the beginning of the drive node. */
    float m_distance_from_start;

private:
    /** Set to true if this drive node should not be used by the AI. */
    bool               m_ai_ignore;

    /** The list of successor drive nodes. */
    std::vector<int>   m_successor_nodes;

    /** The list of predecessors of a node. */
    std::vector<int>   m_predecessor_nodes;

    /** The distance to each of the successors. */
    std::vector<float> m_distance_to_next;

    /** The angle of the line from this node to each neighbour. */
    std::vector<float> m_angle_to_next;

    /** Width of the track, which is the average of the width at the
     *  beginning and at the end. */
    float m_width;

    /** A vector from the center of the quad to the right edge. */
    Vec3 m_center_to_right;

    typedef std::vector<int> PathToNodeVector;
    /** This vector is only used if the drive node has more than one
     *  successor. In this case m_path_to_node[X] will contain the index
     *  of the successor to use in order to reach drive node X for this
     *  drive nodes.  */
    PathToNodeVector  m_path_to_node;

    /** The direction for each of the successors. */
    std::vector<DirectionType>  m_direction;

    /** Stores for each successor the index of the last drive node that
     *  has the same direction (i.e. if index 0 curves left, this vector
     *  will store the index of the last drive node that is still turning
     *  left. */
    std::vector<unsigned int> m_last_index_same_direction;

    /** A unit vector pointing from the center to the right side, orthogonal
     *  to the driving direction. */
    Vec3 m_right_unit_vector;

    /**
     * Sets of checklines you should have activated when you are driving on
     * this node (there is a possibility of more than one set because of
     * alternate ways)
     */
   std::vector< int > m_checkline_requirements;


    // ------------------------------------------------------------------------
   void markAllSuccessorsToUse(unsigned int n,
                                PathToNodeVector *m_path_to_node);

public:
                 DriveNode(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                           const Vec3 &p3, const Vec3 &normal,
                           unsigned int node_index, bool invisible,
                           bool ai_ignore, bool ignored);
    // ------------------------------------------------------------------------
         virtual ~DriveNode() {}
    // ------------------------------------------------------------------------
    void         addSuccessor (unsigned int to);
    // ------------------------------------------------------------------------
    void         setupPathsToNode();
    // ------------------------------------------------------------------------
    void         setChecklineRequirements(int latest_checkline);
    // ------------------------------------------------------------------------
    void         setDirectionData(unsigned int successor, DirectionType dir,
                                  unsigned int last_node_index);
    // ------------------------------------------------------------------------
    /** Returns the number of successors. */
    unsigned int getNumberOfSuccessors() const
                             { return (unsigned int)m_successor_nodes.size(); }
    // ------------------------------------------------------------------------
    /** Returns the i-th successor node. */
    unsigned int getSuccessor(unsigned int i)  const
                                               { return m_successor_nodes[i]; }
    // ------------------------------------------------------------------------
    /** Returns the number of predecessors. */
    unsigned int getNumberOfPredecessors() const
                           { return (unsigned int)m_predecessor_nodes.size(); }
    // ------------------------------------------------------------------------
    /** Returns a predecessor for this node. Note that the first predecessor
     *  is the most 'natural' one, i.e. the one on the main driveline.
     */
    int getPredecessor(unsigned int i) const { return m_predecessor_nodes[i]; }
    // ------------------------------------------------------------------------
    /** Returns the distance to the j-th. successor. */
    float        getDistanceToSuccessor(unsigned int j) const
                                              { return m_distance_to_next[j]; }
    // ------------------------------------------------------------------------
    /** Returns the angle from this node to the j-th. successor. */
    float        getAngleToSuccessor(unsigned int j) const
                                                 { return m_angle_to_next[j]; }
    // ------------------------------------------------------------------------
    /** Returns the distance from start. */
    float        getDistanceFromStart() const
                                              { return m_distance_from_start; }
    // ------------------------------------------------------------------------
    /** Sets the distance from start for this node. */
    void         setDistanceFromStart(float d)   { m_distance_from_start = d; }
    // ------------------------------------------------------------------------
    /** Returns the width of the part for this quad. */
    float        getPathWidth() const                       { return m_width; }
    // ------------------------------------------------------------------------
    /** Returns the center point of the lower edge of this drive node. */
    const Vec3& getLowerCenter() const               { return m_lower_center; }
    // ------------------------------------------------------------------------
    /** Returns the center point of the upper edge of this drive node. */
    const Vec3& getUpperCenter() const               { return m_upper_center; }
    // ------------------------------------------------------------------------
    /** Returns the length of the quad of this node. */
    float       getNodeLength() const
                           { return (m_lower_center-m_upper_center).length(); }
    // ------------------------------------------------------------------------
    bool        ignoreSuccessorForAI(unsigned int i) const;
    // ------------------------------------------------------------------------
    /** Returns which successor node to use in order to be able to reach the
     *  given node n.
     *  \param n Index of the drive node to reach.
     */
    int getSuccessorToReach(unsigned int n)
    {
        // If we have a path to node vector, use its information, otherwise
        // (i.e. there is only one successor anyway) use this one successor.
        return m_path_to_node.size()>0 ? m_path_to_node[n] : 0;
    }   // getSuccesorToReach
    // ------------------------------------------------------------------------
    /** Returns the checkline requirements of this drive node. */
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
    /** Returns a unit vector pointing to the right side of the quad. */
    const Vec3 &getRightUnitVector() const      { return m_right_unit_vector; }
    // ------------------------------------------------------------------------
    /** True if this node should be ignored by the AI. */
    bool        letAIIgnore() const                     { return m_ai_ignore; }
    // ------------------------------------------------------------------------
    virtual void getDistances(const Vec3 &xyz, Vec3 *result) const = 0;

};   // DriveNode

#endif
