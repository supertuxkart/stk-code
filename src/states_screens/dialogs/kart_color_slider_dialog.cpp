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

#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/render_info.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
KartColorSliderDialog::KartColorSliderDialog(PlayerProfile* pp)
                     : ModalDialog(0.75f, 0.75f, MODAL_DIALOG_LOCATION_CENTER)
{
    loadFromFile("kart_color_slider.stkgui");
    m_player_profile = pp;

    SpinnerWidget* color_slider = getWidget<SpinnerWidget>("color-slider");
    color_slider->setValue(int(pp->getDefaultKartColor() * 100.0f));
    m_model_view->getModelViewRenderInfo()->setHue(float(color_slider->getValue()) / 100.0f);
    color_slider->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // KartColorSliderDialog

// ----------------------------------------------------------------------------
KartColorSliderDialog::~KartColorSliderDialog()
{
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
    const bool has_win_anime =
        (((kart_model.getFrame(KartModel::AF_WIN_LOOP_START) > -1 ||
        kart_model.getFrame(KartModel::AF_WIN_START) > -1) &&
        kart_model.getFrame(KartModel::AF_WIN_END) > -1) ||
        (kart_model.getFrame(KartModel::AF_SELECTION_START) > -1 &&
        kart_model.getFrame(KartModel::AF_SELECTION_END) > -1));
    m_model_view->addModel(kart_model.getModel(), model_location,
        has_win_anime ?
        kart_model.getFrame(KartModel::AF_SELECTION_START) > -1 ?
        kart_model.getFrame(KartModel::AF_SELECTION_START) :
        kart_model.getFrame(KartModel::AF_WIN_LOOP_START) > -1 ?
        kart_model.getFrame(KartModel::AF_WIN_LOOP_START) :
        kart_model.getFrame(KartModel::AF_WIN_START) :
        kart_model.getBaseFrame(),
        has_win_anime ?
        kart_model.getFrame(KartModel::AF_SELECTION_END) > -1 ?
        kart_model.getFrame(KartModel::AF_SELECTION_END) :
        kart_model.getFrame(KartModel::AF_WIN_END) :
        kart_model.getBaseFrame(),
        kart_model.getAnimationSpeed());

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
}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation
            KartColorSliderDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "color-slider")
    {
        m_model_view->getModelViewRenderInfo()->setHue(float(
            getWidget<SpinnerWidget>("color-slider")->getValue()) / 100.0f);
    }
    else if (eventSource == "close")
    {
        float color = float(getWidget<SpinnerWidget>("color-slider")
            ->getValue());
        m_player_profile->setDefaultKartColor(color / 100.0f);
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
