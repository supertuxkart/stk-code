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

#ifndef HEADER_NAVMESH_HPP
#define HEADER_NAVMESH_HPP

#include "tracks/nav_poly.hpp"

#include <vector>
#include <string>
#include <set>

#include "utils/vec3.hpp"


namespace irr
{
    namespace video { struct S3DVertex; }
}
using namespace irr;


class btTransform;
class XMLNode;

/**
* \ingroup tracks
*
*  \brief This class stores a set of navigatoin polygons. It uses a 
*  'simplified singleton' design pattern: it has a static create function 
*	to create exactly one instance, a destroy function, and a get function 
*	(that does not have the side effect  of the 'normal singleton'  design 
*	pattern to create an instance). Besides saving on the if statement in get(), 
*	this is necessary since certain race modes might not have a navigaton 
*	mesh at all (e.g. race mode). So get() returns NULL in this case, and 
*	this is tested where necessary.
 \ingroup tracks
*/
class NavMesh
{

private:

	static NavMesh                  *m_nav_mesh;

	/** The actual set of nav polys that constitute the nav mesh */
    std::vector<NavPoly>            m_polys;    
	
	/** The set of vertices that are part of this nav mesh*/
    std::vector< Vec3 >             m_verts;

	/** Number of vertices */
    unsigned int                    m_n_verts;
	/** Number of polygons */
	unsigned int                    m_n_polys;
	/** Maximum vertices per polygon */
	unsigned int                    m_nvp;

    void readVertex(const XMLNode *xml, Vec3* result) const;
    //void readFace(const XMLNode *xml, Vec3* result) const;
    NavMesh(const std::string &filename);
    ~NavMesh();

public:
    
	/** Creates a NavMesh instance. */
    static void create(const std::string &filename)
    {
        assert(m_nav_mesh==NULL);

        // m_nav_mesh assigned in the constructor because it needs to defined 
        // for NavPoly which is constructed  in NavMesh()
        new NavMesh(filename);
    }

	/** Cleans up the nav mesh. It is possible that this function is called
	*  even if no instance exists (e.g. in race). So it is not an
	*  error if there is no instance. */
    static void destroy()
    { 
      if(m_nav_mesh)
      {
        delete m_nav_mesh;
        m_nav_mesh = NULL;
      }
    }
   
	/** Returns the one instance of this object. It is possible that there
	*  is no instance created (e.g. in normal race, since it doesn't have
	*  a nav mesh), so we don't assert that an instance exist, and we
	*  also don't create one if it doesn't exists. */
    static NavMesh          *get() { return m_nav_mesh; }

	/** Returns a const reference to a NavPoly */
    const NavPoly&          getNavPoly(int n) const 
                                { return m_polys[n]; }

	/** Returns a const reference to a vertex(Vec3) */
    const Vec3&             getVertex(int n) const  
                                { return m_verts[n]; }

	/** Returns a const reference to a vector containing all vertices */
    const std::vector<Vec3>& getAllVertices() const
                                { return m_verts;   }

	/** Returns the total number of polys */
    unsigned int            getNumberOfPolys() const  
                                { return m_n_polys; }

	/** Returns the total number of vertices */
    unsigned int            getNumberOfVerts() const  
                                { return m_n_verts; }

	/** Returns maximum vertices per polygon */
    unsigned int            getMaxVertsPerPoly() const 
                                { return m_nvp; }

	/** Returns the center of a polygon */
    const Vec3&             getCenterOfPoly(int n) const
                                {return m_polys[n].getCenter();}

	/** Returns a const referece to a vector containing the indices 
	 *	of polygons adjacent to a given polygon */
    const std::vector<int>& getAdjacentPolys(int n) const
                                {return m_polys[n].getAdjacents();}

	/** Returns a const reference to a vector containing the vertices
	 *	of a given polygon. */
    const std::vector<Vec3> getVertsOfPoly(int n)
                                {return m_polys[n].getVertices();}

};


#endif
