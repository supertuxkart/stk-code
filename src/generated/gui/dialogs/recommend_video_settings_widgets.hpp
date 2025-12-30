// Auto-generated from dialogs/recommend_video_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

namespace GUIEngine {

struct RecommendVideoSettingsWidgets
{
    LabelWidget* title = nullptr;
    CheckBoxWidget* performance = nullptr;
    CheckBoxWidget* balance = nullptr;
    CheckBoxWidget* graphics = nullptr;
    CheckBoxWidget* sparing = nullptr;
    SpinnerWidget* blur_priority = nullptr;
    RibbonWidget* buttons = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* start_test = nullptr;

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        performance = screen->getWidget<CheckBoxWidget>("performance");
        balance = screen->getWidget<CheckBoxWidget>("balance");
        graphics = screen->getWidget<CheckBoxWidget>("graphics");
        sparing = screen->getWidget<CheckBoxWidget>("sparing");
        blur_priority = screen->getWidget<SpinnerWidget>("blur_priority");
        buttons = screen->getWidget<RibbonWidget>("buttons");
        cancel = screen->getWidget<IconButtonWidget>("cancel");
        start_test = screen->getWidget<IconButtonWidget>("start_test");
    }
};

}  // namespace GUIEngine
