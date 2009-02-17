//  $Id: xml_reader.hpp 694 2006-08-29 07:42:36Z hiker $
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
#ifdef HAVE_IRRLICHT
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"
#include "utils/vec3.hpp"

XMLNode::XMLNode(io::IXMLReader *xml)
{
    for(unsigned int i=0; i<xml->getAttributeCount(); i++)
    {
        std::string   name  = core::stringc(xml->getAttributeName(i)).c_str();
        core::stringw value = xml->getAttributeValue(i);
        m_attributes[name] = value;
    }   // for i
}   // XMLNode

// ----------------------------------------------------------------------------
/** If 'attribute' was defined, set 'value' to the value of the
*  attribute and return true, otherwise return false and do not
*  change value.
*  \param attribute Name of the attribute.
*  \param value Value of the attribute.
*/
bool XMLNode::get(const std::string &attribute, std::string *value)
{
    std::map<std::string, core::stringw>::iterator o;
    o = m_attributes.find(attribute);
    if(o==m_attributes.end()) return false;
    *value=core::stringc(o->second).c_str();
    return true;
}   // get

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, core::vector3df *value)
{
    std::string s = "";
    if(!get(attribute, &s)) return false;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=3) return false;
    value->X = (float)atof(v[0].c_str());
    value->Y = (float)atof(v[1].c_str());
    value->Z = (float)atof(v[2].c_str());
    return true;
}   // get(vector3df)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, Vec3 *value)
{
    std::string s = "";
    if(!get(attribute, &s)) return false;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=3) return false;
    value->setX((float)atof(v[0].c_str()));
    value->setY((float)atof(v[1].c_str()));
    value->setZ((float)atof(v[2].c_str()));
    return true;
}   // get(Vec3)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, video::SColorf *color)
{
    std::string s;
    if(!get(attribute, &s)) return false;

    std::vector<std::string> v = StringUtils::split(s,' ');
    if(v.size()!=4) return false;
    color->set((float)atof(v[0].c_str()),
               (float)atof(v[1].c_str()),
               (float)atof(v[2].c_str()),
               (float)atof(v[3].c_str()));
    return true;
}   // get(SColor)
// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, int *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;
    *value = atoi(s.c_str());
    return true;
}   // get(int)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, float *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;
    *value = (float)atof(s.c_str());
    return true;
}   // get(int)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, bool *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;
    *value = s==""     || s[0]=='T' || s[0]=='t' ||
             s=="true" || s=="TRUE" || s=="#t"   || s=="#T";
    return true;
}   // get(bool)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, std::vector<std::string> *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;

    *value = StringUtils::split(s,' ');
    return true;
}   // get(vector<string>)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, std::vector<float> *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back((float)atof(v[i].c_str()));
    }
    return true;
}   // get(vector<float>)

// ----------------------------------------------------------------------------
bool XMLNode::get(const std::string &attribute, std::vector<int> *value)
{
    std::string s;
    if(!get(attribute, &s)) return false;

    std::vector<std::string> v = StringUtils::split(s,' ');
    value->clear();
    for(unsigned int i=0; i<v.size(); i++)
    {
        value->push_back(atoi(v[i].c_str()));
    }
    return true;
}   // get(vector<int>)

// ----------------------------------------------------------------------------

#endif
