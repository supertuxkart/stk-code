//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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


#ifndef HEADER_ICON_SELECTION_HPP
#define HEADER_ICON_SELECTION_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"
#include "states_screens/options/user_screen.hpp"

class PlayerProfile;
namespace GUIEngine
{
    class RibbonWidget;
    class DynamicRibbonWidget;
}

/**
 * \brief Modal dialog allowing players to pick an icon for their profile
 *        from available native karts.
 * \ingroup states_screens
 */
class IconSelectionDialog : public GUIEngine::ModalDialog
{
private:
    /** The player profile being customized. */
    PlayerProfile* m_player_profile;

    /** Parent screen to notify when icons are updated. */
    BaseUserScreen* m_parent_screen;

    /** The scrollable grid containing all kart icons. */
    GUIEngine::DynamicRibbonWidget* m_kart_icons_widget;

    /** The action buttons bar (Cancel / Apply). */
    GUIEngine::RibbonWidget* m_buttons_widget;

    /** Codename of the currently selected kart in the grid. */
    std::string m_selected_kart;

public:
    IconSelectionDialog(PlayerProfile* pp, BaseUserScreen* user_screen);

    ~IconSelectionDialog();

    virtual void beforeAddingWidgets() OVERRIDE;

    virtual void init() OVERRIDE;

    GUIEngine::EventPropagation processEvent(const std::string& eventSource) OVERRIDE;
};


#endif
