// Auto-generated from screens/user_screen.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct UserScreenWidgets
{
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
    IconButtonWidget* back = nullptr;

    void bind(Screen* screen)
    {
        players = screen->getWidget<DynamicRibbonWidget>("players");
        online = screen->getWidget<CheckBoxWidget>("online");
        remember_user = screen->getWidget<CheckBoxWidget>("remember-user");
        label_remember = screen->getWidget<LabelWidget>("label_remember");
        label_username = screen->getWidget<LabelWidget>("label_username");
        username = screen->getWidget<TextBoxWidget>("username");
        label_password = screen->getWidget<LabelWidget>("label_password");
        password = screen->getWidget<TextBoxWidget>("password");
        password_reset = screen->getWidget<ButtonWidget>("password_reset");
        message = screen->getWidget<LabelWidget>("message");
        options = screen->getWidget<RibbonWidget>("options");
        new_user = screen->getWidget<IconButtonWidget>("new_user");
        delete_ = screen->getWidget<IconButtonWidget>("delete");
        rename = screen->getWidget<IconButtonWidget>("rename");
        default_kart_color = screen->getWidget<IconButtonWidget>("default_kart_color");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        ok = screen->getWidget<IconButtonWidget>("ok");
        back = screen->getWidget<IconButtonWidget>("back");
    }
};

}  // namespace GUIEngine
