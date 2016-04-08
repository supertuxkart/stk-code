//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "online/xml_request.hpp"

#include "config/user_config.hpp"
#include "online/request_manager.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif
#include <curl/curl.h>

#include <assert.h>

namespace Online
{
    /** Creates a HTTP(S) request that will automatically parse the answer into
     *  a XML structure.
     *  \param manage_memory whether or not the RequestManager should take care of
     *         deleting the object after all callbacks have been done.
     *  \param priority by what priority should the RequestManager take care of
     *         this request.
     */
    XMLRequest::XMLRequest(bool manage_memory, int priority)
              : HTTPRequest(manage_memory, priority)
    {
        m_info     = "";
        m_success  = false;
        m_xml_data = NULL;
    }   // XMLRequest

    // ------------------------------------------------------------------------
    /** Cleans up the XML tree. */
    XMLRequest::~XMLRequest()
    {
        delete m_xml_data;
    }   // ~XMLRequest

    // ------------------------------------------------------------------------
    /** On a successful download converts the string into an XML tree.
     */
    void XMLRequest::afterOperation()
    {
        m_xml_data = file_manager->createXMLTreeFromString(getData());
        if (hadDownloadError())
        {
            Log::error("XMLRequest::afterOperation",
                       "curl_easy_perform() failed: %s",
                       getDownloadErrorMessage());
        }

        m_success = false;
        std::string rec_success;
        if (m_xml_data->get("success", &rec_success))
        {
            m_success = (rec_success == "yes");
            m_xml_data->get("info", &m_info);

            if (!m_success)
            {
                Log::debug("XMLRequest::afterOperation",
                           "Request returned error: %ls", m_info.c_str());
            }
        }
        else
        {
            m_info = _("Unable to connect to the server. Check your internet "
                       "connection or try again later.");
        }
        HTTPRequest::afterOperation();
    }   // afterOperation

} // namespace Online
