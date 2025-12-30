// Auto-generated from screens/options/options_audio.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct OptionsAudioWidgets
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
    CheckBoxWidget* music_enabled = nullptr;
    SpinnerWidget* music_volume = nullptr;
    CheckBoxWidget* sfx_enabled = nullptr;
    SpinnerWidget* sfx_volume = nullptr;

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
        music_enabled = screen->getWidget<CheckBoxWidget>("music_enabled");
        music_volume = screen->getWidget<SpinnerWidget>("music_volume");
        sfx_enabled = screen->getWidget<CheckBoxWidget>("sfx_enabled");
        sfx_volume = screen->getWidget<SpinnerWidget>("sfx_volume");
    }
};

}  // namespace GUIEngine
