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
#include "online/profile_manager.hpp"

namespace GUIEngine { class Widget; }


/** Online profile base screen. Used for displaying friends, achievements,
 *  and settings. It handles the tabs which are common to each
 *  of those screens, and keeps track of the profile to display.
 * \ingroup states_screens
 */
class OnlineProfileBase : public GUIEngine::Screen
{
protected:
    OnlineProfileBase(const char* filename);

    /** Pointer to the various widgets on the screen. */
    GUIEngine::LabelWidget * m_header;
    GUIEngine::RibbonWidget* m_profile_tabs;
    GUIEngine::IconButtonWidget * m_friends_tab;
    GUIEngine::IconButtonWidget * m_achievements_tab;
    GUIEngine::IconButtonWidget * m_settings_tab;

    /** The profile that should be shown. */
    Online::OnlineProfile *m_visiting_profile;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    virtual void beforeAddingWidget() OVERRIDE;
};

#endif
