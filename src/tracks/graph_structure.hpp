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
#include "tracks/graph_node.hpp"
#include "utils/vec3.hpp"
#include "utils/no_copy.hpp"

namespace irr
{
    namespace scene { class ISceneNode; class IMesh; class IMeshBuffer; }
    namespace video { class ITexture; }
}
using namespace irr;

class FrameBuffer;
class RTT;

/**
 * \brief Virtual base class for a graph structure.
 *
 *  A graph structure has a certain type:
 *  GT_RACE  :  Graph used by a lap race.
 *  GT_BATTLE:  Graph used by a battle arena.
 *
 * \ingroup tracks
 */
class GraphStructure : public NoCopy
{
public:
    enum GraphType {GT_RACE, GT_BATTLE};

protected:
    /** The type of this graph. */
    GraphType         m_graph_type;

    void              cleanupDebugMesh();
    void              destroyRTT();

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

    void  createMesh(bool show_invisible=true,
                     bool enable_transparency=false,
                     const video::SColor *track_color=NULL,
                     const video::SColor *lap_color=NULL);

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
    virtual const unsigned int            getNumNodes() const = 0;
    virtual const std::vector<GraphNode*> getAllNodes() const = 0;
    virtual void                          setType() = 0;
};   // GraphStructure

#endif
