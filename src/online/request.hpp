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

#ifndef HEADER_ONLINE_REQUEST_HPP
#define HEADER_ONLINE_REQUEST_HPP

#include <string>

#include "utils/cpp2011.h"
#include "utils/string_utils.hpp"
#include "utils/synchronised.hpp"
#include "io/file_manager.hpp"

namespace Online{

    /**
      * Stores a request for the http connector. They will be sorted by priorities.
      * \ingroup online
      */
    class Request
    {
    protected:
        /** The priority of this request. The higher the value the more
        important this request is. */
        int                 m_priority;
        /** Cancel this request if it is active. */
        bool                m_cancel;


        /** True if the memory for this Request should be managed by
        *  http connector (i.e. this object is freed once the request
        *  is handled). Otherwise the memory is not freed, so it must
        *  be freed by the calling function. */
        bool                m_manage_memory;

        Synchronised<bool>              m_done;

        virtual void beforeOperation() {}
        virtual void operation() = 0;
        virtual void afterOperation();

    public:

        Request(int priority, bool manage_memory=true);
        virtual ~Request();

        void execute();

        // ------------------------------------------------------------------------
        /** Returns the priority of this request. */
        int getPriority() const { return m_priority; }
        // ------------------------------------------------------------------------
        /** Signals that this request should be canceled. */
        void cancel() { m_cancel = true; }
        // ------------------------------------------------------------------------
        /** Returns if this request is to be canceled. */
        bool isCancelled() const { return m_cancel; }
        // ------------------------------------------------------------------------
        /** Specifies if the memory should be managed by network_http. */
        void setManageMemory(bool m) { m_manage_memory = m; }
        // ------------------------------------------------------------------------
        /** Returns if the memory for this object should be managed by
        *  by network_http (i.e. freed once the request is handled). */
        bool manageMemory() const { return m_manage_memory; }

        bool isDone(){return m_done.getAtomic();}

        virtual bool isAllowedToAdd() {return true;}

        /** This class is used by the priority queue to sort requests by priority.
         */
        class Compare
        {
        public:
            /** Compares two requests, returns if the first request has a lower
             *  priority than the second one. */
            bool operator() (const Request *a, const Request *b) const
            { return a->getPriority() < b->getPriority(); }
        };   // Compare
    };   // Request


    // ========================================================================


    class QuitRequest : public Request
    {
    public :
        QuitRequest();
        virtual void operation() OVERRIDE {}
    };


    // ========================================================================


    class HTTPRequest : public Request
    {

    protected :

        typedef std::map<std::string, std::string> Parameters;
        Parameters * m_parameters;

        /** The progress indicator. 0 till it is started and the first
        *  packet is downloaded. At the end eithe -1 (error) or 1
        *  (everything ok) at the end. */
        Synchronised<float>             m_progress;
        std::string                     m_url;
        bool                            m_added;

        virtual void afterOperation();
        std::string downloadPage();

        static int progressDownload(void *clientp,
                                    double dltotal,
                                    double dlnow,
                                    double ultotal,
                                    double ulnow);

        static size_t WriteCallback(void *contents,
                                    size_t size,
                                    size_t nmemb,
                                    void *userp);
        virtual void callback() {}

    public :

        HTTPRequest(const std::string & url = "");
        virtual ~HTTPRequest();

        void setParameter(const std::string & name, const std::string &value){
            if(!m_added)
                (*m_parameters)[name] = value;
        };
        void setParameter(const std::string & name, const irr::core::stringw &value){
            if(!m_added)
                (*m_parameters)[name] = irr::core::stringc(value.c_str()).c_str();
        }
        template <typename T>
        void setParameter(const std::string & name, const T& value){
            if(!m_added)
                (*m_parameters)[name] = StringUtils::toString(value);
        }

        /** Returns the current progress. */
        float getProgress() const { return m_progress.getAtomic(); }
        /** Sets the current progress. */
        void setProgress(float f) { m_progress.setAtomic(f); }

        const std::string &getURL() const {return m_url;}
        void setURL(const std::string & url) { m_url = url;}

        virtual bool isAllowedToAdd() OVERRIDE;

    };

    class XMLRequest : public HTTPRequest
    {
    protected :

        XMLNode *                       m_result;
        irr::core::stringw              m_info;
        bool                            m_success;

        virtual void                    operation() OVERRIDE;
        virtual void                    afterOperation() OVERRIDE;

    public :
        XMLRequest(const std::string & url = "");

        virtual XMLNode *               getResult() const       { return m_result; }
        const irr::core::stringw &      getInfo()   const       { return m_info; }
        bool                            isSuccess() const       { return m_success; }

    };
} //namespace Online

#endif

