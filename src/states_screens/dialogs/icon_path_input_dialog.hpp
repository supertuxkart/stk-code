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


#ifndef HEADER_ICON_PATH_INPUT_HPP
#define HEADER_ICON_PATH_INPUT_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"
#include "states_screens/options/user_screen.hpp"

class PlayerProfile;
namespace GUIEngine
{
    class RibbonWidget;
    class IconButtonWidget;
}


/**
 * \brief Modal dialog allowing players to enter a custom image file path
 *        to be used as their profile icon.
 * \ingroup states_screens
 */
class IconPathInputDialog : public GUIEngine::ModalDialog
{
private:

    /** The player profile being customized. */
    PlayerProfile* m_player_profile;

    /** Parent screen to notify when icons are updated. */
    BaseUserScreen* m_parent_screen;

    /** The action buttons bar (Cancel / Apply). */
    GUIEngine::RibbonWidget* m_buttons_widget;

    /** Button bar containing the "Validate" action button. */
    GUIEngine::RibbonWidget* m_path_input_buttons_widget;

    /** Reference to the Apply button (kept disabled until path is valid). */
    GUIEngine::Widget* m_apply_button;
    
    /** Text box where the player inputs the image file path. */
    GUIEngine::TextBoxWidget *m_icon_path_textbox_widget;
    
    /** Text box used to display status/error messages to the user. */
    GUIEngine::TextBoxWidget *m_icon_path_output_widget;

    /** Icon button used as a preview display for the validated image. */
    GUIEngine::IconButtonWidget* m_icon_render;

    /** Stored valid icon file path. Empty if no valid path was validated yet. */
    std::string m_icon_path;

public:
    IconPathInputDialog(PlayerProfile* pp, BaseUserScreen* user_screen);

    ~IconPathInputDialog();

    virtual void beforeAddingWidgets() OVERRIDE;

    virtual void init() OVERRIDE;

    /** Resets stored icon path and disables apply button and preview icon. */
    void resetState();

    /** Validates the user input path (existence, extension, format and aspect ratio).
     *  Displays error feedback in the dialog if validation fails.
     *  \return The sanitized file path if valid, or an empty string on failure.
     */
    std::string validatePath();

    GUIEngine::EventPropagation processEvent(const std::string& eventSource) OVERRIDE;
};


#endif
