// Auto-generated from dialogs/overworld_dialog.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        title = screen->getWidget<LabelWidget>("title");
        backbtnribbon = screen->getWidget<RibbonWidget>("backbtnribbon");
        backbtn = screen->getWidget<IconButtonWidget>("backbtn");
        touch_device = screen->getWidget<IconButtonWidget>("touch_device");
        choiceribbon = screen->getWidget<RibbonWidget>("choiceribbon");
        exit = screen->getWidget<IconButtonWidget>("exit");
        selectkart = screen->getWidget<IconButtonWidget>("selectkart");
        options = screen->getWidget<IconButtonWidget>("options");
        help = screen->getWidget<IconButtonWidget>("help");
    }
};

}  // namespace GUIEngine
