//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#ifndef __HEADER_OPTIONS_SCREEN_GENERAL_HPP__
#define __HEADER_OPTIONS_SCREEN_GENERAL_HPP__

#include <string>

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget; }

struct Input;

/**
  * \brief Graphics options screen
  * \ingroup states_screens
  */
class OptionsScreenGeneral : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<OptionsScreenGeneral>
{
    OptionsScreenGeneral();
    bool m_inited;

    std::vector<std::string> m_skins;

    void setInternetCheckboxes(bool activate);

public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenGeneral>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded() OVERRIDE;
};

#endif
