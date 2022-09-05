//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "states_screens/dialogs/kart_color_slider_dialog.hpp"

#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "graphics/irr_driver.hpp"
#include <ge_render_info.hpp>
#include "guiengine/engine.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
KartColorSliderDialog::KartColorSliderDialog(PlayerProfile* pp)
                     : ModalDialog(0.75f, 0.75f, MODAL_DIALOG_LOCATION_CENTER)
{
    loadFromFile("kart_color_slider.stkgui");
    m_player_profile = pp;

    // I18N: In kart color choosing dialog
    core::stringw original_color = _("Use original color");
    // I18N: In kart color choosing dialog
    core::stringw choose_color = _("Pick a color from slider");

    m_toggle_slider->clearLabels();
    m_toggle_slider->addLabel(original_color);
    m_toggle_slider->addLabel(choose_color);

    m_buttons_widget = getWidget<RibbonWidget>("buttons");

    if (m_player_profile->getDefaultKartColor() != 0.0f)
    {
        m_toggle_slider->setValue(1);
        m_color_slider->setActive(true);
        m_color_slider->setValue(int(
            m_player_profile->getDefaultKartColor() * 100.0f));
        m_model_view->getModelViewRenderInfo()->setHue(
            float(m_color_slider->getValue()) / 100.0f);
    }
    else
    {
        m_toggle_slider->setValue(0);
        m_color_slider->setActive(false);
        m_model_view->getModelViewRenderInfo()->setHue(0.0f);
    }
    m_toggle_slider->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_color_slider->setValueUpdatedCallback([this](SpinnerWidget* spinner)
    {
        m_model_view->getModelViewRenderInfo()->setHue(
            float(spinner->getValue()) / 100.0f);
    });
}   // KartColorSliderDialog

// ----------------------------------------------------------------------------
KartColorSliderDialog::~KartColorSliderDialog()
{
    PlayerManager::get()->save();
}   // ~KartColorSliderDialog

// ----------------------------------------------------------------------------
void KartColorSliderDialog::beforeAddingWidgets()
{
    m_model_view = getWidget<ModelViewWidget>("model");

    const core::dimension2du screen_size = irr_driver->getActualScreenSize();
    bool need_hd_rtt = (screen_size.Width > 1280 || screen_size.Height > 1280);
    m_model_view->setRTTSize(need_hd_rtt ? 1024 : 512);

    const KartProperties* props =
        kart_properties_manager->getKart(UserConfigParams::m_default_kart);
    if (props == NULL)
    {
        Log::warn("KartColorSliderDialog", "Unknown kart %s, fallback to tux",
            UserConfigParams::m_default_kart.c_str());
        props = kart_properties_manager->getKart(std::string("tux"));
    }
    const KartModel& kart_model = props->getMasterKartModel();

    core::matrix4 model_location;

    float scale = 35.0f;
    if (kart_model.getLength() > 1.45f)
    {
        // if kart is too long, size it down a bit so that it fits
        scale = 30.0f;
    }

    model_location.setScale(core::vector3df(scale, scale, scale));

    // Add the kart model (including wheels and speed weight objects)
    m_model_view->addModel(kart_model.getModel(), model_location,
                           kart_model.getBaseFrame(), kart_model.getBaseFrame());

    model_location.setScale(core::vector3df(1.0f, 1.0f, 1.0f));
    for (unsigned i = 0; i < 4; i++)
    {
        model_location.setTranslation(kart_model
            .getWheelGraphicsPosition(i).toIrrVector());
        m_model_view->addModel(kart_model.getWheelModel(i), model_location);
    }

    for (unsigned i = 0; i < kart_model.getSpeedWeightedObjectsCount();
        i++)
    {
        const SpeedWeightedObject& obj =
            kart_model.getSpeedWeightedObject(i);
        core::matrix4 swol = obj.m_location;
        if (!obj.m_bone_name.empty())
        {
            core::matrix4 inv =
                kart_model.getInverseBoneMatrix(obj.m_bone_name);
            swol = inv * obj.m_location;
        }
        m_model_view->addModel(obj.m_model, swol, -1, -1, 0.0f,
            obj.m_bone_name);
    }

    m_model_view->setRotateContinuously(35.0f);
    m_model_view->update(0);
    m_toggle_slider = getWidget<SpinnerWidget>("toggle-slider");
    m_color_slider = getWidget<SpinnerWidget>("color-slider");
}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
void KartColorSliderDialog::toggleSlider()
{
    if (m_toggle_slider->getValue() == 1)
    {
        m_color_slider->setActive(true);
        m_color_slider->setValue(int(m_player_profile->getDefaultKartColor() * 100.0f));
        m_model_view->getModelViewRenderInfo()->setHue(1.0f);
    }
    else
    {
        m_color_slider->setActive(false);
        m_model_view->getModelViewRenderInfo()->setHue(0.0f);
    }
}   // toggleSlider

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation
            KartColorSliderDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "toggle-slider")
    {
         toggleSlider();
    }
    else if (eventSource == "color-slider")
    {
        m_model_view->getModelViewRenderInfo()->setHue(float(
            m_color_slider->getValue()) / 100.0f);
    }
    else if (eventSource == "buttons")
    {
        const std::string& selection = m_buttons_widget->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            float color = 0.0f;
            if (m_toggle_slider->getValue() == 1)
                color = float(m_color_slider->getValue()) / 100.0f;
            m_player_profile->setDefaultKartColor(color);
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
