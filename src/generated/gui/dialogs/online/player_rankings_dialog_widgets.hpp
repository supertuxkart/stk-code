// Auto-generated from dialogs/online/player_rankings_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        top_ten = screen->getWidget<ListWidget>("top-ten");
        cur_rank = screen->getWidget<LabelWidget>("cur-rank");
        options = screen->getWidget<RibbonWidget>("options");
        refresh = screen->getWidget<IconButtonWidget>("refresh");
        ok = screen->getWidget<IconButtonWidget>("ok");
    }
};

}  // namespace GUIEngine
