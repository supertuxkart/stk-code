// Auto-generated from screens/edit_gp.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
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

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        title = screen->getWidget<LabelWidget>("title");
        tracks = screen->getWidget<ListWidget>("tracks");
        menu = screen->getWidget<RibbonWidget>("menu");
        up = screen->getWidget<IconButtonWidget>("up");
        down = screen->getWidget<IconButtonWidget>("down");
        add = screen->getWidget<IconButtonWidget>("add");
        edit = screen->getWidget<IconButtonWidget>("edit");
        remove = screen->getWidget<IconButtonWidget>("remove");
        save = screen->getWidget<IconButtonWidget>("save");
    }
};

}  // namespace GUIEngine
