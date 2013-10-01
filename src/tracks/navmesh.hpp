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

class NavMesh
{

private:

    static NavMesh                  *m_nav_mesh;

    std::vector<NavPoly>            m_polys;    
    std::vector< Vec3 >             m_verts;
    unsigned int                    m_n_verts;
    unsigned int                    m_n_polys;
    unsigned int                    m_nvp;

    void NavMesh::readVertex(const XMLNode *xml, Vec3* result) const;
    //void NavMesh::readFace(const XMLNode *xml, Vec3* result) const;
    NavMesh(const std::string &filename);
    ~NavMesh();

public:
    
    static void create(const std::string &filename)
    {
        assert(m_nav_mesh==NULL);

        // m_nav_mesh assigned in the constructor because it needs to defined 
        // for NavPoly which is constructed  in NavMesh()
        new NavMesh(filename);
    }

   
    static NavMesh          *get() { return m_nav_mesh; }

    const NavPoly&          getNavPoly(int n) const 
                                { return m_polys[n]; }

    const Vec3&             getVertex(int n) const  
                                { return m_verts[n]; }

    const std::vector<Vec3>& getAllVertices() const
                                { return m_verts;   }
    
    unsigned int            getNumberOfPolys() const  
                                { return m_n_polys; }
  
    unsigned int            getNumberOfVerts() const  
                                { return m_n_verts; }
    
    unsigned int            getMaxVertsPerPoly() const 
                                { return m_nvp; }
    
    const Vec3&             getCenterOfPoly(int n) const
                                {return m_polys[n].getCenter();}

    
    const std::vector<int>& getAdjacentPolys(int n) const
                                {return m_polys[n].getAdjacents();}

    
    const std::vector<Vec3> getVertsOfPoly(int n)
                                {return m_polys[n].getVertices();}

};


#endif