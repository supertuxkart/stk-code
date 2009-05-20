//  $Id$
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

#include "quad_set.hpp"

#include <stdlib.h>

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

/** Constructor, loads the quad set from a file.
 *  \param filename filename to load.
 */
QuadSet::QuadSet(const std::string& filename) {
    load(filename);
}   // QuadSet

// -----------------------------------------------------------------------------
/** This function interprets a point specification as an attribute in the 
    xml quadset file. It understands two different specifications:
    p1="n:p"      : get point p from square n (n, p integers)
    p1="p1,p2,p3" : make a 3d point out of these 3 floating point values
*/
void QuadSet::getPoint(const XMLNode *xml, const std::string &attribute_name, 
                       Vec3* result) const
{
    std::string s;
    xml->get(attribute_name, &s);
    int pos=s.find_first_of(":");
    if(pos>0)   // n:p specification
    {
        std::vector<std::string> l = StringUtils::split(s, ':');
        int n=atoi(l[0].c_str());
        int p=atoi(l[1].c_str());
        *result=(*m_all_quads[n])[p];
    } 
    else
    {
        xml->get(attribute_name, result);
    }

}   // getPoint 
// -----------------------------------------------------------------------------
void QuadSet::load(const std::string &filename) {
    m_xMin = m_zMin =  9999999.9f;
    m_xMax = m_zMax = -9999999.9f;

    XMLNode *xml = file_manager->createXMLTree(filename);
    if(!xml || xml->getName()!="quads") 
    {
        fprintf(stderr, "QuadSet '%s' not found.\n", filename.c_str());
        return;
        //FIXME exit(-1);
    }
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *xml_node = xml->getNode(i);
        if(xml_node->getName()!="quad")
        {
            printf("Unsupported node type '%s' found in '%s' - ignored.\n",
                xml_node->getName().c_str(), filename.c_str());
            continue;
        }
        Vec3 p0, p1, p2, p3;
        getPoint(xml_node, "p0", &p0);
        getPoint(xml_node, "p1", &p1);
        getPoint(xml_node, "p2", &p2);
        getPoint(xml_node, "p3", &p3);
        Quad* q=new Quad(p0,p1,p2,p3);
        m_all_quads.push_back(q);

    }
    delete xml;
}   // load

// -----------------------------------------------------------------------------
/** Returns wether a point is to the left or to the right of a line.
    While all arguments are 3d, only the x and y coordinates are actually used.
*/
float QuadSet::sideOfLine2D(const Vec3 &l1, const Vec3 &l2, const Vec3 &p) const
{
    return (l2.getX()-l1.getX())*(p.getZ()-l1.getZ()) -
           (l2.getZ()-l1.getZ())*(p.getX()-l1.getX());
}   // sideOfLine

// -----------------------------------------------------------------------------
bool QuadSet::pointInQuad(const Quad& q, const btVector3& p) const {
    if(sideOfLine2D(q[0], q[2], p)<0) {
        return sideOfLine2D(q[0], q[1], p) >  0.0 &&
               sideOfLine2D(q[1], q[2], p) >= 0.0;
    } else {
        return sideOfLine2D(q[2], q[3], p) >  0.0 &&
               sideOfLine2D(q[3], q[0], p) >= 0.0;
    }
}   // pointInQuad

// -----------------------------------------------------------------------------
/** Determines the quad on which the position pos is. This is a rather slow
    algorithm, used to get the initial quad position of a kart, but not for
    constantly updating it.
*/
int QuadSet::getQuadAtPos(const Vec3 &pos) const {
    for(unsigned int i=0; i<m_all_quads.size(); i++) {
        if(pointInQuad(*(m_all_quads[i]), pos)) return i;
    }
    return QUAD_NONE;
}   // getQuadAtPos

// -----------------------------------------------------------------------------
