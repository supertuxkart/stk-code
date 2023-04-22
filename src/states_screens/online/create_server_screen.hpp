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

#ifndef HEADER_CREATE_SERVER_SCREEN_HPP
#define HEADER_CREATE_SERVER_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"


namespace GUIEngine { class Widget; class ListWidget; }

/**
  * \brief Handles the main menu
  * \ingroup states_screens
  */
class CreateServerScreen :      public GUIEngine::Screen,
                                public GUIEngine::ScreenSingleton<CreateServerScreen>
{
private:
    int m_prev_mode, m_prev_value;

    bool m_supports_ai;

    friend class GUIEngine::ScreenSingleton<CreateServerScreen>;

    CreateServerScreen();

    GUIEngine::TextBoxWidget * m_name_widget;
    GUIEngine::SpinnerWidget * m_max_players_widget;
    GUIEngine::SpinnerWidget* m_more_options_spinner;

    GUIEngine::LabelWidget * m_more_options_text;
    GUIEngine::LabelWidget * m_info_widget;

    GUIEngine::RibbonWidget * m_game_mode_widget;
    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_create_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;
    GUIEngine::IconButtonWidget * m_back_widget;

    void createServer();
    void updateMoreOption(int game_mode);

public:

    virtual void onUpdate(float delta) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

};   // class CreateServerScreen

#endif
