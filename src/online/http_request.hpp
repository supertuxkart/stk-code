//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 Joerg Henrichs
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

#include "io/file_manager.hpp"
#include "online/request.hpp"
#include "utils/cpp2011.h"
#include "utils/string_utils.hpp"
#include "utils/synchronised.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif
#include <curl/curl.h>
#include <assert.h>
#include <string>

namespace Online
{
    /** A http request.
     */
    class HTTPRequest : public Request
    {
    private:
        typedef std::map<std::string, std::string>  Parameters;

        /** The progress indicator. 0 untill it is started and the first
         *  packet is downloaded. Guaranteed to be <1 while the download
         *  is in progress, it will be set to either -1 (error) or 1
         *  (everything ok) at the end. */
        Synchronised<float> m_progress;

        /** The url to download. */
        std::string m_url;

        /** The POST parameters that will be send with the request. */
        Parameters *m_parameters;

        /** Pointer to the curl data structure for this request. */
        CURL *m_curl_session;

        /** curl return code. */
        CURLcode m_curl_code;

        /** String to store the received data in. */
        std::string m_string_buffer;

    protected:
        virtual void prepareOperation() OVERRIDE;
        virtual void operation() OVERRIDE;
        virtual void afterOperation() OVERRIDE;

        static int progressDownload(void *clientp, double dltotal,
                                    double dlnow,  double ultotal,
                                    double ulnow);

        static size_t writeCallback(void *contents, size_t size,
                                    size_t nmemb,   void *userp);


    public :
                           HTTPRequest(bool manage_memory = false, 
                                       int priority = 1);
        virtual           ~HTTPRequest();
        virtual bool       isAllowedToAdd() OVERRIDE;
        void               setServerURL(const std::string& url);
        // ------------------------------------------------------------------------
        /** Returns the curl error status of the request.
         */
        CURLcode getResult() const { return m_curl_code; }
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
        /** Sets a parameter to 'value' (std::string).
         */
        void addParameter(const std::string & name, const std::string &value)
        {
            assert(isPreparing());
            (*m_parameters)[name] = value;
        };   // addParameter
        // --------------------------------------------------------------------
        /** Sets a parameter to 'value' (stringw).
         */
        void addParameter(const std::string & name, 
                          const irr::core::stringw &value)
        {
            assert(isPreparing());
            (*m_parameters)[name] = irr::core::stringc(value.c_str()).c_str();
        }   // addParameter
        // --------------------------------------------------------------------
        /** Sets a parameter to 'value' (arbitrary types).
         */
        template <typename T>
        void addParameter(const std::string & name, const T& value){
            assert(isPreparing());
            (*m_parameters)[name] = StringUtils::toString(value);
        }   // addParameter
        // --------------------------------------------------------------------
        /** Returns the current progress. */
        float getProgress() const { return m_progress.getAtomic(); }
        // --------------------------------------------------------------------
        /** Sets the current progress. */
        void setProgress(float f)  { m_progress.setAtomic(f); }
        // --------------------------------------------------------------------
        const std::string & getURL() const { assert(isBusy()); return m_url;}
        // --------------------------------------------------------------------
        /** Sets the URL for this request. */
        void setURL(const std::string & url) 
        {
            assert(isPreparing()); 
            m_url = url;
        }   // setURL

    };   // class HTTPRequest

} //namespace Online

#endif

