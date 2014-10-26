//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#ifndef HEADER_MAIN_MENU_SCREEN_HPP
#define HEADER_MAIN_MENU_SCREEN_HPP

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget;       class ListWidget; 
                      class ButtonWidget; class IconButtonWidget; }

/**
  * \brief Handles the main menu
  * \ingroup states_screens
  */
class MainMenuScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<MainMenuScreen>
{
private:
    friend class GUIEngine::ScreenSingleton<MainMenuScreen>;

    /** Keep the widget to avoid looking it up every frame. */
    GUIEngine::IconButtonWidget* m_online;

    /** Keep the widget to to the user name. */
    GUIEngine::ButtonWidget *m_user_id;

    MainMenuScreen();

public:
    /** Temporary disable the online menu while it is being worked at. */
    static bool m_enable_online;

    virtual void onUpdate(float delta) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onDisabledItemClicked(const std::string& item) OVERRIDE;
};

#endif
