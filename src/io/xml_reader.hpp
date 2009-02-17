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
#ifndef HEADER_XML_READER_HPP
#define HEADER_XML_READER_HPP

#include <string>
#include <map>

#include "irrlicht.h"
using namespace irr;

class XMLNode;

class XMLReader
{
private:
     io::IXMLReader *m_reader;
     std::map<std::string, XMLNode *> m_all_nodes;
public:
       XMLReader(io::IXMLReader *r);

       XMLNode *getNode(const std::string &node_name);
};   // XMLReader
#endif
#endif