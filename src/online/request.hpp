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

#ifndef HEADER_ONLINE_REQUEST_HPP
#define HEADER_ONLINE_REQUEST_HPP

#include "io/file_manager.hpp"
#include "utils/cpp2011.h"
#include "utils/string_utils.hpp"
#include "utils/synchronised.hpp"

#ifdef WIN32
#  include <winsock2.h>
#endif
#include <curl/curl.h>
#include <assert.h>
#include <string>

namespace Online{

    /** Stores a request for the HTTP Manager. They will be sorted by 
     *  prioritiy.  Requests have four different states they can be in, and
     *  this state determines which thread can access it. This allows
     *  the use of threading without adding more synchronisation overhead
     *  and code to the main thread. The states are:
     *  - Preparing\n The request is created, and parameter are set. 
     *        Only the main thread can access this object.
     *  - Busy\n The request is put into the http_manager queue. It remains
     *        in this states till its operation is finished. No more changes
     *        to this object by the main thread are allowed, only the manager
     *        thread can change it now.
     *  - Executed\n The request was executed (its operation called and 
     *        finished), but callbacks still need to be done by the manager
     *        thread (main reason for this state is to have asserts in
     *        function accessing data).
     *  - Done\n All callbacks are done (they will be executed by the main 
     *        thread), the request was moved from the manager's request queue
     *        to its finished queue, executed its callbacks and was removed
     *        from the queue. The manager thread will not access this object
     *        anymore, and the main thread is now able to access the request
     *        object again.
     *        
     * \ingroup online
     */
    class Request
    {
    private:
        /** Type of the request. Has 0 as default value. */
        const int              m_type;
        /** True if the memory for this Request should be managed by
        *  http connector (i.e. this object is freed once the request
        *  is handled). Otherwise the memory is not freed, so it must
        *  be freed by the calling function. */
        const bool             m_manage_memory;
        /** The priority of this request. The higher the value the more
        important this request is. */
        const int              m_priority;

        /** The different state of the requst:
         *  - S_PREPARING:\n The request is created and can be configured, it
         *      is not yet started.
         *  - S_BUSY:\n The request is added to the execution queue of the 
         *      http_manager (and potentially executing). This implies that
         *      now only the http_manager thread should access the requests's
         *      data structures.
         *  - S_EXECUTED:\n The request was executed, but was not yet marked
         *       as finished in the http_manager. This importantly indicates
         *       that the main thread should not yet access this request,
         *       since the http thread is still executing it.
         *  - S_DONE:\n The request is finished, and it is marked as
         *      finished in the http_manager. This implies that the main
         *      stk thread can access its data safely now.
         */
        enum State
        {
            S_PREPARING,
            S_BUSY,
            S_EXECUTED,
            S_DONE
        };

    protected:

        /** Cancel this request if it is active. */
        Synchronised<bool>              m_cancel;
        /** Set to though if the reply of the request is in and callbacks are
         *  executed */
        Synchronised<State>             m_state;

        // --------------------------------------------------------------------
        /** The actual operation to be executed. Empty as default, which 
         *  allows to create a 'quit' request without any additional code. */
        virtual void operation() {}
        // --------------------------------------------------------------------
        /** Virtual function to be called before an operation. */
        virtual void prepareOperation() {}
        // --------------------------------------------------------------------
        /** Virtual function to be called after an operation. */
        virtual void afterOperation()   {}

    public:
        enum RequestType
        {
            RT_QUIT = 1
        };

                 Request(bool manage_memory, int priority, int type);
        virtual ~Request() {}
        void     execute();
        void     executeNow();
        // --------------------------------------------------------------------
        /** Executed when a request has finished. */
        virtual void callback() {}
        // --------------------------------------------------------------------
        /** Returns the type of the request. */
        int getType() const  { return m_type; }
        // --------------------------------------------------------------------
        /** Returns if the memory for this object should be managed by
        *  by network_http (i.e. freed once the request is handled). */
        bool manageMemory() const   { return m_manage_memory; }
        // --------------------------------------------------------------------
        /** Returns the priority of this request. */
        int getPriority() const   { return m_priority; }
        // --------------------------------------------------------------------
        /** Signals that this request should be canceled. */
        void cancel() { m_cancel.setAtomic(true); }
        // --------------------------------------------------------------------
        /** Returns if this request is to be canceled. */
        bool isCancelled() const   { return m_cancel.getAtomic(); }
        // --------------------------------------------------------------------
        /** Sets the request state to busy. */
        void setBusy()
        { 
            assert(m_state.getAtomic()==S_PREPARING);
            m_state.setAtomic(S_BUSY); 
        }   // setBusy
        // --------------------------------------------------------------------
        /** Sets the request to be completed. */
        void setExecuted() 
        {
            assert(m_state.getAtomic()==S_BUSY);
            m_state.setAtomic(S_EXECUTED); 
        }   // setExecuted
        // --------------------------------------------------------------------
        /** Should only be called by the manager */
        void setDone()
        {
            assert(m_state.getAtomic()==S_EXECUTED);
            m_state.setAtomic(S_DONE);
        }   // setDone
        // --------------------------------------------------------------------
        /** Returns if this request is done. */
        bool isDone() const { return m_state.getAtomic() == S_DONE; }
        // --------------------------------------------------------------------
        /** Returns if this request is being prepared. */
        bool isPreparing() const { return m_state.getAtomic() == S_PREPARING; }
        // --------------------------------------------------------------------
        /** Returns if this request is busy. */
        bool isBusy() const   { return m_state.getAtomic() == S_BUSY; }
        // --------------------------------------------------------------------
        /** Checks if the request has completed or done (i.e. callbacks were
         *  executed). 
        */
        bool hasBeenExecuted() const 
        {
            State s = m_state.getAtomic();
            return s==S_EXECUTED || s==S_DONE;
        }   // hasBeenExecuted
        // --------------------------------------------------------------------
        /** Virtual method to check if a request has initialized all needed 
         *  members to a valid value. */
        virtual bool isAllowedToAdd()   const   { return isPreparing(); }

        // ====================================================================
        /** This class is used by the priority queue to sort requests by 
         *  priority.
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

    // ========================================================================
    /** A http request expecting a xml return value.
     */
    class XMLRequest : public HTTPRequest
    {
    private:
        /** On a successful download contains the converted XML tree. */
        XMLNode *m_xml_data;

        /** Additional info contained the downloaded data (or an error
         *  message if a problem occurred). */
        irr::core::stringw m_info;

        /** True if the request was successful executed on the server. */
        bool m_success;

    protected:
        virtual void afterOperation() OVERRIDE;

    public :
                 XMLRequest(bool manage_memory = false, int priority = 1);
        virtual ~XMLRequest();

        // ------------------------------------------------------------------------
        /** Get the downloaded XML tree.
         *  \pre request has to be executed.
         *  \return get the complete result from the request reply.
         */
        const XMLNode * getXMLData() const
        {
            assert(hasBeenExecuted());
            return m_xml_data;
        }   // getXMLData

        // ------------------------------------------------------------------------
        /** Returns the additional information (or error message) contained in 
         *  a finished request.
        * \pre request had to be executed.
        * \return get the info from the request reply
        */
        const irr::core::stringw & getInfo()   const
        {
            assert(hasBeenExecuted());
            return m_info;
        }   // getInfo

        // --------------------------------------------------------------------
        /** Returns whether the request was successfully executed on the server.
         * \pre request had to be executed.
         * \return whether or not the request was a success. */
        bool isSuccess() const 
        {
            assert(hasBeenExecuted());
            return m_success; 
        }   // isSuccess

    };   // class XMLRequest

} //namespace Online

#endif

