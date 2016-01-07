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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "tracks/navmesh.hpp"
#include "tracks/nav_poly.hpp"

#include <algorithm>
#include <S3DVertex.h>
#include <triangle3d.h>

#include "LinearMath/btTransform.h"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

NavMesh *NavMesh::m_nav_mesh = NULL;

/** Constructor, loads the mesh information from a given set of polygons
 *    from a navmesh.xml file.
 *    \param filename Name of the file containing all polygons
 */
NavMesh::NavMesh(const std::string &filename)
{

    m_min = Vec3( 99999,  99999,  99999);
    m_max = Vec3(-99999, -99999, -99999);
    m_n_verts=0;
    m_n_polys=0;

    XMLNode *xml = file_manager->createXMLTree(filename);
    if(xml->getName()!="navmesh")
    {
        Log::error("NavMesh", "NavMesh is invalid. \n");
        return;
    }

    // Assigning m_nav_mesh here because constructing NavPoly requires m_nav_mesh to be defined
    m_nav_mesh = this;

    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *xml_node = xml->getNode(i);
        if(xml_node->getName()=="vertices")
        {
            for(unsigned int i=0; i<xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if(!(xml_node_node->getName()=="vertex"))
                {
                    Log::error("NavMesh", "Unsupported type '%s' found in '%s' - ignored. \n",
                        xml_node_node->getName().c_str(),filename.c_str());
                    continue;
                }

                //Reading vertices
                Vec3 p;
                readVertex(xml_node_node, &p);
                m_max.max(p);
                m_min.min(p);
                m_n_verts++;
                m_verts.push_back(p);
            }

        }

        if(xml_node->getName()=="faces")
        {
            for(unsigned int i=0; i<xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if(xml_node_node->getName()!="face")
                {
                    Log::error("NavMesh", "Unsupported type '%s' found in '%s' - ignored. \n",
                        xml_node_node->getName().c_str(),filename.c_str());
                    continue;
                }

                //Reading faces/polys
                std::vector<int> polygonVertIndices;
                std::vector<int> adjacentPolygonIndices;
                xml_node_node->get("indices", &polygonVertIndices);
                xml_node_node->get("adjacents", &adjacentPolygonIndices);
                NavPoly *np = new NavPoly(polygonVertIndices, adjacentPolygonIndices);
                m_polys.push_back(*np);
                m_n_polys++;
            }

        }

        if(xml_node->getName()=="MaxVertsPerPoly")
        {
           xml_node->get("nvp", &m_nvp);
        }

        //delete xml_node;
    }

    delete xml;

} // NavMesh

// ----------------------------------------------------------------------------

NavMesh::~NavMesh()
{
}  // ~NavMesh

// ----------------------------------------------------------------------------
/** Sets the vertices in a irrlicht vertex array to the 4 points of this quad.
 */
void NavMesh::setVertices(int n, video::S3DVertex *v, const video::SColor &color) const
{
    NavPoly poly = NavMesh::get()->getNavPoly(n);
    const std::vector<Vec3>& p = poly.getVertices();

    if (p.size() !=4) return;

    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible.
    core::vector3df eps(0, 0.1f, 0);
    v[0].Pos = p[0].toIrrVector()+eps;
    v[1].Pos = p[1].toIrrVector()+eps;
    v[2].Pos = p[2].toIrrVector()+eps;
    v[3].Pos = p[3].toIrrVector()+eps;

    core::triangle3df tri(p[0].toIrrVector(), p[1].toIrrVector(),
                          p[2].toIrrVector());
    core::vector3df normal = tri.getNormal();
    normal.normalize();
    v[0].Normal = normal;
    v[1].Normal = normal;
    v[2].Normal = normal;

    core::triangle3df tri1(p[0].toIrrVector(), p[2].toIrrVector(),
                           p[3].toIrrVector());
    core::vector3df normal1 = tri1.getNormal();
    normal1.normalize();
    v[3].Normal = normal1;

    v[0].Color  = color;
    v[1].Color  = color;
    v[2].Color  = color;
    v[3].Color  = color;
}   // setVertices

// ----------------------------------------------------------------------------

/** Reads the vertex information from an XMLNode */
void NavMesh::readVertex(const XMLNode *xml, Vec3* result) const
{
    float x, y, z;
    xml->get("x", &x);
    xml->get("y", &y);
    xml->get("z", &z);
    Vec3 temp(x, y, z);
    *result = temp;
}     //    readVertex
