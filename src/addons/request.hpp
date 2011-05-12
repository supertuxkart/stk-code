//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#ifndef HEADER_REQUEST_HPP
#define HEADER_REQUEST_HPP

#include <string>

#include "utils/synchronised.hpp"

class Addon;

/** Stores a download request. They will be sorted by priorities. */
class Request
{
public:
    /** List of 'http commands' for this object:
     *  HC_SLEEP: No command, sleep
     *  HC_INIT: Object is being initialised
     *  HC_DOWNLOAD_FILE : download a file
     *  HC_QUIT:  Stop loop and terminate thread.
     *  HC_NEWS:  Update the news
     */
    enum HttpCommands {HC_SLEEP,
                       HC_QUIT,
                       HC_INIT,
                       HC_DOWNLOAD_FILE,
                       HC_NEWS    };

private:
    /** The progress indicator. 0 till it is started and the first
    *  packet is downloaded. At the end eithe -1 (error) or 1
    *  (everything ok) at the end. */
    Synchronised<float> m_progress;
    /** The URL to download. */
    std::string         m_url;
    /** Where to store the file (including file name). */
    std::string         m_full_path;
    /** The priority of this request. The higher the value the more
    important this request is. */
    int                 m_priority;
    /** The actual command to use. */
    HttpCommands        m_command;

    /** True if the memory for this Request should be managed by
    *  network_http (i.e. this object is freed once the request
    *  is handled). Otherwise the memory is not freed, so it must
    *  be freed by the calling function. */
    bool                m_manage_memory;

    /** If this is a download for an icon addon, this contains a pointer
     *  to the addon so that we can notify the addon that the icon is
     *  ready. */
    Addon              *m_icon_addon;
public:
          Request(HttpCommands command, int priority, 
                  bool manage_memory=true);
          Request(HttpCommands command, int priority, bool manage_memory,
                  const std::string &url, const std::string &save);
    void  setAddonIconNotification(Addon *a);
    void  notifyAddon();
    // --------------------------------------------------------------------
    /** Returns the URL to download from. */
    const std::string &getURL() const {return m_url;}
    // --------------------------------------------------------------------
    /** Returns the full save file name. */
    const std::string &getSavePath() const {return m_full_path;}
    // --------------------------------------------------------------------
    /** Returns the command to do for this request. */
    HttpCommands       getCommand() const { return m_command; }
    // --------------------------------------------------------------------
    /** Returns the priority of this request. */
    int                getPriority() const { return m_priority; }
    // --------------------------------------------------------------------
    /** Returns the current progress. */
    float getProgress() const { return m_progress.get(); }
    // --------------------------------------------------------------------
    /** Sets the current progress. */
    void setProgress(float f) { m_progress.set(f); }
    // --------------------------------------------------------------------
    /** Used in sorting requests by priority. */
    bool operator<(const Request &r) { return r.m_priority < m_priority;}
    // --------------------------------------------------------------------
    /** Returns if the memory for this object should be managed by
    *  by network_http (i.e. freed once the request is handled). */
    bool manageMemory() const { return m_manage_memory; }
    // --------------------------------------------------------------------
};   // Request

#endif

