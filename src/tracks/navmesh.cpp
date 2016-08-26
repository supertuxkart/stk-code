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

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/quad.hpp"
#include "utils/log.hpp"

#include <algorithm>

NavMesh *NavMesh::m_nav_mesh = NULL;

/** Constructor, loads the mesh information from a given set of polygons
 *    from a navmesh.xml file.
 *    \param filename Name of the file containing all polygons
 */
NavMesh::NavMesh(const std::string &filename)
{

    m_min = Vec3( 99999,  99999,  99999);
    m_max = Vec3(-99999, -99999, -99999);

    XMLNode *xml = file_manager->createXMLTree(filename);
    if (xml->getName() != "navmesh")
    {
        Log::error("NavMesh", "NavMesh is invalid.");
        delete xml;
        return;
    }

    std::vector<Vec3> all_vertices;
    for (unsigned int i = 0; i < xml->getNumNodes(); i++)
    {
        const XMLNode *xml_node = xml->getNode(i);
        if (xml_node->getName() == "vertices")
        {
            for (unsigned int i = 0; i < xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if (!(xml_node_node->getName() == "vertex"))
                {
                    Log::error("NavMesh", "Unsupported type '%s' found"
                        "in '%s' - ignored.",
                        xml_node_node->getName().c_str(), filename.c_str());
                    continue;
                }

                // Reading vertices
                Vec3 p;
                readVertex(xml_node_node, &p);
                m_max.max(p);
                m_min.min(p);
                all_vertices.push_back(p);
            }
        }

        if (xml_node->getName() == "faces")
        {
            for(unsigned int i = 0; i < xml_node->getNumNodes(); i++)
            {
                const XMLNode *xml_node_node = xml_node->getNode(i);
                if (xml_node_node->getName() != "face")
                {
                    Log::error("NavMesh", "Unsupported type '%s' found in '%s'"
                        " - ignored.",
                        xml_node_node->getName().c_str(), filename.c_str());
                    continue;
                }

                // Reading quads
                std::vector<int> quad_index;
                std::vector<int> adjacent_quad_index;
                xml_node_node->get("indices", &quad_index);
                xml_node_node->get("adjacents", &adjacent_quad_index);
                assert(quad_index.size() == 4);

                m_adjacent_quads.push_back(adjacent_quad_index);
                m_quads.push_back(new Quad(
                    all_vertices[quad_index[0]], all_vertices[quad_index[1]],
                    all_vertices[quad_index[2]], all_vertices[quad_index[3]]));
            }
        }
    }
    delete xml;

} // NavMesh

// ----------------------------------------------------------------------------

NavMesh::~NavMesh()
{
    for (unsigned int i = 0; i < m_quads.size(); i++)
    {
        delete m_quads[i];
    }
    m_quads.clear();
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
}   // readVertex

// ----------------------------------------------------------------------------
const Vec3& NavMesh::getCenterOfQuad(unsigned int n) const
{
    assert(m_quads.size() > 0 && n < m_quads.size());
    return m_quads[n]->getCenter();
}   // getCenterOfQuad
