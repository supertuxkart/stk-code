//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Marc Coll
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

#ifndef HEADER_GRAND_PRIX_EDITOR_SCREEN_HPP
#define HEADER_GRAND_PRIX_EDITOR_SCREEN_HPP

#include "dialogs/enter_gp_name_dialog.hpp"
#include "guiengine/screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"


namespace GUIEngine { class Widget; }

class GrandPrix;

/**
  * \brief screen where the user can edit his own grand prix
  * \ingroup states_screens
  */
class GrandPrixEditorScreen :
    public GUIEngine::Screen,
    public GUIEngine::ScreenSingleton<GrandPrixEditorScreen>,
    public EnterGPNameDialog::INewGPListener,
    public MessageDialog::IConfirmDialogListener
{
    friend class GUIEngine::ScreenSingleton<GrandPrixEditorScreen>;

         GrandPrixEditorScreen();

    void setSelection(const GrandPrix* gpdata);
    void loadGPList();
    void loadTrackList(const std::string& gpname);
    void showEditScreen(GrandPrix* gp);

    void onNewGPWithName(const irr::core::stringw& newName);
    void onConfirm();

    GrandPrix*   m_selection;
    std::string      m_action;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
};

#endif
