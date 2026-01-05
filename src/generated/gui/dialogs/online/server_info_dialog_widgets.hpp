// Auto-generated from dialogs/online/server_info_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ServerInfoDialogWidgets
{
    LabelWidget* title = nullptr;
    LabelWidget* server_info_1 = nullptr;
    LabelWidget* server_info_2 = nullptr;
    LabelWidget* server_info_3 = nullptr;
    LabelWidget* server_info_4 = nullptr;
    ListWidget* player_list = nullptr;
    LabelWidget* label_password = nullptr;
    TextBoxWidget* password = nullptr;
    RibbonWidget* options = nullptr;
    IconButtonWidget* cancel = nullptr;
    IconButtonWidget* bookmark = nullptr;
    IconButtonWidget* join = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        server_info_1 = container->getWidget<LabelWidget>("server-info-1");
        server_info_2 = container->getWidget<LabelWidget>("server-info-2");
        server_info_3 = container->getWidget<LabelWidget>("server-info-3");
        server_info_4 = container->getWidget<LabelWidget>("server-info-4");
        player_list = container->getWidget<ListWidget>("player-list");
        label_password = container->getWidget<LabelWidget>("label_password");
        password = container->getWidget<TextBoxWidget>("password");
        options = container->getWidget<RibbonWidget>("options");
        cancel = container->getWidget<IconButtonWidget>("cancel");
        bookmark = container->getWidget<IconButtonWidget>("bookmark");
        join = container->getWidget<IconButtonWidget>("join");
    }
};

}  // namespace GUIEngine
