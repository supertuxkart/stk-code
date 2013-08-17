//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#ifndef __HEADER_ONLINE_PROFILE_BASE_HPP__
#define __HEADER_ONLINE_PROFILE_BASE_HPP__

#include <string>
#include <irrString.h>

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"
#include "online/profile.hpp"

namespace GUIEngine { class Widget; }


/**
  * \brief Online profiel overview screen
  * \ingroup states_screens
  */
class OnlineProfileBase : public GUIEngine::Screen
{
protected:
    OnlineProfileBase(const char* filename);
    GUIEngine::LabelWidget * m_header;
    GUIEngine::RibbonWidget* m_profile_tabs;
    GUIEngine::IconButtonWidget * m_overview_tab;
    GUIEngine::IconButtonWidget * m_friends_tab;

    Online::Profile *           m_visiting_profile;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
};

#endif
