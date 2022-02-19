//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
//                2013 Glenn De Jonghe
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

#ifndef HEADER_HTTP_REQUEST_HPP
#define HEADER_HTTP_REQUEST_HPP

#include "online/request.hpp"
#include "utils/cpp2011.hpp"
#include "utils/string_utils.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif
#include <atomic>
#include <curl/curl.h>
#include <assert.h>
#include <string>

#if defined(CURLOPT_XFERINFODATA)
#define PROGRESSDATA     CURLOPT_XFERINFODATA
#define PROGRESSFUNCTION CURLOPT_XFERINFOFUNCTION
typedef curl_off_t progress_t;
#else
#define PROGRESSDATA     CURLOPT_PROGRESSDATA
#define PROGRESSFUNCTION CURLOPT_PROGRESSFUNCTION
typedef double progress_t;
#endif

namespace Online
{
    class API
    {
    public:
        static const std::string USER_PATH;
        static const std::string SERVER_PATH;
    };

    /** A http request.
     */
    class HTTPRequest : public Request
    {
    private:
        /** The progress indicator. 0 untill it is started and the first
         *  packet is downloaded. Guaranteed to be <1 while the download
         *  is in progress, it will be set to either -1 (error) or 1
         *  (everything ok) at the end. */
        std::atomic<float> m_progress;

        std::atomic<double> m_total_size;

        /** The url to download. */
        std::string m_url;

        /** The POST parameters that will be send with the request. */
        std::string m_parameters;


        /** Pointer to the curl data structure for this request. */
        CURL *m_curl_session = NULL;

        /** curl return code. */
        CURLcode m_curl_code;

        /** String to store the received data in. */
        std::string m_string_buffer;

        struct curl_slist* m_http_header = NULL;
    protected:
        /** Contains a filename if the data should be saved into a file
         *  instead of being kept in in memory. Otherwise this is "". */
        std::string m_filename;

        bool m_disable_sending_log;
        /* If true, it will not call curl_easy_setopt CURLOPT_POSTFIELDS so
         * it's just a GET request. */
        bool m_download_assets_request = false;

        virtual void prepareOperation() OVERRIDE;
        virtual void operation() OVERRIDE;
        virtual void afterOperation() OVERRIDE;

        static int progressDownload(void *clientp, progress_t dltotal,
                                    progress_t dlnow,  progress_t ultotal,
                                    progress_t ulnow);

        static size_t writeCallback(void *contents, size_t size,
                                    size_t nmemb,   void *userp);
        void init();

    public :
        HTTPRequest(int priority = 1);
        HTTPRequest(const std::string &filename, int priority = 1);
        HTTPRequest(const char * const filename, int priority = 1);
        virtual           ~HTTPRequest()
        {
            if (m_http_header)
                curl_slist_free_all(m_http_header);
            if (m_curl_session)
            {
                curl_easy_cleanup(m_curl_session);
                m_curl_session = NULL;
            }
        }
        virtual bool       isAllowedToAdd() const OVERRIDE;
        void               setApiURL(const std::string& url, const std::string &action);
        void               setAddonsURL(const std::string& path);

        // ------------------------------------------------------------------------
        /** Returns true if there was an error downloading the file. */
        virtual bool hadDownloadError() const { return m_curl_code != CURLE_OK; }
        // ------------------------------------------------------------------------
        void setDownloadAssetsRequest(bool val)
                                               { m_download_assets_request = val; }
        // ------------------------------------------------------------------------
        /** Returns the curl error message if an error has occurred.
         *  \pre m_curl_code!=CURLE_OK
         */
        const char* getDownloadErrorMessage() const
        {
            assert(hadDownloadError());
            return curl_easy_strerror(m_curl_code);
        }   // getDownloadErrorMessage

        // ------------------------------------------------------------------------
        /** Returns the downloaded string.
         *  \pre request has to be done
         *  \return get the result string from the request reply
         */
        const std::string & getData() const
        {
            assert(hasBeenExecuted());
            return m_string_buffer;
        }   // getData

        // --------------------------------------------------------------------
        /** Sets a parameter to 'value' (std::string). */
        void addParameter(const std::string & name, const std::string &value)
        {
            // Call the template, so that the strings are escaped properly
            addParameter(name, value.c_str());
        }   // addParameter

        // --------------------------------------------------------------------
        /** Sets a parameter to 'value' (stringw). */
        void addParameter(const std::string &name,
                          const irr::core::stringw &value)
        {
            std::string s = StringUtils::wideToUtf8(value);

            // Call the template to escape strings properly
            addParameter(name, s.c_str());
        }   // addParameter

        // --------------------------------------------------------------------
        /** Sets a parameter to 'value' (arbitrary types). */
        template <typename T>
        void addParameter(const std::string &name, const T& value)
        {
            assert(isPreparing());
            std::string s = StringUtils::toString(value);

            char *s1 = curl_easy_escape(m_curl_session, name.c_str(), (int)name.size());
            char *s2 = curl_easy_escape(m_curl_session, s.c_str(), (int)s.size());
            m_parameters.append(std::string(s1) + "=" + s2 + "&");
            curl_free(s1);
            curl_free(s2);
        }   // addParameter

        // --------------------------------------------------------------------
        /** Returns the current progress. */
        float getProgress() const { return m_progress.load(); }

        // --------------------------------------------------------------------
        /** Sets the current progress. */
        void setProgress(float f) { m_progress.store(f); }

        // --------------------------------------------------------------------
        const std::string & getURL() const { assert(isBusy()); return m_url;}

        // --------------------------------------------------------------------
        /** Sets the URL for this request. */
        void setURL(const std::string & url)
        {
            assert(isPreparing());
            m_url = url;
        }   // setURL
        // --------------------------------------------------------------------
        const std::string& getFileName() const           { return m_filename; }
        // --------------------------------------------------------------------
        double getTotalSize() const             { return m_total_size.load(); }
        // --------------------------------------------------------------------
        void setTotalSize(double d)                  { m_total_size.store(d); }
    };   // class HTTPRequest
} //namespace Online
#endif // HEADER_HTTP_REQUEST_HPP
