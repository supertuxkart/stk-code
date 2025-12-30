// Auto-generated from screens/grand_prix_editor.stkgui
// Do not edit manually - regenerate with tools/generate_gui_headers.py
#pragma once

#include "guiengine/screen.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"

namespace GUIEngine {

struct GrandPrixEditorWidgets
{
    IconButtonWidget* back = nullptr;
    DynamicRibbonWidget* gplist = nullptr;
    RibbonWidget* gpgroups = nullptr;
    LabelWidget* gpname = nullptr;
    DynamicRibbonWidget* tracks = nullptr;
    RibbonWidget* menu = nullptr;
    IconButtonWidget* new_ = nullptr;
    IconButtonWidget* copy = nullptr;
    IconButtonWidget* edit = nullptr;
    IconButtonWidget* remove = nullptr;
    IconButtonWidget* rename = nullptr;

    void bind(Screen* screen)
    {
        back = screen->getWidget<IconButtonWidget>("back");
        gplist = screen->getWidget<DynamicRibbonWidget>("gplist");
        gpgroups = screen->getWidget<RibbonWidget>("gpgroups");
        gpname = screen->getWidget<LabelWidget>("gpname");
        tracks = screen->getWidget<DynamicRibbonWidget>("tracks");
        menu = screen->getWidget<RibbonWidget>("menu");
        new_ = screen->getWidget<IconButtonWidget>("new");
        copy = screen->getWidget<IconButtonWidget>("copy");
        edit = screen->getWidget<IconButtonWidget>("edit");
        remove = screen->getWidget<IconButtonWidget>("remove");
        rename = screen->getWidget<IconButtonWidget>("rename");
    }
};

}  // namespace GUIEngine
