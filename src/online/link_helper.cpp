//  SuperTuxKart - A fun racing game with go-karts
//  Copyright (C) 2017 QwertyChouskie
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

#include "online/link_helper.hpp"
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <ShellApi.h>
#endif

using namespace Online;

namespace Online
{
    void LinkHelper::OpenURL (std::string url) {
#ifdef WIN32
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else 
# ifdef APPLE
        std::string op = std::string("open ").append(url);
# else
        std::string op = std::string("xdg-open ").append(url);
# endif
        system(op.c_str());
#endif
    }
}