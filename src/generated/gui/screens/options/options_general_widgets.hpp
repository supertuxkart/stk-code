// Auto-generated from screens/options/options_general.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct OptionsGeneralWidgets
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
    CheckBoxWidget* show_login = nullptr;
    CheckBoxWidget* enable_internet = nullptr;
    CheckBoxWidget* enable_lobby_chat = nullptr;
    LabelWidget* label_lobby_chat = nullptr;
    CheckBoxWidget* enable_race_chat = nullptr;
    LabelWidget* label_race_chat = nullptr;
    CheckBoxWidget* enable_handicap = nullptr;
    ButtonWidget* assets_settings = nullptr;

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
        show_login = screen->getWidget<CheckBoxWidget>("show-login");
        enable_internet = screen->getWidget<CheckBoxWidget>("enable-internet");
        enable_lobby_chat = screen->getWidget<CheckBoxWidget>("enable-lobby-chat");
        label_lobby_chat = screen->getWidget<LabelWidget>("label-lobby-chat");
        enable_race_chat = screen->getWidget<CheckBoxWidget>("enable-race-chat");
        label_race_chat = screen->getWidget<LabelWidget>("label-race-chat");
        enable_handicap = screen->getWidget<CheckBoxWidget>("enable-handicap");
        assets_settings = screen->getWidget<ButtonWidget>("assets_settings");
    }
};

}  // namespace GUIEngine
