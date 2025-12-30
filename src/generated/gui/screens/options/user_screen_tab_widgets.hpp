// Auto-generated from screens/options/user_screen_tab.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct UserScreenTabWidgets
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
    DynamicRibbonWidget* players = nullptr;
    CheckBoxWidget* online = nullptr;
    CheckBoxWidget* remember_user = nullptr;
    LabelWidget* label_remember = nullptr;
    LabelWidget* label_username = nullptr;
    TextBoxWidget* username = nullptr;
    LabelWidget* label_password = nullptr;
    TextBoxWidget* password = nullptr;
    ButtonWidget* password_reset = nullptr;
    LabelWidget* message = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* new_user = nullptr;
    IconButtonWidget* delete_ = nullptr;
    IconButtonWidget* rename = nullptr;
    IconButtonWidget* default_kart_color = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* ok = nullptr;

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
        players = container->getWidget<DynamicRibbonWidget>("players");
        online = container->getWidget<CheckBoxWidget>("online");
        remember_user = container->getWidget<CheckBoxWidget>("remember-user");
        label_remember = container->getWidget<LabelWidget>("label_remember");
        label_username = container->getWidget<LabelWidget>("label_username");
        username = container->getWidget<TextBoxWidget>("username");
        label_password = container->getWidget<LabelWidget>("label_password");
        password = container->getWidget<TextBoxWidget>("password");
        password_reset = container->getWidget<ButtonWidget>("password_reset");
        message = container->getWidget<LabelWidget>("message");
        options = container->getWidget<RibbonWidget>("options");
        new_user = container->getWidget<IconButtonWidget>("new_user");
        delete_ = container->getWidget<IconButtonWidget>("delete");
        rename = container->getWidget<IconButtonWidget>("rename");
        default_kart_color = container->getWidget<IconButtonWidget>("default_kart_color");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        ok = container->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
