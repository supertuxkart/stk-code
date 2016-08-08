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

#ifndef HEADER_NAVMESH_HPP
#define HEADER_NAVMESH_HPP

#include <vector>
#include <string>

#include "utils/vec3.hpp"

class Quad;
class XMLNode;

/**
 *  \brief This class stores a set of navigation quads. It uses a
 *  'simplified singleton' design pattern: it has a static create function
 *  to create exactly one instance, a destroy function, and a get function
 *  (that does not have the side effect of the 'normal singleton' design
 *  pattern to create an instance). Besides saving on the if statement in
 *  get(), this is necessary since certain race modes might not have a
 *  navigation mesh at all (e.g. race mode). So get() returns NULL in this
 *  case, and this is tested where necessary.
 * \ingroup tracks
 */
class NavMesh
{

private:
    static NavMesh                *m_nav_mesh;

    /** The 2d bounding box, used for hashing. */
    Vec3                          m_min;
    Vec3                          m_max;

    /** The actual set of quads that constitute the nav mesh */
    std::vector<Quad*>            m_quads;

    std::vector<std::vector<int>> m_adjacent_quads;

    void readVertex(const XMLNode *xml, Vec3* result) const;
    // ------------------------------------------------------------------------
    NavMesh(const std::string &filename);
    // ------------------------------------------------------------------------
    ~NavMesh();

public:
    /** Creates a NavMesh instance. */
    static void create(const std::string &filename)
    {
        assert(m_nav_mesh == NULL);
        m_nav_mesh = new NavMesh(filename);
    }
    // ------------------------------------------------------------------------
    /** Cleans up the nav mesh. It is possible that this function is called
     *  even if no instance exists (e.g. in race). So it is not an
     *  error if there is no instance.
     */
    static void destroy()
    {
        if (m_nav_mesh)
        {
            delete m_nav_mesh;
            m_nav_mesh = NULL;
        }
    }
    // ------------------------------------------------------------------------
    /** Returns the one instance of this object. It is possible that there
     *  is no instance created (e.g. in normal race, since it doesn't have
     *  a nav mesh), so we don't assert that an instance exist, and we
     *  also don't create one if it doesn't exists.
     */
    static NavMesh *get()                                { return m_nav_mesh; }
    // ------------------------------------------------------------------------
    /** Return the minimum and maximum coordinates of this navmesh. */
    void getBoundingBox(Vec3 *min, Vec3 *max)       { *min=m_min; *max=m_max; }
    // ------------------------------------------------------------------------
    /** Returns a const reference to a quad */
    const Quad& getQuad(unsigned int n) const
    {
        assert(m_quads.size() > 0 && n < m_quads.size());
        return *(m_quads[n]);
    }
    // ------------------------------------------------------------------------
    /** Returns a const referece to a vector containing the indices
     *  of quads adjacent to a given quad
     */
    const std::vector<int>& getAdjacentQuads(unsigned int n) const
    {
        assert(m_adjacent_quads.size() > 0 && n < m_adjacent_quads.size() &&
            m_quads.size() == m_adjacent_quads.size());
        return m_adjacent_quads[n];
    }
    // ------------------------------------------------------------------------
    /** Returns the total number of quads */
    unsigned int getNumberOfQuads() const
    {
        assert(m_quads.size() > 0);
        return m_quads.size();
    }
    // ------------------------------------------------------------------------
    /** Returns the center of a quad */
    const Vec3& getCenterOfQuad(unsigned int n) const;

};
#endif
