//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015  SuperTuxKart Team
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_GRAPH_STRUCTURE_HPP
#define HEADER_GRAPH_STRUCTURE_HPP

#include <string>

#include <dimension2d.h>
#include <SColor.h>
#include "utils/vec3.hpp"
#include "utils/no_copy.hpp"

namespace irr
{
    namespace scene { class ISceneNode; class IMesh; class IMeshBuffer; }
    namespace video { class ITexture; struct S3DVertex; }
}
using namespace irr;

class FrameBuffer;
class RTT;

/**
 *  \brief Virtual base class for a graph structure.
 *  This is mainly used for drawing minimap in game.
 *
 *  \ingroup tracks
 */
class GraphStructure : public NoCopy
{
protected:

    /** Used by soccer field with navmesh to draw goal line. */
    enum NodeColor
    {
        COLOR_BLUE,
        COLOR_RED
    };

    void                    cleanupDebugMesh();
    void                    destroyRTT();

private:
    RTT* m_new_rtt;

    /** The node of the graph mesh. */
    scene::ISceneNode       *m_node;

    /** The mesh of the graph mesh. */
    scene::IMesh            *m_mesh;

    /** The actual mesh buffer storing the graph. */
    scene::IMeshBuffer      *m_mesh_buffer;

    /** The minimum coordinates of the graph. */
    Vec3                     m_min_coord;

    /** Scaling for mini map. */
    float                    m_scaling;

    void createMesh(bool show_invisible=true,
                    bool enable_transparency=false,
                    const video::SColor *track_color=NULL);

    virtual void set3DVerticesOfGraph(int i, video::S3DVertex *v,
                                      const video::SColor &color) const = 0;
    virtual void getGraphBoundingBox(Vec3 *min, Vec3 *max) const = 0;
    virtual const bool isNodeInvisible(int n) const = 0;
    virtual const bool isNodeInvalid(int n) const = 0;
    virtual const bool hasLapLine() const = 0;
    virtual const bool differentNodeColor(int n, NodeColor* c) const = 0;

public:
             GraphStructure();
    virtual ~GraphStructure() {};
    void     createDebugMesh();
    void     makeMiniMap(const core::dimension2du &where,
                         const std::string &name,
                         const video::SColor &fill_color,
                         video::ITexture** oldRttMinimap,
                         FrameBuffer** newRttMinimap);
    void     mapPoint2MiniMap(const Vec3 &xyz, Vec3 *out) const;
    virtual const unsigned int getNumNodes() const = 0;
};   // GraphStructure

#endif
