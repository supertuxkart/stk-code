// Auto-generated from dialogs/online/vote_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/rating_bar_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct VoteDialogWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* info = nullptr;
    RatingBarWidget* rating = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        info = container->getWidget<LabelWidget>("info");
        rating = container->getWidget<RatingBarWidget>("rating");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
    }
};

}  // namespace GUIEngine
