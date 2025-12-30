// Auto-generated from dialogs/addons_loading.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        icon = screen->getWidget<IconButtonWidget>("icon");
        name = screen->getWidget<LabelWidget>("name");
        size = screen->getWidget<LabelWidget>("size");
        revision = screen->getWidget<LabelWidget>("revision");
        rating = screen->getWidget<RatingBarWidget>("rating");
        flags = screen->getWidget<LabelWidget>("flags");
        description = screen->getWidget<BubbleWidget>("description");
        progress = screen->getWidget<ProgressBarWidget>("progress");
        actions = screen->getWidget<RibbonWidget>("actions");
        back = screen->getWidget<IconButtonWidget>("back");
        install = screen->getWidget<IconButtonWidget>("install");
        uninstall = screen->getWidget<IconButtonWidget>("uninstall");
    }
};

}  // namespace GUIEngine
