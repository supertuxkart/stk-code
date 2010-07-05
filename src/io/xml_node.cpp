//  $Id: xml_node.hpp 694 2006-08-29 07:42:36Z hiker $
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

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"
#include "utils/vec3.hpp"

XMLNode::XMLNode(io::IXMLReader *xml)
{
    while(xml->getNodeType()!=io::EXN_ELEMENT && xml->read());
    readXML(xml);
}   // XMLNode

// ----------------------------------------------------------------------------
/** Reads a XML file and convert it into a XMLNode tree.
 *  \param filename Name of the XML file to read.
 */
XMLNode::XMLNode(const std::string &filename)
{
    io::IXMLReader *xml = file_manager->createXMLReader(filename);
    bool is_first_element = true;
    while(xml->read())
    {
        switch (xml->getNodeType()) 
        {
        case io::EXN_ELEMENT:
            {
                if(!is_first_element)
                {
                    fprintf(stderr, 
                            "More than one root element in '%s' - ignored.\n",
                            filename.c_str());
                }
                readXML(xml);
                is_first_element = false;
                break;
            }
        case io::EXN_ELEMENT_END:  break;   // Ignore all other types
        case io::EXN_UNKNOWN:      break;
        case io::EXN_COMMENT:      break;
        case io::EXN_TEXT:         break;
        default:                   break;
        }   // switch
    }   // while
    xml->drop();
}   // XMLNode

// ----------------------------------------------------------------------------
/** Destructor. */
XMLNode::~XMLNode()
{
    for(unsigned int i=0; i<m_nodes.size(); i++)
    {
        delete m_nodes[i];
    }
    m_nodes.clear();
}   // ~XMLNode

// ----------------------------------------------------------------------------
/** Stores all attributes, and reads in all children.
 *  \param xml The XML reader.
 */
void XMLNode::readXML(io::IXMLReader *xml)
{
    m_name = std::string(core::stringc(xml->getNodeName()).c_str());

    for(unsigned int i=0; i<xml->getAttributeCount(); i++)
    {
        std::string   name  = core::stringc(xml->getAttributeName(i)).c_str();
        core::stringw value = xml->getAttributeValue(i);
        m_attributes[name] = value;
    }   // for i

    // If no children, we are done
    if(xml->isEmptyElement()) 
        return;

    /** Read all children elements. */
    while(xml->read())
    {
        switch (xml->getNodeType()) 
        {
        case io::EXN_ELEMENT:
            m_nodes.push_back(new XMLNode(xml));
            break;
        case io::EXN_ELEMENT_END:
            // End of this element found.
            return;
            break;
        case io::EXN_UNKNOWN:            break;
        case io::EXN_COMMENT:            break;
        case io::EXN_TEXT:               break;
        default:                         break;
        }   // switch
    }   // while
}   // readXML

// ----------------------------------------------------------------------------
/** Returns the i.th node.
 *  \param i Number of node to return.
 */
const XMLNode *XMLNode::getNode(unsigned int i) const
{
    return m_nodes[i];
}   // getNode

// ----------------------------------------------------------------------------
/** Returns the node with the given name.
 *  \param s Name of the node to return.
 */
const XMLNode *XMLNode::getNode(const std::string &s) const
{
    for(unsigned int i=0; i<m_nodes.size(); i++)
    {
        if(m_nodes[i]->getName()==s) return m_nodes[i];
    }
    return NULL;
}   // getNode

// ----------------------------------------------------------------------------
/** Returns all nodes with the given name.
 *  \param s Name of the nodes to return.
 *  \param s Vector that will be filled with output values.
 */
const void XMLNode::getNodes(const std::string &s, std::vector<XMLNode*>& out) const
{
    for(unsigned int i=0; i<m_nodes.size(); i++)
    {
        if(m_nodes[i]->getName()==s)
        {
            out.push_back(m_nodes[i]);
        }
    }
}   // getNode

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, set 'value' to the value of the
*   attribute and return 1, otherwise return 0 and do not change value.
*  \param attribute Name of the attribute.
*  \param value Value of the attribute.
*/
int XMLNode::get(const std::string &attribute, std::string *value) const
{
    if(m_attributes.size()==0) return 0;
    std::map<std::string, core::stringw>::const_iterator o;
    o = m_attributes.find(attribute);
    if(o==m_attributes.end()) return 0;
    *value=core::stringc(o->second).c_str();
    return 1;
}   // get

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, core::vector2df *value) const
{
    std::string s = "";
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=2) return 0;
    value->X = (float)atof(v[0].c_str());
    value->Y = (float)atof(v[1].c_str());
    return 1;
}   // get(vector2df)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, core::vector3df *value) const
{
    Vec3 xyz;
    if(!get(attribute, &xyz)) return 0;

    *value = xyz.toIrrVector();
    return 1;
}   // get(vector3df)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, Vec3 *value) const
{
    std::string s = "";
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=3) return 0;
    value->setX((float)atof(v[0].c_str()));
    value->setY((float)atof(v[1].c_str()));
    value->setZ((float)atof(v[2].c_str()));
    return 1;
}   // get(Vec3)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, video::SColor *color) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if (v.size()<3 || v.size()>4) return 0;
    if (v.size()==3)
    {
        color->setRed  (atoi(v[0].c_str()));
        color->setGreen(atoi(v[1].c_str()));
        color->setBlue (atoi(v[2].c_str()));
    }
    else
    {
        color->set(atoi(v[3].c_str()), // irrLicht expects ARGB, and we use RGBA in XML files
                   atoi(v[0].c_str()),
                   atoi(v[1].c_str()),
                   atoi(v[2].c_str()));
    }
    return 1;
}   // get(SColor)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, video::SColorf *color) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=4) return 0;
    color->set((float)atof(v[3].c_str()),  // set takes ARGB, but we use RGBA
               (float)atof(v[0].c_str()),
               (float)atof(v[1].c_str()),
               (float)atof(v[2].c_str()));
    return 1;
}   // get(SColor)
// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, int *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = atoi(s.c_str());
    return 1;
}   // get(int)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, unsigned int *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = atoi(s.c_str());
    return 1;
}   // get(unsigned int)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, float *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = (float)atof(s.c_str());
    return 1;
}   // get(int)

// ----------------------------------------------------------------------------
int XMLNode::get(const std::string &attribute, bool *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;
    *value = s==""     || s[0]=='T' || s[0]=='t' || s[0]=='Y' || s[0]=='y' ||
             s=="#t"   || s   =="#T";
    return 1;
}   // get(bool)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, 
                 std::vector<std::string> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    *value = StringUtils::split(s,' ');
    return value->size();
}   // get(vector<string>)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, 
                 std::vector<float> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back((float)atof(v[i].c_str()));
    }
    return value->size();
}   // get(vector<float>)

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, split the value of the attribute by spaces,
 *  set value to this vector array and return the number of elements. Otherwise
 *  return 0 and do not change value.
 *  \param attribute Name of the attribute.
 *  \param value Value of the attribute.
 */
int XMLNode::get(const std::string &attribute, std::vector<int> *value) const
{
    std::string s;
    if(!get(attribute, &s)) return 0;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back(atoi(v[i].c_str()));
    }
    return value->size();
}   // get(vector<int>)

// ----------------------------------------------------------------------------
/** Interprets the attributes 'x', 'y', 'z'  or 'h', 'p', 'r' as a 3d vector
 *  and set the corresponding elements of value. Not all values need to be
 *  defined as attributes (and the correspnding elements of the vector will
 *  not be changed). It returns a bit field for each defined value, i.e. if x 
 *  and y are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::get(core::vector3df *value) const
{
    float f;
    int bits=0;
    core::vector3df result = *value;
    if(get("x", &f)) { value->X = f; bits |= 1; }
    if(get("h", &f)) { value->X = f; bits |= 1; }
    if(get("y", &f)) { value->Y = f; bits |= 2; }
    if(get("p", &f)) { value->Y = f; bits |= 2; }
    if(get("z", &f)) { value->Z = f; bits |= 4; }
    if(get("r", &f)) { value->Z = f; bits |= 4; }
    return bits;
}   // core::vector3df

// ----------------------------------------------------------------------------
/** Interprets the attributes 'x', 'y', 'z' as a 3d vector and set the 
 *  corresponding elements of value. Not all values need to be defined as 
 *  attributes (and the correspnding elements of the vector will not be 
 *  changed). It returns a bit field for each defined value, i.e. if x 
 *  and y are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::getXYZ(core::vector3df *value) const
{
    float f;
    int bits=0;
    core::vector3df result = *value;
    if(get("x", &f)) { value->X = f; bits |= 1; }
    if(get("y", &f)) { value->Y = f; bits |= 2; }
    if(get("z", &f)) { value->Z = f; bits |= 4; }
    return bits;
}   // getXYZ vector3df

// ----------------------------------------------------------------------------
/** Interprets the attributes 'x', 'y', 'z' as a 3d vector and set the 
 *  corresponding elements of value. Not all values need to be defined as 
 *  attributes (and the correspnding elements of the vector will not be 
 *  changed). It returns a bit field for each defined value, i.e. if x 
 *  and y are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::getXYZ(Vec3 *value) const
{
    float f;
    int bits=0;
    Vec3 result = *value;
    if(get("x", &f)) { value->setX(f); bits |= 1; }
    if(get("y", &f)) { value->setY(f); bits |= 2; }
    if(get("z", &f)) { value->setZ(f); bits |= 4; }
    return bits;
}   // getXYZ Vec3

// ----------------------------------------------------------------------------
/** Interprets the attributes 'h', 'p', 'r' as a 3d vector and set the 
 *  corresponding elements of value. Not all values need to be defined as 
 *  attributes (and the correspnding elements of the vector will not be 
 *  changed). It returns a bit field for each defined value, i.e. if x and y 
 *  are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::getHPR(core::vector3df *value) const
{
    float f;
    int bits=0;
    core::vector3df result = *value;
    if(get("h", &f)) { value->X = f; bits |= 1; }
    if(get("p", &f)) { value->Y = f; bits |= 2; }
    if(get("r", &f)) { value->Z = f; bits |= 4; }
    return bits;
}   // getHPR vector3df

// ----------------------------------------------------------------------------
/** Interprets the attributes 'h', 'p', 'r' as a 3d vector and set the 
 *  corresponding elements of value. Not all values need to be defined as 
 *  attributes (and the correspnding elements of the vector will not be 
 *  changed). It returns a bit field for each defined value, i.e. if x and y 
 *  are defined, 3 is returned.
 *  \param value Vector to return the values in.
 */
int XMLNode::getHPR(Vec3 *value) const
{
    float f;
    int bits=0;
    Vec3 result = *value;
    if(get("h", &f)) { value->setX(f); bits |= 1; }
    if(get("p", &f)) { value->setY(f); bits |= 2; }
    if(get("r", &f)) { value->setZ(f); bits |= 4; }
    return bits;
}   // getHPR Vec3

// ----------------------------------------------------------------------------
