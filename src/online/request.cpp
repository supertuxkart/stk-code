//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "online/request.hpp"

#include "config/user_config.hpp"
#include "online/http_manager.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif

#include <curl/curl.h>

#include <assert.h>


namespace Online{

    class HTTPManager;

    // ========================================================================
    /**
     *  Creates a request that can be handled by the HTTPManager
     *  \param manage_memory whether or not the HTTPManager should take care of
     *         deleting the object after all callbacks have been done
     *  \param priority by what priority should the HTTPManager take care of
     *         this request
     *  \param type indicates whether the request has a special task for the
     *         HTTPManager
     */
    Request::Request(bool manage_memory, int priority, int type)
        : m_type(type), m_manage_memory(manage_memory), m_priority(priority)
    {
        m_cancel.setAtomic(false);
        m_state.setAtomic(S_PREPARING);
    }   // Request

    // ------------------------------------------------------------------------
    /** Executes the request. This calles prepareOperation, operation, and
     *  afterOperation.
     */
    void Request::execute()
    {
        assert(isBusy());
        prepareOperation();
        operation();
        setExecuted();
        afterOperation();
    }   // execute
    // ------------------------------------------------------------------------
    /** Executes the request now, i.e. in the main thread and without involving
     *  the manager thread.. This calles prepareOperation, operation, and
     *  afterOperation.
     */
    void Request::executeNow()
    {
        assert(isPreparing());
        setBusy();
        execute();
        callback();
        setDone();
    }   // executeNow

    // ========================================================================
    /** Creates a HTTP(S) request that will have a raw string as result. (Can 
     *  of course be used if the result doesn't matter.)
     *  \param manage_memory whether or not the HTTPManager should take care of
     *         deleting the object after all callbacks have been done.
     *  \param priority by what priority should the HTTPManager take care of 
     *         this request.
     */
    HTTPRequest::HTTPRequest(bool manage_memory, int priority)
               : Request(manage_memory, priority, 0)
    {
        m_url           = "";
        m_string_buffer = "";
        m_parameters    = new Parameters();
        m_progress.setAtomic(0);
    }   // HTTPRequest

    // ------------------------------------------------------------------------
    HTTPRequest::~HTTPRequest()
    {
        delete m_parameters;
    }   // ~HTTPRequest

    // ------------------------------------------------------------------------
    /** A handy shortcut that appends the given path to the URL of the server.
     *  \param path The path to add to the server.
     */
    void HTTPRequest::setServerURL(const std::string& path)
    {
        setURL((std::string)UserConfigParams::m_server_multiplayer+path);
    }   // setServerURL

    // ------------------------------------------------------------------------
    /** Checks the request if it has enough (correct) information to be
     *  executed (and thus allowed to add to the queue).
     */
    bool HTTPRequest::isAllowedToAdd()
    {
        return Request::isAllowedToAdd() && m_url.substr(0, 5) == "http:";
    }   // isAllowedToAdd

    // ------------------------------------------------------------------------
    /** Sets up the curl data structures.
     */
    void HTTPRequest::prepareOperation()
    {
        m_curl_session = curl_easy_init();
        if(!m_curl_session)
        {
            Log::error("HTTPRequest::prepareOperation",
                       "LibCurl session not initialized.");
            return;
        }
        curl_easy_setopt(m_curl_session, CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(m_curl_session, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(m_curl_session, CURLOPT_WRITEFUNCTION, 
                                         &HTTPRequest::writeCallback);
        curl_easy_setopt(m_curl_session, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(m_curl_session, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(m_curl_session, CURLOPT_PROGRESSFUNCTION,
                                         &HTTPRequest::progressDownload);
        curl_easy_setopt(m_curl_session, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_LIMIT, 10);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_TIME, 20);
        //https
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Host: api.stkaddons.net");
        curl_easy_setopt(m_curl_session, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(m_curl_session, CURLOPT_CAINFO, 
                    (file_manager->getAsset("web.tuxfamily.org.pem")).c_str());
        curl_easy_setopt(m_curl_session, CURLOPT_SSL_VERIFYPEER, 0L);
        //curl_easy_setopt(m_curl_session, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(m_curl_session, CURLOPT_WRITEDATA, &m_string_buffer);
    }   // prepareOperation

    // ------------------------------------------------------------------------
    /** The actual curl download happens here.
     */
    void HTTPRequest::operation()
    {
        if(!m_curl_session)
            return;
        Parameters::iterator iter;
        std::string postString("");
        for (iter = m_parameters->begin(); iter != m_parameters->end(); ++iter)
        {
           if(iter != m_parameters->begin())
               postString.append("&");
           char* escaped = curl_easy_escape(m_curl_session , 
                                            iter->first.c_str(), 
                                            iter->first.size());
           postString.append(escaped);
           curl_free(escaped);
           postString.append("=");
           escaped = curl_easy_escape(m_curl_session,
                                      iter->second.c_str(),
                                      iter->second.size());
           postString.append(escaped);
           curl_free(escaped);
        }
        Log::info("HTTPRequest::operation", "Sending : %s",
                  postString.c_str());
        curl_easy_setopt(m_curl_session, CURLOPT_POSTFIELDS,
                         postString.c_str());
        std::string uagent( std::string("SuperTuxKart/") + STK_VERSION );
            #ifdef WIN32
                    uagent += (std::string)" (Windows)";
            #elif defined(__APPLE__)
                    uagent += (std::string)" (Macintosh)";
            #elif defined(__FreeBSD__)
                    uagent += (std::string)" (FreeBSD)";
            #elif defined(linux)
                    uagent += (std::string)" (Linux)";
            #else
                    // Unknown system type
            #endif
        curl_easy_setopt(m_curl_session, CURLOPT_USERAGENT, uagent.c_str());

        m_curl_code = curl_easy_perform(m_curl_session);
        Request::operation();
    }   // operation

    // ------------------------------------------------------------------------
    /** Cleanup once the download is finished. The value of progress is 
     *  guaranteed to be >=0 and <1 while the download is in progress, and
     *  will only be set to 1 on a successfull finish here.
     */
    void HTTPRequest::afterOperation()
    {
        if(m_curl_code == CURLE_OK)
            setProgress(1.0f);
        else
            setProgress(-1.0f);
        Request::afterOperation();
        curl_easy_cleanup(m_curl_session);
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
                                      double download_total, double download_now,
                                      double upload_total,   double upload_now)
    {
        HTTPRequest *request = (HTTPRequest *)clientp;

        HTTPManager* http_manager = HTTPManager::get();

        // Check if we are asked to abort the download. If so, signal this
        // back to libcurl by returning a non-zero status.
        if(http_manager->getAbort() || request->isCancelled() )
        {
            // Indicates to abort the current download, which means that this
            // thread will go back to the mainloop and handle the next request.
            return 1;
        }

        float f;
        if(download_now < download_total)
        {
            f = (float)download_now / (float)download_total;
            // In case of floating point rouding errors make sure that
            // 1.0 is only reached when downloadFileInternal is finished
            if (f>=1.0f) f=0.99f;
        }
        else
        {
            // Don't set progress to 1.0f; this is done in afterOperation()
            // after checking curls return code!
            f= download_total==0 ? 0 : 0.99f;
        }
        request->setProgress(f);
        return 0;
    }   // progressDownload

    // ========================================================================
    /** Creates a HTTP(S) request that will automatically parse the answer into
     *  a XML structure.
     *  \param manage_memory whether or not the HTTPManager should take care of
     *         deleting the object after all callbacks have been done.
     *  \param priority by what priority should the HTTPManager take care of
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
        if(getResult() != CURLE_OK)
            Log::error("XMLRequest::afterOperation", 
                       "curl_easy_perform() failed: %s",
                        curl_easy_strerror(getResult()));
        m_success = false;
        std::string rec_success;
        if(m_xml_data->get("success", &rec_success))
        {
            m_success = rec_success =="yes";
            m_xml_data->get("info", &m_info);
        }
        else
            m_info = _("Unable to connect to the server. Check your internet "
                       "connection or try again later.");
        HTTPRequest::afterOperation();
    }   // afterOperation


} // namespace Online
