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

#include "io/xml_reader.hpp"
#include "io/xml_node.hpp"

XMLReader::XMLReader(io::IXMLReader *xml)
{
    while(xml->read())
    {
        switch (xml->getNodeType()) 
        {
        case io::EXN_ELEMENT:
            {
                // FIXME: is there an easier way to convert 
                // stringw to std::string?
                std::string s = core::stringc(xml->getNodeName()).c_str();
                const XMLNode *node = new XMLNode(s, xml);
                m_all_nodes.push_back(node);
                break;
            }
        case io::EXN_ELEMENT_END:  // Ignore all other types
        case io::EXN_UNKNOWN:     
        case io::EXN_COMMENT:
        case io::EXN_TEXT:
        default:        break;
        }   // switch
    }   // while
}   // XMLReader

// ----------------------------------------------------------------------------
unsigned int XMLReader::getNumNodes() const
{
    return m_all_nodes.size();
}   // getNumNodes

// ----------------------------------------------------------------------------
const XMLNode *XMLReader::getNode(unsigned int n) const
{
    return m_all_nodes[n];
}   // getNode

// ----------------------------------------------------------------------------
const XMLNode *XMLReader::getNode(const std::string &node_name) const
{
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        if(m_all_nodes[i]->getName()==node_name) return m_all_nodes[i];
    }
    return NULL;
}   // getNode
#endif
