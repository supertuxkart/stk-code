// Auto-generated from dialogs/recommend_video_settings.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
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

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        performance = container->getWidget<CheckBoxWidget>("performance");
        balance = container->getWidget<CheckBoxWidget>("balance");
        graphics = container->getWidget<CheckBoxWidget>("graphics");
        sparing = container->getWidget<CheckBoxWidget>("sparing");
        blur_priority = container->getWidget<SpinnerWidget>("blur_priority");
        buttons = container->getWidget<RibbonWidget>("buttons");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        start_test = container->getWidget<IconButtonWidget>("start_test");
    }
};

}  // namespace GUIEngine
