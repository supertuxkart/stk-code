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


#ifndef __HEADER_STORY_MODE_LOBBY_HPP__
#define __HEADER_STORY_MODE_LOBBY_HPP__

#include <string>

#include "guiengine/screen.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"

namespace GUIEngine 
{
    class CheckBoxWidget;
    class TextBoxWidget;
    class Widget; 
}


/**
  * \brief Audio options screen
  * \ingroup states_screens
  */
class StoryModeLobbyScreen : public GUIEngine::Screen, 
                             public EnterPlayerNameDialog::INewPlayerListener,
                             public GUIEngine::ScreenSingleton<StoryModeLobbyScreen>
{
    StoryModeLobbyScreen();

private:
    /** Online check box. */
    GUIEngine::CheckBoxWidget *m_online_cb;

    /** User name entry field. */
    GUIEngine::TextBoxWidget *m_username_tb;

    /** Password widget. */
    GUIEngine::TextBoxWidget *m_password_tb;

    /** The dynamic ribbon containing all players. */
    GUIEngine::DynamicRibbonWidget* m_players;

    void selectUser(int index);
    void makeEntryFieldsVisible(bool online);
    virtual void onDialogClose();
public:
    friend class GUIEngine::ScreenSingleton<StoryModeLobbyScreen>;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown();

    /** \brief implement optional callback from parent class GUIEngine::Screen */
    virtual void unloaded();

    /** \brief implement callback from EnterPlayerNameDialog::INewPlayerListener */
    virtual void onNewPlayerWithName(const irr::core::stringw& newName);
};

#endif
