//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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
#include <set>

#include "tracks/graph_node.hpp"
#include "tracks/quad_set.hpp"
#include "utils/aligned_array.hpp"
#include "utils/no_copy.hpp"

#include <dimension2d.h>
namespace irr
{
    namespace scene { class ISceneNode; class IMesh; class IMeshBuffer; }
    namespace video { class ITexture; }
}
using namespace irr;

class CheckLine;
class FrameBuffer;

/**
 *  \brief This class stores a graph of quads. It uses a 'simplified singleton'
 *  design pattern: it has a static create function to create exactly instance,
 *  a destroy function, and a get function (that does not have the side effect
 *  of the 'normal singleton'  design pattern to create an instance). Besides
 *  saving on the if statement in get(), this is necessary since certain race
 *  modes might not have a quad graph at all (e.g. battle mode). So get()
 *  returns NULL in this case, and this is tested where necessary.
 * \ingroup tracks
  */
class QuadGraph : public NoCopy
{

private:
    static QuadGraph        *m_quad_graph;

    /** The actual graph data structure. */
    std::vector<GraphNode*>  m_all_nodes;
    /** For debug mode only: the node of the debug mesh. */
    scene::ISceneNode       *m_node;
    /** For debug only: the mesh of the debug mesh. */
    scene::IMesh            *m_mesh;
    /** For debug only: the actual mesh buffer storing the quads. */
    scene::IMeshBuffer      *m_mesh_buffer;

    /** The length of the first loop. */
    float                    m_lap_length;

    /** The minimum coordinates of the quad graph. */
    Vec3                     m_min_coord;

    /** Scaling for mini map. */
    float                    m_scaling;

    /** Stores the filename - just used for error messages. */
    std::string              m_quad_filename;

    /** Wether the graph should be reverted or not */
    bool                     m_reverse;

    void setDefaultSuccessors();
    void computeChecklineRequirements(GraphNode* node, int latest_checkline);
    void computeDirectionData();
    void determineDirection(unsigned int current, unsigned int succ_index);
    float normalizeAngle(float f);

    void addSuccessor(unsigned int from, unsigned int to);
    void load         (const std::string &filename);
    void computeDistanceFromStart(unsigned int start_node, float distance);
    void createMesh(bool show_invisible=true,
                    bool enable_transparency=false,
                    const video::SColor *track_color=NULL,
                    const video::SColor *lap_color=NULL);
    unsigned int getStartNode() const;
         QuadGraph     (const std::string &quad_file_name,
                        const std::string graph_file_name,
                        const bool reverse);
        ~QuadGraph     ();
public:
    static const int UNKNOWN_SECTOR;

    void         createDebugMesh();
    void         cleanupDebugMesh();
    void         getSuccessors(int node_number,
                               std::vector<unsigned int>& succ,
                               bool for_ai=false) const;
    void         spatialToTrack(Vec3 *dst, const Vec3& xyz,
                                const int sector)               const;
    void         findRoadSector(const Vec3& XYZ, int *sector,
                            std::vector<int> *all_sectors=NULL) const;
    int          findOutOfRoadSector(const Vec3& xyz,
                                     const int curr_sector=UNKNOWN_SECTOR,
                                     std::vector<int> *all_sectors=NULL
                                     ) const;
    void         setDefaultStartPositions(AlignedArray<btTransform>
                                                       *start_transforms,
                                         unsigned int karts_per_row,
                                         float forwards_distance=1.5f,
                                         float sidewards_distance=1.5f,
                                         float upwards_distance=0.0f) const;
    void        makeMiniMap(const core::dimension2du &where,
                            const std::string &name,
                            const video::SColor &fill_color,
                            video::ITexture** oldRttMinimap,
                            FrameBuffer** newRttMinimap);
    void         mapPoint2MiniMap(const Vec3 &xyz, Vec3 *out) const;
    void         updateDistancesForAllSuccessors(unsigned int indx,
                                                 float delta,
                                                 unsigned int count);
    void         setupPaths();
    void         computeChecklineRequirements();
// ----------------------------------------------------------------------======
    /** Returns the one instance of this object. It is possible that there
     *  is no instance created (e.g. in battle mode, since it doesn't have
     *  a quad graph), so we don't assert that an instance exist, and we
     *  also don't create one if it doesn't exists. */
    static QuadGraph  *get() { return m_quad_graph; }
    // ----------------------------------------------------------------------==
    /** Creates a QuadGraph instance. */
    static void create(const std::string &quad_file_name,
                       const std::string graph_file_name,
                       const bool reverse)
    {
        assert(m_quad_graph==NULL);
        // assignment to m_quad_graph is done in the constructor, since
        // functions called from the constructor need it to be defined.
        new QuadGraph(quad_file_name, graph_file_name, reverse);
    }   // create
    // ------------------------------------------------------------------------
    /** Cleans up the quad graph. It is possible that this function is called
     *  even if no instance exists (e.g. in battle mode). So it is not an
     *  error if there is no instance. */
    static void destroy()
    {
        if(m_quad_graph)
        {
            delete m_quad_graph;
            m_quad_graph = NULL;
        }
    }   // destroy
    // ------------------------------------------------------------------------
    /** Returns the number of nodes in the graph. */
    unsigned int getNumNodes() const { return (unsigned int)m_all_nodes.size();}
    // ------------------------------------------------------------------------
    /** Return the distance to the j-th successor of node n. */
    float        getDistanceToNext(int n, int j) const
                         { return m_all_nodes[n]->getDistanceToSuccessor(j);}
    // ------------------------------------------------------------------------
    /** Returns the angle of the line between node n and its j-th.
     *  successor. */
    float        getAngleToNext(int n, int j) const
                         { return m_all_nodes[n]->getAngleToSuccessor(j);   }
    // ------------------------------------------------------------------------
    /** Returns the number of successors of a node n. */
    int          getNumberOfSuccessors(int n) const
                         { return m_all_nodes[n]->getNumberOfSuccessors();  }
    // ------------------------------------------------------------------------
    /** Returns the quad that belongs to a graph node. */
    const Quad&  getQuadOfNode(unsigned int j) const
          { return QuadSet::get()->getQuad(m_all_nodes[j]->getQuadIndex()); }
    // ------------------------------------------------------------------------
    /** Returns the quad that belongs to a graph node. */
    GraphNode&   getNode(unsigned int j) const{ return *m_all_nodes[j]; }
    // ------------------------------------------------------------------------
    /** Returns the distance from the start to the beginning of a quad. */
    float        getDistanceFromStart(int j) const
                           { return m_all_nodes[j]->getDistanceFromStart(); }
    // ------------------------------------------------------------------------
    /** Returns the length of the main driveline. */
    float        getLapLength() const {return m_lap_length; }
    // ------------------------------------------------------------------------
    /** Returns true if the graph is to be reversed. */
    bool         isReverse() const {return m_reverse; }

};   // QuadGraph

#endif
