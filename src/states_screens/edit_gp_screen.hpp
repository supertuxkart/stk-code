//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Marc Coll
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

#ifndef HEADER_EDIT_GP_SCREEN_HPP
#define HEADER_EDIT_GP_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "states_screens/dialogs/message_dialog.hpp"

#include <vector>


namespace GUIEngine { class Widget; }
namespace irr { namespace gui { class STKModifiedSpriteBank; } }

class GrandPrixData;

/**
  * \brief screen where the user can edit a grand prix
  * \ingroup states_screens
  */
class EditGPScreen :
    public GUIEngine::Screen,
    public GUIEngine::ScreenSingleton<EditGPScreen>,
    public MessageDialog::IConfirmDialogListener
{
    friend class GUIEngine::ScreenSingleton<EditGPScreen>;

         EditGPScreen();

    void onConfirm() OVERRIDE;
    void onCancel() OVERRIDE;

    void loadList(const int selected);
    void setModified(const bool modified);
    void setSelected(const int selected);
    void edit();
    bool save();
    void back();

    bool canMoveUp() const;
    bool canMoveDown() const;

    void enableButtons();

    GrandPrixData*                     m_gp;
    GUIEngine::ListWidget*             m_list;
    irr::gui::STKModifiedSpriteBank*   m_icon_bank;
    std::vector<int>                   m_icons;
    int                                m_selected;
    bool                               m_modified;

    std::string                        m_action;

public:

                ~EditGPScreen();

    void         setSelectedGP(GrandPrixData* gp);

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
};

#endif
