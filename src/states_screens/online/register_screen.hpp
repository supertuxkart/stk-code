//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifndef HEADER_REGISTER_SCREEN_HPP
#define HEADER_REGISTER_SCREEN_HPP

#include "guiengine/screen.hpp"

namespace GUIEngine { class Widget;       class LabelWidget;
                      class RibbonWidget; class TextBoxWidget; }
namespace Online    { class XMLRequest;                }

class PlayerProfile;
class BaseUserScreen;

/**
  * \brief Screen to register an online account.
  * \ingroup states_screens
  */
class RegisterScreen : public GUIEngine::Screen,
                       public GUIEngine::ScreenSingleton<RegisterScreen>
{
private:
    friend class GUIEngine::ScreenSingleton<RegisterScreen>;

    void makeEntryFieldsVisible();
    void handleLocalName(const irr::core::stringw &local_name);
    void doRegister();
    void init() OVERRIDE;
    RegisterScreen();

    /** Save the pointer to the info widget, it is widely used. */
    GUIEngine::LabelWidget *m_info_widget;

    /** Save the pointer to the options widget, it is widely used. */
    GUIEngine::RibbonWidget *m_options_widget;

    /** Save the pointer to the options widget, it is widely used. */
    GUIEngine::TextBoxWidget *m_password_widget;

    /** The XML request to the server. */
    Online::XMLRequest *m_signup_request;

    /** Pointer to an existing player if the screen is doing a rename,
     *  NULL otherwise. */
    PlayerProfile *m_existing_player;

    /** True if the info message (email was sent...) is shown. */
    bool m_info_message_shown;

    /** Which kind of account to create: new online account, new account
     *  using an existing online account, offline account. */
    enum { ACCOUNT_NEW_ONLINE, 
           ACCOUNT_EXISTING_ONLINE, 
           ACCOUNT_OFFLINE } m_account_mode;

    /** A pointer to the parent UserScreen, in order to allow this screen
     *  to pass information back. */
    BaseUserScreen *m_parent_screen;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE {};
    virtual void onUpdate(float dt) OVERRIDE;
    virtual bool onEscapePressed() OVERRIDE;
    virtual void onDialogClose() OVERRIDE;
    virtual void onFocusChanged(GUIEngine::Widget *previous, 
                                GUIEngine::Widget *focus,
                                int playerID) OVERRIDE;
    void setRename(PlayerProfile *player);

    void acceptTerms();
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    // ------------------------------------------------------------------------
    /** Set the parent screen. */
    void setParent(BaseUserScreen *us) { m_parent_screen = us; }
};   // class RegisterScreen

#endif
