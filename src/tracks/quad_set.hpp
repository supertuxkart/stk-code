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

#ifndef HEADER_QUAD_SET_HPP
#define HEADER_QUAD_SET_HPP

#include <vector>
#include <string>

#include "io/xml_node.hpp"
#include "utils/vec3.hpp"

class QuadSet {
private:
    /** Internal class to store a single quad. */
    class Quad {
    public:
        /** The four points of a quad. */
        Vec3 m_p[4];
        /** The center, which is used by the AI. This saves some 
            computations at runtime. */
        Vec3 m_center;
        /** Constructor, takes 4 points. */
        Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
        {
            m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;
            m_center = 0.25f*(p0+p1+p2+p3);
            m_center.setX((p0.getX()+p1.getX()+p2.getX()+p3.getX())/4.0f);
            m_center.setY((p0.getY()+p1.getY()+p2.getY()+p3.getY())/4.0f);
            m_center.setZ((p0.getZ()+p1.getZ()+p2.getZ()+p3.getZ())/4.0f);
        }
        /** Returns the i-th. point of a quad. */
        const btVector3& operator[](int i) const {return m_p[i];    }
        /** Returns the center of a quad. */
        const Vec3& getCenter ()      const {return m_center;  }
    };   // class Quad
    // =======================================================================
    /** The 2d bounding box, used for hashing. */
    // FIXME: named with z being the forward axis
    float               m_xMin, m_xMax, m_zMin, m_zMax;
    /** The list of all quads. */
    std::vector<Quad*>  m_allQuads;
    void load    (const std::string &filename);
    void getPoint(const XMLNode *xml, const std::string &attribute_name, 
                  Vec3 *result) const;
    float sideOfLine2D(const Vec3 &l1, const Vec3 &l2, const Vec3 &p) const;

public:
    static const int QUAD_NONE=-1;

         QuadSet       (const std::string& filename);
    int  getQuad       (const Vec3& p)                     const;
    int  getCurrentQuad(const Vec3& p, int oldQuad)        const;
    bool pointInQuad   (const Quad& q, const btVector3& p) const;
    /** Returns true if the point p is in the n-th. quad. */
    bool pointInQuad   (int n,         const btVector3& p) const
                                {return pointInQuad(*m_allQuads[n],p);}
    void getBoundingBox(float* xMin, float* xMax,
                        float* zMin, float* zMax) const
                                { *xMin=m_xMin; *xMax=m_xMax;
                                  *zMin=m_zMin; *zMax=m_zMax;}
    /** Returns the number of quads. */
    unsigned int getSize() const {return (unsigned int)m_allQuads.size(); }
    /** Returns the center of quad n. */
    const Vec3& getCenterOfQuad(int n) const 
                                 {return m_allQuads[n]->getCenter();       }
};   // QuadSet
#endif
