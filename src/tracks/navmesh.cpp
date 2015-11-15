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
 *	from a navmesh.xml file.
 *	\param filename Name of the file containing all polygons
 */
NavMesh::NavMesh(const std::string &filename)
{
    
    m_n_verts=0;
    m_n_polys=0;
    
    XMLNode *xml = file_manager->createXMLTree(filename);
    if(!xml || xml->getName()!="navmesh")
    {
        Log::error("NavMesh", "NavMesh '%s' not found. \n", filename.c_str());
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

/** Reads the vertex information from an XMLNode */
void NavMesh::readVertex(const XMLNode *xml, Vec3* result) const
{
	float x, y, z;
	xml->get("x", &x);
	xml->get("y", &y);
	xml->get("z", &z);
	Vec3 temp(x, y, z);
	*result = temp;
}	 //	readVertex

