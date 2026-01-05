// Auto-generated from dialogs/online/player_rankings_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct PlayerRankingsDialogWidgets
{
    LabelWidget* title = nullptr;
    ListWidget* top_ten = nullptr;
    LabelWidget* cur_rank = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* refresh = nullptr;
    IconButtonWidget* ok = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        top_ten = container->getWidget<ListWidget>("top-ten");
        cur_rank = container->getWidget<LabelWidget>("cur-rank");
        options = container->getWidget<RibbonWidget>("options");
        refresh = container->getWidget<IconButtonWidget>("refresh");
        ok = container->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
