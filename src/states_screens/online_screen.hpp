//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_ONLINE_SCREEN_HPP
#define HEADER_ONLINE_SCREEN_HPP

#include "config/player_manager.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine { class Widget; class ListWidget; }

/**
  * \brief Handles the main menu
  * \ingroup states_screens
  */
class OnlineScreen :    public GUIEngine::Screen,
                        public GUIEngine::ScreenSingleton<OnlineScreen>
{
private:
    friend class GUIEngine::ScreenSingleton<OnlineScreen>;

    OnlineScreen();
    ~OnlineScreen();

    GUIEngine::IconButtonWidget * m_back_widget;

    GUIEngine::RibbonWidget * m_top_menu_widget;
    GUIEngine::IconButtonWidget * m_quick_play_widget;
    GUIEngine::IconButtonWidget * m_find_server_widget;
    GUIEngine::IconButtonWidget * m_create_server_widget;

    GUIEngine::LabelWidget * m_online_status_widget;

    GUIEngine::RibbonWidget * m_bottom_menu_widget;
    GUIEngine::IconButtonWidget * m_register_widget;
    GUIEngine::IconButtonWidget * m_profile_widget;
    GUIEngine::IconButtonWidget * m_sign_out_widget;

    PlayerProfile::OnlineState m_recorded_state;

    bool hasStateChanged();
    void setInitialFocus();

    void doQuickPlay();
public:

    virtual void onUpdate(float delta) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

    /** \brief Implements the callback when a dialog gets closed. */
    virtual void onDialogClose() OVERRIDE;
};

#endif
