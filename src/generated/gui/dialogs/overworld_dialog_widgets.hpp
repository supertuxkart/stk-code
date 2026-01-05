// Auto-generated from dialogs/overworld_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct OverworldDialogWidgets
{
    LabelWidget* title = nullptr;
    RibbonWidget* backbtnribbon = nullptr;
    IconButtonWidget* backbtn = nullptr;
    IconButtonWidget* touch_device = nullptr;
    RibbonWidget* choiceribbon = nullptr;
    IconButtonWidget* exit = nullptr;
    IconButtonWidget* selectkart = nullptr;
    IconButtonWidget* options = nullptr;
    IconButtonWidget* help = nullptr;

    void bind(AbstractTopLevelContainer* container)
    {
        title = container->getWidget<LabelWidget>("title");
        backbtnribbon = container->getWidget<RibbonWidget>("backbtnribbon");
        backbtn = container->getWidget<IconButtonWidget>("backbtn");
        touch_device = container->getWidget<IconButtonWidget>("touch_device");
        choiceribbon = container->getWidget<RibbonWidget>("choiceribbon");
        exit = container->getWidget<IconButtonWidget>("exit");
        selectkart = container->getWidget<IconButtonWidget>("selectkart");
        options = container->getWidget<IconButtonWidget>("options");
        help = container->getWidget<IconButtonWidget>("help");
    }
};

}  // namespace GUIEngine
