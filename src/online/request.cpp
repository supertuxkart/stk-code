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

#include "online/http_manager.hpp"
#include "utils/translation.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif

#include <assert.h>


namespace Online{

    class HTTPManager;

    // =========================================================================================

    Request::Request(int type, bool manage_memory, int priority)
        : m_type(type), m_manage_memory(manage_memory), m_priority(priority)
    {
        m_cancel.setAtomic(false);
        m_done.setAtomic(false);
    }   // Request

    Request::~Request()
    {
    }

    void Request::execute()
    {
        prepareOperation();
        operation();
        afterOperation();
    }

    void Request::afterOperation()
    {
        m_done.setAtomic(true);
    }

    // =========================================================================================

    HTTPRequest::HTTPRequest(int type, bool manage_memory, int priority)
        : Request(priority, manage_memory, type)
    {
        //Negative numbers are reserved for special requests ment for the HTTP Manager
        assert(type >= 0);
        m_url.setAtomic("");
        MutexLocker(m_parameters);
        m_parameters.getData() = new Parameters;
        m_progress.setAtomic(0);
    }

    HTTPRequest::~HTTPRequest()
    {
        MutexLocker(m_parameters);
        delete m_parameters.getData();
    }

    bool HTTPRequest::isAllowedToAdd()
    {
        bool ok = true;
        m_url.lock();
        if (m_url.getData().size() > 5 && ( m_url.getData().substr(0, 5) != "http:"))
            ok = false;
        m_url.unlock();
        return ok;
    }

    void HTTPRequest::prepareOperation()
    {
        m_curl_session = curl_easy_init();
        if(!m_curl_session)
        {
            Log::error("HTTPRequest::prepareOperation", "LibCurl session not initialized.");
            return;
        }
        curl_easy_setopt(m_curl_session, CURLOPT_URL, m_url.getAtomic().c_str());
        curl_easy_setopt(m_curl_session, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(m_curl_session, CURLOPT_WRITEFUNCTION, &HTTPRequest::WriteCallback);
        curl_easy_setopt(m_curl_session, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(m_curl_session, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(m_curl_session, CURLOPT_PROGRESSFUNCTION, &HTTPRequest::progressDownload);
        curl_easy_setopt(m_curl_session, CURLOPT_CONNECTTIMEOUT, 20);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_LIMIT, 10);
        curl_easy_setopt(m_curl_session, CURLOPT_LOW_SPEED_TIME, 20);
    }

    void HTTPRequest::operation()
    {
        if(!m_curl_session)
            return;
        Parameters::iterator iter;
        std::string postString("");
        m_parameters.lock();
        for (iter = m_parameters.getData()->begin(); iter != m_parameters.getData()->end(); ++iter)
        {
           if(iter != m_parameters.getData()->begin())
               postString.append("&");
           char * escaped = curl_easy_escape(m_curl_session , iter->first.c_str(), iter->first.size());
           postString.append(escaped);
           curl_free(escaped);
           postString.append("=");
           escaped = curl_easy_escape(m_curl_session , iter->second.c_str(), iter->second.size());
           postString.append(escaped);
           curl_free(escaped);
        }
        m_parameters.unlock();
        curl_easy_setopt(m_curl_session, CURLOPT_POSTFIELDS, postString.c_str());
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
    }

    void HTTPRequest::afterOperation()
    {
        if(m_curl_code == CURLE_OK)
            setProgress(1.0f);
        else
            setProgress(-1.0f);
        curl_easy_cleanup(m_curl_session);
        callback();
        Request::afterOperation();
    }

    size_t HTTPRequest::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // ----------------------------------------------------------------------------
    /** Callback function from curl: inform about progress.
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
            // Don't set progress to 1.0f; this is done in loadFileInternal
            // after checking curls return code!
            f= download_total==0 ? 0 : 0.99f;
        }
        request->setProgress(f);
        return 0;
    }   // progressDownload



    // =========================================================================================

    XMLRequest::XMLRequest(int type, bool manage_memory, int priority)
        : HTTPRequest(priority, manage_memory, type)
    {
        m_string_buffer = "";
        m_info.setAtomic("");
        m_success.setAtomic(false);
        MutexLocker(m_result);
        m_result.getData() = NULL;
    }

    XMLRequest::~XMLRequest()
    {
        MutexLocker(m_result);
        delete m_result.getData();
    }

    void XMLRequest::prepareOperation()
    {
        HTTPRequest::prepareOperation();
        curl_easy_setopt(m_curl_session, CURLOPT_WRITEDATA, &m_string_buffer);
    }


    void XMLRequest::operation()
    {
        HTTPRequest::operation();
        MutexLocker(m_result);
        m_result.getData() = file_manager->createXMLTreeFromString(m_string_buffer);
    }

    void XMLRequest::afterOperation()
    {
        if(m_curl_code != CURLE_OK)
            Log::error( "XMLRequest::afterOperation", "curl_easy_perform() failed: %s", curl_easy_strerror(m_curl_code));
        else
            Log::info(  "XMLRequest::afterOperation", "Received : %s",                  m_string_buffer.c_str());
        m_result.lock();;
        bool success = false;
        irr::core::stringw info;
        std::string rec_success;
        if(m_result.getData()->get("success", &rec_success))
        {
            if (rec_success =="yes")
                success = true;
            m_result.getData()->get("info", &info);
        }
        else
            info = _("Unable to connect to the server. Check your internet connection or try again later.");
        m_result.unlock();
        m_info.lock();
        m_info.getData() = info;
        m_info.unlock();
        m_success.setAtomic(success);
        HTTPRequest::afterOperation();
    }

    const XMLNode * XMLRequest::getResult() const
    {
        MutexLocker(m_result);
        return m_result.getData();
    }

    const irr::core::stringw & XMLRequest::getInfo()   const
    {
        MutexLocker(m_info);
        return m_info.getData();
    }
} // namespace Online
