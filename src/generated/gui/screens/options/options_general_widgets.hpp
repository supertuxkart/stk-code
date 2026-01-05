// Auto-generated from screens/options/options_general.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        options_choice = container->getWidget<RibbonWidget>("options_choice");
        tab_general = container->getWidget<IconButtonWidget>("tab_general");
        tab_display = container->getWidget<IconButtonWidget>("tab_display");
        tab_video = container->getWidget<IconButtonWidget>("tab_video");
        tab_audio = container->getWidget<IconButtonWidget>("tab_audio");
        tab_ui = container->getWidget<IconButtonWidget>("tab_ui");
        tab_players = container->getWidget<IconButtonWidget>("tab_players");
        tab_controls = container->getWidget<IconButtonWidget>("tab_controls");
        tab_language = container->getWidget<IconButtonWidget>("tab_language");
        show_login = container->getWidget<CheckBoxWidget>("show-login");
        enable_internet = container->getWidget<CheckBoxWidget>("enable-internet");
        enable_lobby_chat = container->getWidget<CheckBoxWidget>("enable-lobby-chat");
        label_lobby_chat = container->getWidget<LabelWidget>("label-lobby-chat");
        enable_race_chat = container->getWidget<CheckBoxWidget>("enable-race-chat");
        label_race_chat = container->getWidget<LabelWidget>("label-race-chat");
        enable_handicap = container->getWidget<CheckBoxWidget>("enable-handicap");
        assets_settings = container->getWidget<ButtonWidget>("assets_settings");
    }
};

}  // namespace GUIEngine
