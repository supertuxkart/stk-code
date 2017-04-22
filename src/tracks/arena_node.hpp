//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_ARENA_NODE_HPP
#define HEADER_ARENA_NODE_HPP

#include "tracks/quad.hpp"
#include "utils/cpp2011.hpp"

#include <line3d.h>
#include <vector>

/**
  * \ingroup tracks
  */
class ArenaNode : public Quad
{
private:
    core::line3df m_line;

    std::vector<int>   m_adjacent_nodes;

    std::vector<int>   m_nearby_nodes;

public:
    ArenaNode(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
              const Vec3 &normal, unsigned int node_index);
    // ------------------------------------------------------------------------
    virtual ~ArenaNode() {}
    // ------------------------------------------------------------------------
    const std::vector<int>& getAdjacentNodes()     { return m_adjacent_nodes; }
    // ------------------------------------------------------------------------
    std::vector<int>* getNearbyNodes()              { return &m_nearby_nodes; }
    // ------------------------------------------------------------------------
    void setAdjacentNodes(const std::vector<int>& nodes)
    {
        m_adjacent_nodes = nodes;
    }
    // ------------------------------------------------------------------------
    void setNearbyNodes(const std::vector<int>& nodes)
    {
        m_nearby_nodes = nodes;
    }
    // ------------------------------------------------------------------------
    /** Returns true if the quad lies near the edge, which means it doesn't
     *  have 4 adjacent quads.
     */
    bool isNearEdge() const            { return m_adjacent_nodes.size() != 4; }
    // ------------------------------------------------------------------------
    virtual float getDistance2FromPoint(const Vec3 &xyz) const OVERRIDE;

};

#endif
