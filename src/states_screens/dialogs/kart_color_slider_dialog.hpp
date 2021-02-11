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


#ifndef HEADER_KART_COLOR_SLIDER_HPP
#define HEADER_KART_COLOR_SLIDER_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"

class PlayerProfile;
namespace GUIEngine
{
    class CheckBoxWidget;
    class ModelViewWidget;
    class RibbonWidget;
    class SpinnerWidget;
}

/**
 * \ingroup states_screens
 */
class KartColorSliderDialog : public GUIEngine::ModalDialog
{
private:
    PlayerProfile* m_player_profile;

    GUIEngine::SpinnerWidget* m_toggle_slider;

    GUIEngine::ModelViewWidget* m_model_view;

    GUIEngine::SpinnerWidget* m_color_slider;

    GUIEngine::RibbonWidget* m_buttons_widget;

    void toggleSlider();
public:
    KartColorSliderDialog(PlayerProfile* pp);

    ~KartColorSliderDialog();

    virtual void beforeAddingWidgets() OVERRIDE;

    GUIEngine::EventPropagation processEvent(const std::string& eventSource) OVERRIDE;
};


#endif
