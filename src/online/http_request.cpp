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

#include "online/http_request.hpp"

#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Online
{
    const std::string API::USER_PATH = "user/";
    const std::string API::SERVER_PATH = "server/";

    /** Creates a HTTP(S) request that will have a raw string as result. (Can
     *  of course be used if the result doesn't matter.)
     *  \param priority by what priority should the RequestManager take care of
     *         this request.
     */
    HTTPRequest::HTTPRequest(int priority)
               : Request(priority, 0)
    {
        init();
    }   // HTTPRequest

    // ------------------------------------------------------------------------
    /** This constructor configures this request to save the data in a file.
     *  \param filename Name of the file to save the data to.
     *  \param priority by what priority should the RequestManager take care of
     *         this request.
     */
    HTTPRequest::HTTPRequest(const std::string &filename, int priority)
               : Request(priority, 0)
    {
        // A http request should not even be created when internet is disabled
        assert(UserConfigParams::m_internet_status ==
               RequestManager::IPERM_ALLOWED);
        assert(filename.size() > 0);

        init();
        m_filename = file_manager->getAddonsFile(filename);
    }   // HTTPRequest(filename ...)

    // ------------------------------------------------------------------------
    /** Char * needs a separate constructor, otherwise it will be considered
     *  to be the no-filename constructor (char* -> bool).
     */
    HTTPRequest::HTTPRequest(const char* const filename, int priority)
               : Request(priority, 0)
    {
        // A http request should not even be created when internet is disabled
        assert(UserConfigParams::m_internet_status ==
               RequestManager::IPERM_ALLOWED);

        init();
        m_filename = file_manager->getAddonsFile(filename);
    }   // HTTPRequest(filename ...)

    // ------------------------------------------------------------------------
    /** Initialises all member variables.
     */
    void HTTPRequest::init()
    {
        m_url = "";
        m_string_buffer = "";
        m_filename = "";
        m_parameters = "";
        m_result_code = 0;
        m_progress.store(0.0f);
        m_total_size.store(-1.0);
        m_disable_sending_log = false;
        m_download_assets_request = false;
    }   // init

    // ------------------------------------------------------------------------
    /** A handy shortcut that appends the given path to the URL of the
     *  mutiplayer server. It also supports the old (version 1) api,
     *  where a 'action' parameter was sent to 'client-user.php'.
     *  \param path The path to add to the server.(see API::USER_*)
     *  \param action The action to perform. eg: connect, pool
     */
    void HTTPRequest::setApiURL(const std::string& path,
                                const std::string &action)
    {
        // Old (0.8.1) API: send to client-user.php, and add action as a parameter
        if (stk_config->m_server_api_version == 1)
        {
            const std::string final_url = stk_config->m_server_api + "client-user.php";
            setURL(final_url);
            if (action == "change-password")
                addParameter("action", "change_password");
            else if (action == "recover")
                addParameter("action", "recovery");
            else
                addParameter("action", action);
        }
        else
        {
            const std::string final_url = stk_config->m_server_api +
                + "v" + StringUtils::toString(stk_config->m_server_api_version)
                + "/" + path // eg: /user/, /server/
                + action + "/"; // eg: connect/, pool/, get-server-list/

            setURL(final_url);
        }
    }   // setServerURL

    // ------------------------------------------------------------------------
    /** A handy shortcut that appends the given path to the URL of the addons
     *  server.
     *  \param path The path to add to the server, without the trailing slash
     */
     void HTTPRequest::setAddonsURL(const std::string& path)
     {
        setURL(stk_config->m_server_addons + "/" + path);
     }   // set AddonsURL

     // ------------------------------------------------------------------------
    /** Checks the request if it has enough (correct) information to be
     *  executed (and thus allowed to add to the queue).
     */
    bool HTTPRequest::isAllowedToAdd() const
    {
        return Request::isAllowedToAdd() && m_url.substr(0, 5) == "http:";
    }   // isAllowedToAdd

    // ------------------------------------------------------------------------
    void HTTPRequest::logMessage()
    {
        // Avoid printing the password or token, just replace them with *s
        std::string param = m_parameters;

        // List of strings whose values should not be printed. "" is the
        // end indicator.
        static std::string dont_print[] = { "&password=", "&token=", "&current=",
                                            "&new1=", "&new2=", "&password_confirm=",
                                            "&aes-key=", "&iv=", ""};
        unsigned int j = 0;
        while (dont_print[j].size() > 0)
        {
            // Get the string that should be replaced.
            std::size_t pos = param.find(dont_print[j]);
            if (pos != std::string::npos)
            {
                pos += dont_print[j].size();
                while (pos < param.size() && param[pos] != '&')
                {
                    param[pos] = '*';
                    pos++;
                }   // while not end
            }   // if string found
            j++;
        }   // while dont_print[j].size()>0

        Log::info("HTTPRequest", "Sending %s to %s", param.c_str(), m_url.c_str());
    }   // logMessage

    // ------------------------------------------------------------------------
    std::string HTTPRequest::urlEncode(const std::string& value)
    {
        auto unreserved = [](unsigned char c)->bool
        {
            // RFC 3986 unreserved characters: ALPHA / DIGIT / "-" / "." / "_" / "~"
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
                return true;
            return (c == '-' || c == '.' || c == '_' || c == '~');
        };

        std::ostringstream escaped;
        escaped << std::hex << std::uppercase << std::setfill('0');
        for (unsigned char c : value)
        {
            if (unreserved(c))
                escaped << c;
            else
                escaped << '%' << std::setw(2) << static_cast<int>(c);
        }
        return escaped.str();
    }   // urlEncode

} // namespace Online
