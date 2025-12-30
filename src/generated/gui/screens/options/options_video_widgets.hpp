// Auto-generated from screens/options/options_video.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct OptionsVideoWidgets
{
    IconButtonWidget* back = nullptr;
    RibbonWidget* options_choice = nullptr;
    IconButtonWidget* tab_general = nullptr;
    IconButtonWidget* tab_display = nullptr;
    IconButtonWidget* tab_video = nullptr;
    IconButtonWidget* tab_audio = nullptr;
    IconButtonWidget* tab_ui = nullptr;
    IconButtonWidget* tab_players = nullptr;
    IconButtonWidget* tab_controls = nullptr;
    IconButtonWidget* tab_language = nullptr;
    SpinnerWidget* scale_rtts = nullptr;
    LabelWidget* scale_rtts_label = nullptr;
    SpinnerWidget* gfx_level = nullptr;
    SpinnerWidget* blur_level = nullptr;
    SpinnerWidget* vsync = nullptr;
    LabelWidget* vsync_label = nullptr;
    ButtonWidget* custom = nullptr;
    ButtonWidget* benchmarkCurrent = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        options_choice = screen->getWidget<RibbonWidget>("options_choice");
        tab_general = screen->getWidget<IconButtonWidget>("tab_general");
        tab_display = screen->getWidget<IconButtonWidget>("tab_display");
        tab_video = screen->getWidget<IconButtonWidget>("tab_video");
        tab_audio = screen->getWidget<IconButtonWidget>("tab_audio");
        tab_ui = screen->getWidget<IconButtonWidget>("tab_ui");
        tab_players = screen->getWidget<IconButtonWidget>("tab_players");
        tab_controls = screen->getWidget<IconButtonWidget>("tab_controls");
        tab_language = screen->getWidget<IconButtonWidget>("tab_language");
        scale_rtts = screen->getWidget<SpinnerWidget>("scale_rtts");
        scale_rtts_label = screen->getWidget<LabelWidget>("scale_rtts_label");
        gfx_level = screen->getWidget<SpinnerWidget>("gfx_level");
        blur_level = screen->getWidget<SpinnerWidget>("blur_level");
        vsync = screen->getWidget<SpinnerWidget>("vsync");
        vsync_label = screen->getWidget<LabelWidget>("vsync_label");
        custom = screen->getWidget<ButtonWidget>("custom");
        benchmarkCurrent = screen->getWidget<ButtonWidget>("benchmarkCurrent");
    }
};

}  // namespace GUIEngine
