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

#include "addons/request.hpp"

#include <assert.h>

#include "addons/addon.hpp"

Request::Request(HttpCommands command, int priority, bool manage_memory)
       : m_progress(0)
{
    m_command       = command; 
    m_priority      = priority;
    m_url           = ""; 
    m_full_path     = "";
    m_manage_memory = manage_memory;
    m_icon_addon    = NULL;
    m_cancel        = false;
    m_progress.setAtomic(0);
}   // Request

// ----------------------------------------------------------------------------
Request::Request(HttpCommands command, int priority, bool manage_memory,
                 const std::string &url, const std::string &save)
       : m_progress(0)
{
    m_command       = command;
    m_priority      = priority;
    m_url           = url;
    m_full_path     = save;
    m_icon_addon    = NULL;
    m_manage_memory = manage_memory;
    m_cancel        = false;
    m_progress.setAtomic(0);
}   // Request

// ----------------------------------------------------------------------------
void  Request::setAddonIconNotification(Addon *a)
{
    m_icon_addon = a;
}   // setADdonIconNotification

// ----------------------------------------------------------------------------
void  Request::notifyAddon()
{
    if(m_icon_addon)
        m_icon_addon->setIconReady();
}   // notifyAddon