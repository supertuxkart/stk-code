//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#ifndef HTTP_FUNCTIONS_HPP
#define HTTP_FUNCTIONS_HPP

#include <string>
#include "io/xml_node.hpp"

/**
  * \brief HTTP functions to connect with the server
  * \ingroup online
  */

namespace HTTP
{

void init();
void shutdown();

std::string getPage(std::string url);
XMLNode * getXMLFromPage(std::string url);


}

#endif // HTTP_FUNCTIONS_HPP

/*EOF*/
