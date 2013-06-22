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

#ifndef HTTP_CONNECTOR_HPP
#define HTTP_CONNECTOR_HPP

#include <string>
#include "io/xml_node.hpp"
#include <curl/curl.h>
#include <irrString.h>
using namespace irr;

/**
  * \brief Class to connect with a server over HTTP
  * \ingroup online
  */
class HTTPConnector
{
    protected:
        CURL *curl;
        CURLcode res;

    public:
        typedef std::map<std::string, core::stringw> Parameters;
        HTTPConnector(const std::string &url);
        ~HTTPConnector();
        std::string getPage(Parameters & post_parameters);
        XMLNode * getXMLFromPage(Parameters & post_parameters);
}; //class HTTPConnector


#endif // HTTP_CONNECTOR_HPP

/*EOF*/
