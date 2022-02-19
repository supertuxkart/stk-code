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
#include "utils/file_utils.hpp"
#include "utils/translation.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif

#include <curl/curl.h>
#include <assert.h>

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
    /** This constructor configures this request to save the data in a flie.
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
        m_url           = "";
        m_string_buffer = "";
        m_filename      = "";
        m_parameters    = "";
        m_curl_code     = CURLE_OK;
        m_progress.store(0.0f);
        m_total_size.store(-1.0);
        m_disable_sending_log = false;
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
    /** Sets up the curl data structures.
     */
    void HTTPRequest::prepareOperation()
    {
        m_curl_session = curl_easy_init();
        if (!m_curl_session)
        {
            Log::error("HTTPRequest::prepareOperation",
                       "LibCurl session not initialized.");
            return;
        }

        curl_easy_setopt(m_curl_session, CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(m_curl_session, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl_session, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(m_curl_session, PROGRESSDATA, this);
        curl_easy_setopt(m_curl_session, PROGRESSFUNCTION,
                                         &HTTPRequest::progressDownload);
        curl_easy_setopt(m_curl_session, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_LIMIT, 10);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_TIME, 20);
        curl_easy_setopt(m_curl_session, CURLOPT_NOSIGNAL, 1);
        //curl_easy_setopt(m_curl_session, CURLOPT_VERBOSE, 1L);

        // https, load certificate info
        const std::string& ci = file_manager->getCertBundleLocation();
        CURLcode error = curl_easy_setopt(m_curl_session, CURLOPT_CAINFO, ci.c_str());
        if (error != CURLE_OK)
        {
            Log::error("HTTPRequest", "Error setting CAINFO to '%s'",
                ci.c_str());
            Log::error("HTTPRequest", "Error: '%s'.", error,
                curl_easy_strerror(error));
        }
        std::string host = "Host: " + StringUtils::getHostNameFromURL(m_url);
        m_http_header = curl_slist_append(m_http_header, host.c_str());
        assert(m_http_header != nullptr);
        curl_easy_setopt(m_curl_session, CURLOPT_HTTPHEADER, m_http_header);
        curl_easy_setopt(m_curl_session, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_curl_session, CURLOPT_SSL_VERIFYHOST, 2L);
    }   // prepareOperation

    // ------------------------------------------------------------------------
    /** The actual curl download happens here.
     */
    void HTTPRequest::operation()
    {
        if (!m_curl_session)
            return;

        FILE *fout = NULL;
        if (m_filename.size() > 0)
        {
            fout = FileUtils::fopenU8Path(m_filename + ".part", "wb");

            if (!fout)
            {
                Log::error("HTTPRequest",
                           "Can't open '%s' for writing, ignored.",
                           (m_filename+".part").c_str());
                return;
            }
            curl_easy_setopt(m_curl_session,  CURLOPT_WRITEDATA,     fout  );
            curl_easy_setopt(m_curl_session,  CURLOPT_WRITEFUNCTION, fwrite);
        }
        else
        {
            curl_easy_setopt(m_curl_session, CURLOPT_WRITEDATA,
                             &m_string_buffer);
            curl_easy_setopt(m_curl_session, CURLOPT_WRITEFUNCTION,
                             &HTTPRequest::writeCallback);
        }

        // All parameters added have a '&' added
        if (m_parameters.size() > 0)
        {
            m_parameters.erase(m_parameters.size()-1);
        }

        if (m_parameters.size() == 0 && !m_disable_sending_log)
        {
            Log::info("HTTPRequest", "Downloading %s", m_url.c_str());
        }
        else if (Log::getLogLevel() <= Log::LL_INFO && !m_disable_sending_log)
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
        } // end log http request

        if (!m_download_assets_request)
        {
            curl_easy_setopt(m_curl_session, CURLOPT_POSTFIELDS,
                m_parameters.c_str());
        }
        const std::string& uagent = StringUtils::getUserAgentString();
        curl_easy_setopt(m_curl_session, CURLOPT_USERAGENT, uagent.c_str());

        m_curl_code = curl_easy_perform(m_curl_session);
        Request::operation();

        if (fout)
        {
            fclose(fout);
            if (m_curl_code == CURLE_OK)
            {
                if(UserConfigParams::logAddons())
                    Log::info("HTTPRequest", "Download successful.");

                // The behaviour of rename is unspecified if the target
                // file should already exist - so remove it.
                bool ok = file_manager->removeFile(m_filename);
                if (!ok)
                {
                    Log::error("addons",
                               "Could not removed existing addons.xml file.");
                    m_curl_code = CURLE_WRITE_ERROR;
                }
                int ret = FileUtils::renameU8Path(m_filename + ".part", m_filename);
                // In case of an error, set the status to indicate this
                if (ret != 0)
                {
                    Log::error("addons",
                               "Could not rename downloaded addons.xml file!");
                    m_curl_code = CURLE_WRITE_ERROR;
                }
            }   // m_curl_code ==CURLE_OK
        }   // if fout
    }   // operation

    // ------------------------------------------------------------------------
    /** Cleanup once the download is finished. The value of progress is
     *  guaranteed to be >=0 and <1 while the download is in progress, and
     *  will only be set to 1 on a successfull finish here.
     */
    void HTTPRequest::afterOperation()
    {
        if (m_curl_code == CURLE_OK)
            setProgress(1.0f);
        else
            setProgress(-1.0f);

        Request::afterOperation();
        if (m_http_header)
        {
            curl_slist_free_all(m_http_header);
            m_http_header = NULL;
        }
        if (m_curl_session)
        {
            curl_easy_cleanup(m_curl_session);
            m_curl_session = NULL;
        }
    }   // afterOperation

    // ------------------------------------------------------------------------
    /** Callback from curl. This stores the data received by curl in the
     *  buffer of this request.
     *  \param content Pointer to the data received by curl.
     *  \param size Size of one block.
     *  \param nmemb Number of blocks received.
     *  \param userp Pointer to the user buffer.
     */
    size_t HTTPRequest::writeCallback(void *contents, size_t size,
                                      size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }   // writeCallback

    // ----------------------------------------------------------------------------
    /** Callback function from curl: inform about progress. It makes sure that
     *  the value reported by getProgress () is <1 while the download is still
     *  in progress.
     *  \param clientp
     *  \param download_total Total size of data to download.
     *  \param download_now   How much has been downloaded so far.
     *  \param upload_total   Total amount of upload.
     *  \param upload_now     How muc has been uploaded so far.
     */
    int HTTPRequest::progressDownload(void *clientp,
                                      progress_t download_total, progress_t download_now,
                                      progress_t upload_total,   progress_t upload_now)
    {
        HTTPRequest *request = (HTTPRequest *)clientp;

        // Check if we are asked to abort the download. If so, signal this
        // back to libcurl by returning a non-zero status.
        if (RequestManager::isRunning() &&
            (RequestManager::get()->getAbort() || RequestManager::get()->getPaused() ||
             request->isCancelled()) &&
             request->isAbortable()                                     )
        {
            // Indicates to abort the current download, which means that this
            // thread will go back to the mainloop and handle the next request.
            return 1;
        }

        float f;
        request->setTotalSize(download_total);
        if (download_now < download_total)
        {
            f = (float)download_now / (float)download_total;

            // In case of floating point rouding errors make sure that
            // 1.0 is only reached when downloadFileInternal is finished
            if (f >= 1.0f)
                f = 0.99f;
        }
        else
        {
            // Don't set progress to 1.0f; this is done in afterOperation()
            // after checking curls return code!
            f = (download_total == 0) ? 0 : 0.99f;
        }
        request->setProgress(f);

        return 0;
    }   // progressDownload

} // namespace Online
