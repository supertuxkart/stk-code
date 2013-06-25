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
#include "utils/string_utils.hpp"

/**
  * \brief Class to connect with a server over HTTP
  * \ingroup online
  */
class HTTPConnector
{
    protected:
        CURL *curl;
        CURLcode res;
        typedef std::map<std::string, std::string> Parameters;
        Parameters m_parameters;

    public:

        HTTPConnector(const std::string &url);
        ~HTTPConnector();

        //Execute
        std::string getPage();
        XMLNode * getXMLFromPage();

        //Setting parameters to be send with the next request
        void setParameter(const std::string & name, const std::string &value){
            m_parameters[name] = value;
        };
        void setParameter(const std::string & name, const irr::core::stringw &value){
            m_parameters[name] = irr::core::stringc(value.c_str()).c_str();
        }
        template <typename T>
        void setParameter(const std::string & name, const T& value){
            m_parameters[name] = StringUtils::toString(value);
        }

}; //class HTTPConnector


#endif // HTTP_CONNECTOR_HPP

/*EOF*/
