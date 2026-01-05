// Auto-generated from dialogs/addons_loading.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/bubble_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"
#include "guiengine/widgets/rating_bar_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct AddonsLoadingWidgets
{
    IconButtonWidget* icon = nullptr;
    LabelWidget* name = nullptr;
    LabelWidget* size = nullptr;
    LabelWidget* revision = nullptr;
    RatingBarWidget* rating = nullptr;
    LabelWidget* flags = nullptr;
    BubbleWidget* description = nullptr;
    ProgressBarWidget* progress = nullptr;
    RibbonWidget* actions = nullptr;
    IconButtonWidget* back = nullptr;
    IconButtonWidget* install = nullptr;
    IconButtonWidget* uninstall = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        icon = container->getWidget<IconButtonWidget>("icon");
        name = container->getWidget<LabelWidget>("name");
        size = container->getWidget<LabelWidget>("size");
        revision = container->getWidget<LabelWidget>("revision");
        rating = container->getWidget<RatingBarWidget>("rating");
        flags = container->getWidget<LabelWidget>("flags");
        description = container->getWidget<BubbleWidget>("description");
        progress = container->getWidget<ProgressBarWidget>("progress");
        actions = container->getWidget<RibbonWidget>("actions");
        back = container->getWidget<IconButtonWidget>("back");
        install = container->getWidget<IconButtonWidget>("install");
        uninstall = container->getWidget<IconButtonWidget>("uninstall");
    }
};

}  // namespace GUIEngine
