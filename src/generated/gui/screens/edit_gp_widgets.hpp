// Auto-generated from screens/edit_gp.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct EditGpWidgets
{
    IconButtonWidget* back = nullptr;
    LabelWidget* title = nullptr;
    ListWidget* tracks = nullptr;
    RibbonWidget* menu = nullptr;
    IconButtonWidget* up = nullptr;
    IconButtonWidget* down = nullptr;
    IconButtonWidget* add = nullptr;
    IconButtonWidget* edit = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* save = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        back = container->getWidget<IconButtonWidget>("back");
        title = container->getWidget<LabelWidget>("title");
        tracks = container->getWidget<ListWidget>("tracks");
        menu = container->getWidget<RibbonWidget>("menu");
        up = container->getWidget<IconButtonWidget>("up");
        down = container->getWidget<IconButtonWidget>("down");
        add = container->getWidget<IconButtonWidget>("add");
        edit = container->getWidget<IconButtonWidget>("edit");
        remove = container->getWidget<IconButtonWidget>("remove");
        save = container->getWidget<IconButtonWidget>("save");
    }
};

}  // namespace GUIEngine
