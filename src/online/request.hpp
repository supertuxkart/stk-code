//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#ifndef HEADER_ONLINE_REQUEST_HPP
#define HEADER_ONLINE_REQUEST_HPP

#include <string>
#include <curl/curl.h>

#include "utils/cpp2011.h"
#include "utils/string_utils.hpp"
#include "utils/synchronised.hpp"
#include "io/file_manager.hpp"


namespace Online{

    /**
      * Stores a request for the HTTP Manager. They will be sorted by priorities.
      * \ingroup online
      */
    class Request
    {
    private:
        /** Can be used as identifier for the user.
         *  Has 0 as default value.
         *  Only requests ment to control the HTTP Manager should use negative values, for all other uses positive values are obliged
         *  */
        const int              m_type;
        /** True if the memory for this Request should be managed by
        *  http connector (i.e. this object is freed once the request
        *  is handled). Otherwise the memory is not freed, so it must
        *  be freed by the calling function. */
        const bool             m_manage_memory;
        /** The priority of this request. The higher the value the more
        important this request is. */
        const int              m_priority;

    protected:

        /** Cancel this request if it is active. */
        Synchronised<bool>              m_cancel;
        /** Set to though if the reply of the request is in and callbacks are executed */
        Synchronised<bool>              m_done;

        virtual void prepareOperation() {}
        virtual void operation() {}
        virtual void afterOperation();

    public:
        /** Negative numbers are reserved for requests that are special for the HTTP Manager*/
        enum RequestType
        {
            RT_QUIT = -1
        };

        Request(int type, bool manage_memory, int priority);
        virtual ~Request();

        void execute();
        int getType()                   const   { return m_type; }
        // ------------------------------------------------------------------------
        /** Returns if the memory for this object should be managed by
        *  by network_http (i.e. freed once the request is handled). */
        bool manageMemory()             const   { return m_manage_memory; }
        // ------------------------------------------------------------------------
        /** Returns the priority of this request. */
        int getPriority()               const   { return m_priority; }
        // ------------------------------------------------------------------------
        /** Signals that this request should be canceled. */
        void cancel()                           { m_cancel.setAtomic(true); }
        // ------------------------------------------------------------------------
        /** Returns if this request is to be canceled. */
        bool isCancelled()              const   { return m_cancel.getAtomic(); }
        // ------------------------------------------------------------------------
        /** Returns if this request is done. */
        bool isDone()                   const   { return m_done.getAtomic(); }
        // ------------------------------------------------------------------------
        /** Virtual method to check if a request has initialized all needed members to a valid value. */
        virtual bool isAllowedToAdd()   const   { return true; }

        /** This class is used by the priority queue to sort requests by priority.
         */
        class Compare
        {
        public:
            /** Compares two requests, returns if the first request has a lower
             *  priority than the second one. */
            bool operator() (const Request *a, const Request *b) const
            {
                return a->getPriority() < b->getPriority();
            }
        };   // Compare
    };   // Request


    // ========================================================================


    class HTTPRequest : public Request
    {

    protected :

        typedef std::map<std::string, std::string>      Parameters;

        /** The progress indicator. 0 untill it is started and the first
        *  packet is downloaded. At the end either -1 (error) or 1
        *  (everything ok) at the end. */
        Synchronised<float>                             m_progress;
        Synchronised<std::string>                       m_url;
        Synchronised<Parameters *>                      m_parameters;
        CURL *                                          m_curl_session;
        CURLcode                                        m_curl_code;

        virtual void                                    prepareOperation() OVERRIDE;
        virtual void                                    operation() OVERRIDE;
        virtual void                                    afterOperation() OVERRIDE;
        /** Executed when a request has finished. */
        virtual void                                    callback() {}

        static int                                      progressDownload(   void *clientp,
                                                                            double dltotal,
                                                                            double dlnow,
                                                                            double ultotal,
                                                                            double ulnow);

        static size_t                                   WriteCallback(      void *contents,
                                                                            size_t size,
                                                                            size_t nmemb,
                                                                            void *userp);


    public :
        HTTPRequest(int type = 0, bool manage_memory = false, int priority = 1);
        virtual ~HTTPRequest();

        void setParameter(const std::string & name, const std::string &value){
            MutexLocker(m_parameters);
            (*m_parameters.getData())[name] = value;
        };
        void setParameter(const std::string & name, const irr::core::stringw &value){
            MutexLocker(m_parameters);
            (*m_parameters.getData())[name] = irr::core::stringc(value.c_str()).c_str();
        }
        template <typename T>
        void setParameter(const std::string & name, const T& value){
            MutexLocker(m_parameters);
            (*m_parameters.getData())[name] = StringUtils::toString(value);
        }

        /** Returns the current progress. */
        float getProgress() const { return m_progress.getAtomic(); }
        /** Sets the current progress. */
        void setProgress(float f) { m_progress.setAtomic(f); }

        const std::string getURL()     {
                                            m_url.lock();
                                            const std::string url = m_url.getData();
                                            m_url.unlock();
                                            return url;
                                        }

        void setURL(const std::string & url) { m_url.setAtomic(url);}

        virtual bool isAllowedToAdd() OVERRIDE;

    };

    class XMLRequest : public HTTPRequest
    {
    private:
        Synchronised<XMLNode *>                         m_result;
        std::string                                     m_string_buffer;

    protected :
        Synchronised<irr::core::stringw>                m_info;
        Synchronised<bool>                              m_success;

        virtual void                                    prepareOperation() OVERRIDE;
        virtual void                                    operation() OVERRIDE;
        virtual void                                    afterOperation() OVERRIDE;

    public :
        XMLRequest(int type = 0, bool manage_memory = false, int priority = 1);
        virtual ~XMLRequest();

        const XMLNode *                                 getResult() const;
        const irr::core::stringw &                      getInfo()   const;
        bool                                            isSuccess() const       { return m_success.getAtomic(); }

    };
} //namespace Online

#endif

