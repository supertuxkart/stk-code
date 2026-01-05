// Auto-generated from screens/online/server_selection.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

namespace GUIEngine {

struct ServerSelectionWidgets
{
    IconButtonWidget* back = nullptr;
    IconButtonWidget* bookmark = nullptr;
    IconButtonWidget* reload = nullptr;
    LabelWidget* title_header = nullptr;
    ListWidget* server_list = nullptr;
    TextBoxWidget* searcher = nullptr;
    CheckBoxWidget* private_server = nullptr;
    CheckBoxWidget* ipv6 = nullptr;
    LabelWidget* ipv6_text = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        bookmark = container->getWidget<IconButtonWidget>("bookmark");
        reload = container->getWidget<IconButtonWidget>("reload");
        title_header = container->getWidget<LabelWidget>("title_header");
        server_list = container->getWidget<ListWidget>("server_list");
        searcher = container->getWidget<TextBoxWidget>("searcher");
        private_server = container->getWidget<CheckBoxWidget>("private_server");
        ipv6 = container->getWidget<CheckBoxWidget>("ipv6");
        ipv6_text = container->getWidget<LabelWidget>("ipv6_text");
    }
};

}  // namespace GUIEngine
