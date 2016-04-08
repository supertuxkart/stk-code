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

#ifndef HEADER_XML_NODE_HPP
#define HEADER_XML_NODE_HPP

#include <string>
#include <map>
#include <vector>

#include <irrString.h>
#include <IXMLReader.h>
#include <SColor.h>
#include <vector2d.h>
#include <vector3d.h>
#include <path.h>
using namespace irr;

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/time.hpp"
#include "utils/types.hpp"

class InterpolationArray;
class Vec3;

/**
  * \brief utility class used to parse XML files
  * \ingroup io
  */
class XMLNode : public NoCopy
{
private:
    /** Name of this element. */
    std::string                          m_name;
    /** List of all attributes. */
    std::map<std::string, core::stringw> m_attributes;
    /** List of all sub nodes. */
    std::vector<XMLNode *>               m_nodes;

    void readXML(io::IXMLReader *xml);

    std::string                          m_file_name;

public:
         LEAK_CHECK();
         XMLNode(io::IXMLReader *xml);

         /** \throw runtime_error if the file is not found */
         XMLNode(const std::string &filename);

        ~XMLNode();

    const std::string &getName() const {return m_name; }
    const XMLNode     *getNode(const std::string &name) const;
    const void         getNodes(const std::string &s, std::vector<XMLNode*>& out) const;
    const XMLNode     *getNode(unsigned int i) const;
    unsigned int       getNumNodes() const {return (unsigned int) m_nodes.size(); }
    int get(const std::string &attribute, std::string *value) const;
    int get(const std::string &attribute, core::stringw *value) const;
    int getAndDecode(const std::string &attribute, core::stringw *value) const;
    int get(const std::string &attribute, int32_t  *value) const;
    int get(const std::string &attribute, uint16_t *value) const;
    int get(const std::string &attribute, uint32_t *value) const;
    int get(const std::string &attribute, int64_t  *value) const;
    int get(const std::string &attribute, float *value) const;
    int get(const std::string &attribute, bool *value) const;
    int get(const std::string &attribute, Vec3 *value) const;
    int get(const std::string &attribute, core::vector2df *value) const;
    int get(const std::string &attribute, core::vector3df *value) const;
    int get(const std::string &attribute, video::SColorf *value) const;
    int get(const std::string &attribute, video::SColor *value) const;
    int get(const std::string &attribute, std::vector<std::string> *value) const;
    int get(const std::string &attribute, std::vector<float> *value) const;
    int get(const std::string &attribute, std::vector<int> *value) const;
    int get(const std::string &attribute, InterpolationArray *value) const;
    int get(core::vector3df *value) const;
    int getXYZ(core::vector3df *value) const;
    int getXYZ(Vec3 *vaslue) const;
    int getHPR(core::vector3df *value) const;
    int getHPR(Vec3 *value) const;

    bool hasChildNamed(const char* name) const;

    /** Handy functions to test the bit pattern returned by get(vector3df*).*/
    static bool hasX(int b) { return (b&1)==1; }
    static bool hasY(int b) { return (b&2)==2; }
    static bool hasZ(int b) { return (b&4)==4; }
    static bool hasH(int b) { return (b&1)==1; }
    static bool hasP(int b) { return (b&2)==2; }
    static bool hasR(int b) { return (b&4)==4; }
};   // XMLNode

#endif
