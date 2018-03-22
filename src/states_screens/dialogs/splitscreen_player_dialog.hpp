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


#ifndef HEADER_SPLITSCREEN_PLAYER_DIALOG_HPP
#define HEADER_SPLITSCREEN_PLAYER_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <vector>

class InputDevice;
class PlayerProfile;

namespace GUIEngine
{
    class CheckBoxWidget;
    class IconButtonWidget;
    class LabelWidget;
    class RibbonWidget;
    class SpinnerWidget;
}

/**
 * \brief Dialog that handle user in network lobby
 * \ingroup states_screens
 */
class SplitscreenPlayerDialog : public GUIEngine::ModalDialog
{
private:
    InputDevice* m_device;

    bool m_self_destroy;

    std::vector<PlayerProfile*> m_available_players;

    GUIEngine::LabelWidget* m_message;

    GUIEngine::SpinnerWidget* m_profiles;

    GUIEngine::CheckBoxWidget* m_handicap;

    GUIEngine::RibbonWidget* m_options_widget;

    GUIEngine::IconButtonWidget* m_add;

    GUIEngine::IconButtonWidget* m_connect;

    GUIEngine::IconButtonWidget* m_cancel;

    GUIEngine::IconButtonWidget* m_reset;

public:
    SplitscreenPlayerDialog(InputDevice* device)
        : ModalDialog(0.8f,0.8f), m_device(device), m_self_destroy(false)
    {
        loadFromFile("online/splitscreen_player_dialog.stkgui");
    }
    // ------------------------------------------------------------------------
    ~SplitscreenPlayerDialog() {}
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed()
    {
        m_self_destroy = true;
        return false;
    }
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt)
    {
        // It's unsafe to delete from inside the event handler so we do it here
        if (m_self_destroy)
        {
            ModalDialog::dismiss();
            return;
        }
    }
};

#endif
