//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_LOADER_H
#define HEADER_LOADER_H

#include <plib/ssg.h>
#include <string>
#include <vector>
#include <set>
#include "callback_manager.hpp"

#ifdef _MSC_VER
const char DIR_SEPARATOR='\\';
#else
const char DIR_SEPARATOR='/';
#endif

class Loader : public ssgLoaderOptions
{
public:
    Loader();
    ~Loader();

    virtual void makeModelPath(char* path, const char* fname) const;
    virtual void makeTexturePath(char* path, const char* fname) const;

    std::string getPath(const char* name) const;
    std::string getPath(const std::string name) const {return getPath(name.c_str());}
    void listFiles(std::set<std::string>& result, const std::string& dir)
        const;

    void       addSearchPath(const std::string& path);
    void       addSearchPath(const char* path);
    void       initConfigDir();
    ssgEntity *load(const std::string& filename, CallbackType t, bool optimise=true);
    void       setCallbackType(CallbackType t)   {m_current_callback_type=t;}
private:
    std::vector<std::string> m_search_path;
    CallbackType             m_current_callback_type;

    void         makePath(char* path, const char* dir, const char* fname) const;
    ssgBranch   *createBranch(char *data) const;
    void preProcessObj ( ssgEntity *n, bool mirror );
    ssgBranch   *animInit    (char *data) const;
};

extern Loader* loader;

#endif

