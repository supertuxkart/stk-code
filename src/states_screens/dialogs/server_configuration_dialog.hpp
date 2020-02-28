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

#ifndef HEADER_SERVER_CONFIGURATION_DIALOG_HPP
#define HEADER_SERVER_CONFIGURATION_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "race/race_manager.hpp"

namespace GUIEngine
{
    class SpinnerWidget;
    class LabelWidget;
    class RibbonWidget;
    class IconButtonWidget;
}

class ServerConfigurationDialog : public GUIEngine::ModalDialog
{
private:
    int m_prev_mode;
    int m_prev_value;
    bool m_self_destroy;

    GUIEngine::SpinnerWidget* m_more_options_spinner;

    GUIEngine::LabelWidget* m_more_options_text;

    GUIEngine::RibbonWidget* m_difficulty_widget;
    GUIEngine::RibbonWidget* m_game_mode_widget;
    GUIEngine::RibbonWidget* m_options_widget;
    GUIEngine::IconButtonWidget* m_ok_widget;
    GUIEngine::IconButtonWidget* m_cancel_widget;

    void updateMoreOption(int game_mode);
public:
    ServerConfigurationDialog(bool soccer_goal) : ModalDialog(0.8f, 0.8f)
    {
        m_self_destroy = false;
        switch (RaceManager::get()->getMinorMode())
        {
            case RaceManager::MINOR_MODE_NORMAL_RACE:
            {
                m_prev_mode = 0;
                m_prev_value = 0;
                break;
            }
            case RaceManager::MINOR_MODE_TIME_TRIAL:
            {
                m_prev_mode = 1;
                m_prev_value = 0;
                break;
            }
            case RaceManager::MINOR_MODE_FREE_FOR_ALL:
            {
                m_prev_mode = 2;
                m_prev_value = 0;
                break;
            }
            case RaceManager::MINOR_MODE_CAPTURE_THE_FLAG:
            {
                m_prev_mode = 2;
                m_prev_value = 1;
                break;
            }
            case RaceManager::MINOR_MODE_SOCCER:
            {
                m_prev_mode = 3;
                m_prev_value = soccer_goal ? 1 : 0;
                break;
            }
            default:
            {
                m_prev_mode = 0;
                m_prev_value = 0;
                break;
            }
        }
        loadFromFile("online/server_configuration_dialog.stkgui");
    }
    // ------------------------------------------------------------------------
    void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    void init();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    bool onEscapePressed()
    {
        m_self_destroy = true;
        return false;
    }
    // ------------------------------------------------------------------------
    void onUpdate(float dt)
    {
        if (m_self_destroy)
            ModalDialog::dismiss();
    }
};   // class ServerConfigurationDialog

#endif
