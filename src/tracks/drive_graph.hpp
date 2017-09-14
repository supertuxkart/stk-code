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

#ifndef HEADER_DRIVE_GRAPH_HPP
#define HEADER_DRIVE_GRAPH_HPP

#include <vector>
#include <string>

#include "tracks/graph.hpp"
#include "utils/aligned_array.hpp"
#include "utils/cpp2011.hpp"

#include "LinearMath/btTransform.h"

class DriveNode;
class XMLNode;

/**
 *  \brief A graph made from driveline
 *  \ingroup tracks
 */
class DriveGraph : public Graph
{
private:
    /** The length of the first loop. */
    float m_lap_length;

    /** Stores the filename - just used for error messages. */
    std::string m_quad_filename;

    /** Wether the graph should be reverted or not */
    bool m_reverse;

    // ------------------------------------------------------------------------
    void setDefaultSuccessors();
    // ------------------------------------------------------------------------
    void computeChecklineRequirements(DriveNode* node, int latest_checkline);
    // ------------------------------------------------------------------------
    void computeDirectionData();
    // ------------------------------------------------------------------------
    void determineDirection(unsigned int current, unsigned int succ_index);
    // ------------------------------------------------------------------------
    float normalizeAngle(float f);
    // ------------------------------------------------------------------------
    void addSuccessor(unsigned int from, unsigned int to);
    // ------------------------------------------------------------------------
    void load(const std::string &quad_file_name, const std::string &filename);
    // ------------------------------------------------------------------------
    void getPoint(const XMLNode *xml, const std::string &attribute_name,
                  Vec3 *result) const;
    // ------------------------------------------------------------------------
    void computeDistanceFromStart(unsigned int start_node, float distance);
    // ------------------------------------------------------------------------
    unsigned int getStartNode() const;
    // ------------------------------------------------------------------------
    virtual bool hasLapLine() const OVERRIDE                   { return true; }
    // ------------------------------------------------------------------------
    virtual void differentNodeColor(int n, video::SColor* c) const OVERRIDE;

public:
    static DriveGraph* get()     { return dynamic_cast<DriveGraph*>(m_graph); }
    // ------------------------------------------------------------------------
    DriveGraph(const std::string &quad_file_name,
               const std::string &graph_file_name, const bool reverse);
    // ------------------------------------------------------------------------
    virtual ~DriveGraph() {}
    // ------------------------------------------------------------------------
    void getSuccessors(int node_number, std::vector<unsigned int>& succ,
                       bool for_ai=false) const;
    // ------------------------------------------------------------------------
    void spatialToTrack(Vec3 *dst, const Vec3& xyz, const int sector) const;
    // ------------------------------------------------------------------------
    void setDefaultStartPositions(AlignedArray<btTransform> *start_transforms,
                                  unsigned int karts_per_row,
                                  float forwards_distance = 1.5f,
                                  float sidewards_distance = 1.5f,
                                  float upwards_distance=0.0f) const;
    // ------------------------------------------------------------------------
    void updateDistancesForAllSuccessors(unsigned int indx, float delta,
                                         unsigned int count);
    // ------------------------------------------------------------------------
    void setupPaths();
    // ------------------------------------------------------------------------
    void computeChecklineRequirements();
    // ------------------------------------------------------------------------
    /** Return the distance to the j-th successor of node n. */
    float getDistanceToNext(int n, int j) const;
    // ------------------------------------------------------------------------
    /** Returns the angle of the line between node n and its j-th.
     *  successor. */
    float getAngleToNext(int n, int j) const;
    // ------------------------------------------------------------------------
    /** Returns the number of successors of a node n. */
    int getNumberOfSuccessors(int n) const;
    // ------------------------------------------------------------------------
    /** Returns the quad that belongs to a graph node. */
    DriveNode* getNode(unsigned int j) const;
    // ------------------------------------------------------------------------
    /** Returns the distance from the start to the beginning of a quad. */
    float getDistanceFromStart(int j) const;
    // ------------------------------------------------------------------------
    /** Returns the length of the main driveline. */
    float getLapLength() const                         { return m_lap_length; }
    // ------------------------------------------------------------------------
    bool isReverse() const                                { return m_reverse; }

};   // DriveGraph

#endif
