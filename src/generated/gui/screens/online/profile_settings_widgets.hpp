// Auto-generated from screens/online/profile_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct ProfileSettingsWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    RibbonWidget* profile_tabs = nullptr;
    IconButtonWidget* tab_achievements = nullptr;
    IconButtonWidget* tab_friends = nullptr;
    IconButtonWidget* tab_settings = nullptr;
    ButtonWidget* change_password_button = nullptr;
    ButtonWidget* change_email_button = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        profile_tabs = container->getWidget<RibbonWidget>("profile_tabs");
        tab_achievements = container->getWidget<IconButtonWidget>("tab_achievements");
        tab_friends = container->getWidget<IconButtonWidget>("tab_friends");
        tab_settings = container->getWidget<IconButtonWidget>("tab_settings");
        change_password_button = container->getWidget<ButtonWidget>("change_password_button");
        change_email_button = container->getWidget<ButtonWidget>("change_email_button");
    }
};

}  // namespace GUIEngine
